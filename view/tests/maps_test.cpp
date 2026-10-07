#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "doctest.h"
#include "maps.hpp"

namespace maps = kd::view::maps;

namespace {

// A box on the ground, from (east0, north0) to (east1, north1) and height up, as triangles: its top as two and each
// side as two (the sides stand edge on from above).
std::vector<maps::Triangle> box(double east0, double north0, double east1, double north1, double height) {
    const auto point = [](double e, double n, double y) { return std::array<double, 3>{e, y, -n}; };
    const auto tri = [](std::array<double, 3> a, std::array<double, 3> b, std::array<double, 3> c) {
        maps::Triangle t;
        for (std::size_t k = 0; k < 3; ++k) {
            t.p[k] = a[k];
            t.p[3 + k] = b[k];
            t.p[6 + k] = c[k];
        }
        return t;
    };
    std::vector<maps::Triangle> out;
    const std::array<std::array<double, 2>, 4> corners{
        {{east0, north0}, {east1, north0}, {east1, north1}, {east0, north1}}};
    out.push_back(tri(point(east0, north0, height), point(east1, north0, height), point(east1, north1, height)));
    out.push_back(tri(point(east0, north0, height), point(east1, north1, height), point(east0, north1, height)));
    for (std::size_t k = 0; k < 4; ++k) {
        const auto& a = corners[k];
        const auto& b = corners[(k + 1) % 4];
        out.push_back(tri(point(a[0], a[1], 0.0), point(b[0], b[1], 0.0), point(b[0], b[1], height)));
        out.push_back(tri(point(a[0], a[1], 0.0), point(b[0], b[1], height), point(a[0], a[1], height)));
    }
    return out;
}

// The sun due east at an angle above the horizon, in degrees.
maps::Sun sun_in_the_east(double degrees) {
    maps::Sun sun;
    sun.east = 1.0;
    sun.south = 0.0;
    sun.rise = std::tan(degrees * 3.14159265358979323846 / 180.0);
    return sun;
}

// A channel of the texel at a place, in bytes: 0 openness, 1 contact, 2 sun angle, 3 distance.
int channel(const maps::Maps& m, double east, double north, std::size_t which) {
    const int column = static_cast<int>(std::floor((east - m.west) / m.texel));
    const int row = static_cast<int>(std::floor((north - m.south) / m.texel));
    REQUIRE(column >= 0);
    REQUIRE(row >= 0);
    REQUIRE(column < m.size);
    REQUIRE(row < m.size);
    return m.view[4 * (static_cast<std::size_t>(row) * static_cast<std::size_t>(m.size) +
                       static_cast<std::size_t>(column)) +
                  which];
}

float top_at(const maps::Maps& m, double east, double north) {
    const int column = static_cast<int>(std::floor((east - m.west) / m.texel));
    const int row = static_cast<int>(std::floor((north - m.south) / m.texel));
    return m.tops[static_cast<std::size_t>(row) * static_cast<std::size_t>(m.size) + static_cast<std::size_t>(column)];
}

}  // namespace

// checks: PRE-21 PRE-24 PRE-30
TEST_CASE("no triangles give no maps") {
    const maps::Maps m = maps::make({}, sun_in_the_east(30.0), maps::Params{});
    CHECK(m.size == 0);
    CHECK(m.view.empty());
    CHECK(m.tops.empty());
}

// checks: PRE-24
TEST_CASE("a box's top is the height of every texel under it and the ground has none") {
    const maps::Maps m = maps::make(box(-1.0, -1.0, 1.0, 1.0, 1.0), sun_in_the_east(30.0), maps::Params{});
    REQUIRE(m.size > 0);
    CHECK(m.size % 4 == 0);
    CHECK(top_at(m, 0.0, 0.0) == doctest::Approx(1.0));
    CHECK(top_at(m, 0.9, -0.9) == doctest::Approx(1.0));
    CHECK(top_at(m, 1.3, 0.0) == doctest::Approx(0.0));
    // two metres by two, in texels of 6 cm, within the texels its edge cuts
    const double texels = 2.0 / m.texel * (2.0 / m.texel);
    CHECK(m.footprint == doctest::Approx(texels).epsilon(0.06));
    // the square covers the box and what it reaches: the border is free of it
    for (int i = 0; i < m.size; ++i) {
        const std::array<std::size_t, 4> edges{
            static_cast<std::size_t>(i),
            static_cast<std::size_t>(m.size - 1) * static_cast<std::size_t>(m.size) + static_cast<std::size_t>(i),
            static_cast<std::size_t>(i) * static_cast<std::size_t>(m.size),
            static_cast<std::size_t>(i) * static_cast<std::size_t>(m.size) + static_cast<std::size_t>(m.size - 1)};
        for (const std::size_t at : edges) {
            CHECK(m.view[4 * at] == 255);
            CHECK(m.view[4 * at + 1] == 255);
            CHECK(m.view[4 * at + 2] == 0);
            CHECK(m.view[4 * at + 3] == 0);
        }
    }
}

