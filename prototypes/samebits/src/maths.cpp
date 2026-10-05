// P5's own maths (A3.4): each function brings its argument into a small range by exact steps (floor, frexp, ldexp,
// and a constant split in two so its product with a whole number is exact), then sums a fixed series by Horner's rule.
// Built with no contraction into fused multiply-adds (-ffp-contract=off), every step is one IEEE operation, so the
// result is the same on x86-64 and arm64. Pre-production code (research 00).
#include "maths.hpp"

#include <cmath>
#include <initializer_list>

namespace samebits {

namespace {

// pi/2 in two parts, the first with its low bits zero, so that n times it is exact for |n| below 2^20 (fdlibm's)
constexpr double kHalfPiHigh = 1.57079632673412561417e+00;
constexpr double kHalfPiLow = 6.07710050650619224932e-11;
constexpr double kTwoOverPi = 6.36619772367581382433e-01;
// ln 2 in two parts, likewise (fdlibm's)
constexpr double kLn2High = 6.93147180369123816490e-01;
constexpr double kLn2Low = 1.90821492927058770002e-10;
constexpr double kLog2E = 1.44269504088896338700e+00;
constexpr double kSqrtHalf = 7.07106781186547524401e-01;

// sin r for |r| <= pi/4: the series to r^13
double sine_series(double r) {
    const double r2 = r * r;
    double p = 1.0 / 6227020800.0;
    p = p * r2 - 1.0 / 39916800.0;
    p = p * r2 + 1.0 / 362880.0;
    p = p * r2 - 1.0 / 5040.0;
    p = p * r2 + 1.0 / 120.0;
    p = p * r2 - 1.0 / 6.0;
    return r + r * r2 * p;
}

// cos r for |r| <= pi/4: the series to r^14
double cosine_series(double r) {
    const double r2 = r * r;
    double p = -1.0 / 87178291200.0;
    p = p * r2 + 1.0 / 479001600.0;
    p = p * r2 - 1.0 / 3628800.0;
    p = p * r2 + 1.0 / 40320.0;
    p = p * r2 - 1.0 / 720.0;
    p = p * r2 + 1.0 / 24.0;
    p = p * r2 - 0.5;
    return 1.0 + r2 * p;
}

// x less the nearest whole multiple of pi/2, and that multiple's quarter turn (0 to 3)
double quarter(double x, int* turn) {
    const double n = std::floor(x * kTwoOverPi + 0.5);
    *turn = static_cast<int>(static_cast<long long>(n) & 3);
    return (x - n * kHalfPiHigh) - n * kHalfPiLow;
}

}  // namespace

double sine(double x) {
    int turn = 0;
    const double r = quarter(x, &turn);
    switch (turn) {
        case 0:
            return sine_series(r);
        case 1:
            return cosine_series(r);
        case 2:
            return -sine_series(r);
        default:
            return -cosine_series(r);
    }
}

double cosine(double x) {
    int turn = 0;
    const double r = quarter(x, &turn);
    switch (turn) {
        case 0:
            return cosine_series(r);
        case 1:
            return -sine_series(r);
        case 2:
            return -cosine_series(r);
        default:
            return sine_series(r);
    }
}

double exponent(double x) {
    if (x > 700.0) {
        x = 700.0;
    } else if (x < -700.0) {
        x = -700.0;
    }
    const double n = std::floor(x * kLog2E + 0.5);
    const double r = (x - n * kLn2High) - n * kLn2Low;
    // e^r for |r| <= ln2 / 2: the series to r^13
    double p = 1.0 / 6227020800.0;
    for (const double k :
         {479001600.0, 39916800.0, 3628800.0, 362880.0, 40320.0, 5040.0, 720.0, 120.0, 24.0, 6.0, 2.0, 1.0, 1.0}) {
        p = p * r + 1.0 / k;
    }
    return std::ldexp(p, static_cast<int>(n));
}

double logarithm(double x) {
    int e = 0;
    double m = std::frexp(x, &e);
    if (m < kSqrtHalf) {
        m *= 2.0;
        e -= 1;
    }
    // log m = 2 atanh s, s = (m - 1) / (m + 1), |s| < 0.18: the series to s^19
    const double s = (m - 1.0) / (m + 1.0);
    const double s2 = s * s;
    double p = 1.0 / 19.0;
    for (const double k : {17.0, 15.0, 13.0, 11.0, 9.0, 7.0, 5.0, 3.0, 1.0}) {
        p = p * s2 + 1.0 / k;
    }
    const double whole = static_cast<double>(e);
    return whole * kLn2High + (2.0 * s * p + whole * kLn2Low);
}

double power(double a, double b) {
    return exponent(b * logarithm(a));
}

}  // namespace samebits
