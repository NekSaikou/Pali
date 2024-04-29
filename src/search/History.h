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

  std::array<Move, 128> Killer;

  /// Update heuristics related to quiet moves
  template <Operation OP>
  void updateQuiet(Color Stm, Move Mv, int Depth, int Ply) {
    int Bonus = OP == Operation::Add ? Depth * Depth : -(Depth * Depth);
    int &MHScore = MainHist[Stm][Mv.From][Mv.To];

    // History Gravity:
    // Give less bonus as the score approaches cap
    Bonus -= abs(Bonus) * MHScore / MH_CAP;

    // Main History:
    // Each time a quiet move causes a cutoff, give it some score
    // scaling with depth
    MHScore += Bonus;

    // Killer Move:
    // Try the quiet move that causes cutoff before other quiet moves
    Killer[Ply] = Mv;
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
