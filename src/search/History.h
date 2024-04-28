#pragma once

#include "../core/Color.h"
#include "../core/Move.h"

#include <algorithm>
#include <array>
#include <cstdlib>

namespace pali {

constexpr int MH_CAP = 8192;

struct HTable {
  std::array<std::array<std::array<int, 2>, 64>, 64> MainHist{};

  /// Update heuristics related to quiet moves
  template <Operation OP> void updateQuiet(Color Stm, Move Mv, int Depth) {
    int Bonus = OP == Operation::Add ? Depth * Depth : -(Depth * Depth);
    int &MHScore = MainHist[Stm][Mv.From][Mv.To];

    // History Gravity:
    // Give less bonus as the score approaches cap
    Bonus -= abs(Bonus) * MHScore / MH_CAP;

    MHScore += Bonus;
  }

  /// Decay history to use them in the next search
  void softReset() {
    for (auto &ArrFrom : MainHist)
      for (auto &ArrStm : ArrFrom)
        for (int &Val : ArrStm)
          Val /= 2;
  }

  void clear() { MainHist = {}; }
};

} // namespace pali
