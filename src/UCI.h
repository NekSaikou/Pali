#pragma once

#include <thread>

#include "Perft.h"
#include "Core/Position.h"
#include "Search/Search.h"
#include "Util.h"

void uciLoop();

struct Options {
  int threads = 1;
  int multipv = 0;
  int hash = 16;
};

void command_uci();

void command_isready();

void command_ucinewgame(Search &searcher, Options &options);

void command_position(Search &searcher);

void command_setoption(Options &options);

void command_go(Search &searcher, Options &options);

void command_stop(Search &searcher);
