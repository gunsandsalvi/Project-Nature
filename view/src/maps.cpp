#include "maps.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "kd/num/maths.hpp"

namespace kd::view::maps {

namespace {

// A texel is under a thing from this height up, in metres.
constexpr double kFootprint = 0.03;
// How many directions openness looks in.
constexpr int kDirections = 16;
// A triangle seen edge on from above is no height at all.
constexpr double kLeastArea = 1e-9;
// How far past a triangle's edge a sample still counts as in it.
constexpr double kInside = -1e-9;

struct Place {
    double east = 0.0;
    double north = 0.0;
    double up = 0.0;
};

Place corner(const Triangle& t, std::size_t k) {
    return {t.p[3 * k], -t.p[3 * k + 2], t.p[3 * k + 1]};
}

// A texel's place in an array.
struct Grid {
    int n = 0;
    std::size_t at(int column, int row) const {
        return static_cast<std::size_t>(row) * static_cast<std::size_t>(n) + column;
    }
    bool inside(int column, int row) const { return column >= 0 && row >= 0 && column < n && row < n; }
};

// The tops: each triangle's height at the places inside it, four samples to a texel so a thin thing is not missed.
void raster(const std::vector<Triangle>& triangles, const Grid& grid, double west, double south, double texel,
            std::vector<float>& tops) {
    constexpr std::array<double, 2> kSamples{0.25, 0.75};
    for (const Triangle& t : triangles) {
        const Place a = corner(t, 0);
        const Place b = corner(t, 1);
        const Place c = corner(t, 2);
        const double area = (b.east - a.east) * (c.north - a.north) - (b.north - a.north) * (c.east - a.east);
        if (std::abs(area) < kLeastArea) {
            continue;
        }
        const double least_east = std::min({a.east, b.east, c.east});
        const double most_east = std::max({a.east, b.east, c.east});
        const double least_north = std::min({a.north, b.north, c.north});
        const double most_north = std::max({a.north, b.north, c.north});
        const int column0 = std::max(0, static_cast<int>(std::floor((least_east - west) / texel)));
        const int column1 = std::min(grid.n - 1, static_cast<int>(std::floor((most_east - west) / texel)));
        const int row0 = std::max(0, static_cast<int>(std::floor((least_north - south) / texel)));
        const int row1 = std::min(grid.n - 1, static_cast<int>(std::floor((most_north - south) / texel)));
        for (int row = row0; row <= row1; ++row) {
            for (int column = column0; column <= column1; ++column) {
                for (const double sy : kSamples) {
                    for (const double sx : kSamples) {
                        const double pe = west + (column + sx) * texel;
                        const double pn = south + (row + sy) * texel;
                        const double w1 =
                            ((pe - a.east) * (c.north - a.north) - (pn - a.north) * (c.east - a.east)) / area;
                        const double w2 =
                            ((b.east - a.east) * (pn - a.north) - (b.north - a.north) * (pe - a.east)) / area;
                        const double w0 = 1.0 - w1 - w2;
                        if (w0 >= kInside && w1 >= kInside && w2 >= kInside) {
                            const auto up = static_cast<float>(w0 * a.up + w1 * b.up + w2 * c.up);
                            float& top = tops[grid.at(column, row)];
                            top = std::max(top, up);
                        }
                    }
                }
            }
        }
    }
}

// Which texels lie within a number of texels, square, of a marked one: running sums along the rows and then the
// columns.
std::vector<std::uint8_t> dilate(const std::vector<std::uint8_t>& mask, const Grid& grid, int radius) {
    const auto n = static_cast<std::size_t>(grid.n);
    std::vector<int> along(n * n, 0);
    for (int row = 0; row < grid.n; ++row) {
        std::vector<int> sums(n + 1, 0);
        for (int column = 0; column < grid.n; ++column) {
            sums[static_cast<std::size_t>(column) + 1] =
                sums[static_cast<std::size_t>(column)] + mask[grid.at(column, row)];
        }
        for (int column = 0; column < grid.n; ++column) {
            const int lo = std::max(0, column - radius);
            const int hi = std::min(grid.n - 1, column + radius);
            along[grid.at(column, row)] = sums[static_cast<std::size_t>(hi) + 1] - sums[static_cast<std::size_t>(lo)];
        }
    }
    std::vector<std::uint8_t> out(n * n, 0);
    for (int column = 0; column < grid.n; ++column) {
        std::vector<int> sums(n + 1, 0);
        for (int row = 0; row < grid.n; ++row) {
            sums[static_cast<std::size_t>(row) + 1] = sums[static_cast<std::size_t>(row)] + along[grid.at(column, row)];
        }
        for (int row = 0; row < grid.n; ++row) {
            const int lo = std::max(0, row - radius);
            const int hi = std::min(grid.n - 1, row + radius);
            out[grid.at(column, row)] =
                sums[static_cast<std::size_t>(hi) + 1] - sums[static_cast<std::size_t>(lo)] > 0 ? 1 : 0;
        }
    }
    return out;
}

std::uint8_t byte_of(double share) {
    return static_cast<std::uint8_t>(std::lround(std::clamp(share, 0.0, 1.0) * 255.0));
}

}  // namespace

Maps make(const std::vector<Triangle>& triangles, const Sun& sun, const Params& params) {
    Maps out;
    out.texel = params.texel;
    out.shadow_reach = params.shadow_reach;
    if (triangles.empty()) {
        return out;
    }
    // what the things cover, and how high they stand
    double least_east = std::numeric_limits<double>::max();
    double most_east = std::numeric_limits<double>::lowest();
    double least_north = least_east;
    double most_north = most_east;
    double highest = 0.0;
    for (const Triangle& t : triangles) {
        for (std::size_t k = 0; k < 3; ++k) {
            const Place p = corner(t, k);
            least_east = std::min(least_east, p.east);
            most_east = std::max(most_east, p.east);
            least_north = std::min(least_north, p.north);
            most_north = std::max(most_north, p.north);
            highest = std::max(highest, p.up);
        }
    }
    // a shadow falls as far as the tallest thing's height over the sun's tangent, up to the farthest caster
    const double shadow_length = std::min(params.shadow_reach, highest / std::max(sun.rise, 0.05)) + params.texel;
    const double margin = std::max({params.open_reach, params.contact_width, shadow_length}) + 2.0 * params.texel;
    const double side = std::max(most_east - least_east, most_north - least_north) + 2.0 * margin;
    Grid grid;
    grid.n = (static_cast<int>(std::ceil(side / params.texel)) + 3) / 4 * 4;
    out.size = grid.n;
    out.west = (least_east + most_east) / 2.0 - grid.n * params.texel / 2.0;
    out.south = (least_north + most_north) / 2.0 - grid.n * params.texel / 2.0;
    const auto count = static_cast<std::size_t>(grid.n) * static_cast<std::size_t>(grid.n);
    out.tops.assign(count, 0.0F);
    raster(triangles, grid, out.west, out.south, params.texel, out.tops);
    std::vector<std::uint8_t> under(count, 0);
    for (std::size_t i = 0; i < count; ++i) {
        under[i] = static_cast<double>(out.tops[i]) > kFootprint ? 1 : 0;
        out.footprint += under[i];
    }
    out.view.assign(count * 4, 0);
    for (std::size_t i = 0; i < count; ++i) {
        out.view[4 * i] = 255;
        out.view[4 * i + 1] = 255;
    }
    const auto height = [&](int column, int row) -> double {
        return grid.inside(column, row) ? static_cast<double>(out.tops[grid.at(column, row)]) : 0.0;
    };
    const auto set = [&](int column, int row, std::size_t channel, std::uint8_t value) {
        out.view[4 * grid.at(column, row) + channel] = value;
    };

    // contact: just outside each foot, by the distance to the nearest texel under a thing
    const int foot_radius = static_cast<int>(std::ceil(params.contact_width / params.texel)) + 1;
    std::vector<float> nearest(count, std::numeric_limits<float>::max());
    for (int row = 0; row < grid.n; ++row) {
        for (int column = 0; column < grid.n; ++column) {
            if (under[grid.at(column, row)] == 0) {
                continue;
            }
            for (int dr = -foot_radius; dr <= foot_radius; ++dr) {
                for (int dc = -foot_radius; dc <= foot_radius; ++dc) {
                    if (!grid.inside(column + dc, row + dr) || under[grid.at(column + dc, row + dr)] != 0) {
                        continue;
                    }
                    const double d = std::max(num::hypot(dc, dr) * params.texel - 0.5 * params.texel, 0.0);
                    float& here = nearest[grid.at(column + dc, row + dr)];
                    here = std::min(here, static_cast<float>(d));
                }
            }
        }
    }
    for (int row = 0; row < grid.n; ++row) {
        for (int column = 0; column < grid.n; ++column) {
            const auto d = static_cast<double>(nearest[grid.at(column, row)]);
            if (under[grid.at(column, row)] == 0 && d < params.contact_width) {
                const double u = 1.0 - d / params.contact_width;
                set(column, row, 1, byte_of(1.0 - params.contact_strength * u * u * (3.0 - 2.0 * u)));
            }
        }
    }

    // openness: for the bare ground within reach of a thing, the sky left past the horizon of each of a ring of
    // directions
    const int open_steps = std::max(1, static_cast<int>(std::lround(params.open_reach / params.texel)));
    const std::vector<std::uint8_t> near_open = dilate(under, grid, open_steps);
    std::array<double, kDirections> across{};
    std::array<double, kDirections> down{};
    for (int k = 0; k < kDirections; ++k) {
        const double turn = 2.0 * (k + 0.5) / kDirections;
        across[static_cast<std::size_t>(k)] = num::cospi(turn);
        down[static_cast<std::size_t>(k)] = num::sinpi(turn);
    }
    for (int row = 0; row < grid.n; ++row) {
        for (int column = 0; column < grid.n; ++column) {
            if (near_open[grid.at(column, row)] == 0 || under[grid.at(column, row)] != 0) {
                continue;
            }
            double blocked = 0.0;
            for (std::size_t k = 0; k < kDirections; ++k) {
                double steepest = 0.0;
                for (int i = 1; i <= open_steps; ++i) {
                    const int c = column + static_cast<int>(std::lround(i * across[k]));
                    const int r = row + static_cast<int>(std::lround(i * down[k]));
                    if (!grid.inside(c, r)) {
                        break;
                    }
                    steepest = std::max(steepest, height(c, r) / (i * params.texel));
                }
                blocked += num::sinpi(num::atan2pi(steepest, 1.0));
            }
            set(column, row, 0, byte_of(1.0 - params.open_strength * blocked / kDirections));
        }
    }

    // the sun: for each texel a shadow can reach, the steepest angle up to anything between it and the sun, measured
    // from its own height, and how far off that is
    const int shadow_steps = std::max(1, static_cast<int>(std::lround(shadow_length / params.texel)));
    const std::vector<std::uint8_t> near_shadow = dilate(under, grid, shadow_steps);
    for (int row = 0; row < grid.n; ++row) {
        for (int column = 0; column < grid.n; ++column) {
            if (near_shadow[grid.at(column, row)] == 0) {
                continue;
            }
            const double own = height(column, row);
            double steepest = 0.0;
            double far = 0.0;
            for (int i = 1; i <= shadow_steps; ++i) {
                const int c = column + static_cast<int>(std::lround(i * sun.east));
                const int r = row - static_cast<int>(std::lround(i * sun.south));
                if (!grid.inside(c, r)) {
                    break;
                }
                const double slope = (height(c, r) - own) / (i * params.texel);
                if (slope > steepest) {
                    steepest = slope;
                    far = i * params.texel;
                }
            }
            if (steepest > 0.0) {
                set(column, row, 2, byte_of(2.0 * num::atan2pi(steepest, 1.0)));
                set(column, row, 3, byte_of(far / params.shadow_reach));
            }
        }
    }
    return out;
}

}  // namespace kd::view::maps
