// P7's candidates (A7.3, WLD-10, WLD-24): each world made, its start region found by scoring, its must-haves checked
// on the start's landmass, and the reasons logged; the best few made again at full size and the best three offered.
// Pre-production code (research 00).
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>

#include "hash.hpp"
#include "life.hpp"
#include "world.hpp"

namespace worldgen {

namespace {

using Clock = std::chrono::steady_clock;

double since(Clock::time_point t0) {
    return std::chrono::duration<double>(Clock::now() - t0).count();
}

// People a km² the land feeds in its leanest season with the starting kit (BIO-02): gathering, scavenging and small
// game, about one person to 10 km² of good land (WLD-04).
double feeds(const World& w, int c) {
    const auto cs = static_cast<std::size_t>(c);
    constexpr std::array<double, kBiomes> kBiomeFood = {0.0,  0.0, 0.03, 0.05, 0.1,  0.09, 0.04,
                                                        0.01, 0.1, 0.06, 0.14, 0.02, 0.14};
    double f = kBiomeFood[w.biome[cs]] * (0.6 + (0.1 * w.fertility[cs]));
    if (w.area[cs] > 100.0F || w.lake[cs] > 0.0F) {
        f += 0.05;  // fish
    }
    const float cold = w.cold[cs];
    return f * (cold < 0.0F ? 0.6 : (cold < 5.0F ? 0.8 : 1.0));
}

// The kinds of food a cell offers: plants, each big animal that lives there, fish, and shellfish on a shore.
int kinds(const World& w, int c) {
    const auto cs = static_cast<std::size_t>(c);
    const auto b = static_cast<Biome>(w.biome[cs]);
    int k = (b != Biome::kIce && b != Biome::kDesert) ? 1 : 0;
    for (int s = 0; s < kSpecies - 2; ++s) {
        k += lives_in(static_cast<Species>(s), w, c) ? 1 : 0;
    }
    k += (w.area[cs] > 100.0F || w.lake[cs] > 0.0F) ? 1 : 0;
    k += b == Biome::kShore ? 1 : 0;
    return k;
}

bool flakes(const World& w, int c) {
    return w.deposit(c, kFlint) > 0 || w.deposit(c, kChert) > 0 || w.deposit(c, kObsidian) > 0;
}

// Sums of a value over a square of (2r + 1) cells a side round every cell, on the torus.
std::vector<double> box_sum(const Grid& g, const std::vector<double>& v, int r) {
    std::vector<double> rows(v.size(), 0.0);
    std::vector<double> out(v.size(), 0.0);
    for (int y = 0; y < g.height; ++y) {
        for (int x = 0; x < g.width; ++x) {
            double s = 0.0;
            for (int d = -r; d <= r; ++d) {
                s += v[static_cast<std::size_t>(g.at(x + d, y))];
            }
            rows[static_cast<std::size_t>(g.at(x, y))] = s;
        }
    }
    for (int y = 0; y < g.height; ++y) {
        for (int x = 0; x < g.width; ++x) {
            double s = 0.0;
            for (int d = -r; d <= r; ++d) {
                s += rows[static_cast<std::size_t>(g.at(x, y + d))];
            }
            out[static_cast<std::size_t>(g.at(x, y))] = s;
        }
    }
    return out;
}

const char* biome_name(Biome b) {
    constexpr std::array<const char*, kBiomes> kNames = {
        "sea",    "ice",     "tundra",          "conifer forest", "broadleaf forest", "grassland", "dry scrub",
        "desert", "savanna", "tropical forest", "marsh",          "mountain heights", "shore"};
    return kNames[static_cast<std::size_t>(b)];
}

}  // namespace

void judge(World* w) {
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    const double cell_km = g.metres / 1000.0;
    const double km2 = cell_km * cell_km;
    // a square of the circle's area round each cell: 10 km to food and stone, 2 km to water, 30 km for the region
    const int r_food = std::max(1, static_cast<int>(std::lround(8.86 / cell_km)));
    const int r_water = std::max(1, static_cast<int>(std::lround(1.77 / cell_km)));
    const int r_region = std::max(2, static_cast<int>(std::lround(26.6 / cell_km)));

    std::vector<double> food(n, 0.0);
    std::vector<double> stone(n, 0.0);
    std::vector<double> water(n, 0.0);
    std::vector<double> kind(n, 0.0);
    for (std::size_t c = 0; c < n; ++c) {
        const int ci = static_cast<int>(c);
        if (w->sea(ci)) {
            continue;
        }
        food[c] = feeds(*w, ci) * km2;
        stone[c] = flakes(*w, ci) ? 1.0 : 0.0;
        water[c] = (w->area[c] > 10.0F || w->lake[c] > 0.0F) ? 1.0 : 0.0;
        kind[c] = kinds(*w, ci);
    }
    const std::vector<double> food_near = box_sum(g, food, r_food);
    const std::vector<double> stone_near = box_sum(g, stone, r_food);
    const std::vector<double> water_near = box_sum(g, water, r_water);

    // a shelter: a dry cave or overhang where winters matter, with water within about 2 km and, within about 10 km,
    // food for a band of 25 in the leanest season with a margin, and stone that flakes (WLD-24)
    constexpr double kBand = 25.0;
    std::vector<double> shelter(n, 0.0);
    for (std::size_t c = 0; c < n; ++c) {
        if (w->caves[c] == 0 || w->sea(static_cast<int>(c))) {
            continue;
        }
        const float cold = w->cold[c];
        if (cold >= 2.0F && cold <= 10.0F && water_near[c] > 0.0 && stone_near[c] > 0.0 &&
            food_near[c] >= kBand * 1.2) {
            shelter[c] = 1.0;
        }
    }
    const std::vector<double> shelters = box_sum(g, shelter, r_region);

    // the region: the middle with the most shelters for 3 or 4 bands (BIO-03), then the bigger food margin
    Start best;
    for (std::size_t c = 0; c < n; ++c) {
        if (shelter[c] == 0.0 || shelters[c] < 3.0) {
            continue;
        }
        const double margin = food_near[c] / (kBand * 1.2);
        const double score = std::min(4.0, shelters[c]) + std::min(3.0, margin) + (0.2 * kind[c]);
        if (best.cell < 0 || score > best.score) {
            best = {static_cast<std::int32_t>(c), static_cast<int>(shelters[c]), static_cast<int>(kind[c]), margin,
                    score};
        }
    }
    w->start = best;
    char line[400];
    if (best.cell < 0) {
        double most = 0.0;
        for (const double s : shelters) {
            most = std::max(most, s);
        }
        w->qualifies = false;
        w->score = 0.0;
        std::snprintf(line, sizeof line, "rejected: no start region (at most %d shelters together)",
                      static_cast<int>(most));
        w->reasons = line;
        w->summary = w->reasons;
        return;
    }

    // the start's landmass and what it holds: the arc's needs (WLD-10, TIM-19)
    std::vector<std::uint8_t> mass(n, 0);
    std::vector<std::int32_t> stack{best.cell};
    mass[static_cast<std::size_t>(best.cell)] = 1;
    bool has_stone = false;
    bool has_clay = false;
    bool has_grains = false;
    bool has_wolves = false;
    bool has_tame = false;
    bool has_copper = false;
    std::int64_t land = 0;
    while (!stack.empty()) {
        const std::int32_t c = stack.back();
        stack.pop_back();
        ++land;
        has_stone = has_stone || flakes(*w, c);
        has_clay = has_clay || w->deposit(c, kClay) > 0;
        has_grains = has_grains || w->grains[static_cast<std::size_t>(c)] != 0;
        has_copper = has_copper || w->deposit(c, kCopperOre) > 0;
        has_wolves = has_wolves || lives_in(Species::kWolf, *w, c);
        has_tame = has_tame || lives_in(Species::kWildCattle, *w, c) || lives_in(Species::kWildGoat, *w, c) ||
                   lives_in(Species::kWildSheep, *w, c) || lives_in(Species::kBoar, *w, c);
        for (int k = 0; k < 8; ++k) {
            const int nb =
                g.at(g.x_of(c) + kDx[static_cast<std::size_t>(k)], g.y_of(c) + kDy[static_cast<std::size_t>(k)]);
            if (mass[static_cast<std::size_t>(nb)] == 0 && !w->sea(nb)) {
                mass[static_cast<std::size_t>(nb)] = 1;
                stack.push_back(nb);
            }
        }
    }
    std::string missing;
    const auto lack = [&missing](bool has, const char* what) {
        if (!has) {
            missing += missing.empty() ? what : std::string(", ") + what;
        }
    };
    lack(has_stone, "stone that flakes");
    lack(has_clay, "clay");
    lack(has_grains, "wild grains");
    lack(has_wolves, "wolves");
    lack(has_tame, "a herd animal with a domestic kind");
    lack(has_copper, "copper ore");

    // the world's score (WLD-10): varied land and climates, barriers, unevenly spread resources, and the start's own
    std::array<std::int64_t, kBiomes> biomes{};
    std::int64_t land_all = 0;
    std::int64_t high = 0;
    for (std::size_t c = 0; c < n; ++c) {
        if (w->sea(static_cast<int>(c))) {
            continue;
        }
        ++land_all;
        ++biomes[w->biome[c]];
        high += w->height[c] > 2000.0F ? 1 : 0;
    }
    int varied = 0;
    for (const std::int64_t b : biomes) {
        varied += static_cast<double>(b) >= 0.01 * static_cast<double>(land_all) ? 1 : 0;
    }
    const double mass_share = static_cast<double>(land) / static_cast<double>(std::max<std::int64_t>(1, land_all));
    const double barriers = (1.0 - mass_share) + (10.0 * static_cast<double>(high) /
                                                  static_cast<double>(std::max<std::int64_t>(1, land_all)));
    w->score = (0.3 * varied) + barriers + best.score;
    w->qualifies = missing.empty();
    const int sx = g.x_of(best.cell);
    const int sy = g.y_of(best.cell);
    const double lat = g.latitude(sy);
    if (w->qualifies) {
        std::snprintf(line, sizeof line,
                      "qualifies, score %.2f: start at %.0f°%s, %.0f km east, in %s, %d shelters, food %.1f times the "
                      "bands' need, %d kinds",
                      w->score, std::abs(lat), lat >= 0.0 ? "N" : "S", sx * cell_km,
                      biome_name(static_cast<Biome>(w->biome[static_cast<std::size_t>(best.cell)])), best.shelters,
                      best.margin, best.food_kinds);
    } else {
        std::snprintf(line, sizeof line, "rejected: its start's land lacks %s", missing.c_str());
    }
    w->reasons = line;
    std::snprintf(line, sizeof line, "%.0f%% land, tilt %.0f°, %d biomes; start at %.0f°%s in %s with %d caves",
                  100.0 * static_cast<double>(land_all) / static_cast<double>(n), w->tilt, varied, std::abs(lat),
                  lat >= 0.0 ? "N" : "S", biome_name(static_cast<Biome>(w->biome[static_cast<std::size_t>(best.cell)])),
                  best.shelters);
    w->summary = line;
}

World make_world(std::uint64_t seed, int width, bool full, const Settings& settings, minds::Pool* pool, Times* times) {
    World w;
    w.seed = seed;
    w.full = full;
    w.grid = {width, width / 2, kAroundMetres / width};
    auto t = Clock::now();
    make_plates(&w, pool);
    make_rock(&w, pool);
    times->plates += since(t);
    t = Clock::now();
    erode(&w, full ? settings.coarse_steps * 2 : settings.coarse_steps,
          full ? settings.coarse_years / 2.0 : settings.coarse_years);
    times->erosion += since(t);
    t = Clock::now();
    make_climate(&w, pool);
    times->climate += since(t);
    t = Clock::now();
    make_life(&w, pool);
    if (full) {
        place_herds(&w);
    }
    times->life += since(t);
    t = Clock::now();
    judge(&w);
    times->score += since(t);
    return w;
}

World refine(const World& coarse, const Settings& settings, minds::Pool* pool, Times* times) {
    World w;
    w.seed = coarse.seed;
    w.full = true;
    w.grid = {settings.full_width, settings.full_width / 2, kAroundMetres / settings.full_width};
    auto t = Clock::now();
    make_plates(&w, pool);
    make_rock(&w, pool);
    times->plates += since(t);
    // the coarse world's worn land, laid on the full grid, with the full grid's finer relief from the plates' stage
    // on top, then worn again at full size
    t = Clock::now();
    const Grid& g = w.grid;
    const Grid& gc = coarse.grid;
    World plain;  // the coarse grid's land before wearing, to take the finer relief from
    plain.seed = coarse.seed;
    plain.grid = gc;
    minds::Pool one(1);
    make_plates(&plain, &one);
    const double s = static_cast<double>(gc.width) / g.width;
    std::vector<float> worn(static_cast<std::size_t>(g.cells()));
    for (int y = 0; y < g.height; ++y) {
        for (int x = 0; x < g.width; ++x) {
            const double fx = ((x + 0.5) * s) - 0.5;
            const double fy = ((y + 0.5) * s) - 0.5;
            const int x0 = static_cast<int>(std::floor(fx));
            const int y0 = static_cast<int>(std::floor(fy));
            const double tx = fx - x0;
            const double ty = fy - y0;
            const auto lerp2 = [&](const std::vector<float>& v) {
                const double a = v[static_cast<std::size_t>(gc.at(x0, y0))];
                const double b = v[static_cast<std::size_t>(gc.at(x0 + 1, y0))];
                const double c = v[static_cast<std::size_t>(gc.at(x0, y0 + 1))];
                const double d = v[static_cast<std::size_t>(gc.at(x0 + 1, y0 + 1))];
                return (a + ((b - a) * tx)) + (((c + ((d - c) * tx)) - (a + ((b - a) * tx))) * ty);
            };
            const auto i = static_cast<std::size_t>(g.at(x, y));
            const double base = lerp2(coarse.height);
            const double detail = w.height[i] - lerp2(plain.height);
            // the sea stays where the coarse world put it; the land keeps at least a metre above it
            const double h = base <= 0.0 ? std::min(0.0, base + detail) : std::max(1.0, base + detail);
            worn[i] = static_cast<float>(h);
        }
    }
    w.height = std::move(worn);
    erode(&w, settings.full_steps, settings.full_years);
    times->erosion += since(t);
    t = Clock::now();
    make_climate(&w, pool);
    times->climate += since(t);
    t = Clock::now();
    make_life(&w, pool);
    place_herds(&w);
    times->life += since(t);
    t = Clock::now();
    judge(&w);
    times->score += since(t);
    return w;
}

Offer new_world(const Settings& settings) {
    Offer offer;
    minds::Pool pool(settings.threads);
    // the candidates at the coarse size, each wholly on one thread; more while fewer than three qualify
    auto t = Clock::now();
    std::vector<std::unique_ptr<World>> worlds;
    std::vector<Times> times;
    int made = 0;
    int wanted = settings.candidates;
    for (;;) {
        const int count = wanted - made;
        worlds.resize(static_cast<std::size_t>(wanted));
        times.resize(static_cast<std::size_t>(wanted));
        pool.run(count, [&](int i, int /*thread*/) {
            const int k = made + i;
            minds::Pool one(1);
            worlds[static_cast<std::size_t>(k)] =
                std::make_unique<World>(make_world(candidate_seed(settings.seed, k), settings.coarse_width, false,
                                                   settings, &one, &times[static_cast<std::size_t>(k)]));
        });
        made = wanted;
        int qualified = 0;
        for (const auto& w : worlds) {
            qualified += w->qualifies ? 1 : 0;
        }
        if (qualified >= 3 || made >= settings.most) {
            break;
        }
        wanted = std::min(settings.most, made + 10);
    }
    offer.made = made;
    for (int k = 0; k < made; ++k) {
        const World& w = *worlds[static_cast<std::size_t>(k)];
        offer.qualified += w.qualifies ? 1 : 0;
        char head[40];
        std::snprintf(head, sizeof head, "candidate %d: ", k + 1);
        offer.log.push_back(head + w.reasons);
        const Times& tk = times[static_cast<std::size_t>(k)];
        offer.times.plates += tk.plates;
        offer.times.erosion += tk.erosion;
        offer.times.climate += tk.climate;
        offer.times.life += tk.life;
        offer.times.score += tk.score;
    }
    offer.candidates_seconds = since(t);

    // the best few that qualify, by score and then by number, made again at full size, each on one thread
    t = Clock::now();
    std::vector<int> ranked;
    for (int k = 0; k < made; ++k) {
        if (worlds[static_cast<std::size_t>(k)]->qualifies) {
            ranked.push_back(k);
        }
    }
    std::stable_sort(ranked.begin(), ranked.end(), [&worlds](int a, int b) {
        return worlds[static_cast<std::size_t>(a)]->score > worlds[static_cast<std::size_t>(b)]->score;
    });
    ranked.resize(std::min<std::size_t>(ranked.size(), static_cast<std::size_t>(settings.best)));
    std::vector<World> full(ranked.size());
    std::vector<Times> full_times(ranked.size());
    pool.run(static_cast<int>(ranked.size()), [&](int i, int /*thread*/) {
        minds::Pool one(1);
        full[static_cast<std::size_t>(i)] =
            refine(*worlds[static_cast<std::size_t>(ranked[static_cast<std::size_t>(i)])], settings, &one,
                   &full_times[static_cast<std::size_t>(i)]);
    });
    for (std::size_t i = 0; i < full.size(); ++i) {
        char head[60];
        std::snprintf(head, sizeof head, "candidate %d at full size: ", ranked[i] + 1);
        offer.log.push_back(head + full[i].reasons);
        offer.times.plates += full_times[i].plates;
        offer.times.erosion += full_times[i].erosion;
        offer.times.climate += full_times[i].climate;
        offer.times.life += full_times[i].life;
        offer.times.score += full_times[i].score;
    }
    std::vector<std::size_t> order(full.size());
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = i;
    }
    std::stable_sort(order.begin(), order.end(), [&full](std::size_t a, std::size_t b) {
        const bool qa = full[a].qualifies;
        const bool qb = full[b].qualifies;
        return qa != qb ? qa : full[a].score > full[b].score;
    });
    for (std::size_t i = 0; i < order.size() && offer.three.size() < 3; ++i) {
        if (full[order[i]].qualifies) {
            offer.three.push_back(std::move(full[order[i]]));
        }
    }
    offer.best_seconds = since(t);
    return offer;
}

