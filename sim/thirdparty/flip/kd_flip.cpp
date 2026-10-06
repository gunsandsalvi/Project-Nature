// The one call into NVIDIA's FLIP (README.md): each picture made linear as FLIP's own tool makes an 8-bit one, then
// FLIP's error map read for its mean and the share of pixels above 0.2.
#include "kd_flip.h"

#include <cstddef>
#include <vector>

#include <FLIP.h>

namespace kd_flip {

namespace {

constexpr float kAbove = 0.2f;  // a pixel whose error is above this differs plainly

// A picture's colours in linear light, three floats a pixel.
std::vector<float> linear(const std::uint8_t* rgba, std::int64_t pixels) {
    std::vector<float> out(static_cast<std::size_t>(pixels * 3));
    for (std::int64_t i = 0; i < pixels; ++i) {
        for (std::int64_t c = 0; c < 3; ++c) {
            const float value = static_cast<float>(rgba[i * 4 + c]) / 255.0f;
            out[static_cast<std::size_t>(i * 3 + c)] = FLIP::color3::sRGBToLinearRGB(value);
        }
    }
    return out;
}

}  // namespace

Result evaluate(const std::uint8_t* reference, const std::uint8_t* test, std::int64_t width, std::int64_t height,
                double pixels_a_degree) {
    const std::int64_t pixels = width * height;
    std::vector<float> r = linear(reference, pixels);
    std::vector<float> t = linear(test, pixels);
    FLIP::Parameters parameters;
    parameters.PPD = static_cast<float>(pixels_a_degree);
    float mean = 0.0f;
    float* map = nullptr;
    FLIP::evaluate(r.data(), t.data(), static_cast<int>(width), static_cast<int>(height), false, parameters, false,
                   true, mean, &map);
    std::int64_t above = 0;
    for (std::int64_t i = 0; i < pixels; ++i) {
        above += map[i] > kAbove ? 1 : 0;
    }
    delete[] map;
    return {static_cast<double>(mean), 100.0 * static_cast<double>(above) / static_cast<double>(pixels)};
}

}  // namespace kd_flip
