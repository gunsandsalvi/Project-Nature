#include <algorithm>
#include <cmath>
#include <cstdint>

#include "doctest.h"
#include "patches.hpp"

namespace patches = kd::view::patches;

namespace {

// A channel of the patch at a place, in bytes: 0 growth, 1 wear, 2 damp, 3 the fourth byte.
int channel(const patches::Patches& p, double east, double north, std::size_t which) {
    const int column = static_cast<int>(std::floor((east - p.west) / p.patch));
    const int row = static_cast<int>(std::floor((north - p.south) / p.patch));
    REQUIRE(column >= 0);
    REQUIRE(row >= 0);
    REQUIRE(column < p.size);
    REQUIRE(row < p.size);
    return p.data[4 * (static_cast<std::size_t>(row) * static_cast<std::size_t>(p.size) +
                       static_cast<std::size_t>(column)) +
                  which];
}

}  // namespace

// checks: PRE-20 WLD-31
TEST_CASE("the patch picture is a square of 64 patches of 4 m with the camp in the middle of a patch near its middle") {
    const patches::Params params;
    const patches::Patches p = patches::make(1000.0, 1100.0, params);
    CHECK(p.size == 64);
    CHECK(p.patch == doctest::Approx(4.0));
    CHECK(p.data.size() == 64u * 64u * 4u);
    CHECK(p.west == doctest::Approx(1000.0 - 130.0));
    CHECK(p.south == doctest::Approx(1100.0 - 130.0));
    // the camp is the middle of patch 32, a patch's width from the corner of four
    CHECK(1000.0 - p.west == doctest::Approx(32.5 * 4.0));
    // the fourth byte is full, and the damp byte is kept empty for the shore
    CHECK(channel(p, 1000.0, 1100.0, 3) == 255);
    CHECK(channel(p, 1050.0, 1000.0, 2) == 0);
}

// checks: PRE-20 WLD-31
TEST_CASE("the same camp, numbers and seed give the same picture, and another seed another") {
    patches::Params params;
    params.seed = 11;
    const patches::Patches a = patches::make(500.0, 500.0, params);
    const patches::Patches b = patches::make(500.0, 500.0, params);
    CHECK(a.data == b.data);
    params.seed = 12;
    const patches::Patches c = patches::make(500.0, 500.0, params);
    CHECK(a.data != c.data);
}

// checks: PRE-20 WLD-31
TEST_CASE("growth fills the range in broad masses: smooth within a patch's reach and wide in extent") {
    patches::Params params;
    params.seed = 11;
    const patches::Patches p = patches::make(1000.0, 1000.0, params);
    int low = 255;
    int high = 0;
    long steps = 0;
    long step_count = 0;
    for (int row = 0; row < p.size; ++row) {
        for (int column = 0; column < p.size; ++column) {
            const auto at = [&](int r, int c) {
                return static_cast<int>(p.data[4 * (static_cast<std::size_t>(r) * static_cast<std::size_t>(p.size) +
                                                    static_cast<std::size_t>(c))]);
            };
            low = std::min(low, at(row, column));
            high = std::max(high, at(row, column));
            if (column + 1 < p.size) {
                steps += std::abs(at(row, column) - at(row, column + 1));
                ++step_count;
            }
        }
    }
    // short and dry places and tall, lush places both exist
    CHECK(low < 80);
    CHECK(high > 175);
    // a patch is near its neighbour on average: masses, not static (static would step about a third of the range)
    CHECK(static_cast<double>(steps) / static_cast<double>(step_count) < 22.0);
}

// checks: PRE-20 WLD-31
TEST_CASE("the ground is worn bare at the camp, and the wear leaves with distance") {
    patches::Params params;
    params.seed = 11;
    const double east = 1000.0;
    const double north = 1000.0;
    const patches::Patches p = patches::make(east, north, params);
    // inside the clearing: worn nearly bare, whatever the growth
    CHECK(channel(p, east, north, 1) > 200);
    CHECK(channel(p, east + 2.0, north - 1.0, 1) > 200);
    // well past the clearing and its fade, the clearing's wear is gone: the mean wear is far lower
    long near = 0;
    long far = 0;
    long near_count = 0;
    long far_count = 0;
    for (int row = 0; row < p.size; ++row) {
        for (int column = 0; column < p.size; ++column) {
            const double e = p.west + (column + 0.5) * p.patch;
            const double n = p.south + (row + 0.5) * p.patch;
            const double away = std::hypot(e - east, n - north);
            const int wear = p.data[4 * (static_cast<std::size_t>(row) * static_cast<std::size_t>(p.size) +
                                         static_cast<std::size_t>(column)) +
                                    1];
            if (away < params.clearing) {
                near += wear;
                ++near_count;
            } else if (away > params.clearing + params.clearing_fade + 12.0) {
                far += wear;
                ++far_count;
            }
        }
    }
    REQUIRE(near_count > 0);
    REQUIRE(far_count > 0);
    CHECK(static_cast<double>(near) / static_cast<double>(near_count) > 190.0);
    CHECK(static_cast<double>(far) / static_cast<double>(far_count) < 70.0);
}

// checks: PRE-20 WLD-31
TEST_CASE("bare earth shows only where growth is thin, and most of the area is covered") {
    patches::Params params;
    params.seed = 11;
    const double east = 1000.0;
    const double north = 1000.0;
    const patches::Patches p = patches::make(east, north, params);
    int bare = 0;
    int far_total = 0;
    for (int row = 0; row < p.size; ++row) {
        for (int column = 0; column < p.size; ++column) {
            const double e = p.west + (column + 0.5) * p.patch;
            const double n = p.south + (row + 0.5) * p.patch;
            // outside the camp's clearing, what is worn comes from thin growth alone
            if (std::hypot(e - east, n - north) < params.clearing + params.clearing_fade + 12.0) {
                continue;
            }
            ++far_total;
            const std::size_t at = 4 * (static_cast<std::size_t>(row) * static_cast<std::size_t>(p.size) +
                                        static_cast<std::size_t>(column));
            const int growth = p.data[at];
            const int wear = p.data[at + 1];
            if (wear > 128) {
                ++bare;
                // a patch worn to half is thin: growth under the bare line (a share of 255) with the edge's stray
                // beside it
                CHECK(static_cast<double>(growth) < (params.bare_below + 0.25) * 255.0);
            }
        }
    }
    REQUIRE(far_total > 0);
    CHECK(static_cast<double>(bare) / static_cast<double>(far_total) < 0.25);
}

// checks: PRE-20 WLD-31
TEST_CASE("a place's growth does not depend on where the picture is centred") {
    patches::Params params;
    params.seed = 11;
    const patches::Patches a = patches::make(1000.0, 1000.0, params);
    const patches::Patches b = patches::make(1032.0, 1000.0, params);
    // a patch of a's picture is the same place in b's picture, 32 m east: 8 patches along
    for (int row = 0; row < 64; ++row) {
        for (int column = 8; column < 64; ++column) {
            const std::size_t in_a = 4 * (static_cast<std::size_t>(row) * 64u + static_cast<std::size_t>(column));
            const std::size_t in_b = 4 * (static_cast<std::size_t>(row) * 64u + static_cast<std::size_t>(column - 8));
            CHECK(a.data[in_a] == b.data[in_b]);
        }
    }
}
