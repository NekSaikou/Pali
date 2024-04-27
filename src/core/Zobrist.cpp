#include "Zobrist.h"

#include "Color.h"
#include "Piece.h"
#include "Square.h"

#include <cstdint>
#include <random>

using namespace pali;

uint64_t PIECE_KEYS[6][64];
uint64_t COLOR_KEYS[2][64];
uint64_t EP_KEYS[64];
uint64_t CASTLE_KEYS[16];
uint64_t STM_KEY;

uint64_t pali::getPieceKey(Piece Pc, Square Sq) {
  return PIECE_KEYS[Pc][Sq];
}

uint64_t pali::getColorKey(Color Col, Square Sq) {
  return COLOR_KEYS[Col][Sq];
}

uint64_t pali::getEPKey(Square Sq) { return EP_KEYS[Sq]; }

uint64_t pali::getCastleKey(uint8_t Rights) { return CASTLE_KEYS[Rights]; }

uint64_t pali::getStmKey() { return STM_KEY; }

std::mt19937_64 randHash(0x123456789);

void pali::initZobrist() {
  for (int Sq = 0; Sq < 64; ++Sq) {
    for (int Pc = Piece::Pawn; Pc <= Piece::King; ++Pc)
      PIECE_KEYS[Pc][Sq] = randHash();

    for (int Col = Color::White; Col <= Color::Black; ++Col)
      COLOR_KEYS[Col][Sq] = randHash();

    EP_KEYS[Sq] = randHash();
  }

  for (int Rights = 0; Rights < 16; ++Rights)
    CASTLE_KEYS[Rights] = randHash();

  STM_KEY = randHash();
}
