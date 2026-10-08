#include "patches.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "kd/chance/chance.hpp"
#include "kd/num/maths.hpp"

namespace kd::view::patches {

namespace {

// The noise's octaves: each as many times finer as the ratio, each with its share of the whole.
constexpr std::array<double, 3> kRatios{1.0, 2.6, 6.0};
constexpr std::array<double, 3> kShares{0.5, 0.3, 0.2};
// How far the wear's edge strays, as a share of the whole wear, and the scale of its noise in metres.
constexpr double kEdgeStray = 0.35;
constexpr double kEdgeScale = 9.0;
// The bare patches are as worn as this at their barest: earth shows between the blades, it is not a bald field.
constexpr double kBarePatchWear = 0.5;
// Thin growth breaks into bare patches about this wide in metres (a noise of its own, the fourth draw of a lattice
// point), not one bare field the width of the whole mass.
constexpr double kBareScale = 10.0;
constexpr std::size_t kBareDraw = 3;

double smooth(double t) {
    return t * t * (3.0 - 2.0 * t);
}

// A fraction for a lattice point of an octave, the same on every machine.
double lattice(const Params& params, std::size_t octave, std::int64_t x, std::int64_t y) {
    static const chance::Name kSystem = chance::name("patches");
    static const chance::Name kPurpose = chance::name("growth");
    const chance::Draws draws(params.seed, kSystem, static_cast<std::uint64_t>(x + (std::int64_t{1} << 31)), y,
                              kPurpose);
    return draws.fraction(octave);
}

// Smooth noise from 0 to 1 over a lattice of this spacing, in metres.
double noise(const Params& params, std::size_t octave, double east, double north, double spacing) {
    const double u = east / spacing;
    const double v = north / spacing;
    const double fu = std::floor(u);
    const double fv = std::floor(v);
    const auto x = static_cast<std::int64_t>(fu);
    const auto y = static_cast<std::int64_t>(fv);
    const double tu = smooth(u - fu);
    const double tv = smooth(v - fv);
    const double bottom = lattice(params, octave, x, y) * (1.0 - tu) + lattice(params, octave, x + 1, y) * tu;
    const double top = lattice(params, octave, x, y + 1) * (1.0 - tu) + lattice(params, octave, x + 1, y + 1) * tu;
    return bottom * (1.0 - tv) + top * tv;
}

std::uint8_t byte_of(double share) {
    return static_cast<std::uint8_t>(std::lround(std::clamp(share, 0.0, 1.0) * 255.0));
}

}  // namespace

double growth_at(double east, double north, const Params& params) {
    double sum = 0.0;
    for (std::size_t i = 0; i < kRatios.size(); ++i) {
        sum += kShares[i] * noise(params, i, east, north, params.growth_scale / kRatios[i]);
    }
    // three octaves of value noise bunch round the middle: stretch them to fill 0 to 1
    return std::clamp(0.5 + (sum - 0.5) * 2.0, 0.0, 1.0);
}

Patches make(double camp_east, double camp_north, const Params& params) {
    Patches out;
    out.size = params.size;
    out.patch = params.patch;
    // the camp lies in the middle of a patch, not at the corner of four, so the worn ground peaks where the tent
    // stands and falls away from it, instead of lying flat over a square of four patches
    const double side = params.size * params.patch;
    out.west = camp_east - side / 2.0 - params.patch / 2.0;
    out.south = camp_north - side / 2.0 - params.patch / 2.0;
    out.data.assign(static_cast<std::size_t>(params.size) * static_cast<std::size_t>(params.size) * 4, 0);
    for (int row = 0; row < params.size; ++row) {
        for (int column = 0; column < params.size; ++column) {
            const double east = out.west + (column + 0.5) * params.patch;
            const double north = out.south + (row + 0.5) * params.patch;
            const double growth = growth_at(east, north, params);
            // the clearing: worn bare near the camp, leaving over the fade
            const double away = num::hypot(east - camp_east, north - camp_north);
            const double fade = std::max(params.clearing_fade, 1e-6);
            const double cleared = 1.0 - smooth(std::clamp((away - params.clearing) / fade, 0.0, 1.0));
            // bare earth where growth is thin
            const double thin_growth =
                params.bare_below > 0.0 ? std::clamp((params.bare_below - growth) / params.bare_below, 0.0, 1.0) : 0.0;
            const double patchy =
                std::clamp((noise(params, kBareDraw, east, north, kBareScale) - 0.35) / 0.3, 0.0, 1.0);
            const double thin = thin_growth * patchy;
            // the edge strays, so a clearing and a bare patch are never round
            const double stray = (noise(params, 0, east, north, kEdgeScale) - 0.5) * kEdgeStray;
            // the trodden ground round the camp, and the open dry ground where growth is thin, are two grounds
            const double wear = cleared + stray * cleared;
            const double bare = kBarePatchWear * thin + stray * thin;
            std::uint8_t* at = &out.data[(static_cast<std::size_t>(row) * static_cast<std::size_t>(params.size) +
                                          static_cast<std::size_t>(column)) *
                                         4];
            at[0] = byte_of(growth);
            at[1] = byte_of(wear);
            at[2] = byte_of(bare);
            at[3] = 255;
        }
    }
    return out;
}

}  // namespace kd::view::patches
