#pragma once

#include "Bitboard.h"

#include <cstdint>

namespace fodder {

/// Wrapper class for int8_t representing square number
/// starts from 0 representing a8 to 63 representing h1
class Square {
  int8_t Data;

public:
  constexpr Square() : Data(-1) {}

  constexpr Square(int8_t Data) : Data(Data) {}

  [[nodiscard]] Square operator+(Square Rhs) const { return Data + Rhs; }

  [[nodiscard]] Square operator-(Square Rhs) const { return Data - Rhs; }

  void operator+=(Square Rhs) { Data += Rhs; }

  void operator-=(Square Rhs) { Data -= Rhs; }

  [[nodiscard]] operator int() const { return Data; }

  /// Return a bitboard containing only a piece on the square
  [[nodiscard]] Bitboard toBB() const { return 1ULL << Data; }

  /// Return the index of the rank where the square belongs
  [[nodiscard]] int rank() const { return 7 - (Data >> 3); }

  /// Return the index of the file where the square belongs
  [[nodiscard]] int file() const { return Data & 7; }

  [[nodiscard]] bool exists() const { return Data != None; }

  [[nodiscard]] char const *str() const {
    // clang-format off
    constexpr char const *SQUARE_NAME[64]{
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
    };
    // clang-format on

    return SQUARE_NAME[Data];
  }

  // clang-format off
  enum {
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7, 
    A6, B6, C6, D6, E6, F6, G6, H6, 
    A5, B5, C5, D5, E5, F5, G5, H5, 
    A4, B4, C4, D4, E4, F4, G4, H4, 
    A3, B3, C3, D3, E3, F3, G3, H3, 
    A2, B2, C2, D2, E2, F2, G2, H2, 
    A1, B1, C1, D1, E1, F1, G1, H1, None = -1
  };
  // clang-format on
};

} // namespace fodder
