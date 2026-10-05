// Sorting that no tie can decide (A3.2, A3.4): std::sort leaves equal elements in a different order in the cloud's
// library and the phone's, so the simulation sorts only by an order under which no two elements are equal, such as
// one that breaks ties by id, and checks it.
#pragma once

#include <algorithm>
#include <iterator>

#include "kd/core/check.hpp"

namespace kd::num {

/// Sorts a range by an order that leaves no ties, and stops the run if two elements are tied. Implements RES-05,
/// see A3.4.
template <std::random_access_iterator It, typename Less>
void sort_strict(It first, It last, Less less) {
    std::sort(first, last, less);
    for (It i = first; i != last && std::next(i) != last; ++i) {
        KD_CHECK(less(*i, *std::next(i)), "num::sort_strict: two elements are tied; break ties by id");
    }
}

}  // namespace kd::num
