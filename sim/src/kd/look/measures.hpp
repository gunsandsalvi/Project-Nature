// The colour measures of the look's checks (A4.8, A5.4): what the art lane's checks read from every texture and
// level, and the engine's checks from its pictures, written once (rule 4). Each is in hundredths of OKLab's scale,
// so a lightness of 64.4 is OKLab's 0.644, except the hue, in degrees.
#pragma once

#include <optional>
#include <vector>

#include "kd/look/colour.hpp"

namespace kd::look {

/// A picture's colour measures.
struct Stats {
    double lightness = 0.0;         // the mean lightness
    double colourfulness = 0.0;     // the mean chroma: each pixel's distance from grey
    double hue = 0.0;               // the mean colour's hue, from 0 to 360 degrees, as red 29, yellow 110; 0 for a grey
    double contrast = 0.0;          // the lightness's spread: its standard deviation
    double texel_contrast = 0.0;    // the spread of lightness less its blur over one pixel: how far neighbours differ
    std::optional<double> accents;  // study 6's ground accents, for pictures at least 29 pixels each way
};

/// Implements PRE-20 PRE-22, see A4.8 and A5.4: a whole picture's colour measures, each pixel a texture pixel; alpha
/// is not read.
[[nodiscard]] Stats stats(const Picture& picture);

/// Implements PRE-22, see A4.8: research 19's ground accents (study 6), how strongly the most striking texture
/// pixels stand out from their surroundings: each pixel's OKLab distance from the picture blurred over 2.5 pixels;
/// the 99th percentile of those in each window of 24 by 24 pixels, the windows stepping 12 from 2 pixels inside the
/// top left edge and stopping 2 pixels short of the far edges; the median of the windows' percentiles. None when no
/// window fits. Accepted grounds score 23 to 30 and the speckled one 11 to 12.
[[nodiscard]] std::optional<double> accents(const std::vector<Lab>& pixels, std::int64_t width, std::int64_t height);

/// A separable Gaussian blur of one channel, its kernel 3 sigmas wide each side and at least 1, its edges mirrored
/// about their last pixel, as research 19 measured with (numpy's "reflect").
[[nodiscard]] std::vector<double> blur(const std::vector<double>& channel, std::int64_t width, std::int64_t height,
                                       double sigma);

/// The p-th percentile of the values, read between the two nearest by straight lines, as numpy's default does.
[[nodiscard]] double percentile(std::vector<double> values, double p);

}  // namespace kd::look
