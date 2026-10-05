// Detail on demand: see area.hpp. Pre-production code (research 00).
#include "area.hpp"

#include <algorithm>
#include <array>
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

// A grid's heights joined smoothly between its cells' middles: through each middle, with no sudden turn of the slope
// anywhere (Catmull and Rom's cubic each way), so ground lit by a low sun shows no line where joins of straight slopes
// would turn. The sea's depths count as no more than kJoinedDepth, so a deep sea beyond the shore raises no hill on the
// land: the ground under the sea is drawn at its level whatever its depth, which the picture reads from the cells
// themselves. `at(x, y)` gives cell x, y, wrapping as the grid does; `size` is a cell's side, metres.
constexpr double kJoinedDepth = -30.0;

template <typename At>
double smooth_cells(const At& at, double size, double east, double north) {
    const double gx = (east / size) - 0.5;
    const double gy = (north / size) - 0.5;
    const double x0f = std::floor(gx);
    const double y0f = std::floor(gy);
    const auto weights = [](double t) {
        const double t2 = t * t;
        const double t3 = t2 * t;
        return std::array<double, 4>{0.5 * ((-t3) + (2.0 * t2) - t), 0.5 * ((3.0 * t3) - (5.0 * t2) + 2.0),
                                     0.5 * ((-3.0 * t3) + (4.0 * t2) + t), 0.5 * (t3 - t2)};
    };
    const std::array<double, 4> wx = weights(gx - x0f);
    const std::array<double, 4> wy = weights(gy - y0f);
    const auto x0 = static_cast<std::int64_t>(x0f);
    const auto y0 = static_cast<std::int64_t>(y0f);
    double sum = 0.0;
    for (std::int64_t j = 0; j < 4; ++j) {
        double row = 0.0;
        for (std::int64_t i = 0; i < 4; ++i) {
            row += wx[static_cast<std::size_t>(i)] * std::max(at(x0 + i - 1, y0 + j - 1), kJoinedDepth);
        }
        sum += wy[static_cast<std::size_t>(j)] * row;
    }
    return sum;
}

// The cells' heights, joined smoothly between their middles.
double cell_height(const World& w, double east, double north) {
    const Grid& g = w.grid;
    const auto at = [&](std::int64_t x, std::int64_t y) {
        return static_cast<double>(w.height[static_cast<std::size_t>(g.at(static_cast<int>(x), static_cast<int>(y)))]);
    };
    return smooth_cells(at, g.metres, east, north);
}

// The height of mip level k, joined smoothly between its cells' middles.
double mip_height(const HeightMips& m, int k, double east, double north) {
    const double size = m.metres * static_cast<double>(1 << k);
    const int wk = m.width[static_cast<std::size_t>(k)];
    const int hk = m.height[static_cast<std::size_t>(k)];
    const std::vector<float>& level = m.level[static_cast<std::size_t>(k)];
    const auto at = [&](std::int64_t x, std::int64_t y) {
        const std::int64_t wx = ((x % wk) + wk) % wk;
        const std::int64_t wy = ((y % hk) + hk) % hk;
        return static_cast<double>(level[static_cast<std::size_t>((wy * wk) + wx)]);
    };
    return smooth_cells(at, size, east, north);
}

// How much of the relief on a lattice `side` metres across a grid `spacing` metres apart can hold: all of it from
// twice the spacing, none at the spacing, smoothly between.
double kept(double side, double spacing) {
    const double t = std::clamp((side - spacing) / spacing, 0.0, 1.0);
    return t * t * (3.0 - (2.0 * t));
}

