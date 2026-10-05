// P7's tests (IMPLEMENTATION α0.5a): our Fourier transform, rivers that reach the sea or a lake, the share of land and
// the tilt in their ranges, rock and deposits where geology puts them, rain shadows behind mountains, the candidates'
// offer and its reasons, settling, and the same worlds on one thread and four and as the cloud recorded; and P8's
// (α0.5b): the ground made on demand from the cells and the seed alone. Pre-production code (research 00).
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <algorithm>
#include <cmath>

#include "area.hpp"
#include "chance.hpp"
#include "draws.hpp"
#include "expected.hpp"
#include "fft.hpp"
#include "hash.hpp"
#include "life.hpp"
#include "world.hpp"

using worldgen::Rock;
using worldgen::Setting;
using worldgen::World;

namespace {

worldgen::Settings small_settings(int threads) {
    worldgen::Settings s = worldgen::small(worldgen::Settings{});
    s.threads = threads;
    return s;
}

World small_world(std::uint64_t seed) {
    worldgen::Times times;
    minds::Pool one(1);
    return worldgen::make_world(seed, 256, false, small_settings(1), &one, &times);
}

bool layer_has(const World& w, int c, Rock r) {
    for (std::size_t l = 0; l < 3; ++l) {
        if (w.rock[(static_cast<std::size_t>(c) * 3) + l] == static_cast<std::uint8_t>(r)) {
            return true;
        }
    }
    return false;
}

}  // namespace

// checks: WLD-16
TEST_CASE("our Fourier transform turns back into what it was given") {
    const int width = 64;
    const int height = 32;
    std::vector<worldgen::Complex> grid(static_cast<std::size_t>(width * height));
    for (std::size_t i = 0; i < grid.size(); ++i) {
        grid[i].re = samebits::chance(5, i, 0, worldgen::Draw::kNoise) - 0.5;
    }
    const std::vector<worldgen::Complex> start = grid;
    minds::Pool pool(2);
    worldgen::fft2(&grid, width, height, false, &pool);
    // the mean lies in the first bin
    double mean = 0.0;
    for (const worldgen::Complex& c : start) {
        mean += c.re;
    }
    CHECK(std::abs(grid[0].re - mean) < 1e-9);
    worldgen::fft2(&grid, width, height, true, &pool);
    double most = 0.0;
    for (std::size_t i = 0; i < grid.size(); ++i) {
        most = std::max(most, std::abs(grid[i].re - start[i].re) + std::abs(grid[i].im));
    }
    CHECK(most < 1e-12);
}

// checks: WLD-17, WLD-08
TEST_CASE("every river reaches the sea or a lake") {
    const World w = small_world(worldgen::candidate_seed(3, 0));
    std::vector<std::int32_t> place(static_cast<std::size_t>(w.grid.cells()), -1);
    for (std::size_t i = 0; i < w.order.size(); ++i) {
        place[static_cast<std::size_t>(w.order[i])] = static_cast<std::int32_t>(i);
    }
    int land = 0;
    int reached = 0;
    for (int c = 0; c < w.grid.cells(); ++c) {
        if (w.sea(c)) {
            continue;
        }
        ++land;
        // every cell comes after the cell its water goes to, so following them always ends at the sea
        const std::int32_t r = w.receiver[static_cast<std::size_t>(c)];
        const bool before = w.sea(r) || place[static_cast<std::size_t>(r)] < place[static_cast<std::size_t>(c)];
        int at = c;
        for (int k = 0; k < w.grid.cells() && !w.sea(at); ++k) {
            at = w.receiver[static_cast<std::size_t>(at)];
        }
        reached += (before && w.sea(at)) ? 1 : 0;
    }
    CHECK(land > 1000);
    CHECK(reached == land);
}

// checks: WLD-06
TEST_CASE("each world's share of land and tilt fall within their ranges and spread across them") {
    double least_land = 1.0;
    double most_land = 0.0;
    double least_tilt = 90.0;
    double most_tilt = 0.0;
    for (std::uint64_t seed = 1; seed <= 12; ++seed) {
        World w;
        w.seed = worldgen::candidate_seed(seed, 0);
        w.grid = {128, 64, worldgen::kAroundMetres / 128};
        minds::Pool one(1);
        worldgen::make_plates(&w, &one);
        const auto land =
            static_cast<double>(std::count_if(w.height.begin(), w.height.end(), [](float h) { return h > 0.0F; }));
        const double share = land / static_cast<double>(w.grid.cells());
        least_land = std::min(least_land, share);
        most_land = std::max(most_land, share);
        least_tilt = std::min(least_tilt, w.tilt);
        most_tilt = std::max(most_tilt, w.tilt);
    }
    CHECK(least_land >= 0.25);
    CHECK(most_land <= 0.5);
    CHECK(most_land - least_land > 0.08);
    CHECK(least_tilt >= 15.0);
    CHECK(most_tilt <= 30.0);
    CHECK(most_tilt - least_tilt > 5.0);
}