std::uint64_t World::checksum() const {
    std::uint64_t h = samebits::kHashStart;
    for (const float v : height) {
        samebits::hash_into(&h, v);
    }
    for (const std::uint8_t v : rock) {
        samebits::hash_into(&h, static_cast<std::uint64_t>(v));
    }
    for (const float v : area) {
        samebits::hash_into(&h, v);
    }
    for (std::size_t c = 0; c < temperature.size(); ++c) {
        samebits::hash_into(&h, temperature[c]);
        samebits::hash_into(&h, rain[c]);
        samebits::hash_into(&h, wetness[c]);
    }
    for (std::size_t c = 0; c < biome.size(); ++c) {
        samebits::hash_into(&h, static_cast<std::uint64_t>(biome[c]) | (static_cast<std::uint64_t>(soil[c]) << 8U) |
                                    (static_cast<std::uint64_t>(fertility[c]) << 16U) |
                                    (static_cast<std::uint64_t>(caves[c]) << 24U) |
                                    (static_cast<std::uint64_t>(grains[c]) << 32U));
    }
    for (const std::uint8_t v : deposits) {
        samebits::hash_into(&h, static_cast<std::uint64_t>(v));
    }
    for (const Herd& herd : herds) {
        samebits::hash_into(&h, static_cast<std::uint64_t>(static_cast<std::uint32_t>(herd.cell)));
        samebits::hash_into(&h, herd.count);
        samebits::hash_into(&h, herd.condition);
    }
    samebits::hash_into(&h, static_cast<std::uint64_t>(static_cast<std::uint32_t>(start.cell)));
    return h;
}

}  // namespace worldgen
