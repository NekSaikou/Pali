#include "search/SearchThread.h"

#include "core/Move.h"
#include "core/Position.h"

#include <algorithm>
#include <cstdint>
#include <iostream>

using namespace pali;

template void SearchThread::go<true>(Position &);
template void SearchThread::go<false>(Position &);

template <bool MAIN> void SearchThread::go(Position &RootPos) {
  int BestScore = 0;
  Move BestMove = Move();
  uint64_t PrevTimeSpent = 0;

  HTable.softReset();

  // Iterative deepening
  for (int Depth = 1; Depth <= DepthLim; ++Depth) {
    SearchedPV.clear();
    for (int Pv = 1; Pv <= MultiPV; ++Pv) {
      BestScore = Depth > 5 ? aspirationSearch(RootPos, Depth, BestScore)
                            : negamax(RootPos, Depth, 0, -INF_SCORE, INF_SCORE);

      if (Stopped)
        break;

      // Only print info from main thread
      if (MAIN) {
        // Mate detection
        int Mate = BestScore >= MATE_SCORE - MAX_PLY
                       ? (MATE_SCORE - BestScore + 1) / 2
                       : 0;

        double Nps = timeSpent() > 5
                         ? Nodes / static_cast<double>(timeSpent()) * 1000.0
                         : 0.0;

        // clang-format off
        std::cout << "info score" 
                  << (Mate ? " mate " : " cp ")
                  << (Mate ? Mate : BestScore)
                  << " multipv " << Pv
                  << " seldepth " << SelDepth
                  << " depth " << Depth
                  << " nodes " << Nodes
                  << " time " << timeSpent()
                  << " nps " << static_cast<unsigned int>(Nps)
                  << " hashfull " << TTable.hashfull()
                  << " pv";
        // clang-format on

        for (int i = 0; i < PVTable.Length[0]; ++i)
          std::cout << " " << PVTable.Moves[0][i].uciStr();
        std::cout << std::endl;

        // The best move is the one from the first PV
        if (Pv == 1)
          BestMove = PVTable.Moves[0][0];

        // Exclude searched move from the next multi PV search
        SearchedPV.push_back(PVTable.Moves[0][0]);

        // Soft TM:
        // If a depth takes over a certain amount of time to clear,
        // it's probably not possible to clear the next depth
        if (timeSpent() - PrevTimeSpent >= SoftLim)
          break;

        PrevTimeSpent = timeSpent();
      }
    }
  }

  if (MAIN) {
    // If it's forced draw by 50 moves rule then
    // we might not have any move to play
    if (BestMove.isNullMove()) {
      MoveList Ml;
      RootPos.genQuiet(Ml);
      BestMove = Ml[0];

      return;
    }

    std::cout << "bestmove " << BestMove.uciStr() << std::endl;

    TTable.ageUp();
  }

  abort();
}

int SearchThread::aspirationSearch(const Position &RootPos, int Depth,
                                   int Score) {
  int δ = 15;

  int α = std::max(Score - δ, -INF_SCORE);
  int β = std::min(Score + δ, INF_SCORE);

  int FailHighStreak = 0;

  while (true) {
    int AdjustedDepth = Depth - FailHighStreak;
    Score = negamax(RootPos, AdjustedDepth, 0, α, β);

    if (Stopped)
      return 0;

    if (Score <= α) {
      // If we fail low, it's unlikely to fail high even with lowered β
      // so we can adjust it closer to α before doing research
      β = (α + β) / 2;

      α = std::max(α - δ, -INF_SCORE);

      FailHighStreak = 0;
    }

    else if (Score >= β) {
      β = std::min(Score + δ, INF_SCORE);

      ++FailHighStreak;
    }

    else
      return Score;

    δ *= 2;
  }
}
