#include "UCI.h"

constexpr bool DEBUG = false;

int main (int argc, char *argv[]) {
  // lookup tables initialization
  initSliders();
  initBetweenSQ();
  initZobrist();
  initNNUE();

  if constexpr (DEBUG) {
    // Position pos = Position(START_POS);
  } else {
    std::cout << "Fodder "
              << VERSION_NUMBER
              << " by Nek"
              << std::endl;

    uciLoop();
  }
}
