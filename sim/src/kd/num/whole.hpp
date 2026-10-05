// Whole-number division that rounds down (A3.4): C++ rounds toward zero, which puts a moment before history, or a
// place west of the map's edge, on the wrong side of a boundary.
#pragma once

#include <cstdint>

namespace kd::num {

/// The largest whole number at most n / d, for d above 0. Implements RES-05, see A3.4.
constexpr std::int64_t floor_div(std::int64_t n, std::int64_t d) {
    const std::int64_t q = n / d;
    return (n % d != 0 && n < 0) ? q - 1 : q;
}

/// What is left of n after floor_div, from 0 up to d, for d above 0. Implements RES-05, see A3.4.
constexpr std::int64_t floor_mod(std::int64_t n, std::int64_t d) {
    const std::int64_t r = n % d;
    return r < 0 ? r + d : r;
}

}  // namespace kd::num
