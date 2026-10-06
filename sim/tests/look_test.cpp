#include <cmath>
#include <cstdint>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/look/colour.hpp"
#include "kd/look/measures.hpp"
#include "kd/num/maths.hpp"

namespace look = kd::look;

namespace {

// A meadow-like pattern of greens with a yellow flower in about one pixel of 23, from one integer formula that the
// reference numpy code also uses, so both measure the same picture.
look::Picture pattern(std::int64_t width, std::int64_t height) {
    look::Picture p{width, height, {}};
    for (std::int64_t y = 0; y < height; ++y) {
        for (std::int64_t x = 0; x < width; ++x) {
            std::uint32_t v = (static_cast<std::uint32_t>(x) * 73856093U) ^ (static_cast<std::uint32_t>(y) * 19349663U);
            v *= 2654435761U;
            const bool flower = (v >> 24U) % 23U == 0U;
            p.rgba.push_back(static_cast<std::uint8_t>(flower ? 235U : 90U + v % 60U));
            p.rgba.push_back(static_cast<std::uint8_t>(flower ? 205U : 110U + (v >> 8U) % 50U));
            p.rgba.push_back(static_cast<std::uint8_t>(flower ? 70U : 40U + (v >> 16U) % 40U));
            p.rgba.push_back(static_cast<std::uint8_t>(255U - (x + y) % 7));
        }
    }
    return p;
}

look::Picture flat(std::int64_t width, std::int64_t height, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    look::Picture p{width, height, {}};
    for (std::int64_t i = 0; i < width * height; ++i) {
        p.rgba.insert(p.rgba.end(), {r, g, b, 255});
    }
    return p;
}

}  // namespace

// checks: PRE-20
TEST_CASE("OKLab gives Björn Ottosson's values for known colours") {
    const look::Lab red = look::oklab(255, 0, 0);
    CHECK(red.l == doctest::Approx(0.627955361).epsilon(1e-8));
    CHECK(red.a == doctest::Approx(0.224863061).epsilon(1e-8));
    CHECK(red.b == doctest::Approx(0.125846299).epsilon(1e-8));
    const look::Lab blue = look::oklab(0, 128, 255);
    CHECK(blue.l == doctest::Approx(0.615165359).epsilon(1e-8));
    CHECK(blue.a == doctest::Approx(-0.050648337).epsilon(1e-8));
    CHECK(blue.b == doctest::Approx(-0.204644469).epsilon(1e-8));
    const look::Lab white = look::oklab(255, 255, 255);
    CHECK(std::abs(white.l - 1.0) < 1e-7);
    CHECK(std::abs(white.a) < 1e-7);
    CHECK(std::abs(white.b) < 1e-7);
    const look::Lab black = look::oklab(0, 0, 0);
    CHECK(black.l == 0.0);
    CHECK(black.a == 0.0);
    CHECK(black.b == 0.0);
}

// checks: PRE-20
TEST_CASE("every 8-bit colour comes back from OKLab as it was") {
    int wrong = 0;
    for (unsigned r = 0; r < 256; r += 3) {
        for (unsigned g = 0; g < 256; g += 3) {
            for (unsigned b = 0; b < 256; b += 3) {
                const auto r8 = static_cast<std::uint8_t>(r);
                const auto g8 = static_cast<std::uint8_t>(g);
                const auto b8 = static_cast<std::uint8_t>(b);
                const look::Rgb back = look::srgb(look::oklab(r8, g8, b8));
                wrong += back.r != r8 || back.g != g8 || back.b != b8 ? 1 : 0;
            }
        }
    }
    CHECK(wrong == 0);
}

