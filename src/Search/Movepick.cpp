#include "Search.h"

constexpr MoveScore NOISY_SCORE = 10'000'000;
constexpr MoveScore HISTORY_CAP = NOISY_SCORE - 1;

void scoreMoves(Position &pos, SearchData &sd, MoveList &ml, uint16_t bestMove) {
  for (int i = 0; i < ml.getLength(); i++) {
    Move mv = ml.getMove(i);
    MoveScore score = 0;

    // If matches with TT move, give it the best score
    if (mv.compress() == bestMove) {
      score = INT32_MAX;
    } else {
      if (!mv.isQuiet()) {
        score += NOISY_SCORE; // Flat noisy move bonus

        Piece target = mv.isEP()
          ? Pawn // En passant always capture pawn
          : pos.pieceOnSQ(mv.getTo());

        // Most valuable victim, least valuable attacker
        if (mv.isCapture()) 
          score += MVV_LVA[target][mv.getPiece()];

      } else {
        // History heuristic
        score += std::min(HISTORY_CAP, sd.hh[pos.sideToMove()][mv.getFrom()][mv.getTo()]);
      }
    }

    ml.scoreMove(i, score);
  }
}

Move pickMove(MoveList &ml) {
  MoveScore bestScore = INT32_MIN;
  int bestIx = 0;

  // Find the highest scored move
  for (int i = 0; i < ml.getLength(); i++) {
    if (ml.getScore(i) > bestScore) {
      bestScore = ml.getScore(i);
      bestIx = i;
    }
  }

  // Move the move found to the last position
  ml.swap(bestIx, ml.getLength() - 1);

  // Pop the last move out and return it
  return ml.takeLast();
}
