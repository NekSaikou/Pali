#include "UCI.h"

#ifndef VERSION_NUMBER 
std::string VERSION_NUMBER = "DEBUG";
#endif

int main (int argc, char *argv[]) {
  // lookup tables initialization
  initSliders();
  initBetweenSQ();
  initZobrist();
  initNNUE();

  std::cout << "Fodder "
            << VERSION_NUMBER
            << " by Nek"
            << std::endl;

  uciLoop();
}
