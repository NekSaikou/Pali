#include "UCI.h"

// Input buffer
static std::string input;

// Main search thread
static std::thread mainThread;

void uciLoop() {
  // Initialize searcher object
  Position pos = Position(START_POS);
  std::atomic<bool> stop = false;
  ThreadData td = ThreadData(&pos, &stop);
  HashTable hashTable = HashTable();
  Search searcher = Search(td, &hashTable);

  // UCI options
  Options options;

  searcher.hashTable->init(options.hash);

  // Main loop
  while (true) {
    std::getline(std::cin, input);

    std::vector<std::string> tokens = splitWS(input);

    if (tokens[0] == "uci") command_uci();
    else if (tokens[0] == "quit") break;
    else if (tokens[0] == "exit") break;
    else if (tokens[0] == "isready") command_isready();
    else if (tokens[0] == "ucinewgame") command_ucinewgame(searcher, options);
    else if (tokens[0] == "position") command_position(searcher);
    else if (tokens[0] == "setoption") command_setoption(options);
    else if (tokens[0] == "stop") command_stop(searcher);
    else if (tokens[0] == "go") command_go(searcher, options);
    else std::cerr << "Invalid UCI command \"" << input << "\"" << std::endl;
  }
}

void command_uci() {
  std::cout << "id name Fodder" << std::endl
            << "id author Nek" << std::endl 
            << "uciok" << std::endl;
}

void command_isready() {
  std::cout << "readyok" << std::endl;
}

void command_ucinewgame(Search &searcher, Options &options) {
  *searcher.td.rootPos = Position(START_POS);
  searcher.hashTable->init(options.hash);
}

void command_position(Search &searcher) {
  std::vector<std::string> tokens = splitWS(input);

  if (tokens[1] == "startpos") {
    *searcher.td.rootPos = Position(START_POS);
  }
  else if (tokens[1] == "fen") {
    std::string fen = "";

    int i = 2;
    while (i < tokens.size()) {
      if (tokens[i] == "moves") break;
      fen += tokens[i];
      fen += " ";

      i++;
    }
    *searcher.td.rootPos = Position(fen);
  }
  else std::cerr << "Invalid UCI command \"" << input << "\"" << std::endl;

  int i = 2;
  bool moves = false;
  while (i < tokens.size()) {
    if (tokens[i] == "moves") moves = true;
    if (moves) {
      Square from = squareIx(7 - (tokens[i][1] - '1'), tokens[i][0] - 'a');
      Square to = squareIx(7 - (tokens[i][3] - '1'), tokens[i][2] - 'a');
      Piece promo = NO_PC;
      if (tokens.size() >= 5)
        switch (tokens[i][4]) {
          case 'q': promo = Queen; break;
          case 'n': promo = Knight; break;
          case 'b': promo = Bishop; break;
          case 'r': promo = Rook; break;
        };

      MoveList ml = MoveList();
      searcher.td.rootPos->genLegal<false>(ml);

      for (int j = 0; j < ml.getLength(); j++) {
        Move mv = ml.getMove(j);
        if (mv.getFrom() == from 
        &&  mv.getTo() == to) {
          if (mv.promoType() == promo || promo == NO_PC) {
            // TODO: Make a proper 3 fold detection
            searcher.td.rootPos->makeMove(mv);
          }
        }
      }
    }
    i++;
  }
}

void command_setoption(Options &options) {
  std::vector<std::string> tokens = splitWS(input);
  if (tokens[2] == "Hash") {
    options.hash = std::stoi(tokens[4]);
    std::cout << "Set Hash to " << options.hash << "\n";
  }
  else if (tokens[2] == "Threads") {
    options.threads = std::stoi(tokens[4]);
    std::cout << "Set Threads to " << options.threads << "\n";
  }
}

void command_go(Search &searcher, Options &options) {
  int mtg = 30;
  int depth = 40;
  uint64_t nodes = UINT64_MAX;
  Time wtime = UINT64_MAX;
  Time btime = UINT64_MAX;
  Time winc = 0;
  Time binc = 0;

  std::vector<std::string> tokens = splitWS(input);

  for (int i = 0; i < tokens.size(); i++) {
    if (tokens[i] == "movestogo") mtg = std::stoi(tokens[i + 1]);
    else if (tokens[i] == "wtime") wtime = std::stoi(tokens[i + 1]);
    else if (tokens[i] == "btime") btime = std::stoi(tokens[i + 1]);
    else if (tokens[i] == "winc") winc = std::stoi(tokens[i + 1]);
    else if (tokens[i] == "binc") binc = std::stoi(tokens[i + 1]);
    else if (tokens[i] == "depth") depth = std::stoi(tokens[i + 1]);
    else if (tokens[i] == "nodes") nodes = std::stoi(tokens[i + 1]);
  }

  Time time = searcher.td.rootPos->sideToMove() == White ? wtime : btime;
  Time inc = searcher.td.rootPos->sideToMove() == White ? winc : binc;

  Time allocatedTime = time/mtg + inc + 15;

  searcher.td.info.depth = depth;
  searcher.td.info.nodeslim = nodes;
  searcher.td.info.timelim = allocatedTime;

  // Join main thread if there's a previous one running
  if (mainThread.joinable()) 
      mainThread.join();

  // Spawn main thread
  mainThread = std::thread( [&searcher] { searcher.go<true>(); });
}

void command_stop(Search &searcher) {
  searcher.td.abort();

  if (mainThread.joinable()) 
      mainThread.join();
}
