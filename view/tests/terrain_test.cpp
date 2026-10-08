#include "terrain.hpp"
#include <algorithm>
#include <cmath>
#include "doctest.h"
namespace t = kd::view::terrain;
using kd::view::Projection;

// checks: PRE-21 PRE-24 PRE-30 (T2.8a.1): separate roof/floor rays, offscreen reach and union shadows.
TEST_CASE("terrain sunlight starts at each real receiver and includes distant casters") {
    auto sun = t::light("dusk", "dry", 0, true);
    CHECK(t::reach(6, sun) == doctest::Approx(22.3923048454));
    std::vector<t::Caster> casters{{1, {{20, -1, 0}, {21, 1, 6}}, false}};
    CHECK(t::sunlight({0, 0, 0}, sun, casters) == 0);
    CHECK(t::sunlight({0, 0, 5}, sun, casters) == 1);
    auto scene = t::fixture("shelter");
    CHECK(t::sunlight({3, 3, 0}, sun, scene.casters) == 0);
    CHECK(t::sunlight({3, 3, 3}, sun, scene.casters, 21) > 0);
    casters.push_back(casters[0]);
    CHECK(t::sunlight({0, 0, 0}, sun, casters) == 0);
}
// checks: PRE-21 PRE-30 WLD-13 (T2.8a.3): local fire sees openings, and stays separate from direct sun.
TEST_CASE("shelter fire reaches its doorway but never crosses a solid wall") {
    const auto s = t::fixture("shelter");
    const t::Point fire{3, 2, 0.6};
    CHECK_FALSE(t::blocked({3, 0, 0.6}, fire, s.casters));
    CHECK(t::blocked({0, 3, 0.6}, fire, s.casters));
    CHECK(t::blocked({6, 3, 0.6}, fire, s.casters));
    CHECK(t::sunlight({3, 3, 0.6}, t::light("noon", "dry", 0, true), s.casters) == 0);
}
// checks: PRE-20 PRE-21 PRE-30 PLT-04 (T2.8a.1): static cache invalidation, bounded bytes, dynamic body contact.
TEST_CASE("terrain masks cache revisions and merge moving contacts once") {
    auto s = t::fixture("flat");
    auto sun = t::light("noon", "dry", 0, true);
    t::Masks cache;
    const auto f = s.surfaces[0];
    const auto first = cache.prepare(s, f, sun, {});
    CHECK(cache.builds() == 1);
    CHECK(cache.prepare(s, f, sun, {}).rgba == first.rgba);
    CHECK(cache.builds() == 1);
    ++sun.revision;
    CHECK(cache.prepare(s, f, sun, {}).rgba.size() == first.rgba.size());
    CHECK(cache.builds() == 2);
    ++s.revision;
    CHECK(cache.prepare(s, f, sun, {}).width == first.width);
    CHECK(cache.builds() == 3);
    std::vector<t::Caster> bodies{{99, {{-24.2, -40.2, 0}, {-23.8, -39.8, 1.6}}, false}};
    const auto one = cache.prepare(s, f, sun, bodies);
    bodies.push_back(bodies[0]);
    const auto two = cache.prepare(s, f, sun, bodies);
    CHECK(one.rgba == two.rgba);
    CHECK(one.rgba[2] >= 209);
    CHECK(one.rgba[2] < 255);
    for (int i = 0; i < 70; ++i) {
        auto copy = f;
        copy.id = 1000 + i;
        CHECK(cache.prepare(s, copy, sun, {}).width <= 65);
    }
    CHECK(cache.bytes() <= 64 * 65 * 65 * 5);
    cache.clear();
    CHECK(cache.bytes() == 0);
}
// checks: PRE-23 PRE-24 PRE-26 PRE-33 (T2.8a.2/3): sampled normals, raised picks, bed/water and cave layers.
TEST_CASE("terrain drawing picking and water use the same measured surfaces") {
    const auto s = t::fixture("slope");
    const auto p = t::walk(s, 1, 1);
    REQUIRE(p.found);
    CHECK(p.point.up == doctest::Approx(2.5));
    const auto& f =
        *std::find_if(s.surfaces.begin(), s.surfaces.end(), [&](const auto& v) { return v.id == p.surface; });
    CHECK(f.normal.north < 0);
    CHECK(f.normal.up > 0.99);
    Projection projection;
    projection.size(1080, 2400);
    projection.focus(1, 1);
    const auto pixel = projection.raster(1, 1, 2.5);
    const auto picked = t::pick(s, projection, pixel, false);
    REQUIRE(picked.found);
    CHECK(picked.point.up == doctest::Approx(2.5).epsilon(0.002));
    const auto w = t::fixture("water");
    const auto bed = t::walk(w, 0, -6);
    CHECK(bed.point.up == doctest::Approx(-0.416));
    const auto surface =
        *std::find_if(w.surfaces.begin(), w.surfaces.end(), [](const auto& v) { return v.kind == t::Kind::water; });
    CHECK(t::on(surface, 0.5, 0.25).up - bed.point.up == doctest::Approx(0.416));
    const auto cave = t::fixture("cave");
    CHECK(std::none_of(cave.surfaces.begin(), cave.surfaces.end(),
                       [](const auto& v) { return v.kind == t::Kind::roof; }));
}
// checks: PRE-24 PRE-28 PRE-33 (T2.8a.2): split pieces cross chunks, explicit receiver/roof relations and stable ties.
TEST_CASE("terrain overlap order crosses chunks with stable ties and reports a bad split") {
    std::vector<t::Piece> pieces{
        {100, 100, 0, {0, 0}, {100, 100}, -3, true, false}, {-5, 100, 0, {20, 20}, {40, 40}, -2, false, false},
        {21, 21, 100, {0, 0}, {100, 100}, -1, true, true},  {2, 0, 0, {10, 10}, {50, 50}, 2, false, false},
        {-7, 0, 0, {120, 0}, {150, 20}, 0, false, false},   {-6, 0, 0, {120, 0}, {150, 20}, 0, false, false}};
    const auto ordered = t::order(pieces);
    REQUIRE(ordered.size() == pieces.size());
    const auto index = [&](std::int64_t id) { return std::find(ordered.begin(), ordered.end(), id) - ordered.begin(); };
    CHECK(index(100) < index(-5));
    CHECK(index(-5) < index(21));
    CHECK(index(21) < index(2));
    CHECK(index(-7) < index(-6));
    // A large water sheet must stay over smaller bed chunks and below emerged bodies.
    auto water = pieces[2];
    water.id = 30;
    water.covers = 0;
    water.water = true;
    water.roof = false;
    water.depth = -100;
    water.minimum_height = 0;
    water.maximum_height = 0;
    auto bed = pieces[0];
    bed.minimum_height = -0.5;
    bed.maximum_height = -0.2;
    auto actor = pieces[1];
    actor.depth = -10;
    CHECK(t::order({water, bed, actor}) == std::vector<std::int64_t>{bed.id, water.id, actor.id});
    std::reverse(pieces.begin(), pieces.end());
    CHECK(t::order(pieces) == ordered);
    pieces = {{1, 1, 3, {0, 0}, {10, 10}, 0, false, true},
              {2, 2, 1, {0, 0}, {10, 10}, 1, false, true},
              {3, 3, 2, {0, 0}, {10, 10}, 2, false, true}};
    CHECK(t::order(pieces).empty());
}

