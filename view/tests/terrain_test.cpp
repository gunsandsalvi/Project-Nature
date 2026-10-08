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
