#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace fodder {

/// Stack-allocated vector with fixed capacity
template <typename T, size_t CAP> class ArrayVec {
  std::array<T, CAP> Data;
  size_t Length = 0;

public:
  [[nodiscard]] T &operator[](size_t Idx) { return Data[Idx]; }

  [[nodiscard]] T *begin() { return Data.begin(); }

  [[nodiscard]] T *end() { return Data.begin() + Length; }

  [[nodiscard]] int size() const { return Length; }

  void push_back(T Item) { Data[Length++] = Item; }

  void pop_back() { --Length; }

  void clear() { Length = 0; }

  void swap(int Idx1, int Idx2) { std::swap(Data[Idx1], Data[Idx2]); };

  [[nodiscard]] T takeLast() { return Data[--Length]; };
};

/// Split string at whitespaces
inline std::vector<std::string> tokenize(const std::string &Str) {
  std::stringstream Stream(Str);
  std::vector<std::string> Tokens;
  std::copy(std::istream_iterator<std::string>(Stream),
            std::istream_iterator<std::string>(), std::back_inserter(Tokens));
  return Tokens;
}

inline uint64_t getTimeMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

} // namespace fodder