// Gradient noise about -1 to 1 on a lattice that wraps every `period` cells each way, at x, y and z in cells.
double perlin(std::uint64_t seed, double x, double y, double z, int period) {
    const double xf = std::floor(x);
    const double yf = std::floor(y);
    const double zf = std::floor(z);
    const double fx = x - xf;
    const double fy = y - yf;
    const double fz = z - zf;
    const auto fade = [](double f) { return f * f * f * ((f * ((f * 6.0) - 15.0)) + 10.0); };
    const auto wrap = [period](double v) {
        const auto i = static_cast<std::int64_t>(v);
        return ((i % period) + period) % period;
    };
    // one of the twelve edge directions of a cube at each lattice point
    const auto grad = [&](double cx, double cy, double cz, double dx, double dy, double dz) {
        const std::int64_t key = (((wrap(cz) * period) + wrap(cy)) * period) + wrap(cx);
        const auto g =
            static_cast<int>(samebits::chance(seed, static_cast<std::uint64_t>(key), 0, Draw::kCloud) * 12.0);
        constexpr std::array<std::array<double, 3>, 12> kEdges = {{{1, 1, 0},
                                                                   {-1, 1, 0},
                                                                   {1, -1, 0},
                                                                   {-1, -1, 0},
                                                                   {1, 0, 1},
                                                                   {-1, 0, 1},
                                                                   {1, 0, -1},
                                                                   {-1, 0, -1},
                                                                   {0, 1, 1},
                                                                   {0, -1, 1},
                                                                   {0, 1, -1},
                                                                   {0, -1, -1}}};
        const std::array<double, 3>& e = kEdges[static_cast<std::size_t>(std::min(g, 11))];
        return (e[0] * dx) + (e[1] * dy) + (e[2] * dz);
    };
    const double u = fade(fx);
    const double v = fade(fy);
    const double s = fade(fz);
    const auto lerp = [](double a, double b, double t) { return a + ((b - a) * t); };
    const double x00 = lerp(grad(xf, yf, zf, fx, fy, fz), grad(xf + 1, yf, zf, fx - 1, fy, fz), u);
    const double x10 = lerp(grad(xf, yf + 1, zf, fx, fy - 1, fz), grad(xf + 1, yf + 1, zf, fx - 1, fy - 1, fz), u);
    const double x01 = lerp(grad(xf, yf, zf + 1, fx, fy, fz - 1), grad(xf + 1, yf, zf + 1, fx - 1, fy, fz - 1), u);
    const double x11 =
        lerp(grad(xf, yf + 1, zf + 1, fx, fy - 1, fz - 1), grad(xf + 1, yf + 1, zf + 1, fx - 1, fy - 1, fz - 1), u);
    return lerp(lerp(x00, x10, v), lerp(x01, x11, v), s);
}

// Worley's cellular noise, 0 to 1: the distance to the nearest of one point in each of `cells` cells a side that
// wrap, at x, y and z in cells, over the most it can be.
double worley(std::uint64_t seed, double x, double y, double z, int cells) {
    const double xf = std::floor(x);
    const double yf = std::floor(y);
    const double zf = std::floor(z);
    double nearest = 9.0;
    for (int dz = -1; dz <= 1; ++dz) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const double cx = xf + dx;
                const double cy = yf + dy;
                const double cz = zf + dz;
                const auto wrap = [cells](double v) {
                    const auto i = static_cast<std::int64_t>(v);
                    return ((i % cells) + cells) % cells;
                };
                const auto key = static_cast<std::uint64_t>((((wrap(cz) * cells) + wrap(cy)) * cells) + wrap(cx));
                const double px = cx + samebits::chance(seed, key, static_cast<std::uint64_t>(cells), Draw::kCloud, 1);
                const double py = cy + samebits::chance(seed, key, static_cast<std::uint64_t>(cells), Draw::kCloud, 2);
                const double pz = cz + samebits::chance(seed, key, static_cast<std::uint64_t>(cells), Draw::kCloud, 3);
                const double d = ((px - x) * (px - x)) + ((py - y) * (py - y)) + ((pz - z) * (pz - z));
                nearest = std::min(nearest, d);
            }
        }
    }
    return std::min(1.0, std::sqrt(nearest) / 1.7320508075688772);
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

