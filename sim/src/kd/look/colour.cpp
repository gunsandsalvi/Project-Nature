#include "kd/look/colour.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "kd/core/check.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"

namespace kd::look {

namespace {

// Each 8-bit sRGB value in linear light, by sRGB's own curve.
const std::array<double, 256>& linear_table() {
    static const std::array<double, 256> table = [] {
        std::array<double, 256> t{};
        for (std::size_t i = 0; i < t.size(); ++i) {
            const double c = static_cast<double>(i) / 255.0;
            t[i] = c <= 0.04045 ? c / 12.92 : num::pow((c + 0.055) / 1.055, 2.4);
        }
        return t;
    }();
    return table;
}

// A linear-light channel as an 8-bit sRGB value, clipped to the screen's range first.
std::uint8_t encode(double linear) {
    const double c = std::clamp(linear, 0.0, 1.0);
    const double v = c <= 0.0031308 ? 12.92 * c : 1.055 * num::pow(c, 1.0 / 2.4) - 0.055;
    return static_cast<std::uint8_t>(num::to_int(std::clamp(v, 0.0, 1.0) * 255.0, num::Round::nearest));
}

}  // namespace

bool whole(const Picture& picture) {
    return picture.width > 0 && picture.height > 0 && picture.width <= 16'384 && picture.height <= 16'384 &&
           picture.width * picture.height <= 16'777'216 &&
           picture.rgba.size() == static_cast<std::size_t>(picture.width * picture.height * 4);
}

Lab oklab(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    const auto& lin = linear_table();
    const double lr = lin[r];
    const double lg = lin[g];
    const double lb = lin[b];
    const double l = num::cbrt(0.4122214708 * lr + 0.5363325363 * lg + 0.0514459929 * lb);
    const double m = num::cbrt(0.2119034982 * lr + 0.6806995451 * lg + 0.1073969566 * lb);
    const double s = num::cbrt(0.0883024619 * lr + 0.2817188376 * lg + 0.6299787005 * lb);
    return {0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s,
            1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s,
            0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s};
}

std::vector<Lab> oklab(const Picture& picture) {
    KD_CHECK(whole(picture), "look: a picture holds four bytes for each of its pixels");
    std::vector<Lab> out;
    out.reserve(picture.rgba.size() / 4);
    for (std::size_t i = 0; i < picture.rgba.size(); i += 4) {
        out.push_back(oklab(picture.rgba[i], picture.rgba[i + 1], picture.rgba[i + 2]));
    }
    return out;
}

Rgb srgb(const Lab& colour) {
    KD_CHECK(std::isfinite(colour.l) && std::isfinite(colour.a) && std::isfinite(colour.b),
             "look: an OKLab colour is finite");
    const double l = colour.l + 0.3963377774 * colour.a + 0.2158037573 * colour.b;
    const double m = colour.l - 0.1055613458 * colour.a - 0.0638541728 * colour.b;
    const double s = colour.l - 0.0894841775 * colour.a - 1.2914855480 * colour.b;
    const double l3 = l * l * l;
    const double m3 = m * m * m;
    const double s3 = s * s * s;
    return {encode(4.0767416621 * l3 - 3.3077115913 * m3 + 0.2309699292 * s3),
            encode(-1.2684380046 * l3 + 2.6097574011 * m3 - 0.3413193965 * s3),
            encode(-0.0041960863 * l3 - 0.7034186147 * m3 + 1.7076147010 * s3)};
}

Picture adjust(const Picture& picture, const Change& change) {
    KD_CHECK(std::isfinite(change.lightness) && std::isfinite(change.hue) && std::isfinite(change.colourfulness) &&
                 std::isfinite(change.contrast),
             "look: a change's four numbers are finite");
    KD_CHECK(change.colourfulness >= 0.0 && change.contrast >= 0.0,
             "look: a change's colourfulness and contrast are at least 0%");
    const std::vector<Lab> lab = oklab(picture);
    double sum = 0.0;
    for (const Lab& c : lab) {
        sum += c.l;
    }
    const double mean = sum / static_cast<double>(lab.size());
    const double turn = std::fmod(change.hue, 360.0) / 180.0;  // half turns, as kd::num's sinpi and cospi take
    const double turn_cos = num::cospi(turn);
    const double turn_sin = num::sinpi(turn);
    const double contrast = change.contrast / 100.0;
    const double colourfulness = change.colourfulness / 100.0;
    const double lift = change.lightness / 100.0;
    Picture out = picture;
    for (std::size_t i = 0; i < lab.size(); ++i) {
        const Lab& c = lab[i];
        const Lab moved{mean + lift + (c.l - mean) * contrast, (turn_cos * c.a - turn_sin * c.b) * colourfulness,
                        (turn_sin * c.a + turn_cos * c.b) * colourfulness};
        const Rgb rgb = srgb(moved);
        out.rgba[i * 4] = rgb.r;
        out.rgba[i * 4 + 1] = rgb.g;
        out.rgba[i * 4 + 2] = rgb.b;
    }
    return out;
}

}  // namespace kd::look
