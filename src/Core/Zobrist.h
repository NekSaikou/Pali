#pragma once

#include "BasicTypes.h"

inline HashKey PIECE_KEYS[6][64];
inline HashKey COLOR_KEYS[2][64];
inline HashKey EP_KEYS[64];
inline HashKey CASTLE_KEYS[16];
inline HashKey STM_KEY;

static HashKey seed = 557755;

[[nodiscard]] inline HashKey randHash() {
  seed ^= seed << 13;
  seed ^= seed >> 7;
  seed ^= seed << 17;
  return seed;
}

inline void initZobrist() {
  for (int sq = 0; sq < 64; sq++) {
    for (int pc = Pawn; pc <= King; pc++) {
      PIECE_KEYS[pc][sq] = randHash();
    }
    for (int col = White; col <= Black; col++) {
      COLOR_KEYS[col][sq] = randHash();
    }

    EP_KEYS[sq] = randHash();
  }

  for (int i = 0; i < 16; i++) {
    CASTLE_KEYS[i] = randHash();
  }

  STM_KEY = randHash();
}

[[nodiscard]] inline HashKey getPieceKey(Piece pc, Square sq) {
  return PIECE_KEYS[pc][sq];
}

[[nodiscard]] inline HashKey getColorKey(Color col, Square sq) {
  return COLOR_KEYS[col][sq];
}

[[nodiscard]] inline HashKey getEPKey(Square sq) {
  return EP_KEYS[sq];
}

[[nodiscard]] inline HashKey getCastleKey(uint8_t rights) {
  return CASTLE_KEYS[rights];
}

[[nodiscard]] inline HashKey getSTMKey() {
  return STM_KEY;
}
