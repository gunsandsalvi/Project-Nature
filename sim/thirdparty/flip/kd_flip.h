// The one call into NVIDIA's FLIP (README.md), for kd/look/frame.cpp: plain types only, so the simulation's own code
// never sees FLIP's float maths.
#pragma once

#include <cstdint>

namespace kd_flip {

/// FLIP's verdict on a test picture against its reference.
struct Result {
    double mean = 0.0;   // the mean error, from 0 to 1
    double above = 0.0;  // the share of pixels whose error is above 0.2, in percent
};

/// FLIP on two pictures of `width` by `height` pixels, 8-bit sRGB with alpha, four bytes a pixel, row by row; alpha is
/// not read. `pixels_a_degree` is how many pixels one degree of the viewer's sight spans.
[[nodiscard]] Result evaluate(const std::uint8_t* reference, const std::uint8_t* test, std::int64_t width,
                              std::int64_t height, double pixels_a_degree);

}  // namespace kd_flip
