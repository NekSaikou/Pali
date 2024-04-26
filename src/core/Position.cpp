#include "Position.h"

#include "../nnue/Network.h"
#include "Attacks.h"
#include "Bitboard.h"
#include "Color.h"
#include "Move.h"
#include "Piece.h"
#include "Square.h"
#include "Util.h"
#include "Zobrist.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <utility>

using namespace fodder;

Position::Position(const std::string &Fen) {
  Acc[0] = NNUE.InputBias;
  Acc[1] = NNUE.InputBias;

  auto Tokens = tokenize(Fen);

  // clang-format off
  // Parse position of pieces
  Square Sq = 0;
  for (char c : Tokens[0]) {
    if (isalpha(c)) {
      Piece Pc;
      Color Col;
      
      switch (tolower(c)) {
      case 'p': Pc = Piece::Pawn;   break;
      case 'n': Pc = Piece::Knight; break;
      case 'b': Pc = Piece::Bishop; break;
      case 'r': Pc = Piece::Rook;   break;
      case 'q': Pc = Piece::Queen;  break;
      case 'k': Pc = Piece::King;   break;
      }

      if (isupper(c))
        Col = Color::White;
      else
        Col = Color::Black;

      addPiece(Pc, Col, Sq);
      
      Sq += 1;
    }

    else if (isdigit(c))
      Sq += c - '0';
  }
  // clang-format on

  // Parse side to move
  Stm = Tokens[1][0] == 'w' ? Color::White : Color::Black;

  // Identical position should have a different hash
  // if side to move is different
  if (Stm.isWhite())
    updateHash(getStmKey());

  // clang-format off
  // Parse castling rights
  for (char c : Tokens[2]) {
    switch (c) {
    case 'K': Rights |= 0b0001; break;
    case 'Q': Rights |= 0b0010; break;
    case 'k': Rights |= 0b0100; break;
    case 'q': Rights |= 0b1000; break;
    }
  }
  // clang-format on
  updateHash(getCastleKey(Rights));

  // Parse enpassant square
  if (Tokens[3][0] != '-') {
    EpSq = 8 * (7 - (Tokens[3][1] - '1')) + (Tokens[3][0] - 'a');
    updateHash(getEPKey(EpSq));
  }

  // Parse halfmove clock
  Hmc = std::stoi(Tokens[4]);
}

// clang-format off
Bitboard Position::attacksAt(Square sq) const {
  Bitboard occ = allBB();
  return getPawnAttack(sq, Color::White) & getBB(Piece::Pawn, Color::Black) |
         getPawnAttack(sq, Color::Black) & getBB(Piece::Pawn, Color::White) |
         getKnightAttack(sq)      & getBB(Piece::Knight) |
         getKingAttack(sq)        & getBB(Piece::King) |
         getBishopAttack(sq, occ) & getBB(Piece::Bishop) |
         getRookAttack(sq, occ)   & getBB(Piece::Rook) |
         getQueenAttack(sq, occ)  & getBB(Piece::Queen);
}

Bitboard Position::attacksAt(Square sq, Bitboard occ) const {
  return getPawnAttack(sq, Color::White) & getBB(Piece::Pawn, Color::Black) |
         getPawnAttack(sq, Color::Black) & getBB(Piece::Pawn, Color::White) |
         getKnightAttack(sq)      & getBB(Piece::Knight) |
         getKingAttack(sq)        & getBB(Piece::King) |
         getBishopAttack(sq, occ) & getBB(Piece::Bishop) |
         getRookAttack(sq, occ)   & getBB(Piece::Rook) |
         getQueenAttack(sq, occ)  & getBB(Piece::Queen);
}
// clang-format on

