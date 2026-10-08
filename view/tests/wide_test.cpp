#include "wide.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include "doctest.h"
using namespace kd::view;
// checks: PRE-28, PRE-42, WLD-13, TIM-17: actual IDs and time-driven membership, owned sorted results.
TEST_CASE("wide forms retain actual camp and walker identities independent of source order") {
    const kd::num::Torus torus(200000000, 100000000);
    std::vector<DrawRecord> rows(3);
    rows[0].id = 9;
    rows[0].camp = 0;
    rows[0].east = 1;
    rows[0].phase = .5;
    rows[1].id = 3;
    rows[1].camp = 0;
    rows[1].east = 2;
    rows[2].id = 7;
    rows[2].camp = 1;
    rows[2].east = 2;
    std::vector<CampRecord> camps{{100, {0, 0}}, {200, {200, 0}}};
    const auto tiny = wide_records(rows, camps, torus, {0, 0}, "tiny");
    REQUIRE(tiny.records.size() == 3);
    if (tiny.records.size() != 3) return;
    CHECK(tiny.records[0].id == 3);
    CHECK(tiny.records[2].phase == .5);
    const auto grouped = wide_records(rows, camps, torus, {0, 0}, "group");
    REQUIRE(grouped.records.size() == 2);
    if (grouped.records.size() != 2) return;
    CHECK(grouped.records[0].members == std::vector<std::uint64_t>{3, 9});
    CHECK(grouped.records[0].east == 1.5);
    CHECK(grouped.records[0].id == 100);
    std::reverse(rows.begin(), rows.end());
    CHECK(wide_records(rows, camps, torus, {0, 0}, "group").records[0].key == grouped.records[0].key);
    const auto camp = wide_records(rows, camps, torus, {0, 0}, "overview-fixture");
    REQUIRE(camp.records.size() == 2);
    if (camp.records.size() != 2) return;
    CHECK(camp.records[0].east == 0);
    CHECK(camp.records[0].count == 2);
    rows[2].east = 257;
    CHECK(wide_records(rows, camps, torus, {0, 0}, "group").records.size() == 3);
    CHECK(grouped.records[0].members.size() == 2);  // an old publication remains owned
}
// checks: PRE-43/PRE-46/WLD-01: wrapped neighbours and shared canonical samples independent of request/cache order.
TEST_CASE("ground keys and details wrap without changing at an origin or cache boundary") {
    const kd::num::Torus torus(10000, 5000);
    const auto a = canonical_tile(torus, -1, -1, 4, 42);
    const auto b = canonical_tile(torus, 9999, 4999, 4, 42);
    CHECK(a.power == 4);
    CHECK(a.variant >= 0);
    CHECK(a.variant < 3);
    CHECK(a.key == b.key);
    CHECK(a.west_south == b.west_south);
    CHECK(a.details == b.details);
    CHECK(a.east_north == kd::num::Point{10000, 5000});
    REQUIRE(a.details.size() == 3);
    const auto neighbour = canonical_tile(torus, a.west_south.x - 1, a.west_south.y, 4, 42);
    CHECK(neighbour.neighbours[2] == a.key);
    CHECK(neighbour.border_keys[1] == a.border_keys[0]);
    CHECK(neighbour.border_keys[2] == a.border_keys[3]);
    CHECK(a.border_keys[0] != a.border_keys[1]);
    for (const auto point : a.details) {
        CHECK(point.x >= a.west_south.x);
        CHECK(point.x < a.east_north.x);
    }
    const auto tiles = ground_tiles(torus, {9990, 4990}, {-.2, -.2, .2, .2}, 4, 42);
    REQUIRE(tiles.size() == 4);
    CHECK(std::any_of(tiles.begin(), tiles.end(), [](const auto& t) { return t.west_south == kd::num::Point{0, 0}; }));
    const auto whole = ground_tiles(torus, {0, 0}, {-50, -25, 50, 25}, 4, 42);
    CHECK_FALSE(whole.empty());
    CHECK(whole.size() <= 256);
    const auto invalid = ground_tiles(torus, {0, 0}, {0, 0, std::numeric_limits<double>::quiet_NaN(), 1}, 4, 42);
    CHECK(invalid.empty());
}
// checks: PRE-03/PRE-33/WLD-01: exact drawing-origin rebasing preserves all decided raster stops and inverse picks.
TEST_CASE("camera rebasing preserves every raster stop and wrapped absolute picks") {
    const auto torus = kd::world::World::kTorus;
    for (int power = -12; power <= 6; ++power) {
        Projection p;
        p.size(1080, 2400);
        p.zoom(std::ldexp(1.0, power) / p.density(), {540, 1200}, true);
        p.focus(20000.25, -30000.75);
        const auto before = p.raster(20002.5, -30001.25, 2);
        const auto physical = p.project(20002.5, -30001.25, 2);
        const auto shift = p.rebase();
        REQUIRE(shift.dx != 0);
        REQUIRE(shift.dy != 0);
        if (shift.dx == 0 || shift.dy == 0) continue;
        CHECK(shift.dx % 409600 == 0);
        CHECK(shift.dy % 409600 == 0);
        const auto after = p.raster(20002.5 - static_cast<double>(shift.dx) / 100.,
                                    -30001.25 - static_cast<double>(shift.dy) / 100., 2);
        CHECK(after.x == before.x);
        CHECK(after.y == before.y);
        const auto source = p.at_origin({static_cast<double>(shift.dx) / 100., static_cast<double>(shift.dy) / 100.});
        const auto source_pixel = source.raster(20002.5, -30001.25, 2);
        CHECK(source_pixel.x == before.x);
        CHECK(source_pixel.y == before.y);
        const auto pick = p.ground(physical, 2);
        const auto absolute =
            torus.moved(torus.wrap(shift.dx, shift.dy), {std::llround(pick.x * 100), std::llround(pick.y * 100)});
        CHECK(absolute == torus.wrap(2000250, -3000125));
        CHECK(p.rebase() == kd::num::Offset{});
    }
}

