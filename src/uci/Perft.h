#pragma once

#include "core/Position.h"

#include <atomic>
#include <cstdint>

namespace pali {

uint64_t perft(const Position &Pos, int Depth, std::atomic<bool> &Stopped);

}