void Position::genNoisy(MoveList &Ml) const {
  const Bitboard Occ = allBB();

  auto addCaptures = [this, &Ml](Square From, Bitboard Attacks, Piece Pc) {
    // Only include overlaps with enemy pieces
    Attacks &= getBB(Stm.inverse());
    while (Attacks)
      Ml.push_back({From, Attacks.takeLsb(), MFlag::Capture, Pc});
  };

  { // Generate king moves
    // Only one king can exist for each side.
    Square From = getBB(Piece::King, Stm).lsb();
    Bitboard Attacks = getKingAttack(From);

    addCaptures(From, Attacks, Piece::King);
  }

  { // Generate knight moves
    Bitboard FromsBB = getBB(Piece::Knight, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getKnightAttack(From);

      addCaptures(From, Attacks, Piece::Knight);
    }
  }

  { // Generate bishop moves
    Bitboard FromsBB = getBB(Piece::Bishop, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getBishopAttack(From, Occ);

      addCaptures(From, Attacks, Piece::Bishop);
    }
  }

  { // Generate rook moves
    Bitboard FromsBB = getBB(Piece::Rook, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getRookAttack(From, Occ);

      addCaptures(From, Attacks, Piece::Rook);
    }
  }

  { // Generate queen moves
    Bitboard FromsBB = getBB(Piece::Queen, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getQueenAttack(From, Occ);

      addCaptures(From, Attacks, Piece::Queen);
    }
  }

  { // Generate pawn moves
    // The rank a pawn is on before promotion
    Bitboard PromoRank = Stm.isWhite() ? Bitboard::RANK_7 : Bitboard::RANK_2;
    Bitboard FromsBB = getBB(Piece::Pawn, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getPawnAttack(From, Stm);

      // Add en passant capture if possible
      if (EpSq.exists() && Attacks.getBit(EpSq)) {
        Ml.push_back({From, EpSq, MFlag::EnPassant, Piece::Pawn});
        Attacks.pop(EpSq); // Avoid adding duplicate move
      }

      // Check if it's a promotion
      else if (PromoRank.getBit(From)) {
        Attacks &= getBB(Stm.inverse());
        while (Attacks) {
          Square To = Attacks.takeLsb();

          // Add each promotion type
          // Capture promotions flags: 12 => 15
          for (int i = 12; i <= 15; ++i)
            Ml.push_back({From, To, static_cast<MFlag>(i), Piece::Pawn});
        }

        continue; // No more move to generate
      }

      // Normal pawn captures
      addCaptures(From, Attacks, Piece::Pawn);
    }
  }
}

void Position::genQuiet(MoveList &Ml) const {
  const Bitboard Occ = allBB();

  auto addNormal = [this, &Ml](Square From, Bitboard Attacks, Piece Pc) {
    // Don't include any overlap
    Attacks &= ~allBB();
    while (Attacks)
      Ml.push_back({From, Attacks.takeLsb(), MFlag::Normal, Pc});
  };

  { // Generate king moves
    // Only one king can exist for each side.
    Square From = getBB(Piece::King, Stm).lsb();
    Bitboard Attacks = getKingAttack(From);

    addNormal(From, Attacks, Piece::King);
  }

  { // Generate knight moves
    Bitboard FromsBB = getBB(Piece::Knight, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getKnightAttack(From);

      addNormal(From, Attacks, Piece::Knight);
    }
  }

  { // Generate bishop moves
    Bitboard FromsBB = getBB(Piece::Bishop, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getBishopAttack(From, Occ);

      addNormal(From, Attacks, Piece::Bishop);
    }
  }

  { // Generate rook moves
    Bitboard FromsBB = getBB(Piece::Rook, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getRookAttack(From, Occ);

      addNormal(From, Attacks, Piece::Rook);
    }
  }

  { // Generate queen moves
    Bitboard FromsBB = getBB(Piece::Queen, Stm);
    while (FromsBB) {
      Square From = FromsBB.takeLsb();
      Bitboard Attacks = getQueenAttack(From, Occ);

      addNormal(From, Attacks, Piece::Queen);
    }
  }

  { // Generate pawn moves
    Bitboard PromoRank = Stm.isWhite() ? Bitboard::RANK_7 : Bitboard::RANK_2;
    Bitboard ThirdRank = Stm.isWhite() ? Bitboard::RANK_3 : Bitboard::RANK_6;
    Bitboard PawnsBB = getBB(Piece::Pawn, Stm);

    // Pawn push destinations excluding occupied squares
    Bitboard PushesBB =
        Stm.isWhite() ? PawnsBB >> 8 & ~Occ : PawnsBB << 8 & ~Occ;

    // clang-format off
    // Double pushes destinations excluding occupied square
    // Step 1) AND PushesBB with ThirdRank to get all unopposed pawns pushed
    //         from the second rank
    // Step 2) Shift everything forward one square
    // Step 3) NAND the new bitboard with all pieces to remove blocked pawns
    // clang-format on
    Bitboard DPsBB = Stm.isWhite() ? (PushesBB & ThirdRank) >> 8 & ~Occ
                                   : (PushesBB & ThirdRank) << 8 & ~Occ;

    // Generate single pushes
    while (PushesBB) {
      Square To = PushesBB.takeLsb();
      Square From = Stm.isWhite() ? To + Square(8) : To - Square(8);

      // The move might be promotion
      if (PromoRank.getBit(From)) {
        // Add each promotion type
        // Normal promotions flags: 8 => 11
        for (int i = 8; i <= 11; ++i)
          Ml.push_back({From, To, static_cast<MFlag>(i), Piece::Pawn});

        continue; // Avoid adding duplicate move
      }

      Ml.push_back({From, To, MFlag::Normal, Piece::Pawn});
    }

    // Generate double pushes
    while (DPsBB) {
      Square To = DPsBB.takeLsb();
      Square From = Stm.isWhite() ? To + Square(16) : To - Square(16);

      Ml.push_back({From, To, MFlag::DoublePush, Piece::Pawn});
    }
  }

  { // Generate castling moves
    enum CRights { C_WhiteK = 1, C_WhiteQ = 2, C_BlackK = 4, C_BlackQ = 8 };

    auto addCastle = [this, &Ml, Occ](Square KingFrom, Square KingTo,
                                      Square RookFrom, CRights Castle) {
      Bitboard Path = getBetweenSq(KingFrom, RookFrom);
      if (Rights & Castle && // Has rights
          !(Occ & Path) &&   // Nothing in the way
          !isInCheck() &&    // Not in check
          [this, KingFrom, KingTo]() {
            // Nothing attacking the king's path
            Bitboard KingPath = getBetweenSq(KingFrom, KingTo);
            while (KingPath)
              if (attacksAt(KingPath.takeLsb()) & getBB(Stm.inverse()))
                return false;

            return true;
          }())
        Ml.push_back({KingFrom, KingTo, MFlag::Castle, Piece::King});
    };

    if (Stm.isWhite()) {
      addCastle(Square::E1, Square::G1, Square::H1, C_WhiteK);
      addCastle(Square::E1, Square::C1, Square::A1, C_WhiteQ);
    } else {
      addCastle(Square::E8, Square::G8, Square::H8, C_BlackK);
      addCastle(Square::E8, Square::C8, Square::A8, C_BlackQ);
    }
  }
}