// checks: WLD-09, WLD-14
TEST_CASE("rock and deposits lie where geology puts them") {
    int chalk = 0;
    int flint = 0;
    int obsidian = 0;
    int copper = 0;
    int volcanoes = 0;
    for (std::uint64_t seed = 1; seed <= 4; ++seed) {
        const World w = small_world(worldgen::candidate_seed(seed, 1));
        for (int c = 0; c < w.grid.cells(); ++c) {
            const auto cs = static_cast<std::size_t>(c);
            const auto setting = static_cast<Setting>(w.setting[cs]);
            // chalk and limestone only where seas or basins lay
            if (layer_has(w, c, Rock::kChalk) || layer_has(w, c, Rock::kLimestone)) {
                ++chalk;
                CHECK(setting == Setting::kBasin);
            }
            // volcanoes stand on arcs and rifts, along plate edges
            if (w.volcano[cs] != 0) {
                ++volcanoes;
                CHECK((setting == Setting::kArc || setting == Setting::kRift));
            }
            if (w.sea(c)) {
                continue;
            }
            // flint only in chalk, chert only in limestone, and both in the gravel below them
            if (w.deposit(c, worldgen::kFlint) > 0) {
                ++flint;
                CHECK((layer_has(w, c, Rock::kChalk) || w.surface(c) == Rock::kGravel));
            }
            if (w.deposit(c, worldgen::kChert) > 0) {
                CHECK(layer_has(w, c, Rock::kLimestone));
            }
            // obsidian only at young volcanoes with thick lava
            if (w.deposit(c, worldgen::kObsidian) > 0) {
                ++obsidian;
                CHECK((w.volcano[cs] == 2 || w.surface(c) == Rock::kGlassyLava));
            }
            // copper ore only in volcanic ranges, over granite
            if (w.deposit(c, worldgen::kCopperOre) > 0) {
                ++copper;
                CHECK(setting == Setting::kArc);
                CHECK(layer_has(w, c, Rock::kGranite));
            }
        }
    }
    CHECK(chalk > 0);
    CHECK(flint > 0);
    CHECK(volcanoes > 0);
    CHECK(obsidian > 0);
    CHECK(copper > 0);
}

// checks: WLD-16
TEST_CASE("mountains bring rain to their windward side and shadow their lee") {
    // a continent across the westerlies with a ridge running north and south through it
    World w;
    w.seed = 7;
    w.tilt = 23.0;
    w.grid = {256, 128, worldgen::kAroundMetres / 256};
    const worldgen::Grid& g = w.grid;
    w.height.assign(static_cast<std::size_t>(g.cells()), -3000.0F);
    for (int y = 0; y < g.height; ++y) {
        for (int x = 60; x < 200; ++x) {
            const double ridge = (x - 130) / 4.0;
            w.height[static_cast<std::size_t>(g.at(x, y))] =
                static_cast<float>(200.0 + (2500.0 * std::exp(-(ridge * ridge))));
        }
    }
    minds::Pool pool(2);
    worldgen::make_climate(&w, &pool);
    const int y = static_cast<int>((45.0 + 90.0) / 180.0 * g.height);  // 45° north, in the westerlies
    const double windward = w.rain[static_cast<std::size_t>(g.at(124, y))];
    const double lee = w.rain[static_cast<std::size_t>(g.at(136, y))];
    CHECK(windward > 1.3 * lee);
    // and the heights are colder
    CHECK(w.temperature[static_cast<std::size_t>(g.at(130, y))] <
          w.temperature[static_cast<std::size_t>(g.at(100, y))] - 10.0F);
}

// checks: WLD-10, WLD-24, RES-05
TEST_CASE("new world offers at most three, best first, each with every must-have, and logs every candidate") {
    const worldgen::Offer one = worldgen::new_world(small_settings(1));
    const worldgen::Offer four = worldgen::new_world(small_settings(4));
    REQUIRE(!one.three.empty());
    CHECK(one.three.size() <= 3);
    CHECK(one.log.size() >= static_cast<std::size_t>(one.made));
    for (std::size_t i = 0; i < one.three.size(); ++i) {
        const World& w = one.three[i];
        CHECK(w.qualifies);
        CHECK(w.full);
        CHECK(w.start.cell >= 0);
        CHECK(w.start.shelters >= 3);
        CHECK(w.reasons.find("qualifies") == 0);
        if (i > 0) {
            CHECK(w.score <= one.three[i - 1].score);
        }
    }
    for (const std::string& line : one.log) {
        CHECK((line.find("qualifies") != std::string::npos || line.find("rejected") != std::string::npos));
    }
    // a seed always offers the same three, on one thread or four
    REQUIRE(one.three.size() == four.three.size());
    for (std::size_t i = 0; i < one.three.size(); ++i) {
        CHECK(one.three[i].checksum() == four.three[i].checksum());
    }
}

