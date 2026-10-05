// P7's land (A7.2 steps 1 to 4, WLD-09): plates and their edges, rock, then uplift and erosion by the stream power law
// with Priority-Flood, so every river reaches the sea or a lake (WLD-17). Pre-production code (research 00).
#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>

#include "chance.hpp"
#include "draws.hpp"
#include "maths.hpp"
#include "noise.hpp"
#include "world.hpp"

namespace worldgen {

namespace {

constexpr double kKmAround = kAroundMetres / 1000.0;
constexpr double kKmPoles = kKmAround / 2.0;
constexpr int kMostPlates = 12;

struct Plate {
    double u = 0.0;  // its seed's place
    double v = 0.0;
    double east = 0.0;  // its motion, cm a year
    double north = 0.0;
    double weight = 0.0;  // km², so plates differ in size
    bool continental = false;
};

// Each edge cell's meeting: what kind, how fast the plates close, and which plate rides over the other.
struct Meeting {
    std::uint8_t kind = 0;  // as World::edge
    std::uint8_t over = 0;  // the overriding plate where one sinks under the other
    float closing = 0.0F;   // cm a year, negative where they part
};

double wrap_half(double d) {
    return d - std::floor(d + 0.5);
}

// The distance between two places on the torus, km.
double km_between(double u0, double v0, double u1, double v1) {
    const double dx = wrap_half(u1 - u0) * kKmAround;
    const double dy = wrap_half(v1 - v0) * kKmPoles;
    return std::sqrt((dx * dx) + (dy * dy));
}

double smoothstep(double a, double b, double x) {
    const double t = std::clamp((x - a) / (b - a), 0.0, 1.0);
    return t * t * (3.0 - (2.0 * t));
}

double bump(double d, double width) {
    const double t = d / width;
    return samebits::exponent(-(t * t));
}

std::vector<Plate> plates_of(std::uint64_t seed) {
    const int count = 6 + static_cast<int>(samebits::chance(seed, 0, 0, Draw::kWorld, 2) * 7.0);
    std::vector<Plate> plates(static_cast<std::size_t>(count));
    for (int p = 0; p < count; ++p) {
        Plate& plate = plates[static_cast<std::size_t>(p)];
        const auto i = static_cast<std::uint64_t>(p);
        plate.u = samebits::chance(seed, 1, i, Draw::kWorld, 0);
        plate.v = samebits::chance(seed, 1, i, Draw::kWorld, 1);
        const double angle = samebits::chance(seed, 1, i, Draw::kWorld, 2) * 6.28318530717958623200;
        const double speed = 1.0 + (5.0 * samebits::chance(seed, 1, i, Draw::kWorld, 3));
        plate.east = speed * samebits::cosine(angle);
        plate.north = speed * samebits::sine(angle);
        const double size = 250.0 * samebits::chance(seed, 1, i, Draw::kWorld, 4);
        plate.weight = size * size;
    }
    return plates;
}

// The cells' fields that the stages after plates share, kept on the world.
struct Fields {
    std::vector<float> crust;  // 0 oceanic to 1 continental
    std::vector<float> edge_km;
    std::vector<std::int32_t> source;  // the nearest edge cell
};

// The mean over a square of (2r + 1) cells a side round every cell, on the torus, by running sums along rows and
// then columns.
std::vector<double> box_mean(const Grid& g, const std::vector<double>& v, int r) {
    std::vector<double> rows(v.size());
    std::vector<double> out(v.size());
    const double count = (2.0 * r) + 1.0;
    for (int y = 0; y < g.height; ++y) {
        double s = 0.0;
        for (int d = -r; d <= r; ++d) {
            s += v[static_cast<std::size_t>(g.at(d, y))];
        }
        for (int x = 0; x < g.width; ++x) {
            rows[static_cast<std::size_t>(g.at(x, y))] = s / count;
            s += v[static_cast<std::size_t>(g.at(x + r + 1, y))] - v[static_cast<std::size_t>(g.at(x - r, y))];
        }
    }
    for (int x = 0; x < g.width; ++x) {
        double s = 0.0;
        for (int d = -r; d <= r; ++d) {
            s += rows[static_cast<std::size_t>(g.at(x, d))];
        }
        for (int y = 0; y < g.height; ++y) {
            out[static_cast<std::size_t>(g.at(x, y))] = s / count;
            s += rows[static_cast<std::size_t>(g.at(x, y + r + 1))] - rows[static_cast<std::size_t>(g.at(x, y - r))];
        }
    }
    return out;
}

// Fixed chunks of rows: each row's cells are written only by that row's work, so the threads never change the result.
void by_rows(const Grid& g, minds::Pool* pool, const std::function<void(int)>& row) {
    pool->run(g.height, [&row](int y, int /*thread*/) { row(y); });
}

}  // namespace

std::uint64_t candidate_seed(std::uint64_t seed, int i) {
    return samebits::mix(seed ^ samebits::mix(static_cast<std::uint64_t>(i) + 0x9e37U));
}

void make_plates(World* w, minds::Pool* pool) {
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    const std::uint64_t seed = w->seed;
    w->tilt = 15.0 + (15.0 * samebits::chance(seed, 0, 0, Draw::kWorld, 0));
    w->land_share = 0.27 + (0.21 * samebits::chance(seed, 0, 0, Draw::kWorld, 1));
    std::vector<Plate> plates = plates_of(seed);
    const int count = static_cast<int>(plates.size());

    const Noise warp_u(seed, 1, 2, 6, 0.5);
    const Noise warp_v(seed, 2, 2, 6, 0.5);
    const Noise crust_noise(seed, 3, 2, 6, 0.55);
    const Noise relief(seed, 4, 4, 7, 0.55);
    const Noise background(seed, 5, 2, 4, 0.5);

    // each cell to the plate whose seed is nearest, by a warped distance with each plate's weight
    w->plate.assign(n, 0);
    by_rows(g, pool, [&](int y) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            const double u = g.u_of(c);
            const double v = g.v_of(c);
            const double wu = u + (0.11 * warp_u.at(u, v));
            const double wv = v + (0.11 * warp_v.at(u, v));
            double best = 0.0;
            int nearest = 0;
            for (int p = 0; p < count; ++p) {
                const Plate& plate = plates[static_cast<std::size_t>(p)];
                const double d = km_between(wu, wv, plate.u, plate.v);
                const double score = (d * d) - plate.weight;
                if (p == 0 || score < best) {
                    best = score;
                    nearest = p;
                }
            }
            w->plate[static_cast<std::size_t>(c)] = static_cast<std::uint8_t>(nearest);
        }
    });

    // continental plates, in an order drawn from the seed, until their area passes the share of land asked for
    std::array<std::int64_t, kMostPlates> cells{};
    for (const std::uint8_t p : w->plate) {
        ++cells[p];
    }
    std::vector<std::pair<double, int>> drawn;
    drawn.reserve(static_cast<std::size_t>(count));
    for (int p = 0; p < count; ++p) {
        drawn.emplace_back(samebits::chance(seed, 2, static_cast<std::uint64_t>(p), Draw::kWorld, 0), p);
    }
    std::sort(drawn.begin(), drawn.end());
    std::int64_t continental = 0;
    for (const auto& [key, p] : drawn) {
        if (static_cast<double>(continental) >= w->land_share * 1.2 * static_cast<double>(n)) {
            break;
        }
        plates[static_cast<std::size_t>(p)].continental = true;
        continental += cells[static_cast<std::size_t>(p)];
    }

    // crust: continental plates are continents in most of their area and oceanic plates hold a few microcontinents;
    // the plates' leaning is blurred over about 150 km before the noise decides, so coasts are ragged and only follow
    // a plate's edge where the edge itself raises them
    std::vector<double> lean(n);
    for (std::size_t c = 0; c < n; ++c) {
        lean[c] = plates[w->plate[c]].continental ? 0.3 : -0.45;
    }
    const int blur = std::max(1, static_cast<int>(150000.0 / g.metres / 2.0));
    for (int pass = 0; pass < 2; ++pass) {
        lean = box_mean(g, lean, blur);
    }
    std::vector<float> crust(n);
    by_rows(g, pool, [&](int y) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            const double k = crust_noise.at(g.u_of(c), g.v_of(c)) + lean[static_cast<std::size_t>(c)];
            crust[static_cast<std::size_t>(c)] = static_cast<float>(smoothstep(-0.05, 0.12, k));
        }
    });

    // edges: where a cell meets another plate, how fast the two close along the line between them
    std::vector<Meeting> meeting(n);
    std::vector<std::int32_t> edges;
    for (std::size_t c = 0; c < n; ++c) {
        const int ci = static_cast<int>(c);
        const int p = w->plate[c];
        double closing = 0.0;
        int other = -1;
        int touching = 0;
        for (int k = 0; k < 8; ++k) {
            const auto nb = static_cast<std::size_t>(
                g.at(g.x_of(ci) + kDx[static_cast<std::size_t>(k)], g.y_of(ci) + kDy[static_cast<std::size_t>(k)]));
            const int q = w->plate[nb];
            if (q == p) {
                continue;
            }
            const double len = kStep[static_cast<std::size_t>(k)];
            const double nx = kDx[static_cast<std::size_t>(k)] / len;
            const double ny = kDy[static_cast<std::size_t>(k)] / len;
            const Plate& a = plates[static_cast<std::size_t>(p)];
            const Plate& b = plates[static_cast<std::size_t>(q)];
            closing -= ((b.east - a.east) * nx) + ((b.north - a.north) * ny);
            other = other < 0 ? q : other;
            ++touching;
        }
        if (touching == 0) {
            continue;
        }
        closing /= touching;
        Meeting& m = meeting[c];
        m.closing = static_cast<float>(closing);
        const bool cont_here = crust[c] > 0.5F;
        // the other side's crust, from the other plate's kind
        const bool cont_there = plates[static_cast<std::size_t>(other)].continental;
        if (closing > 1.0) {
            if (cont_here && cont_there) {
                m.kind = 1;
            } else {
                m.kind = 2;
                // the continental side rides over; between two oceanic plates, the one drawn first
                if (cont_here != cont_there) {
                    m.over = static_cast<std::uint8_t>(cont_here ? p : other);
                } else {
                    m.over = static_cast<std::uint8_t>(std::min(p, other));
                }
            }
        } else if (closing < -1.0) {
            m.kind = 3;
        } else {
            m.kind = 4;
        }
        edges.push_back(ci);
    }

    // every cell's nearest edge cell, spreading outward from the edges ring by ring
    std::vector<std::int32_t> source(n, -1);
    std::vector<float> edge_km(n, 1.0e9F);
    std::vector<std::int32_t> ring = edges;
    for (const std::int32_t e : edges) {
        source[static_cast<std::size_t>(e)] = e;
        edge_km[static_cast<std::size_t>(e)] = 0.0F;
    }
    const double reach = 600.0;  // km: beyond it no edge matters
    while (!ring.empty()) {
        std::vector<std::int32_t> next;
        for (const std::int32_t c : ring) {
            const std::int32_t s = source[static_cast<std::size_t>(c)];
            for (int k = 0; k < 8; ++k) {
                const int nb =
                    g.at(g.x_of(c) + kDx[static_cast<std::size_t>(k)], g.y_of(c) + kDy[static_cast<std::size_t>(k)]);
                const auto nbs = static_cast<std::size_t>(nb);
                const double d = km_between(g.u_of(nb), g.v_of(nb), g.u_of(s), g.v_of(s));
                if (d + 1e-9 < edge_km[nbs] && d < reach && source[nbs] != s) {
                    // a nearer edge: taken, and spread again from here
                    edge_km[nbs] = static_cast<float>(d);
                    source[nbs] = s;
                    next.push_back(nb);
                }
            }
        }
        ring = std::move(next);
    }

    // heights and uplift from the crust and the edges; volcanoes after
    w->height.assign(n, 0.0F);
    w->uplift.assign(n, 0.0F);
    w->edge.assign(n, 0);
    by_rows(g, pool, [&](int y) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            const auto cs = static_cast<std::size_t>(c);
            const double u = g.u_of(c);
            const double v = g.v_of(c);
            const double k = crust[cs];
            double h = (-4200.0 + (4600.0 * k)) + ((250.0 + (350.0 * k)) * relief.at(u, v));
            double up = k * 0.00006 * (1.0 + (0.8 * background.at(u, v)));
            const std::int32_t s = source[cs];
            if (s >= 0) {
                const Meeting& m = meeting[static_cast<std::size_t>(s)];
                const double d = edge_km[cs];
                const double strength = std::min(1.0, std::abs(static_cast<double>(m.closing)) / 4.0);
                if (d < 3.0 && m.kind != 0) {
                    w->edge[cs] = m.kind;
                }
                switch (m.kind) {
                    case 1:  // two continents meet: a broad range
                        up += 0.0022 * strength * bump(d, 170.0);
                        h += 300.0 * bump(d, 150.0);
                        break;
                    case 2:  // one plate sinks: a trench on its side, an arc of volcanoes on the other
                        if (w->plate[cs] == m.over) {
                            up += (0.0015 * strength * bump(d - 130.0, 55.0)) + (0.0005 * bump(d, 40.0));
                        } else {
                            h -= 2500.0 * bump(d, 40.0) * (1.0 - k);
                        }
                        break;
                    case 3:  // the plates part: a rift on land, a ridge under the sea
                        if (k > 0.5) {
                            up -= 0.0004 * bump(d, 35.0);
                            h -= 600.0 * bump(d, 35.0);
                        } else {
                            h += 2200.0 * bump(d, 180.0);
                        }
                        break;
                    case 4:  // they slide past: a fault
                        up += 0.00025 * bump(d, 25.0);
                        break;
                    default:
                        break;
                }
            }
            w->height[cs] = static_cast<float>(h);
            w->uplift[cs] = static_cast<float>(up);
        }
    });

    // volcanoes on a lattice about 40 km apart, each standing where an arc or a rift lies (WLD-09)
    w->volcano.assign(n, 0);
    const int vx = static_cast<int>(kKmAround / 40.0);
    const int vy = static_cast<int>(kKmPoles / 40.0);
    for (int j = 0; j < vy; ++j) {
        for (int i = 0; i < vx; ++i) {
            const int point = (j * vx) + i;
            const auto key = static_cast<std::uint64_t>(point);
            const double u = (i + samebits::chance(seed, key, 0, Draw::kVolcano, 0)) / vx;
            const double v = (j + samebits::chance(seed, key, 0, Draw::kVolcano, 1)) / vy;
            const int c = g.at(static_cast<int>(u * g.width), static_cast<int>(v * g.height));
            const auto cs = static_cast<std::size_t>(c);
            const std::int32_t s = source[cs];
            if (s < 0) {
                continue;
            }
            const Meeting& m = meeting[static_cast<std::size_t>(s)];
            const double d = edge_km[cs];
            const double roll = samebits::chance(seed, key, 0, Draw::kVolcano, 2);
            std::uint8_t kind = 0;
            if (m.kind == 2 && w->plate[cs] == m.over && d > 90.0 && d < 170.0 && roll < 0.55) {
                kind = 2;
            } else if (m.kind == 3 && crust[cs] > 0.5F && d < 30.0 && roll < 0.25) {
                kind = 1;
            }
            if (kind == 0) {
                continue;
            }
            w->volcano[cs] = kind;
            const int r = static_cast<int>(std::ceil(30000.0 / g.metres));
            for (int dy = -r; dy <= r; ++dy) {
                for (int dx = -r; dx <= r; ++dx) {
                    const int nb = g.at(g.x_of(c) + dx, g.y_of(c) + dy);
                    const double dkm = std::sqrt(static_cast<double>((dx * dx) + (dy * dy))) * g.metres / 1000.0;
                    const auto nbs = static_cast<std::size_t>(nb);
                    w->height[nbs] += static_cast<float>(1200.0 * bump(dkm, 12.0));
                    w->uplift[nbs] += static_cast<float>(0.0006 * bump(dkm, 12.0));
                }
            }
        }
    }

    // the sea's level, so the land's share is the one the seed asked for (WLD-06)
    std::vector<float> sorted = w->height;
    const auto k = static_cast<std::ptrdiff_t>((1.0 - w->land_share) * static_cast<double>(n));
    std::nth_element(sorted.begin(), sorted.begin() + k, sorted.end());
    const float level = sorted[static_cast<std::size_t>(k)];
    for (float& h : w->height) {
        h -= level;
    }

    // what the rock stage reads
    w->setting.assign(n, 0);
    const Noise basin(seed, 6, 2, 5, 0.5);
    by_rows(g, pool, [&](int y) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            const auto cs = static_cast<std::size_t>(c);
            const std::int32_t s = source[cs];
            const Meeting m = s >= 0 ? meeting[static_cast<std::size_t>(s)] : Meeting{};
            const double d = edge_km[cs];
            Setting set = Setting::kShield;
            if (crust[cs] < 0.5F && !(m.kind == 2 && w->plate[cs] == m.over && d < 220.0)) {
                set = Setting::kSeaFloor;
            } else if (m.kind == 1 && d < 200.0) {
                set = Setting::kRange;
            } else if (m.kind == 2 && w->plate[cs] == m.over && d < 220.0) {
                set = Setting::kArc;
            } else if (m.kind == 3 && d < 60.0) {
                set = Setting::kRift;
            } else if (basin.at(g.u_of(c), g.v_of(c)) > 0.05) {
                set = Setting::kBasin;
            }
            w->setting[cs] = static_cast<std::uint8_t>(set);
        }
    });
}

