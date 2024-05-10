#pragma once

#include <bit>
#include <cstdint>
#include <vector>

namespace pali {

/// Wrapper class for uint64_t representing the board
class Bitboard {
  uint64_t Data;

public:
  constexpr Bitboard() : Data(0) {}

  constexpr Bitboard(uint64_t Data) : Data(Data) {}

  [[nodiscard]] Bitboard operator~() const { return ~Data; }

  [[nodiscard]] Bitboard operator&(Bitboard Rhs) const { return Data & Rhs; }

  [[nodiscard]] Bitboard operator|(Bitboard Rhs) const { return Data | Rhs; }

  [[nodiscard]] Bitboard operator^(Bitboard Rhs) const { return Data ^ Rhs; }

  [[nodiscard]] Bitboard operator<<(int Rhs) const { return Data << Rhs; }

  [[nodiscard]] Bitboard operator>>(int Rhs) const { return Data >> Rhs; }

  void operator&=(Bitboard Rhs) { Data &= Rhs; }

  void operator|=(Bitboard Rhs) { Data |= Rhs; }

  void operator^=(Bitboard Rhs) { Data ^= Rhs; }

  void operator<<=(int Rhs) { Data <<= Rhs; }

  void operator>>=(int Rhs) { Data >>= Rhs; }

  [[nodiscard]] Bitboard operator+(Bitboard Rhs) const { return Data + Rhs; }

  [[nodiscard]] Bitboard operator-(Bitboard Rhs) const { return Data - Rhs; }

  [[nodiscard]] Bitboard operator*(Bitboard Rhs) const { return Data * Rhs; }

  void operator+=(Bitboard Rhs) { Data += Rhs; }

  void operator-=(Bitboard Rhs) { Data -= Rhs; }

  void operator*=(Bitboard Rhs) { Data *= Rhs; }

  [[nodiscard]] Bitboard operator-() const { return -Data; }

  [[nodiscard]] operator uint64_t() const { return Data; }

  /// Return the least significant bit of bitboard
  /// (first occupied square counted from a8 to h1)
  ///
  /// Using this on an empty board is undefined behavior
  [[nodiscard]] int lsb() const { return std::countr_zero(Data); }

  /// Return the number of pieces on the board
  [[nodiscard]] int popcnt() const { return std::popcount(Data); }

  /// Add a piece to the square at the index
  void set(int Idx) { Data |= 1ULL << Idx; }

  /// Remove a piece from the sqaure at the index
  void pop(int Idx) { Data &= ~(1ULL << Idx); }

  /// Return the least significant bit then remove it
  [[nodiscard]] int takeLsb() {
    int Lsb = lsb();
    Data &= Data - 1;
    return Lsb;
  }

  /// Return a bitboard contaning only the piece at the index
  [[nodiscard]] Bitboard getBit(int Idx) const { return Data & (1ULL << Idx); }

  /// Return all subsets of bits set to 1
  [[nodiscard]] std::vector<Bitboard> subsets() const {
    std::vector<Bitboard> Subsets;

    // Enumerate all subsets using Carry-Rippler trick
    uint64_t Mask = 0;
    do {
      Subsets.push_back(Mask);

      Mask = (Mask - Data) & Data;
    } while (Mask);

    return Subsets;
  }

  static constexpr uint64_t RANK_1 = 0xff00000000000000;
  static constexpr uint64_t RANK_2 = 0xff000000000000;
  static constexpr uint64_t RANK_3 = 0xff0000000000;
  static constexpr uint64_t RANK_4 = 0xff00000000;
  static constexpr uint64_t RANK_5 = 0xff000000;
  static constexpr uint64_t RANK_6 = 0xff0000;
  static constexpr uint64_t RANK_7 = 0xff00;
  static constexpr uint64_t RANK_8 = 0xff;

  static constexpr uint64_t FILE_A = 0x101010101010101;
  static constexpr uint64_t FILE_B = 0x202020202020202;
  static constexpr uint64_t FILE_C = 0x404040404040404;
  static constexpr uint64_t FILE_D = 0x808080808080808;
  static constexpr uint64_t FILE_E = 0x1010101010101010;
  static constexpr uint64_t FILE_F = 0x2020202020202020;
  static constexpr uint64_t FILE_G = 0x4040404040404040;
  static constexpr uint64_t FILE_H = 0x8080808080808080;
};

} // namespace pali
