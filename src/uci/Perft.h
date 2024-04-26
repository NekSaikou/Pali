#pragma once

#include "../core/Position.h"

#include <atomic>
#include <cstdint>

namespace fodder {

uint64_t perft(const Position &Pos, int Depth, std::atomic<bool> &Stopped);

}