// checks: PRE-21 PRE-30 (T2.9a.2): signed silhouettes require real finite volumes, not their enclosing boxes.
TEST_CASE("candidate cylinders cones and upper domes intersect their actual finite silhouettes") {
    using Shape = t::Caster::Shape;
    t::Caster trunk{1, {{-.125, -.125, 0}, {.125, .125, 20}}, false, Shape::cylinder};
    CHECK(t::blocked({.12, -1, 19}, {.12, 1, 19}, {trunk}));
    CHECK_FALSE(t::blocked({.12, .12, -1}, {.12, .12, 21}, {trunk}));
    CHECK_FALSE(t::blocked({0, 0, 21}, {0, 0, 22}, {trunk}));
    CHECK(t::blocked({0, 0, 10}, {0, 0, 11}, {trunk}));
    t::Caster cone{2, {{-2.1, -2.1, 0}, {2.1, 2.1, 3.1}}, false, Shape::cone};
    CHECK(t::blocked({.2, -3, 2.5}, {.2, 3, 2.5}, {cone}));
    CHECK_FALSE(t::blocked({1, -3, 2.5}, {1, 3, 2.5}, {cone}));
    CHECK(t::blocked({0, 0, 4}, {0, 0, 2}, {cone}));
    CHECK_FALSE(t::blocked({0, 0, 4}, {0, 0, 3.2}, {cone}));
    CHECK_FALSE(t::blocked({2, 2, -1}, {2, 2, 4}, {cone}));
    t::Caster dome{3, {{-1.5, -1.1, 0}, {1.5, 1.1, 2}}, false, Shape::upper_ellipsoid};
    CHECK(t::blocked({1.4, -2, .01}, {1.4, 2, .01}, {dome}));
    CHECK_FALSE(t::blocked({1.4, -2, 1.9}, {1.4, 2, 1.9}, {dome}));
    CHECK(t::blocked({0, 0, 3}, {0, 0, 1.9}, {dome}));
    CHECK_FALSE(t::blocked({0, -2, -.1}, {0, 2, -.1}, {dome}));
    dome.bounds.high.east = dome.bounds.low.east;
    CHECK_FALSE(t::blocked({-1.5, -2, 1}, {-1.5, 2, 1}, {dome}));
}