bool Position::makeMove(Move Mv) {
  [[maybe_unused]] const auto [From, To, Flag, Pc, Score] = Mv;

  // We will clear pawn captured by en passant
  // and update EpSQ in case of double push on this squar
  Square EpCaptureSq = To - Square(Stm ? 8 : -8);

  ++Hmc;

  // Pawn moved, half move clock resets
  if (Pc == Piece::Pawn)
    Hmc = 0;

  // En passant capture
  if (Mv.isEP())
    clearPiece(Piece::Pawn, Stm.inverse(), EpCaptureSq);

  // Clear out captured piece
  else if (Mv.isCapture()) {
    Piece TargetPc = [this, Mv]() {
      // King can't be captured
      for (int Pc = Piece::Pawn; Pc <= Piece::Queen; ++Pc)
        if (Pieces[Pc].getBit(Mv.To))
          return static_cast<Piece::Type>(Pc);

      std::unreachable();
    }();

    clearPiece(TargetPc, Stm.inverse(), To);
    Hmc = 0;
  }

  // Move the rook when castling
  else if (Mv.isCastle()) {
    switch (To) {
    case Square::G1:
      movePiece(Piece::Rook, Stm, Square::H1, Square::F1);
      break;
    case Square::C1:
      movePiece(Piece::Rook, Stm, Square::A1, Square::D1);
      break;
    case Square::G8:
      movePiece(Piece::Rook, Stm, Square::H8, Square::F8);
      break;
    case Square::C8:
      movePiece(Piece::Rook, Stm, Square::A8, Square::D8);
      break;
    }
  }

  // Move the piece to a the destination and remove it from the old square
  Piece AddedPiece = Mv.isPromo() ? Mv.promoType() : Pc;
  clearPiece(Pc, Stm, From);
  addPiece(AddedPiece, Stm, To);

  // clang-format off
  constexpr uint8_t CASTLING_UPDATE[64] {
    7,  15, 15, 15, 3,  15, 15, 11, 
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
  };
  // clang-format on

  updateHash(getCastleKey(Rights));

  Rights &= CASTLING_UPDATE[From];
  Rights &= CASTLING_UPDATE[To];

  updateHash(getCastleKey(Rights));

  changeSide();

  // Add new en passant target in case of double push
  if (Mv.isDP()) {
    EpSq = EpCaptureSq;
    updateHash(getEPKey(EpSq));
  }

  return !(attacksAt(getBB(Piece::King, Stm.inverse()).lsb()) & getBB(Stm));
}

int Position::evaluate() const {
  const auto &Us = Acc[Stm].Data;
  const auto &Them = Acc[Stm.inverse()].Data;

  const auto screlu = [](int16_t x) {
    constexpr int16_t CR_MIN = 0;
    constexpr int16_t CR_MAX = 255;

    x = std::clamp(x, CR_MIN, CR_MAX);
    return static_cast<int32_t>(x) * static_cast<int32_t>(x);
  };

  int32_t Output = 0; // Need to be 32 bits to avoid overflow

  for (int i = 0; i < HIDDEN_SIZE; i++) {
      Output += screlu(Us[i])   * static_cast<int32_t>(NNUE.OutputWeights[0].Data[i]);
      Output += screlu(Them[i]) * static_cast<int32_t>(NNUE.OutputWeights[1].Data[i]);
  }

  Output /= QA;

  Output += NNUE.OutputBias;

  return Output * SCALE / QAB;
}
