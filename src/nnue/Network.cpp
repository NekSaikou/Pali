#include "Network.h"

#include <fstream>
#include <ios>
#include <string>

using namespace fodder;

Network fodder::NNUE;

void Accumulator::reset() { Data = NNUE.InputBias.Data; }

void fodder::initNNUE(std::string Path) {
  std::ifstream(Path.substr(0, Path.find_last_of("/")) +
                    "/../resources/jigglypuff.nnue",
                std::ios::binary)
      .read(reinterpret_cast<char *>(&NNUE), sizeof(Network));
}
