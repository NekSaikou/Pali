#include "Movepick.h"

void scoreMoves(Position &pos, MoveList &ml) {
  for (int i = 0; i < ml.getLength(); i++) {
    Move mv = ml.getMove(i);
    MoveScore score = 0;

    if (mv.isCapture()) {
      Piece target = pos.pieceOnSQ(mv.getTo());
      // Most valuable victim, least valuable attacker
      score += MVV_LVA[target][mv.getPiece()];
    } else {
      
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
