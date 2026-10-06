// Colour for the look (A4.8, A5.4): pictures as the art and the engine hold them, OKLab both ways, and the change in
// four numbers that matches one texture's colours to another's. Godot-free and written once, for the kindling tool in
// the cloud and the app on the phone (rule 4), with kd::num's maths, so both get the same bits.
#pragma once

#include <cstdint>
#include <vector>

namespace kd::look {

/// A picture as PNG files and Godot's images hold it: 8-bit sRGB with alpha, four bytes a pixel, row by row from the
/// top left.
struct Picture {
    std::int64_t width = 0;
    std::int64_t height = 0;
    std::vector<std::uint8_t> rgba;
};

/// Whether the picture holds four bytes for each of its pixels: at least one pixel, and at most 16384 each way and
/// 16,777,216 in all, which the measures' working copies keep within about 1.3 GiB.
[[nodiscard]] bool whole(const Picture& picture);

/// A colour in OKLab (Björn Ottosson's): lightness from 0 for black to 1 for white, and two axes, green to red (a)
/// and blue to yellow (b), on which distances follow how different colours look.
struct Lab {
    double l = 0.0;
    double a = 0.0;
    double b = 0.0;
};

/// Implements PRE-20, see A5.4: an sRGB colour in OKLab.
[[nodiscard]] Lab oklab(std::uint8_t r, std::uint8_t g, std::uint8_t b);

/// Implements PRE-20, see A5.4: each pixel of a whole picture in OKLab, row by row; alpha is not read.
[[nodiscard]] std::vector<Lab> oklab(const Picture& picture);

/// An 8-bit sRGB colour, as written back into a picture.
struct Rgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

/// Implements PRE-20, see A5.4: an OKLab colour in 8-bit sRGB, each channel clipped to the screen's range in linear
/// light.
[[nodiscard]] Rgb srgb(const Lab& colour);

/// A change of colour in the four numbers a texture's record keeps (A5.4): lightness added, in hundredths of OKLab's
/// (the measures' unit, kd/look/measures.hpp); hue turned, in degrees; colourfulness and contrast, in percent.
struct Change {
    double lightness = 0.0;
    double hue = 0.0;
    double colourfulness = 100.0;
    double contrast = 100.0;
};

/// Implements PRE-20, see A5.4: a whole picture with the change made in OKLab: each pixel's lightness moved from the
/// picture's mean lightness by the contrast, then raised by the lightness; its two colour axes turned by the hue and
/// scaled by the colourfulness; alpha kept.
[[nodiscard]] Picture adjust(const Picture& picture, const Change& change);

}  // namespace kd::look
