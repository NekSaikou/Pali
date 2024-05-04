#pragma once

#include "Piece.h"
#include "Square.h"
#include "Util.h"

#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>

namespace pali {

/// Maximum number of possible moves a position can contain
constexpr int MAX_MOVE = 218;

/// 4-bit move Flags used for move generation
enum class MFlag : uint8_t {
  Normal,
  DoublePush,
  Castle,
  Capture = 4,
  EnPassant,
  KnightPromo = 8,
  BishopPromo,
  RookPromo,
  QueenPromo,
  KnightPromoCapture,
  BishopPromoCapture,
  RookPromoCapture,
  QueenPromoCapture
};

using MScore = int32_t;

struct Move {
  Square From;
  Square To;
  MFlag Flag;
  Piece Pc;
  MScore Score = 0; // Used in move ordering

  constexpr Move() : From(0), To(0), Flag(MFlag::Normal), Pc(Piece::Pawn) {}

  constexpr Move(Square From, Square To, MFlag Flag, Piece Pc)
      : From(From), To(To), Flag(Flag), Pc(Pc) {}

  [[nodiscard]] bool operator==(Move Rhs) const {
    return From == Rhs.From && To == Rhs.To && Flag == Rhs.Flag;
  }

  [[nodiscard]] bool isDP() const { return Flag == MFlag::DoublePush; }

  [[nodiscard]] bool isEP() const { return Flag == MFlag::EnPassant; }

  [[nodiscard]] bool isCastle() const { return Flag == MFlag::Castle; }

  [[nodiscard]] uint8_t castleType() const {
    uint8_t Castle = 0;

    switch (To) {
    case Square::G1:
      Castle = 1;
      break;
    case Square::C1:
      Castle = 2;
      break;
    case Square::G8:
      Castle = 4;
      break;
    case Square::C8:
      Castle = 8;
      break;
    }

    return Castle;
  }

  [[nodiscard]] bool isCapture() const { return std::to_underlying(Flag) & 4; }

  [[nodiscard]] bool isPromo() const { return std::to_underlying(Flag) & 8; }

  [[nodiscard]] Piece promoType() const {
    return static_cast<Piece::Type>((std::to_underlying(Flag) & 3) + 1);
  }

  /// Pack move into 16 bits
  [[nodiscard]] uint16_t pack() const {
    return From | (To << 6) | (std::to_underlying(Flag) << 12);
  }

  /// Unpack information from 16-bits move into the form of [From, To, Flag]
  [[nodiscard]] static std::tuple<Square, Square, MFlag> unpack(uint16_t Mv) {
    return {Square(Mv & 63),      // From
            Square(Mv >> 6 & 63), // To
            static_cast<MFlag>((Mv >> 12))};
  }

  /// String representation in UCI format
  [[nodiscard]] std::string uciStr() const {
    std::string Str;

    Str += From.str();
    Str += To.str();

    constexpr const char *PIECE_SYMBOL[6]{"p", "n", "b", "r", "q", "k"};
    if (isPromo())
      Str += PIECE_SYMBOL[promoType()];

    return Str;
  }

  /// Check if the move is null
  [[nodiscard]] bool isNullMove() const { return From == To; }
};

constexpr Move NULL_MOVE(0, 0, MFlag::Normal, Piece::Pawn);

using MoveList = ArrayVec<Move, MAX_MOVE>;

} // namespace pali
