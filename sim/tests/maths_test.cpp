#include <limits>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/num/maths.hpp"

namespace num = kd::num;

// The answers that are exact, which a correctly rounded function must give on every build; tests/oracle.cpp holds
// every function to MPFR's answers in the cloud.
// checks: RES-05
TEST_CASE("the maths functions give the exact answers where there are some") {
    CHECK(num::sqrt(2.25) == 1.5);
    CHECK(num::cbrt(-27.0) == -3.0);
    CHECK(num::exp(0.0) == 1.0);
    CHECK(num::exp2(-3.0) == 0.125);
    CHECK(num::expm1(0.0) == 0.0);
    CHECK(num::log(1.0) == 0.0);
    CHECK(num::log2(1024.0) == 10.0);
    CHECK(num::log1p(0.0) == 0.0);
    CHECK(num::pow(2.0, 10.0) == 1024.0);
    CHECK(num::pow(-2.0, 3.0) == -8.0);
    CHECK(num::pow(0.0, 0.0) == 1.0);
    CHECK(num::tanh(0.0) == 0.0);
    CHECK(num::erf(0.0) == 0.0);
    CHECK(num::hypot(3.0, 4.0) == 5.0);
    // turns are exact where radians are not: a quarter turn's cosine is 0, a half turn's sine is 0
    CHECK(num::sinpi(0.5) == 1.0);
    CHECK(num::cospi(0.5) == 0.0);
    CHECK(num::sinpi(1.0) == 0.0);
    CHECK(num::cospi(1.0) == -1.0);
    CHECK(num::sinpi(1.0 / 6.0) == 0.5);
    CHECK(num::tanpi(0.25) == 1.0);
    CHECK(num::asinpi(1.0) == 0.5);
    CHECK(num::acospi(-1.0) == 1.0);
    CHECK(num::acospi(0.5) == 1.0 / 3.0);
    CHECK(num::atanpi(1.0) == 0.25);
    CHECK(num::atan2pi(1.0, 0.0) == 0.5);
    CHECK(num::atan2pi(0.0, -1.0) == 1.0);
    CHECK(num::atan2pi(-1.0, -1.0) == -0.75);
}

// checks: RES-05
TEST_CASE("each maths function refuses what lies outside its domain, so no NaN or infinity enters") {
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double big = std::numeric_limits<double>::max();
    CHECK(kd::test::stops([] { num::sqrt(-1.0); }));
    CHECK(kd::test::stops([] { num::log(0.0); }));
    CHECK(kd::test::stops([] { num::log(-1.0); }));
    CHECK(kd::test::stops([] { num::log2(-0.0); }));
    CHECK(kd::test::stops([] { num::log1p(-1.0); }));
    CHECK(kd::test::stops([] { num::exp(710.0); }));
    CHECK(kd::test::stops([] { num::exp2(1024.0); }));
    CHECK(kd::test::stops([] { num::expm1(710.0); }));
    CHECK(kd::test::stops([] { num::pow(-2.0, 0.5); }));
    CHECK(kd::test::stops([] { num::pow(0.0, -1.0); }));
    CHECK(kd::test::stops([] { num::pow(10.0, 400.0); }));
    CHECK(kd::test::stops([=] { num::hypot(big, big); }));
    CHECK(kd::test::stops([] { num::tanpi(0.5); }));
    CHECK(kd::test::stops([] { num::asinpi(1.5); }));
    CHECK(kd::test::stops([] { num::acospi(-1.0000001); }));
    CHECK(kd::test::stops([] { num::atan2pi(0.0, 0.0); }));
    CHECK(kd::test::stops([] { num::atan2pi(-0.0, -0.0); }));
    CHECK(kd::test::stops([=] { num::sinpi(nan); }));
    CHECK(kd::test::stops([=] { num::cbrt(inf); }));
    CHECK(kd::test::stops([=] { num::tanh(-inf); }));
    CHECK(kd::test::stops([=] { num::erf(nan); }));
    CHECK(kd::test::stops([=] { num::atanpi(inf); }));
    // and the edges that are inside are answered
    CHECK_FALSE(kd::test::stops([] { num::sqrt(-0.0); }));
    CHECK_FALSE(kd::test::stops([] { num::exp(-1000.0); }));
    CHECK_FALSE(kd::test::stops([] { num::asinpi(-1.0); }));
}
