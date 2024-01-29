#pragma once

#include <cstdint>

constexpr int32_t MAX_PLY = 128;

using Bitboard = uint64_t;
using HashKey = uint64_t;
using Square = int8_t;
using Rank = int8_t;
using File = int8_t;
using Diag = int8_t;
using Time = uint64_t;

enum Piece : uint8_t { Pawn, Knight, Bishop, Rook, Queen, King, NO_PC = 254 };
enum Color : uint8_t { White, Black };

enum CastlingRights : uint8_t
    { C_WK = 1
    , C_WQ = 2
    , C_BK = 4
    , C_BQ = 8
    };

enum Direction : int 
    { North = -8
    , South = 8
    , East = 1
    , West = -1
    , NorthEast = -7
    , NorthWest = -9
    , SouthEast = 9
    , SouthWest = 7
    };

enum Coordinate : int8_t
    { a8, b8, c8, d8, e8, f8, g8, h8
    , a7, b7, c7, d7, e7, f7, g7, h7
    , a6, b6, c6, d6, e6, f6, g6, h6
    , a5, b5, c5, d5, e5, f5, g5, h5
    , a4, b4, c4, d4, e4, f4, g4, h4
    , a3, b3, c3, d3, e3, f3, g3, h3
    , a2, b2, c2, d2, e2, f2, g2, h2
    , a1, b1, c1, d1, e1, f1, g1, h1, NO_SQ = 127
    };

constexpr char const* SQUARE_NAME[64] {
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

constexpr char const* PIECE_SYMBOL[6] {
  "p", "n", "b", "r", "q", "k"
};
