#pragma once

#include "../core/Color.h"
#include "../core/Move.h"

namespace pali {

struct HTable {
  int Butterfly[2][64][64]{};

  /// Update heuristics related to quiet moves
  void updateQuiet(Color Stm, Move Mv, int Depth) {
    Butterfly[Stm][Mv.From][Mv.To] += Depth * Depth;
  }
};

} // namespace pali
