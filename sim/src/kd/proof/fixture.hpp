// The proof suites' own catalogue (A3.4): the demonstration's sources as they stood when the world suite was written,
// fixed here so the suite's digest moves only when the code does, never when data/demo/ is tuned.
#pragma once

#include <vector>

#include "kd/data/catalogue.hpp"

namespace kd::proof {

/// The fixed sources: base, and demo with its two markers and its crowd.
std::vector<data::SourceFile> fixture_files();

}  // namespace kd::proof
