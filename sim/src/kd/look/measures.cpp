#include "kd/look/measures.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "kd/core/check.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"

namespace kd::look {

namespace {

// An index mirrored into 0 to n - 1 about the end pixels, as numpy's "reflect" pads: -1 is 1, and n is n - 2.
std::int64_t mirror(std::int64_t i, std::int64_t n) {
    if (n == 1) {
        return 0;
    }
    const std::int64_t period = 2 * (n - 1);
    std::int64_t k = i % period;
    if (k < 0) {
        k += period;
    }
    return k < n ? k : period - k;
}

// The Gaussian's weights from -r to r, r being 3 sigmas dropping the fraction and at least 1, summing to 1.
std::vector<double> kernel(double sigma) {
    const std::int64_t r = std::max<std::int64_t>(1, num::to_int(3.0 * sigma, num::Round::toward_zero));
    std::vector<double> k;
    double sum = 0.0;
    for (std::int64_t x = -r; x <= r; ++x) {
        const double t = static_cast<double>(x) / sigma;
        k.push_back(num::exp(-0.5 * t * t));
        sum += k.back();
    }
    for (double& w : k) {
        w /= sum;
    }
    return k;
}

double mean(const std::vector<double>& values) {
    double sum = 0.0;
    for (const double v : values) {
        sum += v;
    }
    return sum / static_cast<double>(values.size());
}

// The standard deviation over all the values, as numpy's default (dividing by their count).
double spread(const std::vector<double>& values) {
    const double m = mean(values);
    double sum = 0.0;
    for (const double v : values) {
        sum += (v - m) * (v - m);
    }
    return num::sqrt(sum / static_cast<double>(values.size()));
}

// The median, the mean of the middle two for an even count.
double median(std::vector<double> values) {
    std::stable_sort(values.begin(), values.end());
    const std::size_t n = values.size();
    return n % 2 == 1 ? values[n / 2] : (values[n / 2 - 1] + values[n / 2]) / 2.0;
}

std::vector<double> channel(const std::vector<Lab>& pixels, double Lab::*part) {
    std::vector<double> out;
    out.reserve(pixels.size());
    for (const Lab& c : pixels) {
        out.push_back(c.*part);
    }
    return out;
}

constexpr std::int64_t kWindow = 24;  // the accents' window, in texture pixels
constexpr std::int64_t kStep = 12;    // the step between windows
constexpr std::int64_t kInset = 2;    // the windows' distance from the edges

}  // namespace

std::vector<double> blur(const std::vector<double>& channel, std::int64_t width, std::int64_t height, double sigma) {
    KD_CHECK(width > 0 && height > 0 && channel.size() == static_cast<std::size_t>(width * height),
             "look: a channel holds one value for each pixel");
    KD_CHECK(std::isfinite(sigma) && sigma > 0.0, "look: a blur's sigma is above 0");
    const std::vector<double> k = kernel(sigma);
    const auto r = static_cast<std::int64_t>(k.size() / 2);
    // down the columns first, then along the rows, each sum in the kernel's order, as research 19's blur
    std::vector<double> down(channel.size(), 0.0);
    for (std::int64_t y = 0; y < height; ++y) {
        for (std::int64_t x = 0; x < width; ++x) {
            double acc = 0.0;
            for (std::int64_t i = 0; i <= 2 * r; ++i) {
                acc += k[static_cast<std::size_t>(i)] *
                       channel[static_cast<std::size_t>(mirror(y + i - r, height) * width + x)];
            }
            down[static_cast<std::size_t>(y * width + x)] = acc;
        }
    }
    std::vector<double> out(channel.size(), 0.0);
    for (std::int64_t y = 0; y < height; ++y) {
        for (std::int64_t x = 0; x < width; ++x) {
            double acc = 0.0;
            for (std::int64_t i = 0; i <= 2 * r; ++i) {
                acc += k[static_cast<std::size_t>(i)] *
                       down[static_cast<std::size_t>(y * width + mirror(x + i - r, width))];
            }
            out[static_cast<std::size_t>(y * width + x)] = acc;
        }
    }
    return out;
}

double percentile(std::vector<double> values, double p) {
    KD_CHECK(!values.empty(), "look: a percentile needs values");
    KD_CHECK(p >= 0.0 && p <= 100.0, "look: a percentile lies from 0 to 100");
    std::stable_sort(values.begin(), values.end());
    const double at = static_cast<double>(values.size() - 1) * p / 100.0;
    const auto below = static_cast<std::size_t>(num::to_int(at, num::Round::down));
    if (below + 1 >= values.size()) {
        return values.back();
    }
    const double part = at - static_cast<double>(below);
    return values[below] + (values[below + 1] - values[below]) * part;
}

std::optional<double> accents(const std::vector<Lab>& pixels, std::int64_t width, std::int64_t height) {
    KD_CHECK(width > 0 && height > 0 && pixels.size() == static_cast<std::size_t>(width * height),
             "look: a picture holds one colour for each pixel");
    if (width - kWindow - kInset <= kInset || height - kWindow - kInset <= kInset) {
        return std::nullopt;
    }
    const std::vector<double> l = channel(pixels, &Lab::l);
    const std::vector<double> a = channel(pixels, &Lab::a);
    const std::vector<double> b = channel(pixels, &Lab::b);
    const std::vector<double> bl = blur(l, width, height, 2.5);
    const std::vector<double> ba = blur(a, width, height, 2.5);
    const std::vector<double> bb = blur(b, width, height, 2.5);
    std::vector<double> distance(pixels.size());
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        distance[i] = num::sqrt((l[i] - bl[i]) * (l[i] - bl[i]) + (a[i] - ba[i]) * (a[i] - ba[i]) +
                                (b[i] - bb[i]) * (b[i] - bb[i]));
    }
    std::vector<double> windows;
    std::vector<double> inside;
    inside.reserve(static_cast<std::size_t>(kWindow * kWindow));
    for (std::int64_t y = kInset; y < height - kWindow - kInset; y += kStep) {
        for (std::int64_t x = kInset; x < width - kWindow - kInset; x += kStep) {
            inside.clear();
            for (std::int64_t dy = 0; dy < kWindow; ++dy) {
                for (std::int64_t dx = 0; dx < kWindow; ++dx) {
                    inside.push_back(distance[static_cast<std::size_t>((y + dy) * width + x + dx)]);
                }
            }
            windows.push_back(percentile(inside, 99.0));
        }
    }
    return median(windows) * 100.0;
}

