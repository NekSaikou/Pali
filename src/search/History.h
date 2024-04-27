#pragma once

#include "../core/Color.h"
#include "../core/Move.h"

#include <algorithm>
#include <array>

namespace pali {

struct HTable {
  std::array<std::array<std::array<int, 2>, 64>, 64> Butterfly{};

  /// Update heuristics related to quiet moves
  void updateQuiet(Color Stm, Move Mv, int Depth) {
    Butterfly[Stm][Mv.From][Mv.To] += Depth * Depth;
  }

  void softReset() {
    for (auto &ArrFrom : Butterfly)
      for (auto &ArrStm : ArrFrom)
        for (int &Val : ArrStm)
          std::clamp(Val, 0, 0);
  }

  void clear() { Butterfly = {}; }
};

} // namespace pali
