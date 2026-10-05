// Detail on demand: see area.hpp. Pre-production code (research 00).
#include "area.hpp"

#include <algorithm>
#include <cmath>

#include "chance.hpp"
#include "draws.hpp"

namespace worldgen {

namespace {

// Smooth value noise, about -1 to 1, on a lattice of `side` metres keyed by chance, wrapping with the world: the
// lattices used divide its 2,000 by 1,000 km evenly.
double lattice(const World& w, double east, double north, double side, std::uint64_t octave) {
    const auto nx = static_cast<std::int64_t>(std::llround(kAroundMetres / side));
    const std::int64_t ny = nx / 2;
    const double fx = east / side;
    const double fy = north / side;
    const double x0f = std::floor(fx);
    const double y0f = std::floor(fy);
    double tx = fx - x0f;
    double ty = fy - y0f;
    tx = tx * tx * (3.0 - (2.0 * tx));
    ty = ty * ty * (3.0 - (2.0 * ty));
    const auto wrap = [](std::int64_t v, std::int64_t m) {
        v %= m;
        return v < 0 ? v + m : v;
    };
    const std::int64_t x0 = wrap(static_cast<std::int64_t>(x0f), nx);
    const std::int64_t y0 = wrap(static_cast<std::int64_t>(y0f), ny);
    const std::int64_t x1 = wrap(x0 + 1, nx);
    const std::int64_t y1 = wrap(y0 + 1, ny);
    const auto at = [&](std::int64_t i, std::int64_t j) {
        const std::int64_t key = (j * nx) + i;
        return (samebits::chance(w.seed, static_cast<std::uint64_t>(key), octave, Draw::kDetail) * 2.0) - 1.0;
    };
    const double a = at(x0, y0) + ((at(x1, y0) - at(x0, y0)) * tx);
    const double b = at(x0, y1) + ((at(x1, y1) - at(x0, y1)) * tx);
    return a + ((b - a) * ty);
}

// The cells' heights, joined smoothly between their middles.
double cell_height(const World& w, double east, double north) {
    const Grid& g = w.grid;
    const double gx = (east / g.metres) - 0.5;
    const double gy = (north / g.metres) - 0.5;
    const double x0f = std::floor(gx);
    const double y0f = std::floor(gy);
    const double tx = gx - x0f;
    const double ty = gy - y0f;
    const int x0 = static_cast<int>(x0f);
    const int y0 = static_cast<int>(y0f);
    const auto h = [&](int x, int y) { return static_cast<double>(w.height[static_cast<std::size_t>(g.at(x, y))]); };
    const double a = h(x0, y0) + ((h(x0 + 1, y0) - h(x0, y0)) * tx);
    const double b = h(x0, y0 + 1) + ((h(x0 + 1, y0 + 1) - h(x0, y0 + 1)) * tx);
    return a + ((b - a) * ty);
}

int cell_at(const World& w, double east, double north) {
    const Grid& g = w.grid;
    return g.at(static_cast<int>(std::floor(east / g.metres)), static_cast<int>(std::floor(north / g.metres)));
}

// The cell whose cover and water a place shows: its own, the edges between cells warped by noise at two scales, so
// cover meets cover along natural lines rather than the cells' square edges.
int cover_cell(const World& w, double east, double north) {
    const double dx = (350.0 * lattice(w, east, north, 800.0, 40)) + (80.0 * lattice(w, east, north, 200.0, 41));
    const double dy = (350.0 * lattice(w, east, north, 800.0, 42)) + (80.0 * lattice(w, east, north, 200.0, 43));
    return cell_at(w, east + dx, north + dy);
}

}  // namespace

double ground_height(const World& w, double east, double north) {
    const double base = cell_height(w, east, north);
    if (base <= 0.0) {
        return base;
    }
    // rougher where the cells' own slope is steep: relief at scales from 400 m down to 3 m, each half the last
    const double m = w.grid.metres;
    const double slope = (std::abs(cell_height(w, east + m, north) - cell_height(w, east - m, north)) +
                          std::abs(cell_height(w, east, north + m) - cell_height(w, east, north - m))) /
                         (2.0 * m);
    double amplitude = 25.0 * (0.2 + std::min(1.0, slope * 8.0));
    double side = 400.0;
    double detail = 0.0;
    for (std::uint64_t o = 0; o < 8; ++o) {
        detail += amplitude * lattice(w, east, north, side, o);
        amplitude *= 0.5;
        side *= 0.5;
    }
    // no detail at the shore, so the coast stays where the cells put it
    return base + (detail * std::clamp(base / 20.0, 0.0, 1.0));
}

std::vector<float> ground_heights(const World& w, double east, double north, int n, double spacing) {
    std::vector<float> out(static_cast<std::size_t>(n) * static_cast<std::size_t>(n));
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            out[(static_cast<std::size_t>(j) * static_cast<std::size_t>(n)) + static_cast<std::size_t>(i)] =
                static_cast<float>(ground_height(w, east + (i * spacing), north + (j * spacing)));
        }
    }
    return out;
}

