#include <bit>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/chance/chance.hpp"
#include "kd/num/digest.hpp"
#include "kd/run/workers.hpp"

namespace chance = kd::chance;

namespace {

constexpr std::uint64_t kSeed = 20261005;
constexpr std::size_t kDraws = 1'000'000;

const chance::Name kWeather = chance::name("weather");
const chance::Name kRain = chance::name("rain");

// The ways a key's neighbours differ: by index, by being, by moment and by world seed. Each gives the n-th key's
// draw, one part counting up and the rest fixed.
const std::vector<std::pair<const char*, std::function<std::uint64_t(std::uint64_t)>>>& sweeps() {
    static const std::vector<std::pair<const char*, std::function<std::uint64_t(std::uint64_t)>>> all = {
        {"indexes", [](std::uint64_t n) { return chance::Draws(kSeed, kWeather, 7, 3600, kRain).bits(n); }},
        {"beings", [](std::uint64_t n) { return chance::Draws(kSeed, kWeather, n, 3600, kRain).bits(0); }},
        {"moments",
         [](std::uint64_t n) {
             return chance::Draws(kSeed, kWeather, 7, static_cast<std::int64_t>(n), kRain).bits(0);
         }},
        {"seeds", [](std::uint64_t n) { return chance::Draws(n, kWeather, 7, 3600, kRain).bits(0); }},
    };
    return all;
}

double unit(std::uint64_t bits) {
    return static_cast<double>(bits >> 11U) * 0x1p-53;
}

}  // namespace

// checks: TIM-16
TEST_CASE("a key's draw is fixed for ever, the same on every build") {
    // Changing any of these would change the history of every world, so they never change.
    CHECK(chance::name("rain").hash == 0x06B452C92A0FB6C1ULL);
    CHECK(chance::Draws(kSeed, kWeather, 7, 3600, kRain).bits(0) == 0xBAE26468924EF8EAULL);
    CHECK(chance::Draws(0, chance::Name{0}, 0, 0, chance::Name{0}).bits(0) == 0xE275E1F0809331AEULL);
    kd::num::Digest digest;
    const chance::Draws draws(kSeed, kWeather, 0, -5, kRain);
    for (std::uint64_t i = 0; i < 1000; ++i) {
        digest.u64(draws.bits(i));
    }
    CHECK(digest.hex() == "645541c6957cd5f8");
}

// checks: TIM-16
TEST_CASE("draws are spread evenly whichever part of the key counts up") {
    for (const auto& [what, draw] : sweeps()) {
        CAPTURE(what);
        // frequencies of 100 values, against a chi-square bound for 99 degrees of freedom (157 at 1 in 10,000)
        std::vector<double> counts(100, 0.0);
        std::vector<std::uint64_t> ones(64, 0);
        for (std::uint64_t n = 0; n < kDraws; ++n) {
            const std::uint64_t b = draw(n);
            counts[static_cast<std::size_t>((static_cast<unsigned __int128>(b) * 100U) >> 64U)] += 1.0;
            for (std::size_t bit = 0; bit < 64; ++bit) {
                ones[bit] += (b >> bit) & 1U;
            }
        }
        const double expected = static_cast<double>(kDraws) / 100.0;
        double chi = 0.0;
        for (double c : counts) {
            chi += (c - expected) * (c - expected) / expected;
        }
        CHECK(chi < 157.0);
        // every bit is set about half the time: within five standard deviations, 2,500 in a million
        for (std::uint64_t o : ones) {
            CHECK(o > kDraws / 2 - 2500);
            CHECK(o < kDraws / 2 + 2500);
        }
    }
}

