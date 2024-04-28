#include "MovePicker.h"

#include "../core/Attacks.h"
#include "../core/Bitboard.h"
#include "../core/Move.h"
#include "../core/Piece.h"
#include "../core/Position.h"
#include "History.h"

#include <cstdint>
#include <cstdlib>

using namespace pali;

Move pickMove(MoveList &Ml);
bool isPsuedoLegal(const Position &Pos, Move Mv);

template Move MovePicker::nextMove<false>();
template Move MovePicker::nextMove<true>();

template <bool NO_QUIET> Move MovePicker::nextMove() {
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

  case Noisy:
    if (NoisyMl.size() > 0) {
      Move Mv = pickMove(NoisyMl);
      if (Mv == BestMove)
        goto repick;

      return Mv;
    }

    goNext();

  case GenQuiet:
    if constexpr (!NO_QUIET) {
      Pos.genQuiet(QuietMl);
      scoreQuiet();
    }

    goNext();

  case Quiet:
    if (QuietMl.size() > 0) {
      Move Mv = pickMove(QuietMl);
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
    /// MainHist history heuristic:
    /// Give moves that cause a lot of cutoff more score
    Mv.Score += HTable.MainHist[Pos.stm()][Mv.From][Mv.To];
  }
}

void MovePicker::scoreNoisy() {
  for (Move &Mv : NoisyMl) {
    /// MVV-LVA (Most Valuable Victim, Least Valuable Attacker)
    /// Give higher score to captures that target more valuable enemy
    /// pieces followed by captures by low value pieces
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
  const Bitboard ValidSquares = ~Pos.getBB(Pos.stm()); // Not own piece

  // Move is obviosly illegal
  if (Mv.isNullMove() || Pc == Piece::None)
    return false;

  // Simple cases:
  // We only need to check if the piece actually land on a valid square
  if (Flag == MFlag::Normal || Flag == MFlag::Capture)
    switch (Pc) {
    case Piece::Knight:
      return ValidSquares & getKnightAttack(From).getBit(To);

    case Piece::Bishop:
      return ValidSquares & getBishopAttack(From, Pos.allBB()).getBit(To);

    case Piece::Rook:
      return ValidSquares & getRookAttack(From, Pos.allBB()).getBit(To);

    case Piece::Queen:
      return ValidSquares & getQueenAttack(From, Pos.allBB()).getBit(To);

    case Piece::King:
      return ValidSquares & getKingAttack(From).getBit(To);
    }

  // Castling:
  // The moving piece must be a king and must be moving 2 squares
  if (Flag == MFlag::Castle)
    return Pc == Piece::King && abs(From - To) == 2;

  // clang-format off
  if (Pc == Piece::Pawn) {
    Bitboard PromoRank = Pos.stm().isWhite() ? Bitboard::RANK_7
                                             : Bitboard::RANK_2;

    Bitboard ThirdRank = Pos.stm().isWhite() ? Bitboard::RANK_3
                                             : Bitboard::RANK_6;

    Bitboard ValidPushes =
        ValidSquares & (Pos.stm().isWhite() ? Pos.getBB(Piece::Pawn) >> 8
                                            : Pos.getBB(Piece::Pawn) << 8);

    Bitboard ValidDPs =
        ValidSquares & (Pos.stm().isWhite() ? (ValidPushes & ThirdRank) >> 8
                                            : (ValidPushes & ThirdRank) << 8);

    // Invalid promotions
    if (Mv.isPromo() && !PromoRank.getBit(From))
      return false;
      
    // Double pushes
    if (Flag == MFlag::DoublePush)
      return ValidDPs.getBit(To);

    // All capture moves
    else if (Mv.isCapture())
      return ValidSquares & getPawnAttack(From, Pos.stm()).getBit(To);

    // Normal pushes and non-captures promotion
    else
      return ValidPushes.getBit(To);
  }
  // clang-format on

  return true;
}
