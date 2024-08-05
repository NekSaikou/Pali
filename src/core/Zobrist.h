#pragma once

#include "core/Color.h"
#include "core/Piece.h"
#include "core/Square.h"

#include <cstdint>

namespace pali {

/// Initialize Zobrist keys
void initZobrist();

/// Return Zobrist key of the piece on the given square
/// based on its type
uint64_t getPieceKey(Piece Pc, Square Sq);

/// Return Zobrist key of the piece on the given square
/// based on its color
uint64_t getColorKey(Color Col, Square Sq);

/// Return Zobrist key of en passant Square
uint64_t getEPKey(Square Sq);

/// Return Zobrist key of the castling rights combination
uint64_t getCastleKey(uint8_t Rights);

/// Return Zobrist key of a side to move color
uint64_t getStmKey();

} // namespace pali
