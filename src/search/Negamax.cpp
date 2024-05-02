#include "LogTable.h"
#include "SearchThread.h"

#include "../core/Move.h"
#include "../core/Piece.h"
#include "../core/Position.h"
#include "../core/Util.h"
#include "MovePicker.h"
#include "TTable.h"

#include <algorithm>
#include <cstdint>

using namespace pali;

int SearchThread::negamax(const Position &Pos, int Depth, int Ply, int α,
                          int β) {
  // Increment node count and perform a checkup every 2048 nodes
  if ((++Nodes & 2047) == 0) {
    if (timeSpent() >= HardLim || Nodes >= NodesLim) {
      abort();
      return 0;
    }
  }

  if (Stopped)
    return 0;

  if (Pos.isDraw())
    return 0;

  // Node has full window
  bool IsPVNode = β - α > 1;

  bool IsInCheck = Pos.isInCheck();

  PVTable.Length[Ply] = Ply;

  // Mate distance pruning
  α = std::max(α, -MATE_SCORE + Ply);
  β = std::min(β, MATE_SCORE - Ply);
  if (α >= β)
    return α;

  // Check extension
  if (IsInCheck)
    ++Depth;

  // Leaf node or max ply exceeded
  if (Depth <= 0 || Ply >= MAX_PLY - 1)
    return qsearch(Pos, Ply + 1, α, β);

  // TT Probing
  auto Tte = TTable.probeEntry(Pos.hash());
  bool TTHit = Tte != nullptr;

  // TT cutoff
  if (TTHit && !IsPVNode && Depth <= Tte->Depth) {
    int TTScore = Tte->Score;

    switch (Tte->bound()) {
    case Bound::Upper:
      if (TTScore <= α)
        return TTScore;

    case Bound::Lower:
      if (TTScore >= β)
        return TTScore;

    case Bound::Exact:
      return TTScore;
    }
  }

  int Eval = TTHit ? Tte->Eval : Pos.evaluate();
  int BestScore = -INF_SCORE;
  uint16_t BestMove = TTHit ? Tte->BestMove : 0;

  if (!IsPVNode && !IsInCheck) {
    bool isKPEndgame =
        (Pos.getBB(Piece::Pawn) | Pos.getBB(Piece::King)) == Pos.allBB();
    int NmpDepth = 3;

    // Static NMP/Reverse Futility Pruning:
    // If eval is a certain amount above β,
    // prune out the node immediately
    int RfpMargin = Depth * 80;
    if (Eval >= β + RfpMargin)
      return Eval;

    // Null Move Pruning:
    // If our position is so good that we still beat β
    // even if we give our opponent a free move,
    // then this node is likely going to fail high.
    //
    // Doesn't work in king and pawn endgame due to zugzwang
    if (Eval >= β && Depth >= NmpDepth && !isKPEndgame) {
      int R = 3 + Depth / 3 + std::min((Eval - β) / 200, 3);

      const_cast<Position &>(Pos).changeSide();

      int NmpScore = -negamax(Pos, Depth - R, Ply + 1, -β, -β + 1);

      const_cast<Position &>(Pos).changeSide();

      if (NmpScore >= β)
        return NmpScore >= MATE_SCORE - MAX_PLY ? β : NmpScore;
    }
  }

  // Internal Iterative Reduction:
  // If we don't have TT move then the search will likely
  // take a long time, so we perform a reduced search then come back later
  if (Depth >= 3 && BestMove == 0 && IsPVNode)
    --Depth;

  Bound Bound = Bound::Upper;
  int MovesMade = 0;
  MovePicker Mp(Pos, Ply, BestMove, HTable);
  while (true) {
    Move Mv = Mp.nextMove<false>();

    // Move picker finished
    if (Mv.isNullMove())
      break;

    // Duplicate PV line
    if (Ply == 0 && isSearched(Mv))
      continue;

    Position PosCopy = Pos;

    // Skip illegal moves
    if (!PosCopy.makeMove(Mv))
      continue;

    // Prefetch TT if the move is legal
    TTable.prefetch(PosCopy.hash());

    ++MovesMade;

    // Late Move Reduction:
    // Moves ordered later are probably worse
    // so we perform search with reduced depth instead
    int Reduction = 0;
    // clang-format off
    if (Depth >= 2 &&
        Mp.Stage >= MovePicker::Quiet &&
        Mv.Score < 2'000'000'000) {
      Reduction = static_cast<int>(0.3 * ln(Depth) * ln(MovesMade) + 0.8);

      Reduction -= IsPVNode;
    }
    // clang-format on

    // Don't accidentally extend
    Reduction = std::max(Reduction, 0);

    // Search the first move with full window
    int Score;
    if (MovesMade == 1)
      Score = -negamax(PosCopy, Depth - 1 - Reduction, Ply + 1, -β, -α);

    // Perform zero window search on the rest
    else {
      int ZwsScore =
          -negamax(PosCopy, Depth - 1 - Reduction, Ply + 1, -α - 1, -α);

      // If the move doesn't fail low, continue searching as PV node
      Score = ZwsScore > α && IsPVNode
                  ? -negamax(PosCopy, Depth - 1, Ply + 1, -β, -α)
                  : ZwsScore;
    };

    if (Score <= BestScore)
      continue;

    BestScore = Score;
    BestMove = Mv.pack();

    // Node fails high
    if (Score >= β) {
      Bound = Bound::Lower;

      if (!Mv.isCapture())
        HTable.updateQuiet<Operation::Add>(Pos.stm(), Mv, Depth, Ply);

      break;
    }

    if (IsPVNode)
      PVTable.update(Ply, Mv);

    Bound = Bound::Exact;
    α = std::max(α, Score);
  }

  // Checkmate or stalemæte
  if (MovesMade == 0)
    return IsInCheck ? -MATE_SCORE + Ply : 0;

  TTable.storeEntry(Pos.hash(), BestMove, BestScore, Eval, Bound, Depth);

  return BestScore;
}
