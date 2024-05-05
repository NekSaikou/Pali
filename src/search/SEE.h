#pragma once

#include "../core/Attacks.h"
#include "../core/Move.h"
#include "../core/Position.h"

#include <cstdint>

namespace pali {

constexpr int16_t SEE_VAL[6]{100, 300, 300, 450, 850, 0};

/// Static Exchange Evaluation:
/// Evaluate exchange result without actually making any move
inline bool see(Position &Pos, Move Mv, int Threshold) {
  if (Mv.isCastle() || Mv.isPromo())
    return true;

  Square Sq = Mv.To;
  Piece AttackingPiece = Mv.Pc;
  Piece Target = Pos.pieceAt(Sq);
  int TargetValue = Target == Piece::None ? 0 : SEE_VAL[Target];

  // Assumes we lose the moved piece in the exchange
  int ExchangeScore = TargetValue - SEE_VAL[AttackingPiece] - Threshold;

  // If the exchange beats the threshold anyway then it will always pass
  if (ExchangeScore >= 0)
    return true;

  // Since we already moved the first piece, we must remove it from the board
  Bitboard Occ = Pos.allBB() & ~Mv.From.toBB() & ~Sq.toBB();

  Bitboard Attackers = Pos.attacksAt(Sq, Occ);

  // Sliders
  Bitboard Bishops = Pos.getBB(Piece::Bishop) | Pos.getBB(Piece::Queen);
  Bitboard Rooks = Pos.getBB(Piece::Rook) | Pos.getBB(Piece::Queen);

  // We already moved our piece, so we start from the opponent's turn
  Color SideToFail = Pos.stm().inverse();

  // Make capture until either side run out of piece or fail the threshold
  do {
    Attackers &= Occ; // Remove used pieces

    // The attacking side runs out of piece
    Bitboard Us = Attackers & Pos.getBB(SideToFail);
    if (Us == 0)
      return SideToFail != Pos.stm();

    // Pick the least valuable piece for the next move
    for (int Pc = Piece::Pawn; Pc <= Piece::King; Pc++) {
      if (Pos.getBB(static_cast<Piece::Type>(Pc)) & Us) {
        AttackingPiece = static_cast<Piece::Type>(Pc);
        Occ ^=
            Square((Us & Pos.getBB(AttackingPiece, SideToFail)).lsb()).toBB();
        break;
      }
    }

    // Check if new line of attacks opens up
    if (AttackingPiece == Piece::Pawn || AttackingPiece == Piece::Bishop ||
        AttackingPiece == Piece::Queen)
      Attackers |= getBishopAttack(Sq, Occ) & Bishops;

    if (AttackingPiece == Piece::Rook || AttackingPiece == Piece::Queen)
      Attackers |= getRookAttack(Sq, Occ) & Rooks;

    // —1 to correct off by one error from < vs <=
    ExchangeScore = -ExchangeScore - 1 - SEE_VAL[AttackingPiece];

    // Switch side then run the next iteration
    SideToFail = SideToFail.inverse();
  } while (ExchangeScore <= 0);

  // If the move fails the threshold but results leaves opponent king
  // in check, the failing side is the opponent
  if (AttackingPiece == Piece::King && Attackers & Pos.getBB(SideToFail))
    SideToFail = SideToFail.inverse();

  return SideToFail != Pos.stm();
}

} // namespace pali
