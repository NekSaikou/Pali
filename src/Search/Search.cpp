#include "Search.h"

template void Search::go<false>();
template void Search::go<true>();

template<bool MAIN_THREAD>
void Search::go() {
  EvalScore bestScore = 0;
  Move bestMove = Move();

  td.reset(); // Start timer and clear UCI info

  // Iterative deepening
  for (int d = 1; d <= td.info.depth; d++) {
    bestScore = d >= 6
      ? aspirationSearch(d, bestScore)
      : negamax(*td.rootPos, d, -INFINITY_SCORE, INFINITY_SCORE);

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

  if constexpr (MAIN_THREAD) {
    // Print out the best move found
    std::cout << "bestmove " << bestMove.string() << std::endl;

    hashTable->ageUp(); // Increment TT age
  }

  // Soft reset heuristics to prepare for next search
  td.sd.clearHeuristics<false>();
  td.abort();
}

EvalScore Search::aspirationSearch(int depth, EvalScore score) {
  EvalScore delta = 15; // Extra starting window
  EvalScore alpha = std::max(static_cast<EvalScore>(score - delta), static_cast<EvalScore>(-INFINITY_SCORE));
  EvalScore beta =  std::min(static_cast<EvalScore>(score + delta), static_cast<EvalScore>(INFINITY_SCORE));

  int d = depth; // Depth used for search

  while (true) {
    score = negamax(*td.rootPos, d, alpha, beta);

    // Stop the search if time ran out
    if (td.mustStop()) return 0;

    if (score <= alpha) {
      beta = (alpha + beta) / 2;
      alpha = std::max(static_cast<EvalScore>(alpha - delta), static_cast<EvalScore>(-INFINITY_SCORE));
      d = depth;
    } else if (score >= beta) {
      beta =  std::min(static_cast<EvalScore>(beta + delta), static_cast<EvalScore>(INFINITY_SCORE));
      depth--;
    } else return score;

    delta *= 2;
  }
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
      switch (tte->bound()) {
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
  alpha = std::max(alpha, static_cast<int16_t>(-CHECKMATE_SCORE + td.sd.ply));
  beta = std::min(beta, static_cast<int16_t>(CHECKMATE_SCORE - td.sd.ply));
  if (alpha >= beta) return alpha;

  // Check extension
  if (pos.inCheck()) depth++;

  // Leaf node or max ply exceeded
  if (depth <= 0 || td.sd.ply >= MAX_PLY - 1) return qsearch(pos, alpha, beta);

  if (!isPVNode && !pos.inCheck()) {
    // Null move pruning
    if (eval >= beta
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

  // Internal iterative reduction
  if (depth >= 4 && tte == std::nullopt) depth--;

  // Late move reduction base
  double lmrBase = std::log(static_cast<double>(depth)) / 3.0;

  // Move loop starts
  int movesSearched = 0;
  while (ml.getLength()) {
    Move mv = pickMove(ml);
    MoveScore mvScore = ml.getScore(ml.getLength());
    Position posCopy = pos;
    posCopy.makeMove(mv);

    movesSearched++;

    // Late move redution
    int R = 0;
    if (!pos.inCheck()
    && depth >= 2
    && td.sd.ply > 0
    && mvScore < KILLER_1 // Don't reduce killer moves or better
    ) {
      R = static_cast<int>(lmrBase * std::log(static_cast<double>(movesSearched)) + 0.8);

      R -= static_cast<int>(isPVNode); // Don't reduce PV nodes as much
      R -= posCopy.inCheck(); // Reduce less if the new move is a check
      R -= mvScore <= HISTORY_MAX ? mvScore / 16384 : 0; // Reduce less for moves with good history

      R = std::max(0, R); // Don't accidentally extend
    }

    // PVS
    if (movesSearched == 1) {
      score = -negamax(posCopy, depth - 1, -beta, -alpha) ;
    } else {
      EvalScore zwsScore = -negamax(posCopy, depth - 1 - R, -alpha - 1, -alpha);

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

  // Checkmate or stalemate
  if (movesSearched == 0) return pos.inCheck() ? -CHECKMATE_SCORE + td.sd.ply : 0;

  // Store TT entry
  hashTable->storeHashEntry(pos.getHash(), bestMove, score, eval, bound, depth);

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

  Bound bound = BoundAlpha; // Cutoff bound to be stored in TT

  // Start of node stuffs
  td.info.seldepth = std::max(td.info.seldepth, td.sd.ply);

  uint16_t bestMove = 0;      // For hash move ordering
  EvalScore eval = NO_SCORE;

  // Probe TT
  std::optional<HashEntry> tte = hashTable->probeHashEntry(pos.getHash());
  if (tte != std::nullopt) {
    eval = tte->eval;
    bestMove = tte->bestMove;

    // TT cutoff
    EvalScore score = tte->score;
    {
      switch (tte->bound()) {
        case BoundNone: break;
        case BoundAlpha: if (score <= alpha) return score; break;
        case BoundBeta: if (score >= beta) return score; break;
        case BoundExact: return score; break;
      }
    }
  } else {
    eval = pos.evaluate();
  }

  EvalScore staticEval = eval;

  if (eval >= beta) return eval;

  alpha = std::max(alpha, eval);

  td.sd.ply++;

  // Only generate noisy moves
  MoveList ml = MoveList();
  pos.genLegal<true>(ml);
  scoreMoves(pos, td.sd, ml, bestMove);

  // Move loop starts
  while (ml.getLength()) {
    Move mv = pickMove(ml);
    Position posCopy = pos;
    posCopy.makeMove(mv);

    td.info.nodes += 1;

    EvalScore score = -qsearch(posCopy, -beta, -alpha);

    if (score <= eval) continue;

    eval = score;

    if (eval >= beta)  {
      bound = BoundBeta;
      break;
    }

    if (eval > alpha) bound = BoundExact;

    alpha = std::max(alpha, eval);
  }

  // End of node stuffs
  td.sd.ply--;

  // Store TT entry
  hashTable->storeHashEntry(pos.getHash(), bestMove, eval, staticEval, bound, 0);

  return eval;
}
