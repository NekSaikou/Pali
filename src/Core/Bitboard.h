#pragma once

#include "BasicTypes.h"

[[nodiscard]] inline Bitboard sqToBB(Square sq) {
  return 1ULL << sq;
}

inline void setBit(Bitboard &bb, Square sq) {
  bb |= sqToBB(sq);
}

inline void popBit(Bitboard &bb, Square sq) {
  bb &= ~sqToBB(sq);
}

[[nodiscard]] inline Bitboard getBit(Bitboard bb, Square sq) {
  return bb & sqToBB(sq);
}

[[nodiscard]] inline Square lsb(Bitboard bb) {
  return __builtin_ctzll(bb);
}

[[nodiscard]] inline int popcnt(Bitboard bb) {
  return __builtin_popcountll(bb);
}

[[nodiscard]] inline Rank rankIx(Square sq) {
  return 7 - (sq >> 3);
}

[[nodiscard]] inline File fileIx(Square sq) {
  return sq & 7;
}

[[nodiscard]] inline Square squareIx(Rank rank, File file) {
  return rank * 8 + file;
}

// Second/seventh rank relative to side for double push/promotion
constexpr Bitboard SECOND_RANK[2] { 0xff000000000000ULL, 0xff00ULL };
constexpr Bitboard SEVENTH_RANK[2] { 0xff00ULL, 0xff000000000000ULL };
