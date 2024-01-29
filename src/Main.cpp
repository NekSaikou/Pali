#include <iostream>
#include "Core/Position.h"
#include "Perft.h"

int main (int argc, char *argv[]) {
  // lookup tables initialization
  initSliders();
  initBetweenSQ();

  // Position pos = Position(START_POS);
  Position pos = Position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

  perft(pos, 7);
}
