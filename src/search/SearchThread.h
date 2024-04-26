#pragma once

#include "../core/Position.h"
#include "TTable.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>

namespace fodder {

// Search scores have to fit into 16 bits
// so they can be stored efficiently in TT

constexpr int NO_SCORE = 32001;
constexpr int INF_SCORE = 32000;
constexpr int MATE_SCORE = 30000;
constexpr int MAX_PLY = 128;

struct PVTable {
  std::array<std::array<Move, 128>, 128> Moves;
  std::array<int, 128> Length;

  void update(int Ply, Move Mv) {
    Moves[Ply][Ply] = Mv;
    for (int i = Ply + 1; i < Length[Ply + 1]; ++i) {
      Moves[Ply][i] = Moves[Ply + 1][i];
    }
    // Extend the lower PV line
    Length[Ply] = Length[Ply + 1];
  }
};

struct SearchThread {
  std::atomic<bool> &Stopped;

  const int DepthLim;
  const uint64_t NodesLim;

  const int MultiPV;
  std::vector<Move> SearchedPV;

  const uint64_t StartTime;
  const uint64_t HardLim;
  const uint64_t SoftLim;

  int SelDepth = 0;
  uint64_t Nodes = 0;

  PVTable PVTable;
  TTable &TTable;

  SearchThread(std::atomic<bool> &Stopped, uint64_t Time, uint64_t Inc,
               uint64_t MoveTime, int MovesToGo, int DepthLim,
               uint64_t NodesLim, int MultiPV, class TTable &TTable)
      : Stopped(Stopped), DepthLim(DepthLim), NodesLim(NodesLim),
        MultiPV(MultiPV), StartTime(getTimeMs()),
        HardLim(std::min(MoveTime, Time / MovesToGo + 3 * Inc / 4)),
        SoftLim(HardLim == MoveTime ? MoveTime : 7 * HardLim / 10),
        TTable(TTable) {}

  template <bool MAIN> void go(Position &RootPos);

  void abort() { Stopped = true; }

  [[nodiscard]] uint64_t timeSpent() { return getTimeMs() - StartTime; }

private:
  /// Main search function
  [[nodiscard]] int negamax(const Position &Pos, int Depth, int Ply, int α,
                            int β);

  /// Quiessence search
  [[nodiscard]] int qsearch(const Position &Pos, int Ply, int α, int β);

  /// Check if move is already searched
  [[nodiscard]] bool isSearched(Move Mv) {
    for (auto SearchedMv : SearchedPV)
      if (Mv == SearchedMv)
        return true;

    return false;
  }
};

} // namespace fodder
