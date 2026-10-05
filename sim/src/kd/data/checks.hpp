// The catalogue's checks (MAT-17): checks on the whole catalogue, beyond what the loader checks in each file, which
// each kind registers as it arrives, beside its line in kd/data/kinds.cpp. They run in the cloud on every change,
// through `kindling catalogue check` and tools/check.sh, and live in sim/ so the phone could run them on combinations
// the cloud never saw (A3.6).
#pragma once

#include <span>
#include <string_view>
#include <vector>

#include "kd/data/toml.hpp"

namespace kd::data {

class Catalogue;

/// A check on the whole catalogue: its name, the items it serves, what it holds the catalogue to, and the check, which
/// adds a problem for each fault it finds, at its file, line and column.
struct Check {
    std::string_view name;
    std::string_view serves;
    std::string_view about;
    void (*run)(const Catalogue& cat, std::vector<Problem>& problems);
};

/// Every check, as kd/data/kinds.cpp registers them, in the order of their names.
std::span<const Check> checks();

/// Runs every check on a catalogue that loaded without problems: what they find. Implements MAT-17, see A3.6.
std::vector<Problem> run_checks(const Catalogue& cat);

}  // namespace kd::data
