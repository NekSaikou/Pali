#pragma once

#include "core/Position.h"
#include "search/History.h"
#include "search/TTable.h"

#include <atomic>
#include <string>
#include <vector>

namespace pali {

struct Options {
  int Hash = 16;
  int MultiPV = 1;
  int Threads = 1;
};

namespace command {

void uci();

void isready();

void ucinewgame(const std::vector<std::string> &Params, Position &RootPos,
                Options &Opts, TTable &TTable, HTable &HTable);

void position(const std::vector<std::string> &Params, Position &RootPos);

void setoption(const std::vector<std::string> &Params, Options &Opts,
               TTable &TTable);

void go(const std::vector<std::string> &Params, const Position &RootPos,
        Options &Opts, std::atomic<bool> &Stopped, TTable &TTable,
        HTable &HTable);

void stop(std::atomic<bool> &Stopped);

void exit();

} // namespace command

} // namespace pali
