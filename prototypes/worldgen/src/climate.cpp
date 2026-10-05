// P7's climate (A7.2 step 5, WLD-16): temperature by latitude, height and season, the wind belts, and rain with rain
// shadows by the linear model of orographic rain (Smith and Barstad 2004), then BIOME1's numbers: the coldest and
// warmest months, growing degree days and the moisture index (Prentice and others 1992). Pre-production code
// (research 00).
#include <algorithm>
#include <cmath>

#include "fft.hpp"
#include "maths.hpp"
#include "noise.hpp"
#include "world.hpp"

namespace worldgen {

namespace {

constexpr double kPi = 3.14159265358979311600e+00;
constexpr double kTwoPi = 6.28318530717958623200e+00;
constexpr double kSecondsAYear = 3.15576e7;

// The linear model's settings: moist stability, the water vapour's scale height, the times to form and fall, and the
// share of the year that storms bring rain.
constexpr double kStability = 0.005;  // 1/s
constexpr double kVapourHeight = 2500.0;
constexpr double kForm = 1000.0;  // s
constexpr double kFall = 1000.0;
constexpr double kStormShare = 0.08;

double radians(double degrees) {
    return degrees * kPi / 180.0;
}

// The sea-level mean temperature at a latitude, °C: about 27 at the equator, 14 at 45° and -25 at the poles.
double sea_level_warmth(double latitude) {
    const double c = std::max(1.0e-9, samebits::cosine(radians(latitude)));
    return -25.0 + (52.0 * samebits::power(c, 0.85));
}

// The rain that storms and the tropics' rising air bring at a latitude, before mountains and distance from the sea,
// mm a year: wet at the equator, dry near 25°, wet again near 48°, dry at the poles.
double background_rain(double latitude) {
    const double a = std::abs(latitude);
    const double tropics = latitude / 10.0;
    const double storms = (a - 48.0) / 13.0;
    return 250.0 + (1900.0 * samebits::exponent(-(tropics * tropics))) +
           (750.0 * samebits::exponent(-(storms * storms)));
}

// A belt's wind, m/s: east and north.
struct Wind {
    double east = 0.0;
    double north = 0.0;
};

// The four winds the model runs: the trades and the westerlies of each half (WLD-16); the polar easterlies use the
// trades' answer of their half.
constexpr std::array<Wind, 4> kWinds = {Wind{-7.0, -2.5}, Wind{-7.0, 2.5}, Wind{10.0, 2.0}, Wind{10.0, -2.0}};

// How much each of the four winds blows at a latitude, blending at the belts' edges near 30° and 60°.
std::array<double, 4> belts(double latitude) {
    const double a = std::abs(latitude);
    const double west = std::clamp((a - 27.0) / 6.0, 0.0, 1.0) * std::clamp((63.0 - a) / 6.0, 0.0, 1.0);
    const double east = 1.0 - west;
    const bool north = latitude >= 0.0;
    return north ? std::array<double, 4>{east, 0.0, west, 0.0} : std::array<double, 4>{0.0, east, 0.0, west};
}

Complex times(Complex a, Complex b) {
    return {(a.re * b.re) - (a.im * b.im), (a.re * b.im) + (a.im * b.re)};
}

Complex over(Complex a, Complex b) {
    const double d = (b.re * b.re) + (b.im * b.im);
    return {((a.re * b.re) + (a.im * b.im)) / d, ((a.im * b.re) - (a.re * b.im)) / d};
}

}  // namespace

void make_climate(World* w, minds::Pool* pool) {
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    const double cell_km = g.metres / 1000.0;

    // how far a wind from the east and one from the west have blown over land to reach each cell, along its row;
    // each row blends them by its belts, so nothing jumps where the belts meet
    std::vector<float> from_east(n, 0.0F);
    std::vector<float> from_west(n, 0.0F);
    pool->run(g.height, [&](int y, int /*thread*/) {
        for (int dir = -1; dir <= 1; dir += 2) {
            std::vector<float>& inland = dir > 0 ? from_west : from_east;
            double d = 5000.0;
            for (int i = 0; i < 2 * g.width; ++i) {
                const int x = dir > 0 ? i % g.width : (g.width - 1) - (i % g.width);
                const int c = g.at(x, y);
                d = w->sea(c) ? 0.0 : std::min(5000.0, d + cell_km);
                if (i >= g.width) {
                    inland[static_cast<std::size_t>(c)] = static_cast<float>(d);
                }
            }
        }
    });
    const auto easterly = [](double latitude) {
        const std::array<double, 4> b = belts(latitude);
        return b[0] + b[1];
    };

    // temperature: by latitude and height, varied a little from place to place as currents and winds vary it, the
    // seasons by tilt and by how far inland (WLD-06, WLD-16)
    const Noise currents(w->seed, 10, 2, 3, 0.5);
    w->temperature.assign(n, 0.0F);
    w->cold.assign(n, 0.0F);
    w->warm.assign(n, 0.0F);
    pool->run(g.height, [&](int y, int /*thread*/) {
        const double lat = g.latitude(y);
        const double t0 = sea_level_warmth(lat);
        const double swing_land =
            1.0 +
            (21.0 * (w->tilt / 23.44) * samebits::power(std::max(1.0e-9, samebits::sine(radians(std::abs(lat)))), 1.1));
        for (int x = 0; x < g.width; ++x) {
            const auto c = static_cast<std::size_t>(g.at(x, y));
            const double h = std::max(0.0F, w->height[c]);
            const double e = easterly(lat);
            const double inland = (e * from_east[c]) + ((1.0 - e) * from_west[c]);
            const double inner = 1.0 - samebits::exponent(-inland / 500.0);
            const double swing = (0.3 * swing_land) + (0.7 * swing_land * inner);
            const double t =
                t0 - (6.0 * h / 1000.0) + (6.0 * currents.at(g.u_of(static_cast<int>(c)), g.v_of(static_cast<int>(c))));
            w->temperature[c] = static_cast<float>(t);
            w->cold[c] = static_cast<float>(t - swing);
            w->warm[c] = static_cast<float>(t + swing);
        }
    });

    // rain over mountains: the land's heights transformed once, then each wind's answer, each row taking its belts'
    std::vector<Complex> land(n);
    for (std::size_t c = 0; c < n; ++c) {
        land[c].re = std::max(0.0F, w->height[c]);
    }
    fft2(&land, g.width, g.height, false, pool);
    std::vector<float> mountains(n, 0.0F);
    std::vector<Complex> field(n);
    for (std::size_t wind = 0; wind < kWinds.size(); ++wind) {
        const Wind wd = kWinds[wind];
        pool->run(g.height, [&](int j, int /*thread*/) {
            const int jj = j < g.height / 2 ? j : j - g.height;
            const double l = kTwoPi * jj / (g.height * g.metres);
            for (int i = 0; i < g.width; ++i) {
                const int ii = i < g.width / 2 ? i : i - g.width;
                const double k = kTwoPi * ii / (g.width * g.metres);
                const auto c =
                    (static_cast<std::size_t>(j) * static_cast<std::size_t>(g.width)) + static_cast<std::size_t>(i);
                const double sigma = (wd.east * k) + (wd.north * l);
                const double k2 = (k * k) + (l * l);
                if (std::abs(sigma) < 1.0e-12 || k2 == 0.0) {
                    field[c] = {};
                    continue;
                }
                const double s2 = sigma * sigma;
                const double n2 = kStability * kStability;
                Complex vertical;  // 1 - i m H
                if (s2 < n2) {
                    const double m = (sigma > 0.0 ? 1.0 : -1.0) * std::sqrt((n2 - s2) / s2 * k2);
                    vertical = {1.0, -m * kVapourHeight};
                } else {
                    vertical = {1.0 + (std::sqrt((s2 - n2) / s2 * k2) * kVapourHeight), 0.0};
                }
                const Complex below = times(times(vertical, {1.0, sigma * kForm}), {1.0, sigma * kFall});
                const Complex h = land[c];
                const Complex above{-sigma * h.im, sigma * h.re};  // i σ ĥ
                field[c] = over(above, below);
            }
        });
        fft2(&field, g.width, g.height, true, pool);
        pool->run(g.height, [&](int y, int /*thread*/) {
            const double share = belts(g.latitude(y))[wind];
            if (share == 0.0) {
                return;
            }
            for (int x = 0; x < g.width; ++x) {
                const auto c = static_cast<std::size_t>(g.at(x, y));
                mountains[c] += static_cast<float>(share * field[c].re);
            }
        });
    }

    // the year's rain: the belts' own, less inland, varied from place to place as storm tracks and currents vary it,
    // plus the mountains' (positive on the windward sides, negative in their shadows), the mountains' scaled by how
    // much water warm air holds
    const Noise tracks(w->seed, 9, 2, 4, 0.5);
    w->rain.assign(n, 0.0F);
    pool->run(g.height, [&](int y, int /*thread*/) {
        const double lat = g.latitude(y);
        const double belt = background_rain(lat);
        const double reach = 1200.0 + (1300.0 * samebits::exponent(-(lat / 15.0) * (lat / 15.0)));
        const double vapour = 0.005 * samebits::exponent(0.067 * (sea_level_warmth(lat) - 15.0));
        for (int x = 0; x < g.width; ++x) {
            const auto c = static_cast<std::size_t>(g.at(x, y));
            double base = belt;
            const double e = easterly(lat);
            const double wet = (e * samebits::exponent(-from_east[c] / reach)) +
                               ((1.0 - e) * samebits::exponent(-from_west[c] / reach));
            // the trades bring the warm sea's rain to the coasts they blow onto, so deserts lie inland and on the
            // coasts they blow off
            const double trades = e * std::clamp((std::abs(lat) - 8.0) / 6.0, 0.0, 1.0);
            base += 900.0 * trades * samebits::exponent(-from_east[c] / 500.0);
            const double oro = vapour * mountains[c] * kSecondsAYear * kStormShare;
            const double varied = 1.0 + (0.6 * tracks.at(g.u_of(static_cast<int>(c)), g.v_of(static_cast<int>(c))));
            w->rain[c] = static_cast<float>(std::max(30.0, wet * ((base * varied) + oro)));
        }
    });

    // BIOME1's numbers, month by month through a soil bucket of 150 mm, two years so the bucket settles
    std::array<double, 12> season{};
    for (int m = 0; m < 12; ++m) {
        season[static_cast<std::size_t>(m)] = -samebits::cosine(kTwoPi * (m + 0.5) / 12.0);
    }
    w->wetness.assign(n, 1.0F);
    w->gdd5.assign(n, 0.0F);
    pool->run(g.height, [&](int y, int /*thread*/) {
        const double half = g.latitude(y) >= 0.0 ? 1.0 : -1.0;
        for (int x = 0; x < g.width; ++x) {
            const auto c = static_cast<std::size_t>(g.at(x, y));
            const double t = w->temperature[c];
            const double swing = (w->warm[c] - w->cold[c]) / 2.0;
            const double rain = w->rain[c] / 12.0;
            double bucket = 150.0;
            double used = 0.0;
            double could = 0.0;
            double gdd = 0.0;
            for (int pass = 0; pass < 2; ++pass) {
                for (std::size_t m = 0; m < 12; ++m) {
                    const double tm = t + (half * swing * season[m]);
                    const double pet = tm > 0.0 ? (4.0 * tm) + (0.04 * tm * tm) : 0.0;
                    bucket = std::min(150.0, bucket + rain);
                    const double aet = std::min(pet, bucket);
                    bucket -= aet;
                    if (pass == 1) {
                        used += aet;
                        could += pet;
                        gdd += std::max(0.0, tm - 5.0) * 30.4;
                    }
                }
            }
            w->wetness[c] = static_cast<float>(could > 0.0 ? used / could : 1.0);
            w->gdd5[c] = static_cast<float>(gdd);
        }
    });
}

}  // namespace worldgen
