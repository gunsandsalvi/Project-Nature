#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

#include "doctest.h"
#include "kd/look/colour.hpp"
#include "kd/look/frame.hpp"
#include "kd/num/maths.hpp"

namespace look = kd::look;

namespace {

look::Picture flat(std::int64_t width, std::int64_t height, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    look::Picture p{width, height, {}};
    for (std::int64_t i = 0; i < width * height; ++i) {
        p.rgba.insert(p.rgba.end(), {r, g, b, 255});
    }
    return p;
}

void paint(look::Picture& p, std::int64_t x0, std::int64_t y0, std::int64_t w, std::int64_t h, std::uint8_t r,
           std::uint8_t g, std::uint8_t b) {
    for (std::int64_t y = y0; y < y0 + h; ++y) {
        for (std::int64_t x = x0; x < x0 + w; ++x) {
            const auto i = static_cast<std::size_t>((y * p.width + x) * 4);
            p.rgba[i] = r;
            p.rgba[i + 1] = g;
            p.rgba[i + 2] = b;
        }
    }
}

double chroma(const look::Lab& c) {
    return kd::num::hypot(c.a, c.b);
}

double hue(const look::Lab& c) {
    const double degrees = kd::num::atan2pi(c.b, c.a) * 180.0;
    return degrees < 0.0 ? degrees + 360.0 : degrees;
}

// A grainy pattern from one integer formula, every pixel different from its neighbours.
look::Picture grain(std::int64_t width, std::int64_t height, std::int64_t shift) {
    look::Picture p{width, height, {}};
    for (std::int64_t y = 0; y < height; ++y) {
        for (std::int64_t x = 0; x < width; ++x) {
            std::uint32_t v =
                (static_cast<std::uint32_t>(x + shift) * 73856093U) ^ (static_cast<std::uint32_t>(y) * 19349663U);
            v *= 2654435761U;
            const auto c = static_cast<std::uint8_t>(80U + (v >> 24U) % 96U);
            p.rgba.insert(p.rgba.end(), {c, c, c, 255});
        }
    }
    return p;
}

// FLIP's pair: a reference of three patterns and a test with a lighter square and one column moved, the pair whose
// mean FLIP's own tool (its commit in sim/thirdparty/flip/README.md) measures as 0.069951 at 80 pixels a degree.
look::Rgb flip_reference(std::int64_t x, std::int64_t y) {
    return {static_cast<std::uint8_t>(60 + (x * 5 + y * 3) % 120), static_cast<std::uint8_t>(80 + (x * x + y * 7) % 90),
            static_cast<std::uint8_t>(40 + ((x * 11) ^ (y * 13)) % 70)};
}

look::Rgb flip_test(std::int64_t x, std::int64_t y) {
    if (x == 30) {
        return flip_reference(31, y);
    }
    look::Rgb c = flip_reference(x, y);
    if (x >= 10 && x < 16 && y >= 8 && y < 14) {
        c = {static_cast<std::uint8_t>(std::min(255, c.r + 70)), static_cast<std::uint8_t>(std::min(255, c.g + 70)),
             static_cast<std::uint8_t>(std::min(255, c.b + 70))};
    }
    return c;
}

template <typename Colour>
look::Picture drawn(std::int64_t width, std::int64_t height, Colour colour) {
    look::Picture p{width, height, {}};
    for (std::int64_t y = 0; y < height; ++y) {
        for (std::int64_t x = 0; x < width; ++x) {
            const look::Rgb c = colour(x, y);
            p.rgba.insert(p.rgba.end(), {c.r, c.g, c.b, 255});
        }
    }
    return p;
}

}  // namespace

// checks: PRE-01 PRE-20
TEST_CASE("the card reads a flat grey frame as flat, without things, texture or colour") {
    const look::Card c = look::card(flat(64, 64, 128, 128, 128));
    const look::Lab grey = look::oklab(128, 128, 128);
    CHECK(c.lightness == doctest::Approx(grey.l * 100.0).epsilon(1e-12));
    CHECK(c.dark == 0.0);
    CHECK(c.lights == doctest::Approx(grey.b * 100.0).epsilon(1e-9));
    CHECK(c.shade == doctest::Approx(grey.b * 100.0).epsilon(1e-9));
    CHECK(c.strong_colour == 0.0);
    CHECK(c.green == 0.0);
    CHECK(c.green_chroma == 0.0);
    CHECK(c.flat == 100.0);
    CHECK(c.things == 0.0);
    CHECK(c.largest_colour == 100.0);
    CHECK(c.texture < 1e-9);
    CHECK(c.masses < 1e-9);
}

