#pragma once

#include "Bitboard.h"
#include "Color.h"
#include "Square.h"

#include <array>

namespace pali {

/// Return a bitboard containing all squares that a pawn can attack
[[nodiscard]] Bitboard getPawnAttack(Square Sq, Color Col);

/// Return a bitboard containing all squares that a knight can attack
[[nodiscard]] Bitboard getKnightAttack(Square Sq);

/// Return a bitboard containing all squares that a king can attack
[[nodiscard]] Bitboard getKingAttack(Square Sq);

/// Return a bitboard containing all squares that a bishop can attack
[[nodiscard]] Bitboard getBishopAttack(Square Sq, Bitboard Occ);

/// Return a bitboard containing all squares that a rook can attack
[[nodiscard]] Bitboard getRookAttack(Square Sq, Bitboard Occ);

/// Return a bitboard containing all squares that a queen can attack
[[nodiscard]] constexpr Bitboard getQueenAttack(Square Sq, Bitboard Occ) {
  return getBishopAttack(Sq, Occ) | getRookAttack(Sq, Occ);
}

/// Return a bitboard containing all squares between the given squares
/// Return empty bitboard if they aren't aligned
[[nodiscard]] Bitboard getBetweenSq(Square Sq1, Square Sq2);

/// Initialize attack lookup tables for all pieces
void initAttackTables();

} // namespace pali
