// P5's tests (IMPLEMENTATION α0.4a): our maths against the platform's, keyed chance, and the toy world's hashes the
// same on one thread and on four, and the same as the cloud's recorded run. Pre-production code (research 00).
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <cmath>

#include "chance.hpp"
#include "expected.hpp"
#include "maths.hpp"
#include "world.hpp"

using samebits::chance;
using samebits::Purpose;

// checks: TIM-16
TEST_CASE("our maths agree with the platform's to within a few last bits") {
    for (int i = -400; i <= 400; ++i) {
        const double x = i * 0.0173;
        CHECK(std::abs(samebits::sine(x) - std::sin(x)) < 1e-13);
        CHECK(std::abs(samebits::cosine(x) - std::cos(x)) < 1e-13);
        CHECK(std::abs(samebits::exponent(x) - std::exp(x)) < 1e-13 * std::exp(x));
        const double y = 0.001 + (i + 400) * 0.37;
        CHECK(std::abs(samebits::logarithm(y) - std::log(y)) < 1e-13 * (1.0 + std::abs(std::log(y))));
        CHECK(std::abs(samebits::power(y, 0.999) - std::pow(y, 0.999)) < 1e-12 * std::pow(y, 0.999));
    }
}

// checks: TIM-16
TEST_CASE("keyed chance gives a key its number whatever was drawn before") {
    const double first = chance(7, 3, 100, Purpose::kTurn);
    for (int i = 0; i < 50; ++i) {
        (void)chance(7, static_cast<std::uint64_t>(i), 100, Purpose::kEat);
    }
    CHECK(chance(7, 3, 100, Purpose::kTurn) == first);
    CHECK(chance(7, 3, 100, Purpose::kWait) != first);
    CHECK(chance(7, 3, 101, Purpose::kTurn) != first);
    CHECK(first >= 0.0);
    CHECK(first < 1.0);
}

// checks: RES-05, TIM-16
TEST_CASE("one thread and four end every day with the same state") {
    samebits::Settings one;
    samebits::Settings four;
    four.threads = 4;
    const auto a = samebits::run(one, 10);
    const auto b = samebits::run(four, 10);
    REQUIRE(a.size() == 10);
    CHECK(a == b);
    // and the days differ from each other: the world moves
    CHECK(a[0] != a[9]);
}

// checks: RES-05
TEST_CASE("the run matches the cloud's recorded one") {
    samebits::Settings settings;
    CHECK(samebits::digest(samebits::run(settings, samebits::kDays)) == samebits::kCloudDigest);
}
