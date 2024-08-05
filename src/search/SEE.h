#pragma once

#include "core/Attacks.h"
#include "core/Bitboard.h"
#include "core/Color.h"
#include "core/Move.h"
#include "core/Piece.h"
#include "core/Position.h"
#include "core/Square.h"

namespace pali {

constexpr int SEE_VAL[6]{100, 300, 300, 450, 850, 0};

/// Static Exchange Evaluation:
/// Evaluate exchange result without actually making any move
inline bool see(const Position &Pos, Move Mv, int Threshold) {
  if (Mv.isCastle() || Mv.isPromo() || Mv.isEP())
    return true;

  Square Sq = Mv.To;
  Piece AttackingPiece = Mv.Pc;
  Piece Target = Pos.pieceAt(Sq);
  int TargetValue = Target == Piece::None ? 0 : SEE_VAL[Target];

  // Assumes we lose the moved piece in the exchange
  int Score = TargetValue - SEE_VAL[Mv.Pc] - Threshold;

  // If the exchange beats the threshold anyway then it will always pass
  if (Score >= 0)
    return true;

  // Since we already moved the first piece, we must remove it from the board
  Bitboard Occ = Pos.allBB() & ~Mv.From.toBB() & ~Sq.toBB();

  Bitboard Attackers = Pos.attacksAt(Sq, Occ);

  // Might have new lines of attack open up during the exchange
  Bitboard Bishops = Pos.getBB(Piece::Bishop) | Pos.getBB(Piece::Queen);
  Bitboard Rooks = Pos.getBB(Piece::Rook) | Pos.getBB(Piece::Queen);

  Color SideToFail = Pos.stm().inverse();

  // Make capture until either side run out of piece or fail the threshold
  while (true) {
    // Remove used pieces
    Attackers &= Occ;

    // The attacking side runs out of piece
    Bitboard Us = Attackers & Pos.getBB(SideToFail);
    if (Us == 0)
      break;

    // Pick the least valuable piece for the next move
    for (int Pc = Piece::Pawn; Pc <= Piece::King; ++Pc) {
      Bitboard AttackerBB = Pos.getBB(static_cast<Piece::Type>(Pc)) & Us;
      if (AttackerBB) {
        AttackingPiece = static_cast<Piece::Type>(Pc);

        Occ &= ~(AttackerBB & -AttackerBB);

        break;
      }
    }

    // —1 to correct off by one error from < vs <=
    Score = -Score - 1 - SEE_VAL[AttackingPiece];

    SideToFail = SideToFail.inverse();

    if (Score >= 0) {
      // If the move fails the threshold but leaves the opposing king
      // in check then the failing side is the opponent
      if (AttackingPiece == Piece::King && Attackers & Pos.getBB(SideToFail))
        SideToFail = SideToFail.inverse();

      break;
    }

    // Check if there is another attacker behind
    if (AttackingPiece == Piece::Pawn || AttackingPiece == Piece::Bishop ||
        AttackingPiece == Piece::Queen)
      Attackers |= getBishopAttack(Sq, Occ) & Bishops;

    if (AttackingPiece == Piece::Rook || AttackingPiece == Piece::Queen)
      Attackers |= getRookAttack(Sq, Occ) & Rooks;
  }

  return SideToFail != Pos.stm();
}

} // namespace pali
