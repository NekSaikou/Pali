#pragma once

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>

#include "../../Core/BasicTypes.h"

constexpr int INPUT_SIZE = 768;
constexpr int HIDDEN_SIZE = 256;

constexpr int SCALE = 400;
constexpr int QA = 255;
constexpr int QB = 64;
constexpr int QAB = QA * QB;

struct Accumulator {
  int16_t val[HIDDEN_SIZE];

  // Set to input bias
  void reset();
};

struct Network {
  Accumulator inputWeights[INPUT_SIZE];
  Accumulator inputBias;
  Accumulator outputWeights[2];
  int16_t outputBias;
};

extern Network NNUE;

void initNNUE();

int32_t screlu(int16_t x);

std::pair<std::size_t, std::size_t> nnueIx(Color col, Piece pc, Square sq);
