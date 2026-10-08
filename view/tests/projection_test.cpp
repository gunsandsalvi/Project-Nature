#include "projection.hpp"
#include <cmath>
#include <string_view>
#include <tuple>
#include "doctest.h"
#include "gestures.hpp"
#include "kd/proof/fixture.hpp"

using namespace kd::view;

// checks: PRE-02, PRE-03, PRE-33 (T2.7a.1): shared endpoints, height units, fixed angle, inverse and zoom anchor.
TEST_CASE("local projection and inverse share a fixed 37 degree basis") {
    Projection p;
    p.size(1080, 2400);
    p.focus(3.125, -8.75);
    const Pixel pixel = p.project(13.125, 1.25, 2.0);
    CHECK(pixel.x == doctest::Approx(1180));
    CHECK(pixel.y == doctest::Approx(712.6130398966357));
    const Pixel ground = p.ground(pixel, 2.0);
    CHECK(ground.x == doctest::Approx(13.125));
    CHECK(ground.y == doctest::Approx(1.25));
    const Pixel anchor{241.25, 517.5};
    const Pixel before = p.ground(anchor);
    p.zoom(1.27, anchor, false);
    p.zoom(1.0, anchor, true);
    CHECK(p.ground(anchor).x == doctest::Approx(before.x));
    CHECK(p.ground(anchor).y == doctest::Approx(before.y));
    CHECK(p.presentation() == 1.0);
    CHECK(p.pixel_scale() == 2);
    p.size(2400, 1080);
    CHECK(p.centre().x == doctest::Approx(3.125));
    CHECK(p.width() == 1202);
    CHECK(p.height() == 542);
}

// checks: PRE-03, PRE-22 (T2.7a.2): fractional pan moves the whole picture, not individual sprite pixels.
TEST_CASE("local raster keeps shared endpoints and whole physical residual pixels") {
    Projection p;
    const Pixel a = p.raster(2, 3);
    p.pan(0.75, -0.6);
    const Pixel b = p.raster(2, 3);
    CHECK(b.x - a.x == std::round(b.x - a.x));
    CHECK(b.y - a.y == std::round(b.y - a.y));
    CHECK(p.residual().x == std::round(p.residual().x));
    CHECK(p.residual().y == std::round(p.residual().y));
    CHECK(p.raster(2, 3).x == b.x);
}

// checks: PRE-03, PRE-22, PRE-33, PLT-02 (T2.7a.2): only a live presentation resamples; picks undo every offset.
TEST_CASE("local pinch retains its source grid and picks undo odd window and residual offsets") {
    Projection p;
    p.size(1081, 2401);
    p.focus(1.013, -0.043);
    const Pixel anchor{333.5, 877.25};
    const Pixel before = p.ground(anchor);
    p.zoom(1.9, anchor, false);
    CHECK(p.resting_density() == 32.0);
    CHECK(p.presentation() == doctest::Approx(1.9));
    const Pixel sprite = p.raster(2.0, 1.0, 0.731);
    const Pixel offset = p.presentation_offset();
    const double factor = p.pixel_scale() * p.presentation();
    const Pixel physical{sprite.x * factor + offset.x, sprite.y * factor + offset.y};
    const Pixel inverse = p.from_screen(physical);
    CHECK(inverse.x == doctest::Approx(sprite.x));
    CHECK(inverse.y == doctest::Approx(sprite.y));
    p.zoom(1.0, anchor, true);
    CHECK(p.resting_density() == 64.0);
    CHECK(p.ground(anchor).x == doctest::Approx(before.x));
    CHECK(p.ground(anchor).y == doctest::Approx(before.y));
    CHECK(p.presentation_offset().x == std::round(p.presentation_offset().x));
    CHECK(p.presentation_offset().y == std::round(p.presentation_offset().y));
    const Pixel focus = p.centre();
    p.size(2401, 1081);
    CHECK(p.centre().x == focus.x);
    CHECK(p.centre().y == focus.y);
    CHECK(p.pixel_scale() == 2);
    p.size(720, 1600);
    CHECK(p.pixel_scale() == 1);
    p.zoom(0.01, {360, 800}, false);
    CHECK(p.presentation() == 0.5);
    CHECK(p.presentation_offset().x <= 0.0);
    CHECK(p.presentation_offset().y <= 0.0);
    CHECK(p.presentation_offset().x + p.width() * p.pixel_scale() * p.presentation() >= 720);
    CHECK(p.presentation_offset().y + p.height() * p.pixel_scale() * p.presentation() >= 1600);
}

