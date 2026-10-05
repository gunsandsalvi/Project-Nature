#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/num/angle.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/mix.hpp"
#include "kd/num/probability.hpp"
#include "kd/num/sort.hpp"
#include "kd/num/torus.hpp"

namespace num = kd::num;

namespace {

// The world of WLD-03: 2,000 km around and 1,000 km from pole to pole, in centimetres.
constexpr num::Torus kWorld(200'000'000, 100'000'000);

// Places on the world: its corners, edges and middle lines, then a seeded spread.
std::vector<num::Point> places() {
    std::vector<num::Point> out;
    const std::int32_t w = kWorld.width();
    const std::int32_t h = kWorld.height();
    for (std::int32_t x : {0, 1, w / 2 - 1, w / 2, w / 2 + 1, w - 1}) {
        for (std::int32_t y : {0, 1, h / 2 - 1, h / 2, h / 2 + 1, h - 1}) {
            out.push_back({x, y});
        }
    }
    for (std::uint64_t i = 1; i <= 200; ++i) {
        const std::uint64_t r = num::mix64(i);
        out.push_back({static_cast<std::int32_t>((r >> 32U) % static_cast<std::uint64_t>(w)),
                       static_cast<std::int32_t>((r & 0xFFFFFFFFU) % static_cast<std::uint64_t>(h))});
    }
    return out;
}

}  // namespace

// checks: WLD-01 RES-05
TEST_CASE("on the torus a way is the reverse of the way back, never longer than half the world, and arrives") {
    const std::vector<num::Point> all = places();
    for (const num::Point& a : all) {
        for (const num::Point& b : all) {
            const num::Offset there = kWorld.offset(a, b);
            const num::Offset back = kWorld.offset(b, a);
            REQUIRE(there.dx == -back.dx);
            REQUIRE(there.dy == -back.dy);
            REQUIRE(2 * there.dx <= kWorld.width());
            REQUIRE(2 * there.dx >= -kWorld.width());
            REQUIRE(2 * there.dy <= kWorld.height());
            REQUIRE(2 * there.dy >= -kWorld.height());
            REQUIRE(kWorld.moved(a, there) == b);
            REQUIRE(kWorld.squared_distance(a, b) == kWorld.squared_distance(b, a));
        }
    }
    // the shortest way crosses the edge where the map wraps
    CHECK(kWorld.offset({1, 5}, {kWorld.width() - 1, 5}) == num::Offset{-2, 0});
    CHECK(kWorld.offset({5, kWorld.height() - 3}, {5, 2}) == num::Offset{0, 5});
    // and exactly half way round, it does not
    CHECK(kWorld.offset({0, 0}, {kWorld.width() / 2, 0}).dx == kWorld.width() / 2);
    CHECK(kWorld.offset({kWorld.width() / 2, 0}, {0, 0}).dx == -kWorld.width() / 2);
}

// checks: WLD-01 RES-05
TEST_CASE("places wrap onto the torus from anywhere") {
    CHECK(kWorld.wrap(-1, -1) == num::Point{kWorld.width() - 1, kWorld.height() - 1});
    CHECK(kWorld.wrap(kWorld.width(), kWorld.height()) == num::Point{0, 0});
    CHECK(kWorld.wrap(std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()) ==
          num::Point{static_cast<std::int32_t>(200'000'000 - 54'775'808), static_cast<std::int32_t>(54'775'807)});
    CHECK(kWorld.moved({0, 0}, {-3 * std::int64_t{kWorld.width()} - 1, 7}) == num::Point{kWorld.width() - 1, 7});
    CHECK(kd::test::stops([] { num::Torus(1, 5); }));
    CHECK(kd::test::stops([] { num::Torus((1 << 30) + 1, 5); }));
}

