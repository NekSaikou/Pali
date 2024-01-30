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

  if (DEBUG) {
    Position pos = Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    pos.makeMove(Move(e2, e4, DoublePush, Pawn));
    pos.makeMove(Move(e7, e5, DoublePush, Pawn));

    Position pos2 = Position("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");

    assert(pos2.getColorBB(White) == pos.getColorBB(White));
    assert(pos2.getColorBB(Black) == pos.getColorBB(Black));
    std::cout << pos.getHash() << std::endl;
    std::cout << pos2.getHash() << std::endl;
    // assert(pos2.getHash() == pos.getHash());
    
    std::atomic<bool> stop = false;
    Search searcher = Search(ThreadData(&pos, &stop));
    std::cout << (int) pos.evaluate() << std::endl;
    // searcher.go<true>();
  } else {
    uciLoop();
  }
}