// checks: PRE-01 PRE-20
TEST_CASE("the card reads a checker of cells as half dark, never flat, all texture and no masses") {
    // 4-pixel cells alternating dark and light: 12 by 12 cells, so 4 squares of 6 by 6 cells
    look::Picture p = flat(48, 48, 200, 200, 200);
    for (std::int64_t cy = 0; cy < 12; ++cy) {
        for (std::int64_t cx = 0; cx < 12; ++cx) {
            if ((cx + cy) % 2 == 0) {
                paint(p, cx * 4, cy * 4, 4, 4, 40, 40, 40);
            }
        }
    }
    const look::Card c = look::card(p);
    CHECK(c.dark == 50.0);
    CHECK(c.lightness ==
          doctest::Approx((look::oklab(40, 40, 40).l + look::oklab(200, 200, 200).l) * 50.0).epsilon(1e-12));
    CHECK(c.flat == 0.0);
    CHECK(c.largest_colour == 50.0);
    CHECK(c.texture > 10.0 * c.masses);
}

// checks: PRE-20
TEST_CASE(
    "the card's lights and shade are the lightest and darkest fifths' yellowness, the lights' hue the brightest's") {
    // 8 by 10 cells: the top half warm and bright, the bottom half dark and blue
    look::Picture p = flat(32, 40, 30, 40, 70);
    paint(p, 0, 0, 32, 20, 255, 220, 150);
    const look::Card c = look::card(p);
    const look::Lab warm = look::oklab(255, 220, 150);
    const look::Lab blue = look::oklab(30, 40, 70);
    CHECK(c.dark == 50.0);
    CHECK(c.lights == doctest::Approx(warm.b * 100.0).epsilon(1e-12));
    CHECK(c.shade == doctest::Approx(blue.b * 100.0).epsilon(1e-12));
    CHECK(c.lights_hue == doctest::Approx(hue(warm)).epsilon(1e-9));
    CHECK(c.lights > 5.0);
    CHECK(c.shade < 0.0);
}

// checks: PRE-20
TEST_CASE("the card counts strong colour and greens by the cells' chroma and hue") {
    // three stripes of 4 by 4 cells: red, a muted green and grey
    look::Picture p = flat(48, 16, 128, 128, 128);
    paint(p, 0, 0, 16, 16, 255, 0, 0);
    paint(p, 16, 0, 16, 16, 90, 140, 60);
    const look::Lab red = look::oklab(255, 0, 0);
    const look::Lab green = look::oklab(90, 140, 60);
    REQUIRE(chroma(red) > 0.15);
    REQUIRE(chroma(green) > 0.04);
    REQUIRE(chroma(green) < 0.15);
    REQUIRE(hue(green) > 110.0);
    REQUIRE(hue(green) < 170.0);
    const look::Card c = look::card(p);
    CHECK(c.strong_colour == doctest::Approx(100.0 / 3.0).epsilon(1e-12));
    CHECK(c.green == doctest::Approx(100.0 / 3.0).epsilon(1e-12));
    CHECK(c.green_chroma == doctest::Approx(chroma(green) * 100.0).epsilon(1e-9));
}

// checks: PRE-01
TEST_CASE("the card counts each spot of 2 by 2 cells on flat ground as one small thing") {
    // 24 by 24 cells of grey, with two darker spots of 2 by 2 cells far apart
    look::Picture p = flat(96, 96, 128, 128, 128);
    paint(p, 20, 20, 8, 8, 60, 60, 60);
    paint(p, 64, 64, 8, 8, 60, 60, 60);
    const look::Card c = look::card(p);
    CHECK(c.things == doctest::Approx(2.0 * 1000.0 / 576.0).epsilon(1e-12));
    CHECK(c.flat < 100.0);
}

// checks: PRE-28
TEST_CASE("a person in a strong colour stands out above nearly every point, and one in the ground's colour at none") {
    // the second person is beyond the first one's surround, 3 blurs of 12 pixels
    look::Picture p = flat(160, 160, 128, 128, 128);
    paint(p, 10, 10, 4, 4, 255, 0, 0);
    std::vector<std::int32_t> objects(std::size_t{160} * 160, 0);
    for (std::int64_t y = 0; y < 4; ++y) {
        for (std::int64_t x = 0; x < 4; ++x) {
            objects[static_cast<std::size_t>((10 + y) * 160 + 10 + x)] = 1;
            objects[static_cast<std::size_t>((140 + y) * 160 + 140 + x)] = 2;
        }
    }
    const look::Salience s = look::salience(p, objects, {1, 2, 3});
    REQUIRE(s.percentiles.size() == 2);
    CHECK(s.percentiles[0] > 99.0);
    CHECK(s.percentiles[1] == 0.0);
    CHECK(s.median == doctest::Approx(s.percentiles[0] / 2.0).epsilon(1e-12));
    CHECK(s.least == 0.0);
}

