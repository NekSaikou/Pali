#pragma once

#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <chrono>

#include "Core/BasicTypes.h"

Time inline getTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>
          (std::chrono::steady_clock::now().time_since_epoch()).count();
}

constexpr char const *whitespace = " \t\n";

// remove leading/trailing whitespace
std::string trim(const std::string &str);

// split whitespace
std::vector<std::string> splitWS(const std::string &str);
