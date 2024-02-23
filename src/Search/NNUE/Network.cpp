#include "Network.h"

#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_CAMEL
#include "../../../incbin/incbin.h"

#ifdef RELEASE_BUILD
INCBIN(Eval, "../net/bee.net");
#else
INCBIN(Eval, "./net/bee.net");
#endif

// This translation unit now has three symbols
// const unsigned char EvalData[];
// const unsigned char *const EvalEnd;
// const unsigned int EvalSize;

Network NNUE;

void initNNUE() {
  std::memcpy(NNUE.inputWeights, &EvalData, sizeof(Network));
}

void Accumulator::reset() {
  std::memcpy(this->val, NNUE.inputBias.val, sizeof(Accumulator));
}

int32_t screlu(int16_t x) {
  constexpr int16_t CR_MIN = 0;
  constexpr int16_t CR_MAX = 255;

  x = std::clamp(x, CR_MIN, CR_MAX);
  return static_cast<int32_t>(x) * static_cast<int32_t>(x);
}

std::pair<std::size_t, std::size_t> nnueIx(Color col, Piece pc, Square sq) {
    constexpr std::size_t COLOR_STRIDE = 64 * 6;
    constexpr std::size_t PIECE_STRIDE = 64;

    std::size_t whiteIx = col * COLOR_STRIDE + pc * PIECE_STRIDE + (sq ^ 0b111'000);
    std::size_t blackIx = (col ^ 1) * COLOR_STRIDE + pc * PIECE_STRIDE + sq;

    return {whiteIx, blackIx};
}