Stats stats(const Picture& picture) {
    const std::vector<Lab> pixels = oklab(picture);
    const std::vector<double> l = channel(pixels, &Lab::l);
    std::vector<double> chroma;
    chroma.reserve(pixels.size());
    double sum_a = 0.0;
    double sum_b = 0.0;
    for (const Lab& c : pixels) {
        chroma.push_back(num::hypot(c.a, c.b));
        sum_a += c.a;
        sum_b += c.b;
    }
    Stats s;
    s.lightness = mean(l) * 100.0;
    s.colourfulness = mean(chroma) * 100.0;
    const double count = static_cast<double>(pixels.size());
    // OKLab's matrices, given to ten places, leave a grey a colourfulness of about a hundred-millionth
    if (num::hypot(sum_a / count, sum_b / count) >= 1e-6) {
        const double degrees = num::atan2pi(sum_b / count, sum_a / count) * 180.0;
        const double hue = degrees < 0.0 ? degrees + 360.0 : degrees + 0.0;  // + 0.0 makes -0 plain 0
        s.hue = hue < 360.0 ? hue : 0.0;
    }
    s.contrast = spread(l) * 100.0;
    const std::vector<double> smooth = blur(l, picture.width, picture.height, 1.0);
    std::vector<double> fine(l.size());
    for (std::size_t i = 0; i < l.size(); ++i) {
        fine[i] = l[i] - smooth[i];
    }
    s.texel_contrast = spread(fine) * 100.0;
    s.accents = accents(pixels, picture.width, picture.height);
    return s;
}

}  // namespace kd::look