// checks: PRE-22
TEST_CASE("a frame's error is its lightness less its many-sample picture's, in hundredths") {
    const std::vector<double> e = look::error(flat(2, 2, 100, 100, 100), flat(2, 2, 110, 110, 110));
    REQUIRE(e.size() == 4);
    const double expected = (look::oklab(100, 100, 100).l - look::oklab(110, 110, 110).l) * 100.0;
    for (const double v : e) {
        CHECK(v == doctest::Approx(expected).epsilon(1e-12));
    }
}

// checks: PRE-22
TEST_CASE("following the camera reads the last frame's nearest pixel, and none from outside it") {
    std::vector<double> e(12);
    for (std::size_t i = 0; i < e.size(); ++i) {
        e[i] = static_cast<double>(i);
    }
    // a pan by one pixel: each new pixel was one to the right
    const auto moved = look::follow(e, 4, 3, {1.0, 0.0, 1.0, 0.0, 1.0, 0.0});
    for (std::int64_t y = 0; y < 3; ++y) {
        for (std::int64_t x = 0; x < 4; ++x) {
            const auto& v = moved[static_cast<std::size_t>(y * 4 + x)];
            if (x < 3) {
                REQUIRE(v.has_value());
                CHECK(v.value_or(-1.0) == static_cast<double>(y * 4 + x + 1));
            } else {
                CHECK_FALSE(v.has_value());
            }
        }
    }
    // a part of a pixel reads the nearest one
    CHECK(look::follow(e, 4, 3, {1.0, 0.0, 0.4, 0.0, 1.0, 0.0})[5].value_or(-1.0) == 5.0);
    CHECK(look::follow(e, 4, 3, {1.0, 0.0, 0.6, 0.0, 1.0, 0.0})[5].value_or(-1.0) == 6.0);
}

// checks: PRE-22
TEST_CASE("flicker is the share of followed pixels whose error changed by more than the threshold") {
    const std::vector<std::optional<double>> before{0.0, 1.0, std::nullopt, 5.0};
    const std::vector<double> after{0.0, 4.5, 9.0, 5.0};
    CHECK(look::flicker(before, after, 3.0) == doctest::Approx(100.0 / 3.0).epsilon(1e-12));
    CHECK(look::flicker({std::nullopt}, {1.0}, 3.0) == 0.0);
}

// checks: PRE-22
TEST_CASE("grain moving with the camera does not flicker, and grain that stays while the camera pans does") {
    // the many-sample pictures are smooth grey; the frames carry grain, which follows the pan or stays put
    const look::Picture smooth = flat(32, 8, 128, 128, 128);
    const std::vector<double> first = look::error(grain(32, 8, 0), smooth);
    const auto followed = look::follow(first, 32, 8, {1.0, 0.0, 1.0, 0.0, 1.0, 0.0});
    CHECK(look::flicker(followed, look::error(grain(32, 8, 1), smooth), 3.0) == 0.0);
    CHECK(look::flicker(followed, look::error(grain(32, 8, 0), smooth), 3.0) > 50.0);
}

// checks: PRE-30
TEST_CASE("the levels of a dark gradient are its distinct colours and its widest band") {
    look::Picture p = flat(16, 2, 12, 12, 12);
    paint(p, 0, 0, 4, 1, 10, 10, 10);
    paint(p, 4, 0, 4, 1, 11, 11, 11);
    for (std::int64_t x = 0; x < 16; ++x) {
        const auto v = static_cast<std::uint8_t>(10 + x % 2);
        paint(p, x, 1, 1, 1, v, v, v);
    }
    const look::Levels l = look::levels(p);
    CHECK(l.distinct == 3);
    CHECK(l.widest == 8);
}

// checks: PRE-01
TEST_CASE("FLIP gives its own tool's mean on a known pair, and nothing between a picture and itself") {
    const look::Picture reference = drawn(48, 32, flip_reference);
    const look::Flip f = look::flip(reference, drawn(48, 32, flip_test), 80.0);
    CHECK(std::abs(f.mean - 0.069951) < 1e-6);
    // 167 of its 1,536 pixels, none of them within 0.0003 of the line, so every chip counts the same
    CHECK(f.above == doctest::Approx(100.0 * 167.0 / 1536.0).epsilon(1e-12));
    const look::Flip same = look::flip(reference, reference, 80.0);
    CHECK(same.mean == 0.0);
    CHECK(same.above == 0.0);
}
