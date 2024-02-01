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

  // Reset the search stack to prepare for next search
  td.ss.reset();
}

EvalScore Search::negamax(Position &pos, int depth, EvalScore alpha, EvalScore beta) {
  bool isPVNode = beta - alpha > 1;
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

  { // Start of node stuffs
    // Extend PV length
    td.pvTable.length[td.ss.ply] = td.ss.ply;

    // Check if we have to return
    if (isDraw(pos)) return 0; // Draw

    td.info.nodes++; // Increment nodes count

    // Mate distance pruning
    alpha = std::max(alpha, static_cast<int16_t>(-CHECKMATE + td.ss.ply));
    beta = std::min(beta, static_cast<int16_t>(CHECKMATE - td.ss.ply));
    if (alpha >= beta) return alpha;

    // Check extension
    if (pos.inCheck()) depth++;

    // Leaf node or max ply exceeded
    // if (depth == 0 || td.ss.ply >= MAX_PLY - 1) return qsearch(pos, alpha, beta);
    if (depth == 0 || td.ss.ply >= MAX_PLY - 1) return pos.evaluate();

    // Update stack and ply
    td.ss.push(pos.getHash());
  }

  // Generate moves
  MoveList ml = MoveList();
  pos.genLegal<false>(ml);

  // Move loop starts
  int movesSearched = 0;
  for (int i = 0; i < ml.getLength(); i++) {
    Move mv = ml.getMove(i);
    Position posCopy = pos;
    posCopy.makeMove(mv);

    // PVS
    EvalScore score = NO_SCORE;
    // TODO: Change `true` to `movesSearched == 0` after move ordering
    if (true) {
      score = -negamax(posCopy, depth - 1, -beta, -alpha) ;
    } else {
      EvalScore zwsScore = -negamax(posCopy, depth- 1, -alpha - 1, -alpha);

      // If the move beats alpha, continue searching as PV node
      score = zwsScore > alpha && isPVNode
        ? -negamax(posCopy, depth - 1, -beta, -alpha)
        : zwsScore;
    };
    movesSearched++;

    // Node fails high
    if (score >= beta) {
      alpha = beta;
      break;
    }

    if (score > alpha) {
      alpha = score;

      // Alpha raised, update PV
      td.pvTable.moves[td.ss.ply - 1][td.ss.ply - 1] = mv;
      for (int i = td.ss.ply; i < td.pvTable.length[td.ss.ply]; i++) {
          td.pvTable.moves[td.ss.ply - 1][i] = td.pvTable.moves[td.ss.ply][i];
      }
      // Extend the lower PV line
      td.pvTable.length[td.ss.ply - 1] = td.pvTable.length[td.ss.ply];
    }
  }

  // End of node stuffs
  td.ss.pop();

  // Checkmate or stalemate
  if (ml.getLength() == 0) return pos.inCheck() ? -CHECKMATE + td.ss.ply : 0;

  return alpha;
}

EvalScore Search::qsearch(Position &pos, EvalScore alpha, EvalScore beta) {
  td.info.seldepth = std::max(td.info.seldepth, td.ss.ply);

  EvalScore eval = pos.evaluate();

  if (eval >= beta) return eval;

  alpha = std::max(alpha, eval);

  // Only generate noisy moves
  MoveList ml = MoveList();
  pos.genLegal<true>(ml);

  // Move loop starts
  for (int i = 0; i < ml.getLength(); i++) {
    Move mv = ml.getMove(i);
    Position posCopy = pos;
    posCopy.makeMove(mv);

    td.info.nodes += 1;

    EvalScore score = qsearch(posCopy, -beta, -alpha);

    if (score <= eval) continue;
    else eval = score;

    if (eval >= beta) return eval;

    alpha = std::max(alpha, eval);
  }

  return eval;
}
