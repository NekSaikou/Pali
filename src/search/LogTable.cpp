#include "LogTable.h"

#include <array>
#include <cmath>

std::array<double, 128> LogTable = {};

void pali::initLogTable() {
  for (int i = 0; i < 128; ++i)
    const_cast<double&>(LogTable[i]) = std::log(i);
}

double pali::ln(int i) { return LogTable[i]; }
