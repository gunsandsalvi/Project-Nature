// The one way from a floating number to a whole one (A3.4). A plain cast is undefined outside the whole number's
// range and differs between chips there (x86-64 gives the smallest number, arm64 the nearest; research 18), so sim/
// converts only here.
#pragma once

#include <cstdint>

namespace kd::num {

/// How a floating number rounds to a whole one.
enum class Round : std::uint8_t {
    down,         // toward minus infinity
    up,           // toward plus infinity
    toward_zero,  // dropping the fraction
    nearest,      // to the nearest, halves away from zero
};

/// x rounded to a whole number; refuses a number that is not finite or whose rounded value lies outside the 64-bit
/// range. Implements RES-05, see A3.4.
std::int64_t to_int(double x, Round round);

}  // namespace kd::num
