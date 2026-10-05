#include "kd/num/angle.hpp"

#include <cmath>

#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"

namespace kd::num {

namespace {

// An angle's half turns, from 0 to just under 2: steps times 2^-31, exact.
double half_turns(Angle a) {
    return static_cast<double>(a.steps) * 0x1p-31;
}

// Steps from half turns: any whole number of steps wraps onto the turn.
Angle of_half_turns(double h) {
    return {static_cast<std::uint32_t>(to_int(h * 0x1p31, Round::nearest))};
}

}  // namespace

Angle angle_of_turns(double turns) {
    // The whole turns dropped first, exactly, so the steps stay within range however many turns there are.
    return of_half_turns(2.0 * (turns - std::floor(turns)));
}

Angle direction(Offset way) {
    return of_half_turns(atan2pi(static_cast<double>(way.dy), static_cast<double>(way.dx)));
}

double sin(Angle a) {
    return sinpi(half_turns(a));
}

double cos(Angle a) {
    return cospi(half_turns(a));
}

}  // namespace kd::num
