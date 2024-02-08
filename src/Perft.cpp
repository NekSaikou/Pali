#include "Perft.h"

uint64_t subperft(Position &pos, int depth) {
  uint64_t nodes = 0;

  if (depth == 0) return 1;

  MoveList ml = MoveList();
  pos.genLegal<false>(ml);

  for (int i = 0; i < ml.getLength(); i++) {
    Move mv = ml.getMove(i);
    Position posCopy = pos;

    posCopy.makeMove(mv);

    nodes += subperft(posCopy, depth - 1);
  }

  return nodes;
}

void perft(Position &pos, int depth) {
  Time startTime = getTimeMs();
  uint64_t nodesTotal = 0;

  MoveList ml = MoveList();
  pos.genLegal<false>(ml);

  for (int i = 0; i < ml.getLength(); i++) {
    Move mv = ml.getMove(i);
    Position posCopy = pos;
    uint64_t currentNodes = nodesTotal;

    posCopy.makeMove(mv);

    nodesTotal += subperft(posCopy, depth - 1);

    // Print out the differences
    std::cout << mv.string() 
              << ": " 
              << (nodesTotal - currentNodes) 
              << std::endl;
  }

  std::cout << "\ntime taken: "
            << getTimeMs() - startTime << " ms"
            << "\nnodes: "
            << nodesTotal 
            << "\nnps: "
            << (uint64_t) ((double) nodesTotal / ((double) getTimeMs() - (double) startTime) * 1000.0 + 0.0001)
            << std::endl;
}