// The reference numbers come from a numpy reference (the accents and the colour statistics) on
// the same pattern.
// checks: PRE-20 PRE-22
TEST_CASE("the colour measures agree with the numpy reference") {
    const look::Stats s = look::stats(pattern(64, 48));
    CHECK(s.lightness == doctest::Approx(60.437577792).epsilon(1e-8));
    CHECK(s.colourfulness == doctest::Approx(10.544908198).epsilon(1e-8));
    CHECK(s.hue == doctest::Approx(117.200519279).epsilon(1e-8));
    CHECK(s.contrast == doctest::Approx(6.219906325).epsilon(1e-8));
    CHECK(s.texel_contrast == doctest::Approx(5.416931153).epsilon(1e-8));
    REQUIRE(s.accents.has_value());
    CHECK(s.accents.value_or(-1.0) == doctest::Approx(25.330732046).epsilon(1e-8));
    const look::Stats smaller = look::stats(pattern(40, 30));
    REQUIRE(smaller.accents.has_value());
    CHECK(smaller.accents.value_or(-1.0) == doctest::Approx(25.13966535).epsilon(1e-8));
    // a window of 24 needs 2 pixels of margin each side: none fits in 28 pixels
    CHECK_FALSE(look::stats(pattern(28, 28)).accents.has_value());
    CHECK(look::stats(pattern(29, 29)).accents.has_value());
}

// checks: PRE-20 PRE-22
TEST_CASE("a flat colour has its own lightness and hue, and no contrast or accents") {
    const look::Lab colour = look::oklab(120, 140, 60);
    const look::Stats s = look::stats(flat(40, 40, 120, 140, 60));
    CHECK(s.lightness == doctest::Approx(colour.l * 100.0).epsilon(1e-12));
    CHECK(s.colourfulness == doctest::Approx(kd::num::hypot(colour.a, colour.b) * 100.0).epsilon(1e-12));
    CHECK(s.hue == doctest::Approx(kd::num::atan2pi(colour.b, colour.a) * 180.0).epsilon(1e-12));
    CHECK(s.contrast < 1e-9);
    CHECK(s.texel_contrast < 1e-9);
    REQUIRE(s.accents.has_value());
    CHECK(s.accents.value_or(-1.0) >= 0.0);
    CHECK(s.accents.value_or(-1.0) < 1e-9);
    // a grey has no hue, and pure red's lies at about 29 degrees
    const look::Stats grey = look::stats(flat(4, 4, 128, 128, 128));
    CHECK(grey.hue == 0.0);
    CHECK(grey.colourfulness < 1e-5);
    CHECK(look::stats(flat(4, 4, 255, 0, 0)).hue == doctest::Approx(29.2338).epsilon(1e-5));
}

