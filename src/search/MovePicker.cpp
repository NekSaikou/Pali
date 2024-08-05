#include "MovePicker.h"

#include "core/Attacks.h"
#include "core/Bitboard.h"
#include "core/Color.h"
#include "core/Move.h"
#include "core/Piece.h"
#include "core/Position.h"
#include "core/Square.h"
#include "search/History.h"
#include "search/SEE.h"

#include <cassert>
#include <cstdint>

using namespace pali;

constexpr int KILLER_SCORE = 2'000'000'000;

Move pickMove(MoveList &Ml);
bool isPsuedoLegal(const Position &Pos, Move Mv);

template Move MovePicker::nextMove<false>();
template Move MovePicker::nextMove<true>();

template <bool QSEARCH> Move MovePicker::nextMove() {
repick:
  switch (Stage) {
  case Best:
    goNext();

    if (isPsuedoLegal(Pos, BestMove))
      return BestMove;

  case GenŅoisy:
    Pos.genNoisy(NoisyMl);
    scoreNoisy();

    goNext();

  case GoodNoisy:
    if (NoisyMl.size() > 0) {
      Move Mv = pickMove(NoisyMl);
      if (Mv == BestMove)
        goto repick;

      if (!see(Pos, Mv, 0)) {
        BadNoisyMl.push_back(Mv);
        goto repick;
      }

      return Mv;
    }

    goNext();

  case GenQuiet:
    if constexpr (!QSEARCH) {
      Pos.genQuiet(QuietMl);
      scoreQuiet();
      goNext();
    }

    else {
      Stage = Finished;
      return NULL_MOVE;
    }

  case Quiet:
    if (QuietMl.size() > 0) {
      Move Mv = pickMove(QuietMl);
      if (Mv == BestMove)
        goto repick;

      return Mv;
    }

    goNext();

  case BadNoisy:
    if (BadNoisyMl.size() > 0) {
      Move Mv = pickMove(BadNoisyMl);
      if (Mv == BestMove)
        goto repick;

      return Mv;
    }

    goNext();

  case Finished:
    return NULL_MOVE;
  }
}

void MovePicker::scoreQuiet() {
  for (Move &Mv : QuietMl) {
    // History Heuristic:
    // Give moves that cause a lot of cutoff more score
    Mv.Score += HTable.MainHist[Pos.stm()][Mv.From][Mv.To];

    if (Mv == HTable.Killer[Ply])
      Mv.Score += KILLER_SCORE;
  }
}

void MovePicker::scoreNoisy() {
  for (Move &Mv : NoisyMl) {
    // MVV-LVA (Most Valuable Victim, Least Valuable Attacker)
    // Give higher score to captures that target more valuable enemy
    // pieces followed by captures by low value pieces
    Piece Target = Mv.isEP() ? Piece::Pawn : Pos.pieceAt(Mv.To);
    Mv.Score += Target.mvvVal() + Mv.Pc.lvaVal();
  }
}

/// Sort moves and then pick out the one with highest score
Move pickMove(MoveList &Ml) {
  MScore BestScore = INT32_MIN;
  int BestIdx = 0;

  for (int i = 0; i < Ml.size(); ++i) {
    if (Ml[i].Score > BestScore) {
      BestScore = Ml[i].Score;
      BestIdx = i;
    }
  }

  Ml.swap(BestIdx, Ml.size() - 1);

  Move Mv = Ml.takeLast();

  return Mv;
}

/// Check if move from TT is at least pseudolegal
bool isPsuedoLegal(const Position &Pos, Move Mv) {
  [[maybe_unused]] const auto [From, To, Flag, Pc, Score] = Mv;

  Color Stm = Pos.stm();

  Bitboard Occ = Pos.allBB();
  Bitboard Us = Pos.getBB(Stm);
  Bitboard Them = Pos.getBB(Stm.inverse());

  if (Mv.isNullMove() || Pc == Piece::None)
    return false;

  if (Us.getBit(To))
    return false;

  if (!Us.getBit(From))
    return false;

  if (Flag == MFlag::Normal || Flag == MFlag::Capture) {
    if (Pc == Piece::Knight)
      return getKnightAttack(From).getBit(To);

    if (Pc == Piece::Bishop)
      return getBishopAttack(From, Occ).getBit(To);

    if (Pc == Piece::Rook)
      return getRookAttack(From, Occ).getBit(To);

    if (Pc == Piece::Queen)
      return getQueenAttack(From, Occ).getBit(To);

    if (Pc == Piece::King)
      return getKingAttack(From).getBit(To);
  }

  if (Pc == Piece::Pawn) {
    Bitboard PromoRank = Stm.isWhite() ? Bitboard::RANK_7 : Bitboard::RANK_2;
    Bitboard ThirdRank = Stm.isWhite() ? Bitboard::RANK_3 : Bitboard::RANK_6;

    if (Mv.isPromo())
      if (!PromoRank.getBit(From))
        return false;

    if (Mv.isEP())
      return Pos.epSq() != Square::None &&
             getPawnAttack(From, Stm).getBit(Pos.epSq());

    if (Mv.isCapture())
      return getPawnAttack(From, Stm).getBit(To);

    Bitboard ValidPush =
        (Stm.isWhite() ? From.toBB() >> 8 : From.toBB() << 8) & ~Occ;

    if (Mv.isDP())
      return Stm.isWhite() ? (ValidPush & ThirdRank) >> 8 & ~Occ
                           : (ValidPush & ThirdRank) << 8 & ~Occ;

    return ValidPush;
  }

  assert(Pc == Piece::King);

  uint8_t Castle = 0;
  Square RookFrom = 11;

  if (To == Square::G1) {
    Castle = 1;
    RookFrom = Square::H1;
  }

  else if (To == Square::C1) {
    Castle = 2;
    RookFrom = Square::A1;
  }

  else if (To == Square::G8) {
    Castle = 4;
    RookFrom = Square::H8;
  }

  else if (To == Square::C8) {
    Castle = 8;
    RookFrom = Square::A8;
  }

  if (Mv.isCastle()) {
    Bitboard Path = getBetweenSq(From, RookFrom);
    return (Pos.rights() & Castle && !(Occ & Path) && !Pos.isInCheck() &&
            [&]() {
              Bitboard KingPath = getBetweenSq(From, To);
              while (KingPath)
                if (Pos.attacksAt(KingPath.takeLsb()) & Them)
                  return false;

              return true;
            }());
  };

  return false;
}
