// Mixing whole numbers (A3.5): the step every keyed draw and every spread-out stream of test inputs is made of.
#pragma once

#include <cstdint>

namespace kd::num {

/// SplitMix64's finaliser (Stafford's variant 13): each bit of the input changes about half the bits of the answer,
/// and no two inputs give the same answer. Implements TIM-16, see A3.5.
constexpr std::uint64_t mix64(std::uint64_t z) {
    z = (z ^ (z >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27U)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31U);
}

}  // namespace kd::num