// checks: PRE-02, PRE-03 (T2.7a.1): absolute adjacent endpoints do not accumulate rounded north/south pitch.
TEST_CASE("local shared edges do not accumulate rounded tile pitch") {
    Projection p;
    p.focus(0.213, -7.917);
    p.zoom(2.0, {540, 1200}, true);
    for (int tile = -100; tile <= 100; ++tile) {
        const double north = tile * 4.0;
        const Pixel edge = p.raster(4.0, north);
        const Pixel neighbour = p.raster(4.0, (tile - 1) * 4.0 + 4.0);
        CHECK(edge.x == neighbour.x);
        CHECK(edge.y == neighbour.y);
        const double continuous = -64.0 * Projection::kA * north;
        CHECK(std::abs(edge.y - (std::round(continuous) - std::round(64.0 * Projection::kA * 7.917) + 601)) < 0.001);
    }
}

// checks: PRE-02, PRE-33 (T2.7a.2): both input orders share pinch/drag; local touches never rotate the world.
TEST_CASE("local gestures drag and pinch without a twist route") {
    kd::view::Gestures g(false);
    g.press(0, 0, 0, 0);
    g.press(1, 100, 0, 0);
    g.move(0, 30, -100, 0.1);
    g.move(1, 30, 100, 0.1);
    const auto moved = g.take();
    CHECK(moved.twist == 0.0);
    CHECK(moved.pan_x == -20.0);
    CHECK(moved.scale == 2.0);
    g.lift(0, 30, -100, 0.2);
    g.lift(1, 30, 100, 0.2);
    CHECK(g.take().lifted);
}

// checks: WLD-13, TIM-17, PRE-02 (T2.7a.1): an owned snapshot survives slot recycling and a cut across the seam.
TEST_CASE("local display owns its snapshot and samples interrupted seam activities") {
    kd::data::Catalogue catalogue;
    REQUIRE(catalogue.load(kd::proof::fixture_files()).empty());
    kd::demo::CrowdWorld crowd(7, catalogue, 1);
    kd::view::CrowdStepper stepper(crowd);
    const auto torus = kd::world::World::kTorus;
    const auto origin = torus.wrap(-200, -100);
    kd::world::Activity old{1, 0, 10, origin, torus.wrap(200, 100)};
    const auto interrupted = old.at(torus, 4);
    kd::world::Activity next{1, 4, 20, interrupted, torus.wrap(800, 500)};
    kd::view::Snapshot fixture;
    fixture.frontier = 20;
    fixture.walkers = {{77, 2, 0}};
    fixture.ways = {old, next};
    fixture.first = {0, 2};
    stepper.snapshots().back() = fixture;
    stepper.snapshots().publish();
    DisplaySnapshot display;
    display.acquire(stepper);
    const auto records = display.sample(torus, origin, 2.5);
    REQUIRE(records.size() == 1);
    CHECK(records[0].id == 77);
    CHECK(records[0].east == doctest::Approx(1.0));
    CHECK(records[0].north == doctest::Approx(0.5));
    CHECK(records[0].appearance == 2);
    CHECK(records[0].activity == 1);
    CHECK(records[0].second == 2.5);
    CHECK(records[0].phase == 0.5);
    // Publish enough times to recycle every buffer slot. The display's owned copy cannot change under it.
    fixture.walkers[0].id = 88;
    for (int i = 0; i < 7; ++i) {
        stepper.snapshots().back() = fixture;
        stepper.snapshots().publish();
    }
    CHECK(display.sample(torus, origin, 2.5)[0].id == 77);
    const auto cut = display.sample(torus, origin, 6.25);
    double east = 0.0;
    double north = 0.0;
    kd::view::place(torus, next, 6.25, origin, east, north);
    CHECK(cut[0].east == east / 100.0);
    CHECK(cut[0].north == north / 100.0);
    CHECK(cut[0].revision == records[0].revision);
    display.acquire(stepper);
    const auto latest = display.sample(torus, origin, 6.25);
    CHECK(latest[0].id == 88);
    CHECK(latest[0].revision > records[0].revision);
    CHECK(latest[0].epoch == records[0].epoch);
    DisplaySnapshot reopened;
    stepper.snapshots().back() = fixture;
    stepper.snapshots().publish();
    reopened.acquire(stepper);
    CHECK(reopened.sample(torus, origin, 6.25)[0].epoch != records[0].epoch);
}