// checks: TIM-16
TEST_CASE("neighbouring keys give unrelated draws") {
    for (const auto& [what, draw] : sweeps()) {
        CAPTURE(what);
        // the correlation of each draw with the next, as fractions, and the bits that differ between them
        double sx = 0.0;
        double sxx = 0.0;
        double sxy = 0.0;
        std::uint64_t flipped = 0;
        std::uint64_t before = draw(0);
        for (std::uint64_t n = 1; n <= kDraws; ++n) {
            const std::uint64_t now = draw(n);
            const double x = unit(before);
            sx += x;
            sxx += x * x;
            sxy += x * unit(now);
            flipped += static_cast<std::uint64_t>(std::popcount(before ^ now));
            before = now;
        }
        const double n = static_cast<double>(kDraws);
        const double mean = sx / n;
        const double correlation = (sxy / n - mean * mean) / (sxx / n - mean * mean);
        CHECK(std::fabs(correlation) < 0.005);
        // half of the 64 bits differ on average: 32, within a tenth
        CHECK(std::fabs(static_cast<double>(flipped) / n - 32.0) < 0.1);
    }
}

// checks: TIM-16
TEST_CASE("whole numbers in a range never fall outside it, and a chance fires as often as it says") {
    const chance::Draws draws(kSeed, kWeather, 1, 2, kRain);
    for (std::uint64_t n : {std::uint64_t{1}, std::uint64_t{2}, std::uint64_t{3}, std::uint64_t{7},
                            (std::uint64_t{1} << 63U) + 1, std::numeric_limits<std::uint64_t>::max()}) {
        for (std::uint64_t i = 0; i < 10'000; ++i) {
            REQUIRE(draws.below(i, n) < n);
        }
    }
    std::vector<int> seen(7, 0);
    for (std::uint64_t i = 0; i < 10'000; ++i) {
        const std::int64_t v = draws.between(i, -3, 3);
        REQUIRE(v >= -3);
        REQUIRE(v <= 3);
        ++seen[static_cast<std::size_t>(v + 3)];
        REQUIRE(draws.between(i, 5, 5) == 5);
        const double f = draws.fraction(i);
        REQUIRE(f >= 0.0);
        REQUIRE(f < 1.0);
    }
    for (int s : seen) {
        CHECK(s > 1200);
    }
    const std::int64_t whole =
        draws.between(9, std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max());
    CHECK(static_cast<std::uint64_t>(whole) == draws.bits(9) + (std::uint64_t{1} << 63U));
    std::uint64_t fired = 0;
    const auto quarter = kd::num::Probability::ratio(1, 4);
    for (std::uint64_t i = 0; i < kDraws; ++i) {
        fired += draws.fires(i, quarter) ? 1 : 0;
    }
    CHECK(fired > 250'000 - 2'200);
    CHECK(fired < 250'000 + 2'200);
    CHECK(kd::test::stops([&] { static_cast<void>(draws.below(0, 0)); }));
    CHECK(kd::test::stops([&] { static_cast<void>(draws.between(0, 2, 1)); }));
}

// checks: TIM-16
TEST_CASE("a new purpose moves no other purpose's draws, in any order, on any thread") {
    const chance::Draws rain(kSeed, kWeather, 0, 86'400, kRain);
    const chance::Draws hail(kSeed, kWeather, 0, 86'400, chance::name("hail"));
    constexpr std::size_t kCount = 100'000;
    std::vector<std::uint64_t> alone(kCount);
    for (std::size_t i = 0; i < kCount; ++i) {
        alone[i] = rain.bits(i);
    }
    // drawn backwards, between draws of a new purpose, and in pieces on four threads
    std::vector<std::uint64_t> mixed(kCount);
    std::size_t same = 0;
    for (std::size_t i = kCount; i-- > 0;) {
        const std::uint64_t other = hail.bits(i);
        mixed[i] = rain.bits(i);
        same += other == mixed[i] ? 1 : 0;
    }
    CHECK(mixed == alone);
    CHECK(same == 0);
    std::vector<std::uint64_t> threaded(kCount);
    kd::run::Workers workers(4);
    workers.for_each(kCount / 1000, [&](std::size_t p) {
        for (std::size_t i = p * 1000; i < (p + 1) * 1000; ++i) {
            threaded[i] = rain.bits(i);
        }
    });
    CHECK(threaded == alone);
}
