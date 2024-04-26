#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

namespace fodder {

using NNUEIndices = std::pair<std::size_t, std::size_t>;

constexpr int INPUT_SIZE = 768;
constexpr int HIDDEN_SIZE = 256;

constexpr int SCALE = 400;
constexpr int QA = 255;
constexpr int QB = 64;
constexpr int QAB = QA * QB;

struct Accumulator {
  std::array<int16_t, HIDDEN_SIZE> Data;

  /// Set to input bias
  void reset();
};

struct Network {
  std::array<Accumulator, INPUT_SIZE> InputWeights;
  Accumulator InputBias;
  std::array<Accumulator, 2> OutputWeights;
  int16_t OutputBias;
};

extern Network NNUE;

void initNNUE(std::string Path);

inline std::pair<std::size_t, std::size_t> nnueIdx(int Col, int Pc, int Sq) {
  constexpr std::size_t COLOR_STRIDE = 64 * 6;
  constexpr std::size_t PIECE_STRIDE = 64;

  auto WhiteIdx = Col * COLOR_STRIDE + Pc * PIECE_STRIDE + (Sq ^ 0b111'000);
  auto BlackIdx = (Col ^ 1) * COLOR_STRIDE + Pc * PIECE_STRIDE + Sq;

  return {WhiteIdx, BlackIdx};
}

} // namespace fodder
