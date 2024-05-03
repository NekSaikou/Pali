#include "LogTable.h"

#include <array>
#include <cmath>

std::array<double, 128> LogTable = {};

void pali::initLogTable() {
  for (int i = 1; i < 128; ++i)
    LogTable[i] = std::log(i);
}

double pali::ln(int i) { return LogTable[i]; }
