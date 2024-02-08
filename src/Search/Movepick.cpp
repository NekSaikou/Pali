#include "Search.h"

constexpr MoveScore NOISY_SCORE = 200'000'000;
constexpr MoveScore QUIET_CAP = NOISY_SCORE - 1;

constexpr MoveScore KILLER_0 = 90'000'000;
constexpr MoveScore KILLER_1 = 80'000'000;

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
        score += sd.hh[pos.sideToMove()][mv.getFrom()][mv.getTo()];

        // Killer heuristic
        if (mv.compress() == sd.killers[sd.ply][0]) score += KILLER_0;
        if (mv.compress() == sd.killers[sd.ply][1]) score += KILLER_1;

        // Make sure quiet moves will not be scored above noisy moves
        score = std::min(QUIET_CAP, score);
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
