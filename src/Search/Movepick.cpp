#include "Movepick.h"

void scoreMoves(Position &pos, MoveList &ml, uint16_t bestMove) {
  for (int i = 0; i < ml.getLength(); i++) {
    Move mv = ml.getMove(i);
    MoveScore score = 0;

    // If matches with TT move, give it the best score
    if (mv.compress() == bestMove) {
      score = INT32_MAX;
    } else {
      if (mv.isCapture()) {
        Piece target = mv.isEP()
          ? Pawn // En passant always capture pawn
          : pos.pieceOnSQ(mv.getTo());

        // Most valuable victim, least valuable attacker
        score += MVV_LVA[target][mv.getPiece()];
      } else {
        
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
