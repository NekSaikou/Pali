#pragma once

#include "../core/Move.h"
#include "../core/Position.h"

#include <array>
#include <cstdint>
#include <utility>

namespace fodder {

/// Manage move generation and ordering
struct MovePicker {
  enum Stage { Best, GenŅoisy, Noisy, GenQuiet, Quiet, Finished };
  Stage Stage = Best;
  MoveList QuietMl;
  MoveList NoisyMl;
  const Position &Pos;
  const Move BestMove;

  MovePicker(const Position &Pos, uint16_t PackedBM)
      : Pos(Pos), BestMove(
                      // Unpack best move
                      [&Pos, PackedBM]() -> Move {
                        auto [From, To, Flag] = Move::unpack(PackedBM);
                        Piece Pc = Pos.pieceAt(From);

                        return {From, To, Flag, Pc};
                      }()) {}

  /// Go to the next move picker stage
  void goNext() { Stage = static_cast<enum Stage>(Stage + 1); }

  /// Get the next best move
  template <bool NO_QUIET> [[nodiscard]] Move nextMove();

private:
  void scoreNoisy();
  void scoreQuiet();
};

} // namespace fodder
