#include "Attacks.h"

Bitboard BISHOP_ATTACKS[64][512] ;
Bitboard ROOK_ATTACKS[64][4096];

Bitboard genSlidingAttack(Square sq, Bitboard occ, Direction shift, Bitboard acc) {
  if (((shift == East || shift == NorthEast || shift == SouthEast) && fileIx(sq) >= 7)
  || (((shift == West || shift == NorthWest || shift == SouthWest) && fileIx(sq) <= 0)
  || (((shift == North || shift == NorthEast || shift == NorthWest) && rankIx(sq) >= 7))
  || (((shift == South || shift == SouthEast || shift == SouthWest) && rankIx(sq) <= 0))
  || (acc & occ) != 0)) 
  { // Return the current mask once collided with a piece or left the board
    return acc;
  }

  sq += shift;
  setBit(acc, sq);

  return acc | genSlidingAttack(sq, occ, shift, acc);
}

Bitboard genBishopAttack(Square sq, Bitboard occ) {
  return genSlidingAttack(sq, occ, NorthEast, 0)
       | genSlidingAttack(sq, occ, NorthWest, 0)
       | genSlidingAttack(sq, occ, SouthEast, 0)
       | genSlidingAttack(sq, occ, SouthWest, 0);
}

Bitboard genRookAttack(Square sq, Bitboard occ) {
  return genSlidingAttack(sq, occ, North, 0) 
       | genSlidingAttack(sq, occ, South, 0)
       | genSlidingAttack(sq, occ, East, 0)
       | genSlidingAttack(sq, occ, West, 0);
}

void initSliders() {
  for (int sq = 0; sq < 64; sq++) {
    // Bishop moves
    {
      HashKey magic = BISHOP_MAGIC[sq];
      Bitboard shift = BISHOP_SHIFTS[sq];
      Bitboard range = BISHOP_MASKS[sq];
      Bitboard subset = 0;
      do { // Carry-Rippler trick 
        int ix = (subset * magic) >> shift;
        BISHOP_ATTACKS[sq][ix] = genBishopAttack(sq, subset);

        subset = (subset - range) & range;
      } while (subset);
    }

    // Rook moves
    {
      HashKey magic = ROOK_MAGIC[sq];
      Bitboard shift = ROOK_SHIFTS[sq];
      Bitboard range = ROOK_MASKS[sq];
      Bitboard subset = 0;
      do { 
        int ix = (subset * magic) >> shift;
        ROOK_ATTACKS[sq][ix] = genRookAttack(sq, subset);

        subset = (subset - range) & range;

      } while (subset);
    }
  }
}
