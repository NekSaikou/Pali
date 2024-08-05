#pragma once

#include "core/Move.h"
#include "core/Piece.h"
#include "core/Position.h"
#include "search/History.h"

#include <cstdint>

namespace pali {

/// Manage move generation and ordering
struct MovePicker {
  enum Stage { Best, GenŅoisy, GoodNoisy, GenQuiet, Quiet, BadNoisy, Finished };
  Stage Stage = Best;
  MoveList QuietMl;
  MoveList NoisyMl;
  MoveList BadNoisyMl;
  const Position &Pos;
  const int Ply;
  const Move BestMove;
  HTable &HTable;

  MovePicker(const Position &Pos, int Ply, uint16_t PackedBM,
             struct HTable &HTable)
      : Pos(Pos), Ply(Ply),
        BestMove(
            // Unpack best move
            [&Pos, PackedBM]() -> Move {
              auto [From, To, Flag] = Move::unpack(PackedBM);
              Piece Pc = Pos.pieceAt(From);

              return {From, To, Flag, Pc};
            }()),
        HTable(HTable) {}

  /// Go to the next move picker stage
  void goNext() { Stage = static_cast<enum Stage>(Stage + 1); }

  /// Get the next best move
  template <bool QSEARCH> [[nodiscard]] Move nextMove();

private:
  void scoreNoisy();
  void scoreQuiet();
};

} // namespace pali
