#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>

#include "../Core/Position.h"
#include "HashTable.h"
#include "../Util.h"

using EvalScore = int16_t;

constexpr EvalScore NO_SCORE = 32001;
constexpr EvalScore INFINITY_SCORE = 32000;
constexpr EvalScore CHECKMATE_SCORE = 30000;

struct PVTable {
  Move moves[MAX_PLY][MAX_PLY];
  int length[MAX_PLY]= {};
};

struct SearchData { // Reset after each search
  int ply = 0; 

  MoveScore hh[2][64][64] = {}; // History heuristic

  uint16_t killers[MAX_PLY][2] = {}; // Killer moves

  inline void push(HashKey hash) {
    this->ply++;
  }

  inline void pop() {
    this->ply--;
  }

  inline void reset() {
    this->ply = 0;
    this->clearHeuristics<true>();
  }

  template<bool HardReset>
  inline void clearHeuristics() {
    std::memset(*this->hh, 0, sizeof(MoveScore[2][64][64]));
    std::memset(*this->killers, 0, sizeof(uint16_t[MAX_PLY][2]));
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
  SearchData sd = SearchData();
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
  [[nodiscard]] EvalScore aspirationSearch(int depth, EvalScore score);

  // Main evaluation function
  [[nodiscard]] EvalScore negamax(Position &pos, int depth, EvalScore alpha, EvalScore beta);

  [[nodiscard]] EvalScore qsearch(Position &pos, EvalScore alpha, EvalScore beta);
  
  [[nodiscard]] inline bool isDraw(Position &pos) {
    if (td.sd.ply == 0) return false; // Can't draw on first move

    if (pos.halfMove() >= 100) return true; // Draw by 50 moves rule

    return false;
  }
};

// Movepicker 

void scoreMoves(Position &pos, SearchData &sd, MoveList &ml, uint16_t bestMove);
Move pickMove(MoveList &ml);

constexpr MoveScore MVV_LVA[6][6] = {
  { 100005, 200005, 300005, 400005, 500005, 600005 },
  { 100004, 200004, 300004, 400004, 500004, 600004 },  
  { 100003, 200003, 300003, 400003, 500003, 600003 },  
  { 100002, 200002, 300002, 400002, 500002, 600002 },
  { 100001, 200001, 300001, 400001, 500001, 600001 },  
  { 100000, 200000, 300000, 400000, 500000, 600000 }
};