// checks: PRE-21 PRE-30 (T2.9a.2): dome and cone contacts follow grounded footprints; raised crowns never contact
// ground.
TEST_CASE("candidate contact follows elliptical footprint and independent root collar") {
    using Shape = t::Caster::Shape;
    auto at = [](t::Caster caster, t::Point p) {
        t::Scene scene;
        scene.casters = {caster};
        t::Surface receiver;
        receiver.id = 100;
        receiver.corners = {p, t::Point{p.east + .01, p.north, p.up}, t::Point{p.east + .01, p.north + .01, p.up},
                            t::Point{p.east, p.north + .01, p.up}};
        t::Masks masks;
        return masks.prepare(scene, receiver, t::light("noon", "dry", 0, false), {}).rgba[2];
    };
    t::Caster dome{3, {{-1.5, -1.1, 0}, {1.5, 1.1, 2}}, false, Shape::upper_ellipsoid};
    CHECK(at(dome, {1.4, 0, 0}) == 209);
    CHECK(at(dome, {1.45, 1.05, 0}) > 240);
    t::Caster cone{4, {{-2.1, -2.1, 0}, {2.1, 2.1, 3.1}}, false, Shape::cone};
    CHECK(at(cone, {2, 2, 0}) == 255);
    t::Caster trunk{1, {{-.125, -.125, 0}, {.125, .125, 20}}, false, Shape::cylinder, .19, .19};
    CHECK(at(trunk, {.17, 0, 0}) == 209);
    CHECK(at(trunk, {.21, 0, 0}) < 255);
    CHECK(at(trunk, {.57, 0, 0}) == 255);
    t::Caster crown{2, {{-3.5, -3.5, 6}, {3.5, 3.5, 20}}, true};
    CHECK(at(crown, {0, 0, 0}) == 255);
}