void make_rock(World* w, minds::Pool* pool) {
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    const Noise variant(w->seed, 7, 4, 5, 0.5);
    const Noise shallow(w->seed, 8, 2, 4, 0.5);
    w->rock.assign(n * 3, 0);
    by_rows(g, pool, [&](int y) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            const auto cs = static_cast<std::size_t>(c);
            const double var = variant.at(g.u_of(c), g.v_of(c));
            std::array<Rock, 3> layers{Rock::kGranite, Rock::kGranite, Rock::kGranite};
            switch (static_cast<Setting>(w->setting[cs])) {
                case Setting::kSeaFloor:
                    layers = {Rock::kBasalt, Rock::kBasalt, Rock::kBasalt};
                    break;
                case Setting::kShield:
                    layers[0] = var > 0.3 ? Rock::kQuartzite : (var < -0.35 ? Rock::kSlate : Rock::kGranite);
                    break;
                case Setting::kBasin:
                    if (var > 0.2) {
                        // chalk only where a shallow, warm sea lay; limestone elsewhere
                        layers = {shallow.at(g.u_of(c), g.v_of(c)) > 0.1 ? Rock::kChalk : Rock::kLimestone,
                                  Rock::kLimestone, Rock::kGranite};
                    } else if (var > -0.05) {
                        layers = {Rock::kLimestone, Rock::kShale, Rock::kGranite};
                    } else if (var > -0.3) {
                        layers = {Rock::kShale, Rock::kSandstone, Rock::kGranite};
                    } else {
                        layers = {Rock::kSandstone, Rock::kShale, Rock::kGranite};
                    }
                    break;
                case Setting::kRange:
                    layers = {var > 0.25 ? Rock::kGranite : (var > -0.2 ? Rock::kSlate : Rock::kQuartzite),
                              Rock::kSlate, Rock::kGranite};
                    break;
                case Setting::kArc:
                    layers = {w->volcano[cs] == 2 ? Rock::kGlassyLava : Rock::kLavaAsh, Rock::kGranite, Rock::kGranite};
                    break;
                case Setting::kRift:
                    layers = {Rock::kBasalt, Rock::kBasalt, Rock::kGranite};
                    break;
            }
            for (std::size_t l = 0; l < 3; ++l) {
                w->rock[(cs * 3) + l] = static_cast<std::uint8_t>(layers[l]);
            }
        }
    });
}

