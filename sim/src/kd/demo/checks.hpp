// The demonstration's own checks (MAT-17), registered with its kinds in kd/data/kinds.cpp.
#pragma once

#include <vector>

#include "kd/data/toml.hpp"

namespace kd::data {
class Catalogue;
}

namespace kd::demo {

/// Implements MAT-17 for the demonstration's markers: every marker can greet or be greeted, since it names some
/// marker in its walks_with or another names it, so none walks the crowd alone.
void check_company(const data::Catalogue& cat, std::vector<data::Problem>& problems);

}  // namespace kd::demo