// checks: RES-05
TEST_CASE("squared distances and square roots are exact, up to the world's extremes") {
    for (std::uint64_t n = 0; n < (1U << 16U); ++n) {
        const std::uint64_t r = num::isqrt(n);
        REQUIRE(r * r <= n);
        REQUIRE((r + 1) * (r + 1) > n);
    }
    for (std::uint64_t k :
         {std::uint64_t{3}, std::uint64_t{100'000'000}, std::uint64_t{1} << 31U, std::uint64_t{0xFFFFFFFFU}}) {
        CHECK(num::isqrt(k * k) == k);
        CHECK(num::isqrt(k * k - 1) == k - 1);
        if (k < 0xFFFFFFFFU) {
            CHECK(num::isqrt(k * k + 1) == k);
        }
    }
    CHECK(num::isqrt(std::numeric_limits<std::uint64_t>::max()) == 0xFFFFFFFFU);
    // the farthest two places can be on the world, and on the largest torus allowed
    const num::Point far{kWorld.width() / 2, kWorld.height() / 2};
    CHECK(kWorld.squared_distance({0, 0}, far) == 12'500'000'000'000'000);
    CHECK(kWorld.distance({0, 0}, far) == 111'803'398);
    const num::Torus largest(1 << 30, 1 << 30);
    CHECK(largest.squared_distance({0, 0}, {1 << 29, 1 << 29}) == std::int64_t{1} << 59U);
    CHECK(largest.distance({0, 0}, {1 << 29, 1 << 29}) == 759'250'124);
    static_assert(num::isqrt(144) == 12);
}

// checks: RES-05
TEST_CASE("angles are turns: exact at the quarters, wrapping as they add") {
    constexpr num::Angle quarter{1U << 30U};
    CHECK(num::cos(num::Angle{0}) == 1.0);
    CHECK(num::sin(num::Angle{0}) == 0.0);
    CHECK(num::sin(quarter) == 1.0);
    CHECK(num::cos(quarter) == 0.0);
    CHECK(num::cos(quarter + quarter) == -1.0);
    CHECK(num::sin(quarter + quarter + quarter) == -1.0);
    CHECK(quarter + quarter + quarter + quarter == num::Angle{0});
    CHECK(num::Angle{0xFFFFFFFFU} + num::Angle{1} == num::Angle{0});
    CHECK((quarter + quarter).turns() == 0.5);
    CHECK(num::Angle{0}.to(quarter) == (1 << 30));
    CHECK(quarter.to(num::Angle{0}) == -(1 << 30));
    CHECK(num::Angle{0}.to(quarter + quarter) == std::numeric_limits<std::int32_t>::min());
    CHECK(num::angle_of_turns(0.25) == quarter);
    CHECK(num::angle_of_turns(-0.25) == num::Angle{3U << 30U});
    CHECK(num::angle_of_turns(7.25) == quarter);
    CHECK(num::angle_of_turns(0.9999999999999999) == num::Angle{0});
    CHECK(num::direction({1, 0}) == num::Angle{0});
    CHECK(num::direction({0, 5}) == quarter);
    CHECK(num::direction({-3, 0}) == num::Angle{2U << 30U});
    CHECK(num::direction({0, -1}) == num::Angle{3U << 30U});
    CHECK(num::direction({7, 7}) == num::Angle{1U << 29U});
    CHECK(kd::test::stops([] { num::direction({0, 0}); }));
}

// checks: RES-05
TEST_CASE("the one conversion to whole numbers rounds as asked and refuses what does not fit") {
    using num::Round;
    CHECK(num::to_int(2.5, Round::nearest) == 3);
    CHECK(num::to_int(-2.5, Round::nearest) == -3);
    CHECK(num::to_int(2.4999999999999996, Round::nearest) == 2);
    CHECK(num::to_int(-2.5, Round::down) == -3);
    CHECK(num::to_int(-2.5, Round::up) == -2);
    CHECK(num::to_int(-2.5, Round::toward_zero) == -2);
    CHECK(num::to_int(0x1p62, Round::down) == std::int64_t{1} << 62U);
    CHECK(num::to_int(-0x1p63, Round::down) == std::numeric_limits<std::int64_t>::min());
    CHECK(num::to_int(-0.0, Round::nearest) == 0);
    CHECK(kd::test::stops([] { num::to_int(0x1p63, Round::down); }));
    CHECK(kd::test::stops([] { num::to_int(-0x1.0000000000001p63, Round::up); }));
    CHECK(kd::test::stops([] { num::to_int(std::numeric_limits<double>::quiet_NaN(), Round::nearest); }));
    CHECK(kd::test::stops([] { num::to_int(std::numeric_limits<double>::infinity(), Round::down); }));
}

// checks: TIM-16 RES-05
TEST_CASE("probabilities are exact thresholds for 64-bit draws") {
    using num::Probability;
    CHECK(Probability::ratio(1, 2).threshold() == std::uint64_t{1} << 63U);
    CHECK(Probability::ratio(1, 4).threshold() == std::uint64_t{1} << 62U);
    CHECK(Probability::ratio(1, 3).threshold() == 0x5555555555555555ULL);
    CHECK(Probability::ratio(0, 9) == Probability::never());
    CHECK(Probability::ratio(7, 7) == Probability::always());
    CHECK_FALSE(Probability::never().fires(0));
    CHECK(Probability::always().fires(std::numeric_limits<std::uint64_t>::max()));
    CHECK(Probability::ratio(1, 2).fires((std::uint64_t{1} << 63U) - 1));
    CHECK_FALSE(Probability::ratio(1, 2).fires(std::uint64_t{1} << 63U));
    CHECK(kd::test::stops([] { Probability::ratio(2, 1); }));
    CHECK(kd::test::stops([] { Probability::ratio(0, 0); }));
}

// checks: RES-05
TEST_CASE("the strict sort orders by an order with no ties, and refuses ties") {
    std::vector<int> v{5, 3, 9, 1};
    kd::num::sort_strict(v.begin(), v.end(), std::less<>());
    CHECK(v == std::vector<int>{1, 3, 5, 9});
    CHECK(kd::test::stops([] {
        std::vector<int> tied{2, 1, 2};
        kd::num::sort_strict(tied.begin(), tied.end(), std::less<>());
    }));
}
