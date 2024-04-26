#include "SearchThread.h"

#include "../core/Move.h"
#include "../core/Position.h"
#include "MovePicker.h"
#include "TTable.h"

#include <algorithm>
#include <cstdint>

using namespace fodder;

int SearchThread::qsearch(const Position &Pos, int Ply, int α, int β) {
  // Increment node count and perform a checkup every 2048 nodes
  if ((++Nodes & 2047) == 0) {
    if (timeSpent() >= HardLim || Nodes >= NodesLim) {
      abort();
      return 0;
    }
  }

  if (Pos.isDraw())
    return 0;

  SelDepth = std::max(SelDepth, Ply);

  int Eval = Pos.evaluate();
  int BestScore = -INF_SCORE;
  uint16_t BestMove = 0;

  // Eval pruning
  if ((BestScore = Eval) >= β)
    return BestScore;

  α = std::max(α, BestScore);

  Bound Bound = Bound::Upper;
  MovePicker Mp(Pos, BestMove);
  while (true) {
    Move Mv = Mp.nextMove<true>();
    Position PosCopy = Pos;

    // Move picker finished
    if (Mv.isNullMove())
      break;

    // Skip illegal moves
    if (!PosCopy.makeMove(Mv))
      continue;

    int Score = -qsearch(PosCopy, Ply + 1, -β, -α);

    if (Score <= BestScore)
      continue;

    BestScore = Score;
    BestMove = Mv.pack();

    // Node fails high
    if (Score >= β) {
      Bound = Bound::Lower;
      break;
    }

    Bound = Bound::Exact;
    α = std::max(α, Score);
  }

  TTable.storeEntry(Pos.hash(), BestMove, BestScore, Eval, Bound, 0);

  return BestScore;
}