// checks: WLD-13, TIM-17, PRE-02 (T2.7a.1): a resting/sleeping activity has no direction to normalise.
TEST_CASE("local stationary records have a safe facing") {
    kd::data::Catalogue catalogue;
    REQUIRE(catalogue.load(kd::proof::fixture_files()).empty());
    kd::demo::CrowdWorld crowd(7, catalogue, 1);
    kd::view::CrowdStepper stepper(crowd);
    kd::view::Snapshot fixture;
    fixture.walkers = {{77, 2, 0}};
    fixture.ways = {{0, 0, 20, {12, 34}, {12, 34}}};
    fixture.first = {0, 1};
    stepper.snapshots().back() = fixture;
    stepper.snapshots().publish();
    DisplaySnapshot display;
    display.acquire(stepper);
    const auto record = display.sample(kd::world::World::kTorus, {12, 34}, 2.5);
    REQUIRE(record.size() == 1);
    CHECK(record[0].facing == 0);
    CHECK(record[0].activity == 0);
    CHECK(record[0].east == 0.0);
    CHECK(record[0].north == 0.0);
}

// checks: PRE-03 PRE-22 PRE-28 (T2.9a.1): every intermediate raster stop reaches the whole world.
TEST_CASE("navigation reaches every power of two through the whole world without losing its anchor") {
    Projection p;
    p.size(1081, 2401);
    const Pixel anchor{133.25, 1701.5};
    const Pixel before = p.ground(anchor);
    p.zoom(2.0, anchor, true);
    for (int power = 6; power >= -12; --power) {
        const double density = std::ldexp(1.0, power);
        p.zoom(density / p.density(), anchor, true);
        CHECK(p.resting_density() == density);
        CHECK(p.presentation() == 1.0);
        CHECK(p.ground(anchor).x == doctest::Approx(before.x));
        CHECK(p.ground(anchor).y == doctest::Approx(before.y));
        CHECK(p.width() == 543);
        CHECK(p.height() == 1203);
    }
    p.zoom(0.0001, anchor, true);
    CHECK(p.resting_density() == std::ldexp(1.0, -12));
    p.zoom(1e12, anchor, true);
    CHECK(p.resting_density() == 64.0);
}

// checks: PRE-22 PRE-28 (T2.9a.1): source selection never treats independent families as consecutive mip levels.
TEST_CASE("camera densities select independent families and integral authored reductions") {
    Projection p;
    const Pixel anchor{540, 1200};
    for (const auto& [density, family, level] : std::vector<std::tuple<double, int, int>>{
             {64, 64, 0}, {32, 64, 1}, {16, 16, 0}, {8, 16, 1}, {4, 4, 0}, {2, 4, 1}}) {
        p.zoom(density / p.density(), anchor, true);
        CHECK(p.source().density == family);
        CHECK(p.source().level == level);
        CHECK(std::string_view(p.source().form) == "individual");
    }
    p.zoom(0.5, anchor, true);
    CHECK(p.source().density == 0);
    CHECK(p.source().level == -1);
    CHECK(std::string_view(p.source().form) == "tiny");
    p.zoom(1.0 / 64.0, anchor, true);
    CHECK(std::string_view(p.source().form) == "group");
    p.zoom(1.0 / 64.0, anchor, true);
    CHECK(std::string_view(p.source().form) == "overview-fixture");
}

// checks: PRE-03 PRE-33 (T2.9a.2): projected culling includes off-screen tall bodies and shadow casters.
TEST_CASE("projected footprint covers the visible ground height overscan and directional shadow reach") {
    Projection p;
    p.size(1080, 2400);
    p.focus(31, -17);
    const auto plain = p.footprint(0, 0, 0, 0);
    const auto cover = p.footprint(30, 32, 40, -20);
    for (const Pixel corner : {Pixel{0, 0}, Pixel{1080, 2400}, Pixel{0, 2400}, Pixel{1080, 0}}) {
        const auto ground = p.ground(corner);
        const auto high = p.ground(corner, 30);
        CHECK(plain.west <= ground.x);
        CHECK(plain.east >= ground.x);
        CHECK(plain.south <= ground.y);
        CHECK(plain.north >= ground.y);
        CHECK(cover.west <= high.x - 40);
        CHECK(cover.east >= high.x);
        CHECK(cover.south <= high.y);
        CHECK(cover.north >= ground.y + 20);
    }
    CHECK(cover.west < plain.west);
    CHECK(cover.south < plain.south);
    CHECK(cover.east > plain.east);
    CHECK(cover.north > plain.north);
}

