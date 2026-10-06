#include "kd/look/frame.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <tuple>

#include "kd/core/check.hpp"
#include "kd/look/measures.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"
#include "kd_flip.h"

namespace kd::look {

namespace {

constexpr std::int64_t kCell = 4;     // the card reads cells of 4 by 4 pixels
constexpr double kDark = 0.45;        // a cell darker than this is dark
constexpr double kStrong = 0.15;      // a chroma above this is strong colour
constexpr double kGreenFrom = 110.0;  // green hues, in degrees: past yellow
constexpr double kGreenTo = 170.0;
constexpr double kGreenChroma = 0.04;   // a cell greyer than this is not green
constexpr std::int64_t kSquare = 6;     // the flat patches' squares, in cells
constexpr double kOneColour = 0.02;     // cells this near their square's mean colour are of one colour
constexpr double kSpotSmall = 1.0;      // a spot is what a blur over this many cells keeps
constexpr double kSpotLarge = 2.5;      // and one over this many takes away: 2 to 5 cells across
constexpr double kSpot = 0.03;          // standing out by more than this lightness
constexpr std::int64_t kSpotApart = 2;  // the strongest within this many cells each way
constexpr double kColourBox = 0.02;     // the commonest colour counts cells in boxes of OKLab this wide each way
constexpr double kMasses = 4.0;         // the masses are what a blur over this many cells leaves
constexpr double kSame = 1e-9;          // salience: differences within this are the blurs' rounding, so equal

void check_whole(const Picture& picture) {
    KD_CHECK(whole(picture), "look: a frame holds four bytes for each of its pixels");
}

double share(std::int64_t part, std::int64_t all) {
    return all > 0 ? 100.0 * static_cast<double>(part) / static_cast<double>(all) : 0.0;
}

// The hue of an OKLab colour's axes, from 0 to 360 degrees.
double hue(double a, double b) {
    const double degrees = num::atan2pi(b, a) * 180.0;
    return degrees < 0.0 ? degrees + 360.0 : degrees;
}

double distance(const Lab& p, const Lab& q) {
    const double dl = p.l - q.l;
    const double da = p.a - q.a;
    const double db = p.b - q.b;
    return num::sqrt(dl * dl + da * da + db * db);
}

// A frame's whole cells, each its pixels' mean colour in OKLab, row by row.
std::vector<Lab> cells(const Picture& frame, std::int64_t across, std::int64_t down) {
    const std::vector<Lab> pixels = oklab(frame);
    std::vector<Lab> out(static_cast<std::size_t>(across * down));
    for (std::int64_t cy = 0; cy < down; ++cy) {
        for (std::int64_t cx = 0; cx < across; ++cx) {
            Lab sum;
            for (std::int64_t y = cy * kCell; y < (cy + 1) * kCell; ++y) {
                for (std::int64_t x = cx * kCell; x < (cx + 1) * kCell; ++x) {
                    const Lab& p = pixels[static_cast<std::size_t>(y * frame.width + x)];
                    sum.l += p.l;
                    sum.a += p.a;
                    sum.b += p.b;
                }
            }
            const auto n = static_cast<double>(kCell * kCell);
            out[static_cast<std::size_t>(cy * across + cx)] = {sum.l / n, sum.a / n, sum.b / n};
        }
    }
    return out;
}

// The mean yellowness of the cells at or above the lightness `from` (rising), or at or below it.
double yellowness(const std::vector<Lab>& c, double from, bool rising) {
    double sum = 0.0;
    std::int64_t n = 0;
    for (const Lab& p : c) {
        if (rising ? p.l >= from : p.l <= from) {
            sum += p.b;
            ++n;
        }
    }
    return n > 0 ? sum / static_cast<double>(n) : 0.0;
}

}  // namespace

Card card(const Picture& frame) {
    check_whole(frame);
    KD_CHECK(frame.width >= kCell && frame.height >= kCell, "look: the card reads frames 4 pixels or more each way");
    const std::int64_t across = frame.width / kCell;
    const std::int64_t down = frame.height / kCell;
    const std::vector<Lab> c = cells(frame, across, down);
    const std::vector<double> l = channel(c, &Lab::l);
    const auto n = static_cast<std::int64_t>(c.size());
    Card out;
    out.lightness = mean(l) * 100.0;

    std::int64_t dark = 0;
    std::int64_t strong = 0;
    std::vector<double> greens;
    for (const Lab& p : c) {
        const double chroma = num::hypot(p.a, p.b);
        dark += p.l < kDark ? 1 : 0;
        strong += chroma > kStrong ? 1 : 0;
        const double h = hue(p.a, p.b);
        if (chroma > kGreenChroma && h >= kGreenFrom && h <= kGreenTo) {
            greens.push_back(chroma);
        }
    }
    out.dark = share(dark, n);
    out.strong_colour = share(strong, n);
    out.green = share(static_cast<std::int64_t>(greens.size()), n);
    out.green_chroma = greens.empty() ? 0.0 : median(greens) * 100.0;

    // the lights' hue and the lights' and the shade's yellowness
    const double brightest = percentile(l, 95.0);
    double a = 0.0;
    double b = 0.0;
    for (const Lab& p : c) {
        if (p.l >= brightest) {
            a += p.a;
            b += p.b;
        }
    }
    out.lights_hue = hue(a, b);
    out.lights = yellowness(c, percentile(l, 80.0), true) * 100.0;
    out.shade = yellowness(c, percentile(l, 20.0), false) * 100.0;

    // flat patches: whole squares of cells all near their mean colour
    std::int64_t squares = 0;
    std::int64_t flat = 0;
    for (std::int64_t y = 0; y + kSquare <= down; y += kSquare) {
        for (std::int64_t x = 0; x + kSquare <= across; x += kSquare) {
            Lab sum;
            for (std::int64_t dy = 0; dy < kSquare; ++dy) {
                for (std::int64_t dx = 0; dx < kSquare; ++dx) {
                    const Lab& p = c[static_cast<std::size_t>((y + dy) * across + x + dx)];
                    sum.l += p.l;
                    sum.a += p.a;
                    sum.b += p.b;
                }
            }
            const auto m = static_cast<double>(kSquare * kSquare);
            const Lab centre{sum.l / m, sum.a / m, sum.b / m};
            bool one = true;
            for (std::int64_t dy = 0; dy < kSquare; ++dy) {
                for (std::int64_t dx = 0; dx < kSquare; ++dx) {
                    one =
                        one && distance(c[static_cast<std::size_t>((y + dy) * across + x + dx)], centre) <= kOneColour;
                }
            }
            ++squares;
            flat += one ? 1 : 0;
        }
    }
    out.flat = share(flat, squares);

    // small things: spots 2 to 5 cells across, each the strongest standing out within 2 cells, the first in reading
    // order among equals
    const std::vector<double> soft = blur(l, across, down, kSpotSmall);
    const std::vector<double> wide = blur(l, across, down, kSpotLarge);
    std::vector<double> spot(l.size());
    for (std::size_t i = 0; i < l.size(); ++i) {
        spot[i] = soft[i] > wide[i] ? soft[i] - wide[i] : wide[i] - soft[i];
    }
    std::int64_t things = 0;
    for (std::int64_t y = 0; y < down; ++y) {
        for (std::int64_t x = 0; x < across; ++x) {
            const double here = spot[static_cast<std::size_t>(y * across + x)];
            bool strongest = here > kSpot;
            for (std::int64_t ny = std::max<std::int64_t>(0, y - kSpotApart);
                 strongest && ny <= std::min(down - 1, y + kSpotApart); ++ny) {
                for (std::int64_t nx = std::max<std::int64_t>(0, x - kSpotApart);
                     nx <= std::min(across - 1, x + kSpotApart); ++nx) {
                    const double there = spot[static_cast<std::size_t>(ny * across + nx)];
                    const bool earlier = ny < y || (ny == y && nx < x);
                    strongest = strongest && (earlier ? there < here : there <= here);
                }
            }
            things += strongest ? 1 : 0;
        }
    }
    out.things = static_cast<double>(things) * 1000.0 / static_cast<double>(n);

    // the commonest colour, counting cells in boxes of OKLab
    std::map<std::tuple<std::int64_t, std::int64_t, std::int64_t>, std::int64_t> boxes;
    std::int64_t most = 0;
    for (const Lab& p : c) {
        const std::int64_t count =
            ++boxes[{num::to_int(p.l / kColourBox, num::Round::down), num::to_int(p.a / kColourBox, num::Round::down),
                     num::to_int(p.b / kColourBox, num::Round::down)}];
        most = std::max(most, count);
    }
    out.largest_colour = share(most, n);

    // the fine texture and the big masses
    std::vector<double> fine(l.size());
    for (std::size_t i = 0; i < l.size(); ++i) {
        fine[i] = l[i] - soft[i];
    }
    out.texture = spread(fine) * 100.0;
    out.masses = spread(blur(l, across, down, kMasses)) * 100.0;
    return out;
}

Salience salience(const Picture& frame, const std::vector<std::int32_t>& objects,
                  const std::vector<std::int32_t>& people) {
    check_whole(frame);
    KD_CHECK(objects.size() == static_cast<std::size_t>(frame.width * frame.height),
             "look: an object picture holds a number for each pixel");
    const std::vector<Lab> pixels = oklab(frame);
    std::vector<double> difference(pixels.size(), 0.0);
    for (double Lab::*part : {&Lab::l, &Lab::a, &Lab::b}) {
        const std::vector<double> c = channel(pixels, part);
        const std::vector<double> centre = blur(c, frame.width, frame.height, 1.5);
        const std::vector<double> surround = blur(c, frame.width, frame.height, 12.0);
        for (std::size_t i = 0; i < c.size(); ++i) {
            difference[i] += (centre[i] - surround[i]) * (centre[i] - surround[i]);
        }
    }
    for (double& d : difference) {
        d = num::sqrt(d);
    }
    std::vector<double> sorted = difference;
    std::stable_sort(sorted.begin(), sorted.end());

    Salience out;
    for (const std::int32_t person : people) {
        double sum = 0.0;
        std::int64_t count = 0;
        for (std::size_t i = 0; i < objects.size(); ++i) {
            if (objects[i] == person) {
                sum += difference[i];
                ++count;
            }
        }
        if (count == 0) {
            continue;
        }
        const double score = sum / static_cast<double>(count);
        const auto below = std::lower_bound(sorted.begin(), sorted.end(), score - kSame) - sorted.begin();
        out.percentiles.push_back(100.0 * static_cast<double>(below) / static_cast<double>(sorted.size()));
    }
    if (!out.percentiles.empty()) {
        out.median = median(out.percentiles);
        out.least = *std::min_element(out.percentiles.begin(), out.percentiles.end());
    }
    return out;
}

std::vector<double> error(const Picture& frame, const Picture& reference) {
    check_whole(frame);
    check_whole(reference);
    KD_CHECK(frame.width == reference.width && frame.height == reference.height,
             "look: a frame and its reference are one size");
    const std::vector<Lab> f = oklab(frame);
    const std::vector<Lab> r = oklab(reference);
    std::vector<double> out(f.size());
    for (std::size_t i = 0; i < f.size(); ++i) {
        out[i] = (f[i].l - r[i].l) * 100.0;
    }
    return out;
}

std::vector<std::optional<double>> follow(const std::vector<double>& error, std::int64_t width, std::int64_t height,
                                          const std::array<double, 6>& motion) {
    KD_CHECK(width > 0 && height > 0 && error.size() == static_cast<std::size_t>(width * height),
             "look: an error picture holds a value for each pixel");
    std::vector<std::optional<double>> out(error.size());
    for (std::int64_t y = 0; y < height; ++y) {
        for (std::int64_t x = 0; x < width; ++x) {
            const auto fx = static_cast<double>(x);
            const auto fy = static_cast<double>(y);
            const double sx = motion[0] * fx + motion[1] * fy + motion[2];
            const double sy = motion[3] * fx + motion[4] * fy + motion[5];
            if (!(sx > -0.5 && sy > -0.5 && sx < static_cast<double>(width) - 0.5 &&
                  sy < static_cast<double>(height) - 0.5)) {
                continue;
            }
            const std::int64_t ix = num::to_int(sx, num::Round::nearest);
            const std::int64_t iy = num::to_int(sy, num::Round::nearest);
            out[static_cast<std::size_t>(y * width + x)] = error[static_cast<std::size_t>(iy * width + ix)];
        }
    }
    return out;
}

double flicker(const std::vector<std::optional<double>>& before, const std::vector<double>& after, double threshold) {
    KD_CHECK(before.size() == after.size(), "look: two frames' errors are one size");
    KD_CHECK(threshold >= 0.0, "look: a flicker's threshold is 0 or more");
    std::int64_t read = 0;
    std::int64_t changed = 0;
    for (std::size_t i = 0; i < after.size(); ++i) {
        const std::optional<double>& was = before[i];
        if (!was) {
            continue;
        }
        ++read;
        const double d = after[i] - *was;
        changed += d > threshold || d < -threshold ? 1 : 0;
    }
    return share(changed, read);
}

Levels levels(const Picture& region) {
    check_whole(region);
    std::set<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>> colours;
    Levels out;
    for (std::int64_t y = 0; y < region.height; ++y) {
        std::int64_t run = 0;
        std::size_t last = 0;
        for (std::int64_t x = 0; x < region.width; ++x) {
            const auto i = static_cast<std::size_t>((y * region.width + x) * 4);
            colours.emplace(region.rgba[i], region.rgba[i + 1], region.rgba[i + 2]);
            const bool same = x > 0 && region.rgba[i] == region.rgba[last] &&
                              region.rgba[i + 1] == region.rgba[last + 1] &&
                              region.rgba[i + 2] == region.rgba[last + 2];
            run = same ? run + 1 : 1;
            out.widest = std::max(out.widest, run);
            last = i;
        }
    }
    out.distinct = static_cast<std::int64_t>(colours.size());
    return out;
}

Flip flip(const Picture& reference, const Picture& test, double pixels_a_degree) {
    check_whole(reference);
    check_whole(test);
    KD_CHECK(reference.width == test.width && reference.height == test.height,
             "look: FLIP compares two pictures of one size");
    KD_CHECK(pixels_a_degree > 0.0, "look: FLIP sees some pixels a degree");
    const kd_flip::Result r =
        kd_flip::evaluate(reference.rgba.data(), test.rgba.data(), reference.width, reference.height, pixels_a_degree);
    return {r.mean, r.above};
}

}  // namespace kd::look
