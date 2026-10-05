// The proof suites (A3.4, A17): seeded computations whose digests must be the same on every build, on any number of
// threads, and on your phone. The cloud writes each suite's digest into the build, and the phone's self-check runs
// the same suites and compares (A2.3). Each later step adds a suite for what it builds.
#pragma once

#include <span>
#include <string>
#include <string_view>

#include "kd/run/workers.hpp"

namespace kd::proof {

struct Suite {
    std::string_view name;
    std::string_view about;
    std::string (*run)(run::Workers& workers);
};

/// Every suite, in the order the self-check shows them.
std::span<const Suite> suites();

/// The digest of one suite by name, or an empty string if there is no such suite.
/// Implements RES-05, see A3.4.
std::string run(std::string_view name, run::Workers& workers);

}  // namespace kd::proof