// checks: PRE-03 PRE-33 (T2.9a.1): gesture release moves continuously while the resting image stays integral.
TEST_CASE("zoom release settles continuously about its anchor and new input cancels it") {
    Projection p;
    const Pixel anchor{177.25, 1001.5};
    p.zoom(1.27, anchor, false);
    const Pixel before = p.ground(anchor);
    const double live = p.density();
    p.release(anchor, 0.160);
    CHECK(p.settling());
    CHECK(p.density() == live);
    CHECK(p.target_density() == 32.0);
    double previous = live;
    for (int frame = 0; frame < 10; ++frame) {
        p.advance(0.016);
        CHECK(p.density() <= previous);
        CHECK(previous / p.density() < 1.04);
        CHECK(p.ground(anchor).x == doctest::Approx(before.x));
        CHECK(p.ground(anchor).y == doctest::Approx(before.y));
        previous = p.density();
    }
    CHECK_FALSE(p.settling());
    CHECK(p.density() == 32.0);
    CHECK(p.presentation() == 1.0);
    p.zoom(1.7, anchor, false);
    p.release(anchor, 0.160);
    p.advance(0.032);
    CHECK(p.resting_density() == 32.0);
    p.pan(12, 9);
    CHECK_FALSE(p.settling());
    const double stopped = p.density();
    p.advance(1.0);
    CHECK(p.density() == stopped);
    p.release(anchor, 0.160);
    p.zoom(1.01, anchor, false);
    CHECK_FALSE(p.settling());
    p.release(anchor, 0.160);
    p.advance(1.0);
    CHECK(p.resting_density() == 64.0);
    CHECK(p.presentation() == 1.0);
}

// checks: WLD-13 PRE-03 TIM-17 (T2.9a.2): skipped packets still expose a complete owned revision manifest.
TEST_CASE("display manifests remain owned and include unchanged identities after skipped packets") {
    kd::data::Catalogue catalogue;
    REQUIRE(catalogue.load(kd::proof::fixture_files()).empty());
    kd::demo::CrowdWorld crowd(7, catalogue, 1);
    kd::view::CrowdStepper stepper(crowd);
    kd::view::Snapshot packet;
    packet.walkers = {{77, 2, 0}, {88, 2, 0}};
    packet.ways = {{0, 0, 20, {12, 34}, {12, 34}}, {0, 0, 20, {22, 44}, {22, 44}}};
    packet.first = {0, 1, 2};
    DisplaySnapshot display;
    stepper.snapshots().back() = packet;
    stepper.snapshots().publish();
    display.acquire(stepper);
    const auto owned = display.manifest(2.5);
    CHECK(owned.epoch == display.epoch());
    CHECK(owned.revision == 1);
    CHECK(owned.second == 2.5);
    CHECK(owned.records.contains("caster"));
    CHECK(owned.records.contains("appearance"));
    CHECK(owned.records.contains("surface"));
    if (!owned.records.contains("caster")) return;
    CHECK(owned.records.at("caster").size() == 2);
    packet.walkers.pop_back();
    stepper.snapshots().back() = packet;
    stepper.snapshots().publish();
    display.acquire(stepper);
    const auto latest = display.manifest(4.0);
    CHECK(latest.revision == 2);
    CHECK(latest.records.at("caster").size() == 1);
    CHECK(latest.records.at("appearance").size() == 1);
    CHECK(owned.records.at("caster").size() == 2);
    CHECK(owned.revision == 1);
    DisplaySnapshot next_world;
    CHECK(next_world.epoch() != display.epoch());
}

TEST_CASE("a phone world pane retains the window's integral pixel scale after a dock takes space") {
    Projection p;
    p.size(1536, 864, 2);
    CHECK(p.pixel_scale() == 2);
    CHECK(p.width() == 770);
    CHECK(p.height() == 434);
    const Pixel point = p.project(3, 4, 0);
    CHECK(p.ground(point).x == doctest::Approx(3));
    CHECK(p.ground(point).y == doctest::Approx(4));
    p.size(1080, 1694, 2);
    CHECK(p.pixel_scale() == 2);
    p.size(720, 1200, 1);
    CHECK(p.pixel_scale() == 1);
}
