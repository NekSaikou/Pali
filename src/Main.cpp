#include <iostream>
#include "Core/BasicTypes.h"
#include "Core/Position.h"
#include "Core/Zobrist.h"
#include "Perft.h"

int main (int argc, char *argv[]) {
  // lookup tables initialization
  initSliders();
  initBetweenSQ();
  initZobrist();

  Position pos = Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
  // Position pos = Position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");


  // perft(pos, 7);
}
