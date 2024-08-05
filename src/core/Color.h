#pragma once

#include <cstdint>

namespace pali {

/// Side color
class Color {
public:
  enum Type : uint8_t { White, Black, None = 255 };

private:
  Type Data;

public:
  constexpr Color() : Data(None) {};

  constexpr Color(Color::Type Col) : Data(Col) {};

  [[nodiscard]] operator int() const { return Data; }

  [[nodiscard]] Color inverse() const {
    return static_cast<Color::Type>(Data ^ 1);
  }

  [[nodiscard]] bool isWhite() const { return Data == White; }
};

} // namespace pali