// checks: PRE-21
TEST_CASE("contact darkens the ground just outside a foot, softly, and not under the thing nor far from it") {
    const maps::Params p;
    const maps::Maps m = maps::make(box(-1.0, -1.0, 1.0, 1.0, 1.0), sun_in_the_east(30.0), p);
    // under the box: left alone, so the thing's own surface is not darkened by its foot
    CHECK(channel(m, 0.0, 0.0, 1) == 255);
    // a texel beyond the edge is the darkest the ground gets, about 1 less the strength
    const int near = channel(m, 1.04, 0.0, 1);
    CHECK(near < 255 * 0.6);
    CHECK(near > 255 * 0.3);
    // and it lightens with distance from the foot to nothing at the width
    const int middle = channel(m, 1.12, 0.0, 1);
    CHECK(middle > near);
    CHECK(middle < 255);
    CHECK(channel(m, 1.0 + p.contact_width + 0.05, 0.0, 1) == 255);
    CHECK(channel(m, 2.0, 0.0, 1) == 255);
}

// checks: PRE-24
TEST_CASE("openness is least against a tall wall of a box, full beyond its reach and on its top") {
    const maps::Params p;
    const maps::Maps m = maps::make(box(-1.0, -1.0, 1.0, 1.0, 2.0), sun_in_the_east(30.0), p);
    const int hugging = channel(m, 1.06, 0.0, 0);
    const int apart = channel(m, 1.0 + 1.0, 0.0, 0);
    CHECK(hugging < apart);
    CHECK(hugging < 255 * 0.75);
    CHECK(apart < 255);
    // the farthest it looks: beyond that the sky is whole
    CHECK(channel(m, 1.0 + p.open_reach + 0.3, 0.0, 0) == 255);
    // on the top of the box nothing blocks the sky, and the thing is left alone
    CHECK(channel(m, 0.0, 0.0, 0) == 255);
    // nearer the corner there is less wall to the side, so more sky than against the middle of a side
    CHECK(channel(m, 1.06, 1.06, 0) > hugging);
}

// checks: PRE-30
TEST_CASE(
    "the sun's angle and distance tell a shadow: behind a tall box, with the box's own range, and none on the sun's "
    "side") {
    const maps::Params p;
    const maps::Maps m = maps::make(box(-1.0, -1.0, 1.0, 1.0, 2.0), sun_in_the_east(30.0), p);
    // the sun's height as a share of a right angle, in bytes: 30 degrees is a third
    const int sun_height = static_cast<int>(255.0 / 3.0);
    // a metre west of the box, the top's edge stands at 2 m over 1 m: 63 degrees, well above the sun
    CHECK(channel(m, -2.0, 0.0, 2) > sun_height + 40);
    // the shadow's length is 2 m over the tangent of 30 degrees: 3.46 m, so 3.3 m from the foot is still in it
    CHECK(channel(m, -1.0 - 3.3, 0.0, 2) >= sun_height);
    // the thing is near, so its distance share is small: a metre of the 14 m the maps hold
    CHECK(channel(m, -2.0, 0.0, 3) == doctest::Approx(255.0 / 14.0).epsilon(0.25));
    // north and south of the box's shadow, and east of it on the sun's side, the sky to the sun is clear
    CHECK(channel(m, 0.0, 2.0, 2) == 0);
    CHECK(channel(m, 0.0, -2.0, 2) == 0);
    CHECK(channel(m, 1.5, 0.0, 2) == 0);
    CHECK(channel(m, 1.5, 0.0, 3) == 0);
    // a lower sun throws a longer shadow, a higher sun a shorter
    const maps::Maps low = maps::make(box(-1.0, -1.0, 1.0, 1.0, 2.0), sun_in_the_east(15.0), p);
    const maps::Maps high = maps::make(box(-1.0, -1.0, 1.0, 1.0, 2.0), sun_in_the_east(60.0), p);
    const int third_of_15 = static_cast<int>(255.0 * 15.0 / 90.0);
    const int third_of_60 = static_cast<int>(255.0 * 60.0 / 90.0);
    CHECK(channel(low, -1.0 - 6.0, 0.0, 2) >= third_of_15);
    CHECK(channel(high, -1.0 - 1.4, 0.0, 2) < third_of_60);
}

// checks: PRE-21 PRE-24
TEST_CASE("a thing as thin as a club is not missed by the texels, and the same things give the same maps") {
    // a plank 0.9 m long and 0.1 m across, 0.1 m high
    const std::vector<maps::Triangle> plank = box(-0.45, -0.05, 0.45, 0.05, 0.1);
    const maps::Maps a = maps::make(plank, sun_in_the_east(28.0), maps::Params{});
    CHECK(a.footprint > 20);
    CHECK(top_at(a, 0.0, 0.0) == doctest::Approx(0.1).epsilon(0.01));
    const maps::Maps b = maps::make(plank, sun_in_the_east(28.0), maps::Params{});
    CHECK(a.view == b.view);
    CHECK(a.tops == b.tops);
    CHECK(a.size == b.size);
    // a plank that low casts a short shadow and darkens the ground at its foot all the same
    CHECK(channel(a, 0.0, 0.1, 1) < 255);
}