HeightMips height_mips(const World& w) {
    HeightMips m;
    m.metres = w.grid.metres;
    m.level.push_back(w.height);
    m.width.push_back(w.grid.width);
    m.height.push_back(w.grid.height);
    while (m.width.back() >= 8 && m.height.back() >= 8) {
        const std::vector<float>& src = m.level.back();
        const int sw = m.width.back();
        const int dw = sw / 2;
        const int dh = m.height.back() / 2;
        std::vector<float> dst(static_cast<std::size_t>(dw) * static_cast<std::size_t>(dh));
        for (int y = 0; y < dh; ++y) {
            for (int x = 0; x < dw; ++x) {
                const auto at = [&](int xx, int yy) {
                    return src[(static_cast<std::size_t>(yy) * static_cast<std::size_t>(sw)) +
                               static_cast<std::size_t>(xx)];
                };
                dst[(static_cast<std::size_t>(y) * static_cast<std::size_t>(dw)) + static_cast<std::size_t>(x)] =
                    0.25F *
                    (at(2 * x, 2 * y) + at((2 * x) + 1, 2 * y) + at(2 * x, (2 * y) + 1) + at((2 * x) + 1, (2 * y) + 1));
            }
        }
        m.level.push_back(std::move(dst));
        m.width.push_back(dw);
        m.height.push_back(dh);
    }
    return m;
}

double ground_height_at(const World& w, const HeightMips& mips, double east, double north, double spacing) {
    // the level whose cells are about the spacing, so each point stands for the ground around it
    int k = 0;
    while (k + 1 < static_cast<int>(mips.level.size()) && mips.metres * static_cast<double>(2 << k) <= spacing) {
        ++k;
    }
    const double base = mip_height(mips, k, east, north);
    if (base <= 0.0) {
        return base;
    }
    const double m = mips.metres * static_cast<double>(1 << k);
    const double slope = (std::abs(mip_height(mips, k, east + m, north) - mip_height(mips, k, east - m, north)) +
                          std::abs(mip_height(mips, k, east, north + m) - mip_height(mips, k, east, north - m))) /
                         (2.0 * m);
    double amplitude = 25.0 * (0.2 + std::min(1.0, slope * 8.0));
    double side = 400.0;
    double detail = 0.0;
    for (std::uint64_t o = 0; o < 8; ++o) {
        const double keep = kept(side, spacing);
        if (keep > 0.0) {
            detail += amplitude * keep * lattice(w, east, north, side, o);
        }
        amplitude *= 0.5;
        side *= 0.5;
    }
    return base + (detail * std::clamp(base / 20.0, 0.0, 1.0));
}

