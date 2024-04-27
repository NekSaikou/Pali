#include "SearchThread.h"

#include "../core/Move.h"
#include "../core/Piece.h"
#include "../core/Position.h"
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

  int Eval;
  int BestScore = -INF_SCORE;
  uint16_t BestMove = 0;

  // TT Probing
  bool TTHit = false;
  auto Tte = TTable.probeEntry(Pos.hash());
  if (Tte != nullptr) {
    Eval = Tte->Eval;
    BestMove = Tte->BestMove;
    TTHit = true;
  }

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

  if (!TTHit)
    Eval = Pos.evaluate();

  if (!IsPVNode && !IsInCheck) {
    // Null move pruning:
    // If our position is so good that we still beat β
    // even if we give our opponent a free move,
    // then this node is likely going to fail high.
    //
    // Doesn't work in king and pawn endgame due to zugzwang
    if (Eval >= β && Depth >= 3 &&
        (Pos.getBB(Piece::Pawn) | Pos.getBB(Piece::King)) != Pos.allBB()) {
      int R = 3 + Depth / 3 + std::min((Eval - β) / 200, 3);

      const_cast<Position &>(Pos).changeSide();

      int NmpScore = -negamax(Pos, Depth - R, Ply + 1, -β, -β + 1);

      const_cast<Position &>(Pos).changeSide();

      if (NmpScore >= β)
        return NmpScore >= MATE_SCORE - MAX_PLY ? β : NmpScore;
    }
  }

  Bound Bound = Bound::Upper;
  int MovesMade = 0;
  MovePicker Mp(Pos, BestMove, HTable);
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

    // Search the first move with full window
    int Score;
    if (MovesMade == 1)
      Score = -negamax(PosCopy, Depth - 1, Ply + 1, -β, -α);

    // Perform zero window search on the rest
    else {
      int ZwsScore = -negamax(PosCopy, Depth - 1, Ply + 1, -α - 1, -α);

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
        HTable.updateQuiet(Pos.stm(), Mv, Depth);

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
