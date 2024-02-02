#include <cassert>
#include <iostream>
#include "Core/BasicTypes.h"
#include "Core/Moves.h"
#include "Core/Position.h"
#include "Core/Zobrist.h"
#include "Search/NNUE/Network.h"
#include "Perft.h"
#include "Search/Search.h"
#include "UCI.h"

constexpr bool DEBUG = false;

int main (int argc, char *argv[]) {
  // lookup tables initialization
  initSliders();
  initBetweenSQ();
  initZobrist();
  initNNUE();

  if constexpr (DEBUG) {

  } else {
    std::cout << "Fodder "
              << "by Nek"
              << std::endl;

    uciLoop();
  }
}