// checks: WLD-08, RES-05
TEST_CASE("settling runs the herds a year, the same on one thread and four") {
    worldgen::Offer a = worldgen::new_world(small_settings(1));
    worldgen::Offer b = worldgen::new_world(small_settings(4));
    REQUIRE(!a.three.empty());
    const std::size_t herds = a.three[0].herds.size();
    REQUIRE(herds > 0);
    std::vector<std::int32_t> before;
    for (const worldgen::Herd& h : a.three[0].herds) {
        before.push_back(h.cell);
    }
    minds::Pool one(1);
    minds::Pool four(4);
    CHECK(worldgen::settle(&a.three[0], 1, &one) > 0.0);
    worldgen::settle(&b.three[0], 1, &four);
    CHECK(a.three[0].checksum() == b.three[0].checksum());
    int moved = 0;
    for (std::size_t i = 0; i < herds; ++i) {
        moved += a.three[0].herds[i].cell != before[i] ? 1 : 0;
        CHECK(a.three[0].herds[i].count > 0.0F);
    }
    CHECK(moved > 0);
}

// checks: RES-05
TEST_CASE("the small run matches the cloud's recorded one") {
    worldgen::Offer offer = worldgen::new_world(small_settings(2));
    std::vector<std::uint64_t> sums;
    sums.reserve(offer.three.size() + 1);
    for (const World& w : offer.three) {
        sums.push_back(w.checksum());
    }
    minds::Pool pool(2);
    worldgen::settle(&offer.three[0], 1, &pool);
    sums.push_back(offer.three[0].checksum());
    CHECK(samebits::digest(sums) == std::string(worldgen::kCloudDigest));
}

// checks: WLD-13, WLD-12, PRE-03
TEST_CASE("the ground made on demand is the same every time, follows its cells, and puts each tree in one square") {
    World w = small_world(5);
    const worldgen::Grid& g = w.grid;
    // a forest well above the sea
    int land = -1;
    for (int c = 0; c < g.cells() && land < 0; ++c) {
        const auto b = static_cast<worldgen::Biome>(w.biome[static_cast<std::size_t>(c)]);
        const bool forest = b == worldgen::Biome::kBroadleaf || b == worldgen::Biome::kConifer;
        land = forest && w.height[static_cast<std::size_t>(c)] > 300.0F ? c : -1;
    }
    REQUIRE(land >= 0);
    const double east = (g.x_of(land) + 0.5) * g.metres;
    const double north = (g.y_of(land) + 0.5) * g.metres;
    // made twice, and in pieces, it is the same
    const std::vector<float> a = worldgen::ground_heights(w, east, north, 33, 2.0);
    const std::vector<float> b = worldgen::ground_heights(w, east, north, 33, 2.0);
    CHECK(a == b);
    CHECK(worldgen::ground_heights(w, east + 8.0, north + 6.0, 1, 1.0)[0] == a[(3 * 33) + 4]);
    // its relief stays near the cell's own height
    const double cell = w.height[static_cast<std::size_t>(land)];
    for (const float h : a) {
        CHECK(std::abs(h - cell) < 0.5 * cell + 60.0);
    }
    // the sea stays flat at its level, with no relief to raise an island
    for (int c = 0; c < g.cells(); ++c) {
        if (w.height[static_cast<std::size_t>(c)] < -200.0F) {
            CHECK(worldgen::ground_height(w, (g.x_of(c) + 0.5) * g.metres, (g.y_of(c) + 0.5) * g.metres) < 0.0);
            break;
        }
    }
    // the colours are the cover's, or water where the ground is under the sea
    const std::vector<std::uint8_t> rgb = worldgen::ground_colours(w, east, north, 33, 2.0, a);
    CHECK(rgb.size() == a.size() * 3);
    // trees: two squares side by side hold together what the square spanning both holds, none twice
    const std::vector<float> left = worldgen::ground_trees(w, east, north, 64.0, 5.0);
    const std::vector<float> right = worldgen::ground_trees(w, east + 64.0, north, 64.0, 5.0);
    const std::vector<float> both = worldgen::ground_trees(w, east, north, 128.0, 5.0);
    REQUIRE(!left.empty());
    std::size_t in_both = 0;
    for (std::size_t i = 0; i < both.size(); i += 3) {
        if (both[i + 1] < 64.0F) {
            in_both += 3;
        }
    }
    CHECK(left.size() + right.size() == in_both);
    for (std::size_t i = 0; i < left.size(); i += 3) {
        CHECK(left[i] >= 0.0F);
        CHECK(left[i] < 64.0F);
        CHECK(left[i + 1] >= 0.0F);
        CHECK(left[i + 1] < 64.0F);
        // each stands on the ground there, above the sea
        CHECK(left[i + 2] == static_cast<float>(worldgen::ground_height(w, east + left[i], north + left[i + 1])));
        CHECK(left[i + 2] > 0.5F);
    }
}
