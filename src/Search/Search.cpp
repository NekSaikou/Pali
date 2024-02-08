#include "Search.h"

template void Search::go<false>();
template void Search::go<true>();

template<bool MAIN_THREAD>
void Search::go() {
  EvalScore bestScore = NO_SCORE;
  Move bestMove = Move();

  td.reset(); // Start timer and clear UCI info

  EvalScore alpha = -INFINITY;
  EvalScore beta = INFINITY;
  // Iterative deepening
  for (int d = 1; d <= td.info.depth; d++) {
    // Call negamax
    bestScore = negamax(*td.rootPos, d, alpha, beta);

    // Stop the search if time ran out
    if (td.mustStop()) break;

    // Only use info from main thread
    if constexpr (MAIN_THREAD) {
      std::cout << "info score cp " << bestScore
                << " seldepth " << td.info.seldepth
                << " depth " << d
                << " nodes " << td.info.nodes
                << " time " << td.timeSpent()
                << " pv ";

      // Print out PV line
      for (int i = 0; i < *td.pvTable.length; i++) {
        std::cout << " " << td.pvTable.moves[0][i].string();
      }
      // End line then flush the buffer
      std::cout << std::endl;
      
      // Set best move to the top PV move
      bestMove = td.pvTable.moves[0][0];
    }
  } // End of iterative deepening loop

  // Print out the best move found
  if constexpr (MAIN_THREAD) 
    std::cout << "bestmove " << bestMove.string() << std::endl;

  // Soft reset heuristics to prepare for next search
  td.sd.clearHeuristics<false>();
  td.abort();
}