GroundChunk ground_chunk(const World& w, const HeightMips& mips, double east, double north, int n, double spacing) {
    GroundChunk out;
    const auto count = static_cast<std::size_t>(n) * static_cast<std::size_t>(n);
    // the chunk's own points with a ring round them, and the grid half as fine with a ring round it, each as seen
    // with its own spacing
    const int m = n + 2;
    const int half = ((n - 1) / 2) + 1;
    const int mp = half + 2;
    std::vector<double> own(static_cast<std::size_t>(m) * static_cast<std::size_t>(m));
    std::vector<double> coarse(static_cast<std::size_t>(mp) * static_cast<std::size_t>(mp));
    for (int j = 0; j < m; ++j) {
        for (int i = 0; i < m; ++i) {
            own[(static_cast<std::size_t>(j) * static_cast<std::size_t>(m)) + static_cast<std::size_t>(i)] =
                ground_height_at(w, mips, east + ((i - 1) * spacing), north + ((j - 1) * spacing), spacing);
        }
    }
    for (int j = 0; j < mp; ++j) {
        for (int i = 0; i < mp; ++i) {
            coarse[(static_cast<std::size_t>(j) * static_cast<std::size_t>(mp)) + static_cast<std::size_t>(i)] =
                ground_height_at(w, mips, east + ((i - 1) * 2.0 * spacing), north + ((j - 1) * 2.0 * spacing),
                                 2.0 * spacing);
        }
    }
    const auto at = [](const std::vector<double>& g, int side, int i, int j) {
        return g[(static_cast<std::size_t>(j) * static_cast<std::size_t>(side)) + static_cast<std::size_t>(i)];
    };
    // the surface's normal from the heights round a point, the sea flat at its level
    const auto normal = [&](const std::vector<double>& g, int side, int i, int j, double step, float* to) {
        const double dx = (std::max(0.0, at(g, side, i + 1, j)) - std::max(0.0, at(g, side, i - 1, j))) / (2.0 * step);
        const double dn = (std::max(0.0, at(g, side, i, j + 1)) - std::max(0.0, at(g, side, i, j - 1))) / (2.0 * step);
        const double len = std::sqrt((dx * dx) + 1.0 + (dn * dn));
        to[0] = static_cast<float>(-dx / len);
        to[1] = static_cast<float>(1.0 / len);
        to[2] = static_cast<float>(dn / len);
    };
    out.surface.resize(count);
    out.truth.resize(count);
    out.normal.resize(count * 3);
    out.to_surface.resize(count);
    out.to_truth.resize(count);
    out.to_normal.resize(count * 3);
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            const std::size_t k =
                (static_cast<std::size_t>(j) * static_cast<std::size_t>(n)) + static_cast<std::size_t>(i);
            const double h = at(own, m, i + 1, j + 1);
            out.truth[k] = static_cast<float>(h);
            out.surface[k] = static_cast<float>(std::max(0.0, h));
            normal(own, m, i + 1, j + 1, spacing, &out.normal[k * 3]);
            // the point of the grid half as fine it slides onto: the even one at or before it each way
            const int ci = (i / 2) + 1;
            const int cj = (j / 2) + 1;
            const double c = at(coarse, mp, ci, cj);
            out.to_truth[k] = static_cast<float>(c);
            out.to_surface[k] = static_cast<float>(std::max(0.0, c));
            normal(coarse, mp, ci, cj, 2.0 * spacing, &out.to_normal[k * 3]);
        }
    }
    return out;
}

std::vector<float> climate_texture(const World& w) {
    const auto n = static_cast<std::size_t>(w.grid.cells());
    std::vector<float> out(n * 4);
    for (std::size_t c = 0; c < n; ++c) {
        out[c * 4] = w.temperature[c];
        out[(c * 4) + 1] = w.cold[c];
        out[(c * 4) + 2] = w.warm[c];
        out[(c * 4) + 3] = w.wetness[c];
    }
    return out;
}

std::vector<std::uint8_t> cover_texture(const World& w) {
    const auto n = static_cast<std::size_t>(w.grid.cells());
    std::vector<std::uint8_t> out(n * 4);
    for (std::size_t c = 0; c < n; ++c) {
        // the sea's cells take the shore's colour, so the land's colour seen from far off, averaged, meets beaches
        const auto b = w.sea(static_cast<int>(c)) ? Biome::kShore : static_cast<Biome>(w.biome[c]);
        const std::array<std::uint8_t, 3> rgb = cover_colour(b);
        out[c * 4] = rgb[0];
        out[(c * 4) + 1] = rgb[1];
        out[(c * 4) + 2] = rgb[2];
        out[(c * 4) + 3] = static_cast<std::uint8_t>(std::lround(255.0F * grown_cover(b)[0]));
    }
    return out;
}

