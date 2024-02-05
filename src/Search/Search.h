#pragma once

#include <algorithm>
#include <atomic>

#include "../Core/Position.h"
#include "Movepick.h"
#include "HashTable.h"
#include "../Util.h"

using EvalScore = int16_t;

constexpr EvalScore NO_SCORE = 32001;
constexpr EvalScore INFINITY = 32000;
constexpr EvalScore CHECKMATE = 30000;

struct PVTable {
  Move moves[MAX_PLY][MAX_PLY];
  int length[MAX_PLY]= {};
};

struct SearchStack { // Reset after each search
  int ply = 0; 

  inline void push(HashKey hash) {
    this->ply++;
  }

  inline void pop() {
    this->ply--;
  }

  inline void reset() {
    this->ply = 0;
  }
};

struct SearchInfo { // UCI control
  // Search control
  Time starttime = 0;
  Time timelim = UINT64_MAX;
  int depth = 100;
  uint64_t nodeslim = UINT64_MAX;
  int movestogo = 25;

  // UCI outputs
  int seldepth = 0;
  uint64_t nodes = 0;
};

struct ThreadData { // All information a thread need
  Position *rootPos;
  SearchStack ss = SearchStack();
  SearchInfo info = SearchInfo();

  PVTable pvTable = PVTable();

  std::atomic<bool> *stop = nullptr;

  ThreadData(Position *rootPos, std::atomic<bool> *stop) {
    this->rootPos = rootPos;
    this->stop = stop;
  }

  [[nodiscard]] inline bool mustStop() {
    return this->stop->load(std::memory_order_relaxed);
  }

  inline void abort() {
    this->stop->store(true, std::memory_order_relaxed);
  }

  inline void reset() {
    this->stop->store(false, std::memory_order_relaxed);
    this->info.starttime = getTimeMs();
    this->info.nodes = 0;
    this->info.seldepth = 0;
  }

  [[nodiscard]] inline Time timeSpent() {
    return getTimeMs() - this->info.starttime;
  }
};

class Search { // Search interface
public:
  Search(ThreadData td, HashTable *hashTable) {
    this->td = td;
    this->hashTable = hashTable;
  }

  ThreadData td = ThreadData(nullptr, nullptr);
  HashTable *hashTable = nullptr;

  // All searches start here
  template<bool MAIN_THREAD>
  void go();

private:
  // Main evaluation function
  [[nodiscard]] EvalScore negamax(Position &pos, int depth, EvalScore alpha, EvalScore beta);

  [[nodiscard]] EvalScore qsearch(Position &pos, EvalScore alpha, EvalScore beta);
  
  [[nodiscard]] inline bool isDraw(Position &pos) {
    if (td.ss.ply == 0) return false; // Can't draw on first move

    if (pos.halfMove() >= 100) return true; // Draw by 50 moves rule

    return false;
  }
};