std::vector<std::uint8_t> ground_colours(const World& w, double east, double north, int n, double spacing,
                                         const std::vector<float>& heights) {
    std::vector<std::uint8_t> out(static_cast<std::size_t>(n) * static_cast<std::size_t>(n) * 3);
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            const double x = east + (i * spacing);
            const double y = north + (j * spacing);
            const int c = cover_cell(w, x, y);
            const std::size_t k =
                (static_cast<std::size_t>(j) * static_cast<std::size_t>(n)) + static_cast<std::size_t>(i);
            std::array<std::uint8_t, 3> rgb{};
            if (heights[k] <= 0.0F || w.lake[static_cast<std::size_t>(c)] > 0.0F) {
                rgb = {85, 134, 139};  // the art book's shallow water
            } else {
                rgb = cover_colour(static_cast<Biome>(w.biome[static_cast<std::size_t>(c)]));
                // patches a few tens of metres across, a little lighter or darker
                const double patch = 1.0 + (0.08 * lattice(w, x, y, 25.0, 20));
                for (std::uint8_t& v : rgb) {
                    v = static_cast<std::uint8_t>(std::clamp(v * patch, 0.0, 255.0));
                }
            }
            out[k * 3] = rgb[0];
            out[(k * 3) + 1] = rgb[1];
            out[(k * 3) + 2] = rgb[2];
        }
    }
    return out;
}

std::vector<float> ground_trees(const World& w, double east, double north, double side, double spacing) {
    std::vector<float> out;
    const auto nx = static_cast<std::int64_t>(std::llround(kAroundMetres / spacing));
    const auto first_i = static_cast<std::int64_t>(std::floor(east / spacing));
    const auto first_j = static_cast<std::int64_t>(std::floor(north / spacing));
    const auto last_i = static_cast<std::int64_t>(std::floor((east + side) / spacing));
    const auto last_j = static_cast<std::int64_t>(std::floor((north + side) / spacing));
    for (std::int64_t j = first_j; j <= last_j; ++j) {
        for (std::int64_t i = first_i; i <= last_i; ++i) {
            const std::int64_t key = (j * nx) + i;
            const auto k = static_cast<std::uint64_t>(key);
            const double x = (static_cast<double>(i) + samebits::chance(w.seed, k, 30, Draw::kDetail, 0)) * spacing;
            const double y = (static_cast<double>(j) + samebits::chance(w.seed, k, 30, Draw::kDetail, 1)) * spacing;
            // a tree whose grid square straddles the edge stands in whichever square holds it
            if (x < east || x >= east + side || y < north || y >= north + side) {
                continue;
            }
            const int c = cover_cell(w, x, y);
            const auto b = static_cast<Biome>(w.biome[static_cast<std::size_t>(c)]);
            // the share of the ground under trees, from the biome's grown cover (WLD-31), in patches
            double share = 0.0;
            switch (b) {
                case Biome::kBroadleaf:
                case Biome::kConifer:
                case Biome::kTropical:
                    share = 0.8;
                    break;
                case Biome::kSavanna:
                case Biome::kMarsh:
                    share = 0.2;
                    break;
                case Biome::kGrassland:
                case Biome::kScrub:
                case Biome::kShore:
                    share = 0.05;
                    break;
                default:
                    share = 0.0;
                    break;
            }
            share *= 0.6 + (0.6 * lattice(w, x, y, 50.0, 31));
            if (samebits::chance(w.seed, k, 30, Draw::kDetail, 2) >= share ||
                w.lake[static_cast<std::size_t>(c)] > 0.0F) {
                continue;
            }
            const double h = ground_height(w, x, y);
            if (h > 0.5) {
                out.push_back(static_cast<float>(x - east));
                out.push_back(static_cast<float>(y - north));
                out.push_back(static_cast<float>(h));
            }
        }
    }
    return out;
}

}  // namespace worldgen