std::vector<std::uint8_t> water_texture(const World& w) {
    const Grid& g = w.grid;
    const auto n = static_cast<std::size_t>(g.cells());
    std::vector<std::uint8_t> out(n * 4, 0);
    // the step from a cell to a neighbour, 0 to 7 as kDx and kDy, or 255 if it is not a neighbour
    const auto step = [&g](int from, int to) {
        for (int k = 0; k < 8; ++k) {
            if (g.at(g.x_of(from) + kDx[static_cast<std::size_t>(k)],
                     g.y_of(from) + kDy[static_cast<std::size_t>(k)]) == to) {
                return static_cast<std::uint8_t>(k);
            }
        }
        return static_cast<std::uint8_t>(255);
    };
    // each cell's main donor: the one of the cells draining into it that drains the most land
    std::vector<std::int32_t> main_donor(n, -1);
    for (int c = 0; c < g.cells(); ++c) {
        const auto cs = static_cast<std::size_t>(c);
        const auto r = static_cast<std::size_t>(w.receiver[cs]);
        if (w.sea(c) || r == cs) {
            continue;
        }
        if (main_donor[r] < 0 || w.area[cs] > w.area[static_cast<std::size_t>(main_donor[r])]) {
            main_donor[r] = c;
        }
    }
    for (int c = 0; c < g.cells(); ++c) {
        const auto cs = static_cast<std::size_t>(c);
        const double area = std::max(1.0, static_cast<double>(w.area[cs]));
        out[cs * 4] = w.sea(c) ? 255 : step(c, w.receiver[cs]);
        out[(cs * 4) + 1] = static_cast<std::uint8_t>(std::clamp(16.0 * std::log2(area), 0.0, 255.0));
        out[(cs * 4) + 2] = static_cast<std::uint8_t>(std::clamp(static_cast<double>(w.lake[cs]), 0.0, 255.0));
        out[(cs * 4) + 3] = main_donor[cs] < 0 ? 255 : step(c, main_donor[cs]);
    }
    return out;
}

std::vector<std::uint8_t> cloud_noise(std::uint64_t seed, int size) {
    const auto s = static_cast<std::size_t>(size);
    std::vector<std::uint8_t> out(s * s * s * 4);
    const auto byte = [](double v) { return static_cast<std::uint8_t>(std::lround(std::clamp(v, 0.0, 1.0) * 255.0)); };
    for (int z = 0; z < size; ++z) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                const double u = (x + 0.5) / size;
                const double v = (y + 0.5) / size;
                const double q = (z + 0.5) / size;
                const auto cells = [&](int k) { return worley(seed, u * k, v * k, q * k, k); };
                // Perlin's billows, three octaves, about 0 to 1
                const double billows = 0.5 + (0.5 * ((0.6 * perlin(seed, u * 4, v * 4, q * 4, 4)) +
                                                     (0.3 * perlin(seed, u * 8, v * 8, q * 8, 8)) +
                                                     (0.15 * perlin(seed, u * 16, v * 16, q * 16, 16))));
                // Worley's cells, inverted so a cell's middle is 1, three octaves
                const double puffs =
                    (0.625 * (1.0 - cells(4))) + (0.25 * (1.0 - cells(8))) + (0.125 * (1.0 - cells(16)));
                // the billows eroded where the cells meet (Schneider 2015's remap)
                const double shape = std::clamp((billows - (puffs - 1.0)) / (1.0 - (puffs - 1.0)), 0.0, 1.0);
                const std::size_t k = ((((static_cast<std::size_t>(z) * s) + static_cast<std::size_t>(y)) * s) +
                                       static_cast<std::size_t>(x)) *
                                      4;
                out[k] = byte(shape);
                out[k + 1] = byte(1.0 - cells(6));
                out[k + 2] = byte(1.0 - cells(12));
                out[k + 3] = byte(1.0 - cells(24));
            }
        }
    }
    // the smaller copies, each point the average of the eight under it
    std::size_t from = 0;
    for (std::size_t n = s; n > 1; n /= 2) {
        const std::size_t m = n / 2;
        const std::size_t to = out.size();
        out.resize(to + (m * m * m * 4));
        for (std::size_t z = 0; z < m; ++z) {
            for (std::size_t y = 0; y < m; ++y) {
                for (std::size_t x = 0; x < m; ++x) {
                    for (std::size_t c = 0; c < 4; ++c) {
                        unsigned sum = 0;
                        for (std::size_t e = 0; e < 8; ++e) {
                            const std::size_t under_z = (2 * z) + (e >> 2U);
                            const std::size_t under_y = (2 * y) + ((e >> 1U) & 1U);
                            const std::size_t under_x = (2 * x) + (e & 1U);
                            sum += out[from + (((((under_z * n) + under_y) * n) + under_x) * 4) + c];
                        }
                        out[to + (((((z * m) + y) * m) + x) * 4) + c] = static_cast<std::uint8_t>((sum + 4) / 8);
                    }
                }
            }
        }
        from = to;
    }
    return out;
}

}  // namespace worldgen
