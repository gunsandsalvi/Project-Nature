#include "kd/proof/maths_cases.hpp"

#include <bit>
#include <cmath>
#include <limits>

#include "kd/core/check.hpp"
#include "kd/num/maths.hpp"

namespace kd::proof {

namespace {

// The numbers one input is made from: up to 256 draws of its own, so inputs never share draws.
struct Stream {
    const chance::Draws& draws;
    std::uint64_t at;
    std::uint64_t next() { return draws.bits(at++); }
};

constexpr double kMax = std::numeric_limits<double>::max();

// A number from 0 up to 1, from 53 bits.
double unit(std::uint64_t u) {
    return static_cast<double>(u >> 11U) * 0x1p-53;
}

// A number from lo to hi, spread evenly by value.
double by_value(Stream& s, double lo, double hi) {
    return lo + (hi - lo) * unit(s.next());
}

// A number from 0 to hi, spread evenly over its bits, so each power of two, the subnormals' included, is as likely.
double by_bits(Stream& s, double hi) {
    return std::bit_cast<double>(s.next() % (std::bit_cast<std::uint64_t>(hi) + 1));
}

// The same, never 0.
double positive_bits(Stream& s, double hi) {
    return std::bit_cast<double>(1 + s.next() % std::bit_cast<std::uint64_t>(hi));
}

// Half by bits, with either sign, and half by value.
double either(Stream& s, double bits_hi, double lo, double hi) {
    if ((s.next() & 1U) == 0) {
        return by_value(s, lo, hi);
    }
    const double x = by_bits(s, bits_hi);
    return (s.next() & 1U) != 0 ? -x : x;
}

Args sqrt_inputs(Stream& s) {
    return {(s.next() & 1U) != 0 ? by_bits(s, kMax) : by_value(s, 0.0, 1e6), 0.0};
}

Args any_inputs(Stream& s) {
    return {either(s, kMax, -1e6, 1e6), 0.0};
}

Args exp_inputs(Stream& s) {
    return {either(s, 709.7, -746.0, 709.78), 0.0};
}

Args exp2_inputs(Stream& s) {
    return {either(s, 1023.9, -1076.0, 1023.99), 0.0};
}

Args log_inputs(Stream& s) {
    return {(s.next() & 1U) != 0 ? positive_bits(s, kMax) : 100.0 - by_value(s, 0.0, 100.0), 0.0};
}

Args log1p_inputs(Stream& s) {
    switch (s.next() % 3) {
        case 0:
            return {by_bits(s, kMax), 0.0};
        case 1:
            return {-by_bits(s, 0x1.fffffffffffffp-1), 0.0};
        default:
            return {100.0 - by_value(s, 0.0, 101.0), 0.0};
    }
}

// A positive x by its bits, and a y that brings the answer anywhere from the smallest number to 2^1020; or a
// negative x to a whole y.
Args pow_inputs(Stream& s) {
    if (s.next() % 5 == 0) {
        return {-by_value(s, 0.25, 4.0), static_cast<double>(static_cast<std::int64_t>(s.next() % 121) - 60)};
    }
    const double x = positive_bits(s, kMax);
    const double target = by_value(s, -1074.0, 1020.0);
    const double l = num::log2(x);
    return {x, l != 0.0 ? target / l : by_value(s, -100.0, 100.0)};
}

Args tanh_inputs(Stream& s) {
    return {either(s, kMax, -20.0, 20.0), 0.0};
}

Args erf_inputs(Stream& s) {
    return {either(s, kMax, -6.0, 6.0), 0.0};
}

Args hypot_inputs(Stream& s) {
    const double x = either(s, kMax / 2, -1e6, 1e6);
    return {x, either(s, kMax / 2, -1e6, 1e6)};
}

Args turn_inputs(Stream& s) {
    return {either(s, kMax, -4.0, 4.0), 0.0};
}

// Anything but a whole number and a half, where the tangent has no value.
Args tanpi_inputs(Stream& s) {
    double x = 0.5;
    while (x - std::floor(x) == 0.5) {
        x = either(s, 0x1p51, -4.0, 4.0);
    }
    return {x, 0.0};
}

Args unit_inputs(Stream& s) {
    return {either(s, 1.0, -1.0, 1.0), 0.0};
}

Args atanpi_inputs(Stream& s) {
    return {either(s, kMax, -100.0, 100.0), 0.0};
}

// Any vector but zero.
Args atan2pi_inputs(Stream& s) {
    Args a{0.0, 0.0};
    while (a[0] == 0.0 && a[1] == 0.0) {
        const double y = either(s, kMax, -1e3, 1e3);
        a = {y, either(s, kMax, -1e3, 1e3)};
    }
    return a;
}

template <Args (*Inputs)(Stream&)>
Args draw(const chance::Draws& draws, std::uint64_t index) {
    Stream s{draws, index << 8U};
    return Inputs(s);
}

const std::array kFunctions = {
    MathsFunction{"sqrt", 1, [](Args a) { return num::sqrt(a[0]); }, &draw<sqrt_inputs>},
    MathsFunction{"cbrt", 1, [](Args a) { return num::cbrt(a[0]); }, &draw<any_inputs>},
    MathsFunction{"exp", 1, [](Args a) { return num::exp(a[0]); }, &draw<exp_inputs>},
    MathsFunction{"exp2", 1, [](Args a) { return num::exp2(a[0]); }, &draw<exp2_inputs>},
    MathsFunction{"expm1", 1, [](Args a) { return num::expm1(a[0]); }, &draw<exp_inputs>},
    MathsFunction{"log", 1, [](Args a) { return num::log(a[0]); }, &draw<log_inputs>},
    MathsFunction{"log2", 1, [](Args a) { return num::log2(a[0]); }, &draw<log_inputs>},
    MathsFunction{"log1p", 1, [](Args a) { return num::log1p(a[0]); }, &draw<log1p_inputs>},
    MathsFunction{"pow", 2, [](Args a) { return num::pow(a[0], a[1]); }, &draw<pow_inputs>},
    MathsFunction{"tanh", 1, [](Args a) { return num::tanh(a[0]); }, &draw<tanh_inputs>},
    MathsFunction{"erf", 1, [](Args a) { return num::erf(a[0]); }, &draw<erf_inputs>},
    MathsFunction{"hypot", 2, [](Args a) { return num::hypot(a[0], a[1]); }, &draw<hypot_inputs>},
    MathsFunction{"sinpi", 1, [](Args a) { return num::sinpi(a[0]); }, &draw<turn_inputs>},
    MathsFunction{"cospi", 1, [](Args a) { return num::cospi(a[0]); }, &draw<turn_inputs>},
    MathsFunction{"tanpi", 1, [](Args a) { return num::tanpi(a[0]); }, &draw<tanpi_inputs>},
    MathsFunction{"asinpi", 1, [](Args a) { return num::asinpi(a[0]); }, &draw<unit_inputs>},
    MathsFunction{"acospi", 1, [](Args a) { return num::acospi(a[0]); }, &draw<unit_inputs>},
    MathsFunction{"atanpi", 1, [](Args a) { return num::atanpi(a[0]); }, &draw<atanpi_inputs>},
    MathsFunction{"atan2pi", 2, [](Args a) { return num::atan2pi(a[0], a[1]); }, &draw<atan2pi_inputs>},
};

#define KD_HARD(fn, x, y) HardCase{#fn, {x, y}},
const HardCase kHard[] = {
#include "hard-cases.inc"
};
#undef KD_HARD

}  // namespace

std::span<const MathsFunction> maths_functions() {
    return kFunctions;
}

const MathsFunction& maths_function(std::string_view name) {
    for (const MathsFunction& f : kFunctions) {
        if (f.name == name) {
            return f;
        }
    }
    kd::fail(__FILE__, __LINE__, "proof::maths_function: no maths function of that name");
}

std::span<const HardCase> hard_cases() {
    return kHard;
}

chance::Draws maths_draws(const MathsFunction& f) {
    return {1, chance::name("maths"), 0, 0, chance::name(f.name)};
}

}  // namespace kd::proof