// checks: PRE-21 PRE-23 PRE-30 (T2.9a.2): source front-foot pivots and signed heights anchor candidate shadows.
TEST_CASE("candidate fixture proxy dimensions match source pivots while diagnostic doorway stays separate") {
    const auto scene = t::fixture("candidate-flat");
    REQUIRE(scene.casters.size() == 9);
    if (scene.casters.size() != 9) return;
    const auto& trunk = scene.casters[0];
    const auto& crown = scene.casters[1];
    const auto& rock = scene.casters[2];
    const auto& tent = scene.casters[3];
    CHECK(trunk.bounds.high.up == 20);
    CHECK(trunk.bounds.high.east - trunk.bounds.low.east == doctest::Approx(.25));
    double lower = 20, west = 0, east = -10;
    for (const auto& caster : scene.casters)
        if (caster.group == 1 && caster.id != 1) {
            lower = std::min(lower, caster.bounds.low.up);
            west = std::min(west, caster.bounds.low.east);
            east = std::max(east, caster.bounds.high.east);
        }
    CHECK(lower == 6);
    CHECK(crown.bounds.high.up == 20);
    CHECK(east - west == 7);
    CHECK(rock.bounds.low.north == -4);
    CHECK(rock.bounds.high.north == doctest::Approx(-1.8));
    CHECK(rock.bounds.high.up == 2);
    CHECK(tent.bounds.low.north == doctest::Approx(4.2));
    CHECK(tent.bounds.high.north == doctest::Approx(8.0));
    CHECK(tent.bounds.high.up == doctest::Approx(2.6));
    const auto shelter = t::fixture("candidate-shelter");
    CHECK(std::none_of(shelter.surfaces.begin(), shelter.surfaces.end(), [](const auto& surface) {
        return surface.kind == t::Kind::roof || surface.kind == t::Kind::wall;
    }));
    const auto water = t::fixture("candidate-water");
    CHECK(std::any_of(water.surfaces.begin(), water.surfaces.end(),
                      [](const auto& surface) { return surface.kind == t::Kind::water; }));
    CHECK(water.casters[3].bounds.low.up == doctest::Approx(t::walk(water, 3, 4).point.up));
    CHECK(t::fixture("shelter").casters.size() > 4);
}

// checks: PRE-21 PRE-30 (T2.9a.2): six source spray groups keep actual sky gaps without shrinking the canopy.
TEST_CASE("candidate birch uses six grouped sprays and retains gaps between their volumes") {
    const auto scene = t::fixture("candidate-flat");
    std::size_t tree = 0, sprays = 0;
    for (const auto& caster : scene.casters) {
        if (caster.group != 1) continue;
        ++tree;
        if (caster.id == 1) continue;
        ++sprays;
        CHECK(caster.round);
        CHECK(caster.bounds.low.up >= 6);
        CHECK(caster.bounds.high.up <= 20);
        CHECK(caster.bounds.low.east >= -8.5);
        CHECK(caster.bounds.high.east <= -1.5);
        CHECK(caster.bounds.high.north - caster.bounds.low.north <= 2);
    }
    CHECK(tree == 7);
    CHECK(sprays == 6);
    CHECK_FALSE(t::blocked({-4.6, -7, 7}, {-4.6, -7, 12}, scene.casters));
    CHECK(t::blocked({-7.05, -7, 7}, {-7.05, -7, 12}, scene.casters));
}

