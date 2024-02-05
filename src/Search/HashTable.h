#pragma once

#include <atomic>
#include <optional>
#include <vector>

#include "../Core/Position.h"

using EvalScore = int16_t;

enum Bound : uint8_t { BoundNone, BoundAlpha, BoundBeta, BoundExact };

struct HashEntry {
  HashKey hashKey;    // The position's hash
  uint16_t bestMove;  // 16-bit version of best move from search
  EvalScore score;    // Search score
  EvalScore eval;     // Static eval
  Bound bound;        // The place where cutoff happens
  uint8_t depth;      // The depth of the search when entry is stored

  inline HashEntry(
    HashKey hashKey,
    uint16_t bestMove,
    EvalScore score,
    EvalScore eval,
    Bound bound,
    uint8_t depth
  ) {
    this->hashKey = hashKey;
    this->bestMove = bestMove;
    this->score = score;
    this->bound = bound;
    this->depth = depth;
  }
};

class HashTable {
  std::vector<HashEntry> data;

public:
  inline void storeHashEntry(
    HashKey hashKey,
    uint16_t bestMove,
    EvalScore score,
    EvalScore eval,
    Bound bound,
    uint8_t depth
  ) {
    HashEntry *prevEntry = &this->data[index(hashKey)];
    // Make sure the new data isn't worse
    if (bound != BoundExact // Not exact PV
    && depth + 3 + bound < prevEntry->depth // From much lower depth
    && hashKey == prevEntry->hashKey // From same position
    ) return; // Don't do anything if every condition is met

    this->data.emplace_back(hashKey, bestMove, score, eval, bound, depth);
  }

  [[nodiscard]] inline std::optional<HashEntry> probeHashEntry(HashKey hash) {
    uint64_t index = this->index(hash);

    // Return hash entry if matches, return nothing otherwise
    return this->data[index].hashKey == hash 
      ? std::optional(this->data[index]) 
      : std::nullopt;
  }

  [[nodiscard]] inline uint64_t index(HashKey hash) {
    return static_cast<uint64_t>(
      (static_cast<__uint128_t>(hash) * 
       static_cast<__uint128_t>(this->data.size())) >> 64
    );
  }

  inline void prefetch(HashKey hash) {
    __builtin_prefetch(&this->data[this->index(hash)]);
  }
};
