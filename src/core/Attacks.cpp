#include "Attacks.h"
#include "Bitboard.h"
#include "Color.h"
#include "Magic.h"
#include "Square.h"

#include <array>
#include <cstdint>

using namespace pali;

Bitboard PAWN_ATTACKS[64][2];
Bitboard KNIGHT_ATTACKS[64];
Bitboard KING_ATTACKS[64];

std::array<std::array<Bitboard, 512>, 64> BISHOP_ATTACKS;
std::array<std::array<Bitboard, 4096>, 64> ROOK_ATTACKS;

Bitboard BETWEEN_SQ[64][64];

Bitboard pali::getPawnAttack(Square Sq, Color Col) {
  return PAWN_ATTACKS[Sq][Col];
}

Bitboard pali::getKnightAttack(Square Sq) { return KNIGHT_ATTACKS[Sq]; }

Bitboard pali::getKingAttack(Square Sq) { return KING_ATTACKS[Sq]; }

Bitboard pali::getBishopAttack(Square Sq, Bitboard Occ) {
  return BISHOP_ATTACKS[Sq][BISHOP_MAGIC[Sq].magicIndex(Occ)];
}

Bitboard pali::getRookAttack(Square Sq, Bitboard Occ) {
  return ROOK_ATTACKS[Sq][ROOK_MAGIC[Sq].magicIndex(Occ)];
}

Bitboard pali::getBetweenSq(Square Sq1, Square Sq2) {
  return BETWEEN_SQ[Sq1][Sq2];
}

void initPawnAttack() {
  for (Square Sq = 0; Sq < 64; Sq += 1) {
    uint64_t CurrentSq = Sq.toBB();

    PAWN_ATTACKS[Sq][Color::White] = ((CurrentSq >> 9) & ~Bitboard::FILE_H) |
                                     ((CurrentSq >> 7) & ~Bitboard::FILE_A);
    PAWN_ATTACKS[Sq][Color::Black] = ((CurrentSq << 9) & ~Bitboard::FILE_A) |
                                     ((CurrentSq << 7) & ~Bitboard::FILE_H);
  }
}

void initKnightAttack() {
  for (Square Sq = 0; Sq < 64; Sq += 1) {
    uint64_t CurrentSq = Sq.toBB();

    KNIGHT_ATTACKS[Sq] =
        ((CurrentSq >> 17) & ~Bitboard::FILE_H) |
        ((CurrentSq << 17) & ~Bitboard::FILE_A) |
        ((CurrentSq >> 15) & ~Bitboard::FILE_A) |
        ((CurrentSq << 15) & ~Bitboard::FILE_H) |
        ((CurrentSq >> 10) & ~Bitboard::FILE_H & ~Bitboard::FILE_G) |
        ((CurrentSq << 10) & ~Bitboard::FILE_A & ~Bitboard::FILE_B) |
        ((CurrentSq >> 6) & ~Bitboard::FILE_A & ~Bitboard::FILE_B) |
        ((CurrentSq << 6) & ~Bitboard::FILE_H & ~Bitboard::FILE_G);
  }
}

void initKingAttack() {
  for (Square Sq = 0; Sq < 64; Sq += 1) {
    uint64_t CurrentSq = Sq.toBB();

    KING_ATTACKS[Sq] = (CurrentSq >> 8) | (CurrentSq << 8) |
                       ((CurrentSq >> 9) & ~Bitboard::FILE_H) |
                       ((CurrentSq << 9) & ~Bitboard::FILE_A) |
                       ((CurrentSq >> 7) & ~Bitboard::FILE_A) |
                       ((CurrentSq << 7) & ~Bitboard::FILE_H) |
                       ((CurrentSq >> 1) & ~Bitboard::FILE_H) |
                       ((CurrentSq << 1) & ~Bitboard::FILE_A);
  }
}

enum {
  North = -8,
  South = 8,
  East = 1,
  West = -1,
  NorthEast = -7,
  NorthWest = -9,
  SouthEast = 9,
  SouthWest = 7
};

Bitboard genSlidingAttack(Square Sq, Bitboard Occ, int Shift, Bitboard Acc) {
  if (((Shift == East || Shift == NorthEast || Shift == SouthEast) &&
       Sq.file() >= 7) ||
      ((Shift == West || Shift == NorthWest || Shift == SouthWest) &&
       Sq.file() <= 0) ||
      ((Shift == North || Shift == NorthEast || Shift == NorthWest) &&
       Sq.rank() >= 7) ||
      ((Shift == South || Shift == SouthEast || Shift == SouthWest) &&
       Sq.rank() <= 0) ||
      (Acc & Occ) != 0)
    return Acc;

  Sq += Shift;
  Acc.set(Sq);

  return Acc | genSlidingAttack(Sq, Occ, Shift, Acc);
}

Bitboard genBishopAttack(Square Sq, Bitboard Occ) {
  return genSlidingAttack(Sq, Occ, NorthEast, 0) |
         genSlidingAttack(Sq, Occ, NorthWest, 0) |
         genSlidingAttack(Sq, Occ, SouthEast, 0) |
         genSlidingAttack(Sq, Occ, SouthWest, 0);
}

Bitboard genRookAttack(Square Sq, Bitboard Occ) {
  return genSlidingAttack(Sq, Occ, North, 0) |
         genSlidingAttack(Sq, Occ, South, 0) |
         genSlidingAttack(Sq, Occ, East, 0) |
         genSlidingAttack(Sq, Occ, West, 0);
}

void initSliders() {
  auto fillTable = [](Square Sq, auto &Entry, auto &Table, auto *genAttack) {
    for (auto Occ : Entry.Mask.subsets())
      Table[Entry.magicIndex(Occ)] = genAttack(Sq, Occ);
  };

  for (int Sq = 0; Sq < 64; ++Sq) {
    fillTable(Sq, BISHOP_MAGIC[Sq], BISHOP_ATTACKS[Sq], genBishopAttack);
    fillTable(Sq, ROOK_MAGIC[Sq], ROOK_ATTACKS[Sq], genRookAttack);
  }
}

void initBetweenSq() {
  for (Square Sq1 = 0; Sq1 < 64; Sq1 += 1) {
    for (Square Sq2 = 0; Sq2 < 64; Sq2 += 1) {
      Bitboard Sqs = Sq1.toBB() | Sq2.toBB();

      int Rank1 = Sq1.rank();
      int Rank2 = Sq2.rank();

      int File1 = Sq1.file();
      int File2 = Sq2.file();

      int Diag1 = 7 + Rank1 - File1;
      int Diag2 = 7 + Rank2 - File2;

      int AntiDiag1 = Rank1 + File1;
      int AntiDiag2 = Rank2 + File2;

      // clang-format off
      // Generate attack between the Squares if the Squares are on
      // the same rank, file or diagonal, bitand is there to ensure
      // that only the aligned rays are stored
      if (Diag1 == Diag2 || AntiDiag1 == AntiDiag2)
        BETWEEN_SQ[Sq1][Sq2] = getBishopAttack(Sq1, Sqs) &
                               getBishopAttack(Sq2, Sqs);

      if (File1 == File2 || Rank1 == Rank2)
        BETWEEN_SQ[Sq1][Sq2] = getRookAttack(Sq1, Sqs) &
                               getRookAttack(Sq2, Sqs);

      /// There's nothing between the same Square
      if (Sq1 == Sq2)
        BETWEEN_SQ[Sq1][Sq2] = 0;
      // clang-format on
    }
  }
}

void pali::initAttackTables() {
  initPawnAttack();
  initKnightAttack();
  initKingAttack();
  initSliders();
  initBetweenSq();
}