namespace {

// How easily each rock wears, relative to granite's.
constexpr std::array<double, kRocks> kWear = {0.6, 0.7, 1.6, 0.8, 1.2, 2.0, 1.0, 1.8, 0.5, 0.9, 2.5, 3.0};

// Priority-Flood (Barnes 2014): from the coast inward, lowest first, so each land cell's water goes to the neighbour
// it was reached from and hollows fill to their spill points. Sets the receivers and the order, and the filled
// heights.
void flood(World* w, std::vector<float>* filled) {
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    w->receiver.assign(n, -1);
    w->order.clear();
    filled->assign(n, 0.0F);
    std::vector<std::uint8_t> reached(n, 0);
    using Item = std::pair<float, std::int32_t>;
    std::priority_queue<Item, std::vector<Item>, std::greater<>> open;
    for (std::size_t c = 0; c < n; ++c) {
        if (!w->sea(static_cast<int>(c))) {
            continue;
        }
        w->receiver[c] = static_cast<std::int32_t>(c);
        reached[c] = 1;
        (*filled)[c] = w->height[c];
        for (int k = 0; k < 8; ++k) {
            const int nb = g.at(g.x_of(static_cast<int>(c)) + kDx[static_cast<std::size_t>(k)],
                                g.y_of(static_cast<int>(c)) + kDy[static_cast<std::size_t>(k)]);
            if (!w->sea(nb)) {
                open.emplace(0.0F, static_cast<std::int32_t>(c));
                break;
            }
        }
    }
    while (!open.empty()) {
        const auto [level, c] = open.top();
        open.pop();
        for (int k = 0; k < 8; ++k) {
            const int nb =
                g.at(g.x_of(c) + kDx[static_cast<std::size_t>(k)], g.y_of(c) + kDy[static_cast<std::size_t>(k)]);
            const auto nbs = static_cast<std::size_t>(nb);
            if (reached[nbs] != 0) {
                continue;
            }
            reached[nbs] = 1;
            (*filled)[nbs] = std::max(w->height[nbs], level);
            w->receiver[nbs] = c;
            w->order.push_back(nb);
            open.emplace((*filled)[nbs], nb);
        }
    }
}

// The land draining through each cell, km², from the far ends of the order to the sea.
void drain(World* w) {
    const double cell_km2 = (w->grid.metres / 1000.0) * (w->grid.metres / 1000.0);
    w->area.assign(static_cast<std::size_t>(w->grid.cells()), 0.0F);
    for (const std::int32_t c : w->order) {
        w->area[static_cast<std::size_t>(c)] = static_cast<float>(cell_km2);
    }
    for (auto it = w->order.rbegin(); it != w->order.rend(); ++it) {
        const auto c = static_cast<std::size_t>(*it);
        const auto r = static_cast<std::size_t>(w->receiver[c]);
        if (!w->sea(static_cast<int>(r))) {
            w->area[r] += w->area[c];
        }
    }
}

// The length from a cell to its receiver, metres.
double step_metres(const Grid& g, int c, int r) {
    const int dx = std::abs(g.x_of(c) - g.x_of(r));
    const int dy = std::abs(g.y_of(c) - g.y_of(r));
    return ((dx == 0 || dy == 0) ? 1.0 : kDiagonal) * g.metres;
}

}  // namespace

