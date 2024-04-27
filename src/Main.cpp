#include "core/Attacks.h"
#include "core/Position.h"
#include "core/Util.h"
#include "core/Zobrist.h"
#include "nnue/Network.h"
#include "search/History.h"
#include "search/TTable.h"
#include "uci/Commands.h"

#include <atomic>
#include <iostream>
#include <string>
#include <vector>

#ifndef VERSION_NUMBER
char const *VERSION_NUMBER = "DEBUG";
#endif

using namespace pali;

int main(int argc, char *argv[]) {
  initAttackTables();
  initZobrist();
  initNNUE(argv[0]);

  std::cout << "Pali " << VERSION_NUMBER << " by Nek" << std::endl;

  Options Opts;
  Position RootPos(STARTPOS);
  TTable TTable;
  thread_local HTable HTable;
  std::atomic<bool> Stopped = true;
  std::string Input;

  while (true) {
    std::getline(std::cin, Input);
    auto Tokens = tokenize(Input);

    if (Tokens.size() == 0)
      continue;

    std::string Cmd = Tokens[0];
    std::vector<std::string> Params(Tokens.begin() + 1, Tokens.end());

    if (Cmd == "uci")
      command::uci();

    else if (Cmd == "isready")
      command::isready();

    else if (Cmd == "ucinewgame")
      command::ucinewgame(Params, RootPos, Opts);

    else if (Cmd == "position")
      command::position(Params, RootPos);

    else if (Cmd == "setoption")
      command::setoption(Params, Opts, TTable);

    else if (Cmd == "go")
      command::go(Params, RootPos, Opts, Stopped, TTable, HTable);

    else if (Cmd == "stop")
      command::stop(Stopped);

    else if (Cmd == "quit" || Cmd == "exit")
      command::exit();
  }
}
