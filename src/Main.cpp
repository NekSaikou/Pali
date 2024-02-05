#include "UCI.h"

constexpr bool DEBUG = false;

int main (int argc, char *argv[]) {
  // lookup tables initialization
  initSliders();
  initBetweenSQ();
  initZobrist();
  initNNUE();

  if constexpr (DEBUG) {
    std::cout << sizeof(HashEntry) << std::endl;
  } else {
    std::cout << "Fodder "
              << "by Nek"
              << std::endl;

    uciLoop();
  }
}
