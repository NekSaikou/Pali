#include "Commands.h"

#include "../core/Move.h"
#include "../core/Position.h"
#include "../search/SearchThread.h"
#include "../search/TTable.h"
#include "Perft.h"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace pali;

std::thread MainThread;
std::vector<std::thread> HelperThreads;

void joinThreads() {
  if (MainThread.joinable())
    MainThread.join();

  for (auto &Thread : HelperThreads)
    if (Thread.joinable())
      Thread.join();

  HelperThreads.clear();
}

void pali::command::uci() {
  std::cout << "id name Pali\n"
            << "id author Nek\n"
            << "option name MultiPV type spin default 1 min 1 max 16\n"
            << "option name Hash type spin default 16 min 1 max 262144\n"
            << "option name Threads type spin default 1 min 1 max 512\n"
            << "option name Clear Hash type button\n"
            << "uciok\n";
}

void pali::command::isready() { std::cout << "readyok\n"; }

void pali::command::ucinewgame(const std::vector<std::string> &Params,
                                    Position &RootPos, Options &Opt) {
  RootPos = Position(STARTPOS);

  // TODO: Clear/resize hash table
}

void pali::command::position(const std::vector<std::string> &Params,
                                  Position &RootPos) {
  for (auto It = Params.begin(); It < Params.end(); ++It) {
    if (*It == "startpos")
      RootPos = Position(STARTPOS);

    else if (*It == "fen") {
      std::string Fen;
      while (++It < Params.end() && *It != "moves")
        Fen += *It + " ";

      RootPos = Position(Fen);
      It--; // We might be on the token "moves"
    }

    else if (*It == "moves") {
      while (++It < Params.end()) {
        MoveList Ml = MoveList();
        RootPos.genNoisy(Ml);
        RootPos.genQuiet(Ml);


        for (Move Mv : Ml)
          if (Mv.uciStr() == *It)
            RootPos.makeMove(Mv);
      }
    }
  }
}

void pali::command::setoption(const std::vector<std::string> &Params,
                                   Options &Opts, TTable &TTable) {
  // setoption name [option name] value [value]
  for (auto It = Params.begin(); It < Params.end(); ++It) {
    if (*It == "name") {
      if (*(It + 1) == "Hash")
        TTable.resize(std::stoi(*(It + 3)));

      else if (*(It + 1) == "MultiPV")
        Opts.MultiPV = std::stoi(*(It + 3));

      else if (*(It + 1) == "Threads")
        Opts.Threads = std::stoi(*(It + 3));

      else if (*(It + 1) == "Clear")
        TTable.clear();
    }
  }
}

void pali::command::go(const std::vector<std::string> &Params,
                            const Position &RootPos, Options &Opts,
                            std::atomic<bool> &Stopped, TTable &TTable) {
  // Join any running thread
  joinThreads();

  // Mark as not stopped
  Stopped = false;

  int movestogo = 20;
  int depth = 255;
  uint64_t movetime = UINT64_MAX;
  uint64_t nodes = UINT64_MAX;
  uint64_t wtime = UINT64_MAX;
  uint64_t btime = UINT64_MAX;
  uint64_t winc = 0;
  uint64_t binc = 0;

  for (auto It = Params.begin(); It < Params.end(); ++It) {
    if (*It == "perft") {
      int Depth = std::stoi(*(It + 1));
      MainThread = std::thread(
          [&RootPos, &Stopped, Depth]() { perft(RootPos, Depth, Stopped); });

      return;
    }

    else if (*It == "wtime")
      wtime = std::stoi(*(It + 1));

    else if (*It == "btime")
      btime = std::stoi(*(It + 1));

    else if (*It == "winc")
      winc = std::stoi(*(It + 1));

    else if (*It == "binc")
      binc = std::stoi(*(It + 1));

    else if (*It == "depth")
      depth = std::stoi(*(It + 1));

    else if (*It == "nodes")
      nodes = std::stoi(*(It + 1));

    else if (*It == "movetime")
      movetime = std::stoi(*(It + 1));

    else if (*It == "movestogo")
      movestogo = std::stoi(*(It + 1));
  }

  uint64_t Time = RootPos.stm().isWhite() ? wtime : btime;
  uint64_t Inc = RootPos.stm().isWhite() ? winc : binc;

  SearchThread St = SearchThread(Stopped, Time, Inc, movetime, movestogo, depth,
                                 nodes, Opts.MultiPV, TTable);

  MainThread = std::thread(
      [St](Position Pos) { SearchThread(St).go<true>(Pos); }, RootPos);

  HelperThreads.reserve(Opts.Threads - 1);
  for (int i = 1; i < Opts.Threads; i++)
    HelperThreads.push_back(std::thread(
        [St](Position Pos) { SearchThread(St).go<false>(Pos); }, RootPos));
}

void pali::command::stop(std::atomic<bool> &Stopped) {
  Stopped = true;

  joinThreads();
}

void pali::command::exit() {
  joinThreads();

  std::exit(0);
}
