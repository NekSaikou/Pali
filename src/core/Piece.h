#pragma once

#include <cstdint>

namespace fodder {

/// Chess piece
class Piece {
public:
  enum Type : uint8_t { Pawn, Knight, Bishop, Rook, Queen, King, None = 255 };

private:
  Type Data;

public:
  constexpr Piece() : Data(None){};

  constexpr Piece(Piece::Type Pc) : Data(Pc){};

  [[nodiscard]] operator int() const { return Data; }

  /// Piece value for SEE
  [[nodiscard]] int16_t seeVal() const {
    constexpr int16_t PC_VALUE[6]{100, 300, 300, 450, 800, 30000};
    return PC_VALUE[Data];
  }

  /// Piece value for MVV-LVA
  [[nodiscard]] int32_t lvaVal() const {
    return 6 - Data;
  }

  /// Piece value for MVV-LVA
  [[nodiscard]] int32_t mvvVal() const {
    return Data * 100000;
  }

  [[nodiscard]] char const *str() const {
    constexpr char const *PIECE_SYMBOL[7]{"p", "n", "b", "r", "q", "k"};
    return PIECE_SYMBOL[Data];
  }
};

class Pawn : public Piece {

};

} // namespace fodder