EvalScore Search::negamax(Position &pos, int depth, EvalScore alpha, EvalScore beta) {
  bool isPVNode = beta - alpha > 1; // PV nodes have full window
  Bound bound = BoundAlpha; // Cutoff bound to be stored in TT
  uint16_t bestMove = 0; // Best move to be stored in TT

  EvalScore eval = NO_SCORE; // Static eval of the position
  EvalScore score = NO_SCORE; // Search score of the position
  // Instructed to stop searching
  if (td.mustStop()) return 0;

  // Perform a checkup every 1024 nodes
  if ((td.info.nodes & 1023) == 0) {
    if (td.timeSpent() >= td.info.timelim 
    ||  td.info.nodes >= td.info.nodeslim) {
      td.abort();
      return 0;
    }
  }

  // Start of node stuffs
  td.info.nodes++; // Increment nodes count
  
  // TT probing
  std::optional<HashEntry> tte = hashTable->probeHashEntry(pos.getHash());
  if (tte != std::nullopt) {
    score = tte->score;
    eval = tte->eval;
    bestMove = tte->bestMove;

    // TT cutoff
    if (!isPVNode // Not PV node
    && depth <= tte->depth // Depth lower than TT entry
    ) {
      switch (tte->bound) {
        case BoundNone: break;
        case BoundAlpha: if (score <= alpha) return score; break;
        case BoundBeta: if (score >= beta) return score; break;
        case BoundExact: return score; break;
      }
    }
  } else {
    eval = pos.evaluate();
  }
  // Extend PV length
  td.pvTable.length[td.sd.ply] = td.sd.ply;

  // Check if we have to return
  if (isDraw(pos)) return 0; // Draw

  // Mate distance pruning
  alpha = std::max(alpha, static_cast<int16_t>(-CHECKMATE + td.sd.ply));
  beta = std::min(beta, static_cast<int16_t>(CHECKMATE - td.sd.ply));
  if (alpha >= beta) return alpha;

  // Check extension
  if (pos.inCheck()) depth++;

  // Leaf node or max ply exceeded
  if (depth <= 0 || td.sd.ply >= MAX_PLY - 1) return qsearch(pos, alpha, beta);

  if (!isPVNode) {
    // Null move pruning
    if (!pos.inCheck()
    &&  eval >= beta    
    &&  td.sd.ply > 0   
    &&  depth >= 3      
    &&  pos.getColoredPieceBB(pos.sideToMove(), Pawn) // Still have pawns left
    ) {
      int R = 3 + depth / 3 + std::min((eval - beta) / 200, 3);

      // Give opponent a free move
      pos.changeSide();

      EvalScore nmpScore = -negamax(pos, depth - R, -beta, -beta + 1);

      // Revert the side to move
      pos.changeSide();

      if (nmpScore >= beta) return beta;
    }
  }

  // Update stack and ply
  td.sd.push(pos.getHash());
  
  // Generate moves
  MoveList ml = MoveList();
  pos.genLegal<false>(ml);
  scoreMoves(pos, td.sd, ml, bestMove);

  // Move loop starts
  int movesSearched = 0;
  while (ml.getLength()) {
    Move mv = pickMove(ml);
    Position posCopy = pos;
    posCopy.makeMove(mv);

    movesSearched++;

    // PVS
    if (movesSearched == 1) {
      score = -negamax(posCopy, depth - 1, -beta, -alpha) ;
    } else {
      EvalScore zwsScore = -negamax(posCopy, depth- 1, -alpha - 1, -alpha);

      // If the move beats alpha, continue searching as PV node
      score = zwsScore > alpha && isPVNode
        ? -negamax(posCopy, depth - 1, -beta, -alpha)
        : zwsScore;
    };

    // Node fails high
    if (score >= beta) {
      alpha = beta;
      // Update TT entry
      bound = BoundBeta;
      bestMove = mv.compress();

      if (mv.isQuiet()) {
        // Store history heuristic
        td.sd.hh[pos.sideToMove()][mv.getFrom()][mv.getTo()] += depth * depth;

        // Store killer moves
        // Make sure the same move doesn't get stored twice
        if (mv.compress() != td.sd.killers[td.sd.ply][0]) {
          td.sd.killers[td.sd.ply][1] = td.sd.killers[td.sd.ply][0];
          td.sd.killers[td.sd.ply][0] = mv.compress();
        }
      }


      break;
    }

    if (score > alpha) {
      alpha = score;
      // Update TT entry
      bound = BoundExact;
      bestMove = mv.compress();

      // Alpha raised, update PV
      if (isPVNode) {
        td.pvTable.moves[td.sd.ply - 1][td.sd.ply - 1] = mv;
        for (int i = td.sd.ply; i < td.pvTable.length[td.sd.ply]; i++) {
            td.pvTable.moves[td.sd.ply - 1][i] = td.pvTable.moves[td.sd.ply][i];
        }
        // Extend the lower PV line
        td.pvTable.length[td.sd.ply - 1] = td.pvTable.length[td.sd.ply];
      }
    }
  }

  // End of node stuffs
  td.sd.pop();

  // Store TT entry
  hashTable->storeHashEntry(pos.getHash(), bestMove, score, eval, bound, depth);

  // Checkmate or stalemate
  if (movesSearched == 0) return pos.inCheck() ? -CHECKMATE + td.sd.ply : 0;

  return alpha;
}

EvalScore Search::qsearch(Position &pos, EvalScore alpha, EvalScore beta) {
  // Perform a checkup every 1024 nodes
  if ((td.info.nodes & 1023) == 0) {
    if (td.timeSpent() >= td.info.timelim 
    ||  td.info.nodes >= td.info.nodeslim) {
      td.abort();
      return 0;
    }
  }

  // Start of node stuffs
  td.info.seldepth = std::max(td.info.seldepth, td.sd.ply);

  EvalScore eval = pos.evaluate();

  if (eval >= beta) return eval;

  alpha = std::max(alpha, eval);

  td.sd.ply++;

  // Only generate noisy moves
  MoveList ml = MoveList();
  pos.genLegal<true>(ml);
  scoreMoves(pos, td.sd, ml, 0);

  // Move loop starts
  while (ml.getLength()) {
    Move mv = pickMove(ml);
    Position posCopy = pos;
    posCopy.makeMove(mv);

    td.info.nodes += 1;

    EvalScore score = -qsearch(posCopy, -beta, -alpha);

    if (score <= eval) continue;

    eval = score;

    if (eval >= beta) break;

    alpha = std::max(alpha, eval);
  }

  // End of node stuffs
  td.sd.ply--;

  return eval;
}