// checks: PRE-03/PLT-04/WLD-13: large views retain counts while bounding copied records and member storage.
TEST_CASE("wide cutoff remains explicit and bounded while preserving sampled population counts") {
    const auto torus = kd::world::World::kTorus;
    std::vector<DrawRecord> rows(10000);
    for (std::size_t i = 0; i < rows.size(); ++i) {
        rows[i].id = i + 1;
        rows[i].camp = 0;
        rows[i].east = 1.001;
    }
    const std::vector<CampRecord> camps{{100, {0, 0}}};
    const auto groups = wide_records(rows, camps, torus, {0, 0}, "group");
    REQUIRE(groups.records.size() == 1);
    if (groups.records.size() != 1) return;
    CHECK(groups.records[0].count == 10000);
    CHECK(groups.member_count == 10000);
    CHECK(groups.records[0].members.size() == 8192);
    CHECK(groups.truncated);
    CHECK(groups.records[0].east == doctest::Approx(1.001));
    const auto tiny = wide_records(rows, camps, torus, {0, 0}, "tiny");
    CHECK(tiny.records.size() == 512);
    CHECK(tiny.member_count == 10000);
    CHECK(tiny.truncated);
    std::reverse(rows.begin(), rows.end());
    const auto reversed = wide_records(rows, camps, torus, {0, 0}, "tiny");
    REQUIRE(reversed.records.size() == tiny.records.size());
    if (reversed.records.size() != tiny.records.size()) return;
    for (std::size_t i = 0; i < tiny.records.size(); ++i) CHECK(reversed.records[i].id == tiny.records[i].id);
}

