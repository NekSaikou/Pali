#pragma once

#include <atomic>
#include <optional>
#include <vector>

#include "../Core/Position.h"

using EvalScore = int16_t;

enum Bound : uint8_t { BoundNone, BoundAlpha, BoundBeta, BoundExact };

struct HashEntry {
  HashKey hashKey = 0;      // The position's hash
  uint16_t bestMove = 0;    // 16-bit version of best move from search
  EvalScore score = 32001;  // Search score
  EvalScore eval = 32001;   // Static eval
  Bound bound = BoundNone;  // The place where cutoff happens
  uint8_t depth = 0;        // The depth of the search when entry is stored

  inline HashEntry() {};

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
    // Make sure the new entry isn't worse
    if (bound != BoundExact // Not exact PV
    && hashKey == prevEntry->hashKey // From same position
    && depth + 3 + 2 * bound < prevEntry->depth // From much lower depth
    ) return; // Don't do anything if every condition is met

    uint64_t ttIx = this->index(hashKey);
    this->data[ttIx].hashKey = hashKey;
    this->data[ttIx].bestMove = bestMove;
    this->data[ttIx].score = score;
    this->data[ttIx].eval = eval;
    this->data[ttIx].bound = bound;
    this->data[ttIx].depth = depth;
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

  inline void clear() {
    std::fill(this->data.begin(), this->data.end(), HashEntry());
  }

  inline void init(uint64_t size) {
    // size in MB
    const uint64_t hashSize = 0x100000 * size;
    const uint64_t numEntries = (hashSize / sizeof(HashEntry)) - 2;
    this->data.resize(numEntries);
    this->clear();
  }
};
