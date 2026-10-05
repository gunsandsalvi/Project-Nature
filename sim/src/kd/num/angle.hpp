// Angles (A3.4): a fraction of a turn in 2^32 steps, which wraps by itself and adds exactly, and which the
// trigonometric functions read exactly as the half turns of CORE-MATH's sinpi and cospi.
#pragma once

#include <cstdint>

#include "kd/num/torus.hpp"

namespace kd::num {

/// An angle in steps of 1/2^32 of a turn, counterclockwise from east: a quarter turn is 2^30 steps, and one step
/// about a third of a thousandth of an arc second. Implements RES-05, see A3.4.
struct Angle {
    std::uint32_t steps = 0;

    friend constexpr bool operator==(Angle, Angle) = default;
    friend constexpr Angle operator+(Angle a, Angle b) { return {a.steps + b.steps}; }
    friend constexpr Angle operator-(Angle a, Angle b) { return {a.steps - b.steps}; }

    /// The angle in turns, from 0 to just under 1, exactly.
    [[nodiscard]] constexpr double turns() const { return static_cast<double>(steps) * 0x1p-32; }

    /// The turn from this angle to another, the short way, in steps from minus half a turn to just under half.
    [[nodiscard]] constexpr std::int32_t to(Angle other) const {
        return static_cast<std::int32_t>(other.steps - steps);
    }
};

/// The angle of a number of turns, any finite number, rounded to the nearest step. Implements RES-05, see A3.4.
Angle angle_of_turns(double turns);

/// The direction of a way that is not zero, counterclockwise from east. Implements RES-05, see A3.4.
Angle direction(Offset way);

/// The sine and cosine of an angle, exact at every quarter turn. Implements RES-05, see A3.4.
double sin(Angle a);
double cos(Angle a);

}  // namespace kd::num