// checks: PRE-22
TEST_CASE("the blur mirrors its edges and the percentile reads between values, as numpy does") {
    const std::vector<double> channel{0.0, 1.0, 0.0, 0.0, 2.0, 0.5, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    const std::vector<double> numpy{0.316300618161, 0.350470316824, 0.562350751118, 0.912420343324, 1.049540616574,
                                    0.260588895628, 0.318112742568, 0.574339451147, 0.88367220524,  0.961877363602,
                                    0.206852072335, 0.287765159328, 0.575322316966, 0.827550786947, 0.846117229635};
    const std::vector<double> blurred = look::blur(channel, 5, 3, 1.0);
    REQUIRE(blurred.size() == numpy.size());
    for (std::size_t i = 0; i < numpy.size(); ++i) {
        CHECK(blurred[i] == doctest::Approx(numpy[i]).epsilon(1e-10));
    }
    CHECK(look::percentile({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 99.0) == doctest::Approx(9.91).epsilon(1e-12));
    CHECK(look::percentile({4.0, 1.0, 3.0, 2.0}, 50.0) == 2.5);
    CHECK(look::percentile({7.0}, 99.0) == 7.0);
}

// checks: PRE-20
TEST_CASE("a change in four numbers moves each measure by as much, and no change leaves the picture as it was") {
    const look::Picture before = pattern(64, 48);
    CHECK(look::adjust(before, {}).rgba == before.rgba);
    const look::Stats was = look::stats(before);
    const look::Picture after = look::adjust(before, {3.0, 20.0, 110.0, 90.0});
    const look::Stats is = look::stats(after);
    CHECK(is.lightness - was.lightness == doctest::Approx(3.0).epsilon(0.02));
    CHECK(is.hue - was.hue == doctest::Approx(20.0).epsilon(0.01));
    CHECK(is.colourfulness / was.colourfulness == doctest::Approx(1.1).epsilon(0.01));
    CHECK(is.contrast / was.contrast == doctest::Approx(0.9).epsilon(0.01));
    // alpha is kept
    for (std::size_t i = 3; i < before.rgba.size(); i += 4) {
        REQUIRE(after.rgba[i] == before.rgba[i]);
    }
    // a hue turned a whole turn is the hue unchanged
    CHECK(look::adjust(before, {0.0, 360.0, 100.0, 100.0}).rgba == before.rgba);
}

// checks: PRE-20
TEST_CASE("the look refuses a picture whose bytes do not match its size, and a change out of range") {
    look::Picture short_one = flat(4, 4, 1, 2, 3);
    short_one.rgba.pop_back();
    CHECK_FALSE(look::whole(short_one));
    CHECK(kd::test::stops([=] { (void)look::stats(short_one); }));
    CHECK(kd::test::stops([] { (void)look::adjust(flat(2, 2, 1, 2, 3), {0.0, 0.0, -1.0, 100.0}); }));
    CHECK_FALSE(look::whole(look::Picture{20'000, 1, std::vector<std::uint8_t>(80'000)}));
}

// A checker of single texture pixels, each texture pixel `across` by `down` screen pixels, with a blended column and
// row at every edge, as the smooth-pixel filter draws them.
look::Picture checker(std::int64_t width, std::int64_t height, double across, double down) {
    look::Picture p{width, height, {}};
    for (std::int64_t y = 0; y < height; ++y) {
        for (std::int64_t x = 0; x < width; ++x) {
            const auto tx = static_cast<std::int64_t>(static_cast<double>(x) / across);
            const auto ty = static_cast<std::int64_t>(static_cast<double>(y) / down);
            const std::uint8_t v = (tx + ty) % 2 == 0 ? 200 : 120;
            p.rgba.insert(p.rgba.end(), {v, v, v, 255});
        }
    }
    return p;
}

// checks: PRE-01 PRE-22
TEST_CASE("the texture pixel's size is read from a checker's repeat, whatever its edges' blending") {
    const look::TexelSize two = look::texel_size(checker(120, 90, 2.0, 2.0));
    CHECK(two.across == doctest::Approx(2.0).epsilon(0.02));
    CHECK(two.down == doctest::Approx(2.0).epsilon(0.02));
    CHECK(two.strength > 0.5);
    // a texture pixel 2.5 wide and 1.5 tall, as on the tilted ground, its edges falling between screen pixels
    const look::TexelSize tilted = look::texel_size(checker(200, 150, 2.5, 1.5));
    CHECK(tilted.across == doctest::Approx(2.5).epsilon(0.04));
    CHECK(tilted.down == doctest::Approx(1.5).epsilon(0.04));
    // a pattern repeating every 8 texture pixels, as the test board's dark lines do
    look::Picture lined = flat(160, 120, 200, 200, 200);
    for (std::int64_t y = 0; y < 120; ++y) {
        for (std::int64_t x = 0; x < 160; ++x) {
            if (x % 16 == 0 || y % 16 == 0) {
                const auto at = static_cast<std::size_t>((y * 160 + x) * 4);
                lined.rgba[at] = lined.rgba[at + 1] = lined.rgba[at + 2] = 30;
            }
        }
    }
    CHECK(look::texel_size(lined, 8).across == doctest::Approx(2.0).epsilon(0.02));
    CHECK(look::texel_size(lined, 8).down == doctest::Approx(2.0).epsilon(0.02));
    // a flat picture does not repeat
    CHECK(look::texel_size(flat(40, 40, 9, 9, 9)).across == 0.0);
}
