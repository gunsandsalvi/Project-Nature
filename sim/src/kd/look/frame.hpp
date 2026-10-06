// The look's measures of the engine's frames (A4.8, A5.5): the target card's statistics, people's salience, the
// flicker left after following the camera's motion, the levels of a dark gradient, and FLIP. Godot-free and written
// once, for the kindling tool in the cloud and the app on the phone (rule 4), in kd::num's maths but FLIP's own.
// Lightness, colour and their differences are in hundredths of OKLab's scale, as kd/look/measures.hpp's; shares are
// in percent.
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "kd/look/colour.hpp"

namespace kd::look {

/// The target card's statistics of a frame (A5.5), read on cells of 4 by 4 pixels, each its pixels' mean colour in
/// OKLab: about 4 by 4 screen pixels on your phone, or 2 by 2 texture pixels at the closest zoom.
struct Card {
    double lightness = 0.0;       // the mean lightness: A4.3's moments
    double dark = 0.0;            // the share of cells darker than 45
    double lights_hue = 0.0;      // the brightest twentieth's hue, in degrees: golden is about 35 to 79
    double lights = 0.0;          // the lightest fifth's mean yellowness (OKLab b): warm by day is about +5 to +9
    double shade = 0.0;           // the darkest fifth's: near neutral is about -2.4 to +2.1, below it blue
    double strong_colour = 0.0;   // the share of cells whose chroma is above 15: specks only
    double green = 0.0;           // the share of green cells: hue 110 to 170 degrees, chroma above 4
    double green_chroma = 0.0;    // the green cells' median chroma: muted greens stay low
    double flat = 0.0;            // the share of squares of 6 by 6 cells of one colour: no flat ground
    double things = 0.0;          // spots 2 to 5 cells across a thousand cells, each the strongest within 2 cells
    double largest_colour = 0.0;  // the share of cells in the commonest colour
    double texture = 0.0;         // the fine texture: the spread of lightness less its blur over one cell
    double masses = 0.0;          // the big masses: the spread of lightness blurred over four cells
};

/// Implements PRE-01 PRE-20, see A5.5: the target card's statistics of a whole frame, from its whole cells; alpha is
/// not read. A frame is at least 4 pixels each way.
[[nodiscard]] Card card(const Picture& frame);

/// People's salience in a frame: each person's standing among all the frame's points.
struct Salience {
    std::vector<double> percentiles;  // each person's, in the order asked; none for a person not in the frame
    double median = 0.0;              // the median person's, 0 when no person shows
    double least = 0.0;               // the lowest person's, 0 when no person shows
};

/// Implements PRE-28, see A4.8: how much each person stands out, from the frame and its object picture (an object's
/// number for each pixel, 0 for none): each point's centre-against-surround difference (its colour blurred over 1.5
/// pixels against over 12, as an OKLab distance); a person's mean over their pixels, as the percentile of all the
/// frame's points below it by more than a billionth, the blurs' rounding.
[[nodiscard]] Salience salience(const Picture& frame, const std::vector<std::int32_t>& objects,
                                const std::vector<std::int32_t>& people);

/// Implements PRE-22, see A4.8: a frame's lightness error against its many-sample picture of the same view, pixel by
/// pixel: the frame's less the reference's.
[[nodiscard]] std::vector<double> error(const Picture& frame, const Picture& reference);

/// Implements PRE-22, see A4.8: the last frame's error moved to follow the camera's known motion: each pixel (x, y)
/// of the new frame reads the old error's nearest pixel to (m0 x + m1 y + m2, m3 x + m4 y + m5); none where that
/// falls outside the old frame.
[[nodiscard]] std::vector<std::optional<double>> follow(const std::vector<double>& error, std::int64_t width,
                                                        std::int64_t height, const std::array<double, 6>& motion);

/// Implements PRE-22, see A4.8: the share of pixels whose error changed by more than the threshold (in hundredths)
/// between two frames, the first already following the camera; pixels it could not follow are left out, and none
/// left gives 0.
[[nodiscard]] double flicker(const std::vector<std::optional<double>>& before, const std::vector<double>& after,
                             double threshold);

/// The levels a dark gradient shows on screen.
struct Levels {
    std::int64_t distinct = 0;  // the distinct colours in the region
    std::int64_t widest = 0;    // the longest run of one colour along a row, in pixels: a band
};

/// Implements PRE-30, see A4.8: the levels of a region of a dark gradient, such as a moonlit slope.
[[nodiscard]] Levels levels(const Picture& region);

/// FLIP's verdict on a frame against its reference.
struct Flip {
    double mean = 0.0;   // the mean error, from 0 to 1
    double above = 0.0;  // the share of pixels whose error is above 0.2: plainly different
};

/// Implements PRE-01, see A4.8: NVIDIA's FLIP (its vendored C++, sim/thirdparty/flip) on two pictures of one size,
/// seen from `pixels_a_degree` (about 80 for your phone held at 30 cm).
[[nodiscard]] Flip flip(const Picture& reference, const Picture& test, double pixels_a_degree);

}  // namespace kd::look