// checks: PRE-03/PRE-28/WLD-13: off-screen populations cannot consume the bounded visible representation slots.
TEST_CASE("wide footprint keeps complete visible groups and hides remote population before cutoff") {
    const auto torus = kd::world::World::kTorus;
    std::vector<DrawRecord> rows(1000);
    for (std::size_t i = 0; i < rows.size(); ++i) {
        rows[i].id = i + 1;
        rows[i].camp = 0;
        rows[i].east = 10000;
    }
    rows.back().id = 10000;
    rows.back().east = 1;
    const std::vector<CampRecord> camps{{100, {0, 0}}};
    const Footprint near{-10, -10, 10, 10};
    const auto tiny = wide_records(rows, camps, torus, {0, 0}, "tiny", &near);
    REQUIRE(tiny.records.size() == 1);
    if (tiny.records.size() != 1) return;
    CHECK(tiny.records[0].id == 10000);
    CHECK(tiny.member_count == 1);
    CHECK(tiny.population_count == 1000);
    CHECK_FALSE(tiny.truncated);
    rows[0].east = 200;  // same visible canonical cell, member itself beyond the footprint
    const auto group = wide_records(rows, camps, torus, {0, 0}, "group", &near);
    REQUIRE(group.records.size() == 1);
    if (group.records.size() != 1) return;
    CHECK(group.records[0].count == 2);
    CHECK(group.records[0].members == std::vector<std::uint64_t>{1, 10000});
    const auto camp = wide_records(rows, camps, torus, {0, 0}, "overview-fixture", &near);
    REQUIRE(camp.records.size() == 1);
    if (camp.records.size() != 1) return;
    CHECK(camp.records[0].count == 1000);
}
// checks: PRE-43/PRE-46: adapting render tile size cannot re-roll the same canonical fine detail.
TEST_CASE("coarse ground demand retains the canonical fine detail ids and positions") {
    const auto torus = kd::world::World::kTorus;
    const auto fine = canonical_tile(torus, 0, 0, 4, 42), coarse = canonical_tile(torus, 0, 0, 8, 42);
    CHECK(fine.details == coarse.details);
    const auto near = canonical_tile(torus, 400, 400, 2, 42);
    CHECK(near.details == fine.details);
    CHECK(near.detail_keys == fine.detail_keys);
    CHECK(fine.detail_keys == coarse.detail_keys);
    REQUIRE(fine.detail_keys.size() == 3);
    if (fine.detail_keys.size() != 3) return;
    CHECK(fine.detail_keys[0] != fine.detail_keys[1]);
    const auto adjacent = canonical_tile(torus, 1600, 0, 4, 42);
    CHECK(fine.detail_keys != adjacent.detail_keys);
    CHECK(fine.stamp_key == adjacent.stamp_key);
    for (int x = 0; x < 16; ++x)
        for (int y = 0; y < 16; ++y) {
            const auto cell =
                canonical_tile(torus, static_cast<std::int64_t>(x) * 1600, static_cast<std::int64_t>(y) * 1600, 4, 42);
            CHECK(cell.stamp_key == coarse.stamp_key);
            CHECK(cell.accent == coarse.accent);
        }
}

// checks: WLD-01/PRE-28/PRE-43: portrait-wide torus repeats share one cache identity and cover every local copy.
TEST_CASE("wide ground demand covers repeated torus images with bounded canonical cache identities") {
    const kd::num::Torus torus(10000, 5000);
    const auto tiles = ground_tiles(torus, {0, 0}, {-200, -100, 200, 100}, 4, 42);
    REQUIRE(!tiles.empty());
    if (tiles.empty()) return;
    CHECK(tiles.size() <= 256);
    for (const auto point : std::vector<Pixel>{{-199, -99}, {199, 99}, {0, 0}, {101, -51}}) {
        CHECK(std::any_of(tiles.begin(), tiles.end(), [&](const auto& t) {
            return point.x >= t.local.west && point.x <= t.local.east && point.y >= t.local.south &&
                   point.y <= t.local.north;
        }));
    }
    std::set<std::string> keys;
    for (const auto& t : tiles) keys.insert(t.key);
    CHECK(keys.size() < tiles.size());
    const auto again = ground_tiles(torus, {1, 1}, {-200.01, -100.01, 199.99, 99.99}, 4, 42);
    CHECK(again.size() == tiles.size());
    for (std::size_t i = 0; i < std::min(again.size(), tiles.size()); ++i) {
        CHECK(again[i].key == tiles[i].key);
        CHECK(again[i].details == tiles[i].details);
        CHECK(again[i].local.west + .01 == doctest::Approx(tiles[i].local.west));
    }
}

// checks: PRE-03/PLT-07: saved raster phase retains exact pixel placement after complete torus traversals.
TEST_CASE("saved raster origin reopens the exact view after multiple torus laps") {
    Projection p;
    p.size(1081, 2401);
    p.zoom(1. / 64 / p.density(), {540.5, 1200.5}, true);
    p.focus(6020000.25, -2030000.75);
    const auto shift = p.rebase();
    REQUIRE(shift.dx != 0);
    if (shift.dx == 0) return;
    const auto saved = p.raster_origin();
    const auto focus = p.centre();
    const auto before = p.raster(focus.x + 7.125, focus.y + 9.875, 2);
    Projection reopened;
    reopened.size(1081, 2401);
    reopened.zoom(p.density() / reopened.density(), {540.5, 1200.5}, true);
    REQUIRE(reopened.restore_raster_origin(saved));
    if (reopened.raster_origin() != saved) return;
    reopened.focus(focus.x, focus.y);
    const auto after = reopened.raster(focus.x + 7.125, focus.y + 9.875, 2);
    CHECK(after.x == before.x);
    CHECK(after.y == before.y);
    CHECK(reopened.residual().x == p.residual().x);
    CHECK(reopened.residual().y == p.residual().y);
    CHECK_FALSE(reopened.restore_raster_origin({INT64_MIN, 0}));
    CHECK_FALSE(reopened.restore_raster_origin({100000000000001LL, 0}));
    CHECK(reopened.raster_origin() == saved);
}
