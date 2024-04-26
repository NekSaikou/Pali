#include "Perft.h"

#include "../core/Move.h"
#include "../core/Position.h"
#include "../core/Util.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <iostream>

using namespace fodder;

uint64_t subperft(const Position &Pos, int Depth, std::atomic<bool> &Stopped) {
  if (Depth == 0 || Stopped)
    return 1;

  uint64_t Nodes = 0;

  MoveList Ml;
  Pos.genQuiet(Ml);
  Pos.genNoisy(Ml);

  for (Move Mv : Ml) {
    Position PosCopy = Pos;

    if (!PosCopy.makeMove(Mv))
      continue;

    Nodes += subperft(PosCopy, Depth - 1, Stopped);
  }

  return Nodes;
}

uint64_t fodder::perft(const Position &Pos, int Depth,
                          std::atomic<bool> &Stopped) {
  uint64_t TimeStart = getTimeMs();
  uint64_t NodesTotal = 0;

  MoveList Ml;
  Pos.genQuiet(Ml);
  Pos.genNoisy(Ml);

  for (Move Mv : Ml) {
    Position PosCopy = Pos;
    uint64_t CurrNodes = NodesTotal;

    if (!PosCopy.makeMove(Mv))
      continue;

    NodesTotal += subperft(PosCopy, Depth - 1, Stopped);

    if (Stopped)
      return 0;

    // Print out the differences
    std::cout << Mv.uciStr() << ": "
              << (Depth > 1 ? (NodesTotal - CurrNodes) : 1) << std::endl;
  }

  uint64_t Δt = getTimeMs() - TimeStart;

  std::cout << "\ntime taken: " << getTimeMs() - TimeStart << " ms"
            << "\nnodes: " << NodesTotal
            << "\nnps: " << NodesTotal / std::max(Δt, (uint64_t)1) * 1000
            << std::endl;

  return NodesTotal;
}
