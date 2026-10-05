// P7's smooth noise: see noise.hpp. Pre-production code (research 00).
#include "noise.hpp"

#include <cmath>

#include "chance.hpp"
#include "draws.hpp"

namespace worldgen {

Noise::Noise(std::uint64_t seed, std::uint64_t field, int base, int octaves, double keep) {
    double amplitude = 1.0;
    double total = 0.0;
    for (int o = 0; o < octaves; ++o) {
        Octave octave;
        octave.ny = base << o;
        octave.nx = octave.ny * 2;
        octave.amplitude = amplitude;
        total += amplitude;
        const std::size_t n = static_cast<std::size_t>(octave.nx) * static_cast<std::size_t>(octave.ny);
        octave.values.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            octave.values[i] =
                (samebits::chance(seed, field, static_cast<std::uint64_t>(o), Draw::kNoise, i) * 2.0) - 1.0;
        }
        octaves_.push_back(std::move(octave));
        amplitude *= keep;
    }
    for (Octave& octave : octaves_) {
        octave.amplitude /= total;
    }
}

double Noise::at(double u, double v) const {
    double sum = 0.0;
    for (const Octave& o : octaves_) {
        const double x = u * o.nx;
        const double y = v * o.ny;
        const double fx = std::floor(x);
        const double fy = std::floor(y);
        double tx = x - fx;
        double ty = y - fy;
        tx = tx * tx * (3.0 - (2.0 * tx));
        ty = ty * ty * (3.0 - (2.0 * ty));
        int x0 = static_cast<int>(fx) % o.nx;
        int y0 = static_cast<int>(fy) % o.ny;
        x0 += x0 < 0 ? o.nx : 0;
        y0 += y0 < 0 ? o.ny : 0;
        const int x1 = x0 + 1 == o.nx ? 0 : x0 + 1;
        const int y1 = y0 + 1 == o.ny ? 0 : y0 + 1;
        const auto at = [&o](int i, int j) {
            return o
                .values[(static_cast<std::size_t>(j) * static_cast<std::size_t>(o.nx)) + static_cast<std::size_t>(i)];
        };
        const double a = at(x0, y0) + ((at(x1, y0) - at(x0, y0)) * tx);
        const double b = at(x0, y1) + ((at(x1, y1) - at(x0, y1)) * tx);
        sum += o.amplitude * (a + ((b - a) * ty));
    }
    return sum;
}

}  // namespace worldgen