void erode(World* w, int steps, double years) {
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    // the stream power law with m = 1/2 and n = 1, solved implicitly from the sea upward (Braun and Willett 2013); a
    // coarse grid's wider cells drain more land each, so their rivers' power is scaled to keep slopes alike
    const double k0 = 1.0e-5 * std::sqrt(976.5625 / g.metres);
    std::vector<float> filled;
    for (int s = 0; s < steps; ++s) {
        flood(w, &filled);
        drain(w);
        for (std::size_t c = 0; c < n; ++c) {
            if (w->sea(static_cast<int>(c)) && w->uplift[c] > 0.0F) {
                w->height[c] += static_cast<float>(w->uplift[c] * years);
            }
        }
        for (const std::int32_t c : w->order) {
            const auto cs = static_cast<std::size_t>(c);
            const std::int32_t r = w->receiver[cs];
            const double hr = w->sea(r) ? 0.0 : w->height[static_cast<std::size_t>(r)];
            const double h = w->height[cs];
            const double raised = h + (w->uplift[cs] * years);
            if (h < hr) {
                // in a hollow: it rises with the land, and nothing cuts it
                w->height[cs] = static_cast<float>(raised);
                continue;
            }
            const double a = w->area[cs] * 1.0e6;
            const double f = k0 * kWear[w->rock[cs * 3]] * years * std::sqrt(a) / step_metres(g, c, r);
            w->height[cs] = static_cast<float>((raised + (f * hr)) / (1.0 + f));
        }
    }
    flood(w, &filled);
    drain(w);
    w->lake.assign(n, 0.0F);
    for (const std::int32_t c : w->order) {
        const auto cs = static_cast<std::size_t>(c);
        const float depth = filled[cs] - w->height[cs];
        w->lake[cs] = depth > 0.5F ? depth : 0.0F;
    }

    // simple rules after: silt on the flood plains of big rivers and at their mouths, gravel in mountain valleys and
    // in fans at mountain feet (WLD-09)
    std::vector<float> slope(n, 0.0F);
    std::vector<float> above(n, 0.0F);  // the steepest slope of the cells draining into each
    for (const std::int32_t c : w->order) {
        const auto cs = static_cast<std::size_t>(c);
        const std::int32_t r = w->receiver[cs];
        const double hr = w->sea(r) ? 0.0 : w->height[static_cast<std::size_t>(r)];
        slope[cs] = static_cast<float>(std::max(0.0, w->height[cs] - hr) / step_metres(g, c, r));
        above[static_cast<std::size_t>(r)] = std::max(above[static_cast<std::size_t>(r)], slope[cs]);
    }
    for (const std::int32_t c : w->order) {
        const auto cs = static_cast<std::size_t>(c);
        const float a = w->area[cs];
        const std::int32_t r = w->receiver[cs];
        const bool silt = (a > 2000.0F && slope[cs] < 0.0015F && w->height[cs] < 600.0F) ||
                          (w->sea(r) && a > 5000.0F) || w->lake[cs] > 0.0F;
        const bool gravel = (a > 300.0F && slope[cs] < 0.02F && slope[cs] >= 0.0015F) ||
                            (a > 20.0F && above[cs] > 0.04F && slope[cs] < 0.01F);
        if (!silt && !gravel) {
            continue;
        }
        const Rock top = silt ? Rock::kSilt : Rock::kGravel;
        if (top != w->surface(c)) {
            w->rock[(cs * 3) + 2] = w->rock[(cs * 3) + 1];
            w->rock[(cs * 3) + 1] = w->rock[cs * 3];
            w->rock[cs * 3] = static_cast<std::uint8_t>(top);
        }
    }
}

}  // namespace worldgen
