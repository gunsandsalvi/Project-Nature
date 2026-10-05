#include "kd/num/convert.hpp"

#include <cmath>

#include "kd/core/check.hpp"

namespace kd::num {

std::int64_t to_int(double x, Round round) {
    // Rounding to a whole number is exact, so these are the same on every machine.
    double r = 0.0;
    switch (round) {
        case Round::down:
            r = std::floor(x);
            break;
        case Round::up:
            r = std::ceil(x);
            break;
        case Round::toward_zero:
            r = std::trunc(x);
            break;
        case Round::nearest:
            r = std::round(x);
            break;
    }
    // A NaN fails both comparisons, and an infinity one of them.
    KD_CHECK(r >= -0x1p63 && r < 0x1p63, "num::to_int needs a finite number within the 64-bit range");
    return static_cast<std::int64_t>(r);
}

}  // namespace kd::num
