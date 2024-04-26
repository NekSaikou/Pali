#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>

namespace fodder {

enum class Bound { Upper, Lower, Exact };

// Transposition table entry
struct TTEntry {
  uint64_t Hash = 0;
  uint16_t BestMove = 0;
  int16_t Score = 32001;
  int16_t Eval = 32001;
  uint8_t Other = 0;
  uint8_t Depth = 0;

  TTEntry(){};

  TTEntry(uint64_t Hash, uint16_t BestMove, int16_t Score, int16_t Eval,
          Bound Bound, uint8_t Age, uint8_t Depth)
      : Hash(Hash), BestMove(BestMove), Score(Score),
        Other(static_cast<int>(Bound) | (Age << 2)), Depth(Depth) {}

  Bound bound() const { return static_cast<Bound>(Other & 3); }

  uint8_t age() const { return (Other & 252) >> 2; }
};

class TTable {
  std::vector<TTEntry> Data;
  std::atomic<uint8_t> Age = 0;

public:
  // Default at 16 MB
  TTable() { resize(16); }

  void ageUp() {
    // Has to fit into 6 bits
    if (++Age > 63)
      Age = 0;
  }

  // clang-format off
  [[nodiscard]] uint64_t index(uint64_t Hash) const {
    return static_cast<uint64_t>(
      (static_cast<__uint128_t>(Hash) *
       static_cast<__uint128_t>(Data.size())) >> 64);
  }
  // clang-format on

  void storeEntry(uint64_t Hash, uint16_t BestMove, int16_t Score, int16_t Eval,
                  Bound Bound, uint8_t Depth) {
    TTEntry &PrevEntry = Data[index(Hash)];

    // Make sure the new entry isn't worse
    if (Bound != Bound::Exact &&       // Not exact PV
        Hash == PrevEntry.Hash &&      // From same position
        Depth < PrevEntry.Depth - 4 && // From much lower depth
        Age == PrevEntry.age())        // From same search
      return;

    PrevEntry.Hash = Hash;
    PrevEntry.BestMove = BestMove;
    PrevEntry.Score = Score;
    PrevEntry.Eval = Eval;
    PrevEntry.Other = static_cast<uint8_t>(Bound) | (Age << 2);
    PrevEntry.Depth = Depth;
  }

  /// Probe for hash entry, return nothing if hash doesn't match
  [[nodiscard]] const TTEntry *const probeEntry(uint64_t Hash) const {
    const TTEntry &Entry = Data[index(Hash)];
    return Entry.Hash == Hash ? &Entry : nullptr;
  }

  void prefetch(uint64_t Hash) const { __builtin_prefetch(&Data[index(Hash)]); }

  void clear() { std::fill(Data.begin(), Data.end(), TTEntry()); }

  /// Resize transposition table to size in MB
  void resize(uint64_t Size) {
    const uint64_t HashSize = 0x100000 * Size;
    const uint64_t numEntries = (HashSize / sizeof(TTEntry)) - 2;
    Data.resize(numEntries);
    Age.store(0, std::memory_order_relaxed);
    clear();
  }

  // 0    => Empty hash table
  // 1000 => Full hash table
  [[nodiscard]] int hashfull() {
    int Cnt = 0;

    for (int i = 1; i <= 1000; ++i)
      if (Data[i].Hash != 0)
        ++Cnt;

    return Cnt;
  }
};

} // namespace fodder