// checks: PRE-21/PRE-30: a narrow sparse crown occludes only its sky solid angle, unlike an opaque roof.
TEST_CASE("hemisphere sky keeps sparse canopy open while solid roof blocks its actual coverage") {
    const t::Point receiver{-5, -7.5, 0};
    const auto scene = t::fixture("candidate-flat");
    std::vector<t::Caster> crowns;
    for (const auto& c : scene.casters)
        if (c.group == 1 && c.id != 1) crowns.push_back(c);
    REQUIRE(crowns.size() == 6);
    if (crowns.size() != 6) return;
    CHECK(t::blocked(receiver, {receiver.east, receiver.north, 100}, crowns));
    const auto sparse = t::sky_visibility(receiver, crowns);
    CHECK(sparse > .8);
    CHECK(sparse < 1);
    CHECK(t::sky_visibility(receiver, {}) == 1);
    const std::vector<t::Caster> roof{{50, {{-100, -100, 2}, {100, 100, 2.2}}, false}};
    CHECK(t::sky_visibility(receiver, roof) == 0);
    auto duplicated = crowns;
    duplicated.insert(duplicated.end(), crowns.begin(), crowns.end());
    CHECK(t::sky_visibility(receiver, duplicated) == sparse);
    CHECK(t::sky_visibility(receiver, roof, 50) == 1);
    t::Surface floor;
    floor.id = 100;
    floor.corners = {receiver, t::Point{-4.99, -7.5, 0}, t::Point{-4.99, -7.49, 0}, t::Point{-5, -7.49, 0}};
    t::Scene owned;
    owned.casters = crowns;
    t::Masks masks;
    const auto mask = masks.prepare(owned, floor, t::light("noon", "dry", 0, false), {});
    CHECK(mask.rgba[1] > 200);
    CHECK(mask.rgba[1] == std::round(sparse * 255));
    CHECK(t::sky_visibility(receiver, crowns, 0, crowns) == sparse);
    t::Masks moving_cache;
    t::Scene open_scene;
    const auto moving = moving_cache.prepare(open_scene, floor, t::light("noon", "dry", 0, false), crowns);
    CHECK(moving.rgba[1] == mask.rgba[1]);
    const auto twice = moving_cache.prepare(open_scene, floor, t::light("noon", "dry", 0, false), duplicated);
    CHECK(twice.rgba == moving.rgba);
}
// checks: PRE-21/PRE-30: contact is a tight physical band, scaled to the grounded footprint, with no raised halo.
TEST_CASE("contact halo scales with grounded footprint and never makes a broad shaft or tent ring") {
    using Shape = t::Caster::Shape;
    const auto at = [](t::Caster caster, t::Point p) {
        t::Scene scene;
        scene.casters = {caster};
        t::Surface floor;
        floor.id = 100;
        floor.corners = {p, t::Point{p.east + .01, p.north, 0}, t::Point{p.east + .01, p.north + .01, 0},
                         t::Point{p.east, p.north + .01, 0}};
        t::Masks masks;
        return masks.prepare(scene, floor, t::light("noon", "dry", 0, false), {}).rgba[2];
    };
    const t::Caster small{1, {{-.1, -.1, 0}, {.1, .1, 20}}, false, Shape::cylinder};
    const t::Caster large{4, {{-2.1, -2.1, 0}, {2.1, 2.1, 3.1}}, false, Shape::cone};
    CHECK(at(small, {.16, 0, 0}) == 255);
    CHECK(at(large, {2.16, 0, 0}) < 255);
    CHECK(at(small, {.11, 0, 0}) < 255);
    CHECK(at(small, {0, 0, 0}) == 209);
    CHECK(at(large, {2.5, 0, 0}) == 255);
    auto raised = large;
    raised.bounds.low.up = 6;
    raised.bounds.high.up = 20;
    CHECK(at(raised, {0, 0, 0}) == 255);
}

// checks: PRE-21 PRE-23 PRE-30 (T2.9a.2): opaque cover dimensions exclude the taller sparse pole tips.
TEST_CASE("candidate tent opaque cover ends below pole tips and keeps the authored ring datum") {
    const auto scene = t::fixture("candidate-flat");
    const auto found =
        std::find_if(scene.casters.begin(), scene.casters.end(), [](const auto& c) { return c.id == 4; });
    REQUIRE(found != scene.casters.end());
    if (found == scene.casters.end()) return;
    const auto& cone = *found;
    CHECK(cone.bounds.low.east == doctest::Approx(1.1));
    CHECK(cone.bounds.high.east == doctest::Approx(4.9));
    CHECK(cone.bounds.low.north == doctest::Approx(4.2));
    CHECK(cone.bounds.high.north == doctest::Approx(8.0));
    CHECK(cone.bounds.high.up == doctest::Approx(2.6));
    CHECK_FALSE(t::blocked({3, 6.1, 3.2}, {3, 6.1, 2.8}, {cone}));
    CHECK_FALSE(t::blocked({5, 6.1, 3.2}, {5, 6.1, 0}, {cone}));
    CHECK(t::blocked({3, 6.1, 3.2}, {3, 6.1, 2.5}, {cone}));
    const auto water = t::fixture("candidate-water");
    CHECK(water.casters[3].bounds.low.up == doctest::Approx(t::walk(water, 3, 4).point.up));
}
