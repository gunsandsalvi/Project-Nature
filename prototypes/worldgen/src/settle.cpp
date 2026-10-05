// P7's settling (A7.2 step 8, WLD-08): the chosen world's plant cover, water and herds run for ten years at the play
// paces, with no people: water every game day, plant cover every five days, herds every day (WLD-12). Its rules are
// stand-ins of about the work production's will do, for the time it takes. Pre-production code (research 00).
#include <algorithm>
#include <chrono>
#include <cmath>

#include "chance.hpp"
#include "draws.hpp"
#include "life.hpp"
#include "maths.hpp"
#include "world.hpp"

namespace worldgen {

namespace {

// Each land biome's cover once grown: trees, bushes, and grass and herbs (WLD-31).
struct Cover {
    float trees = 0.0F;
    float bushes = 0.0F;
    float grass = 0.0F;
};
constexpr std::array<Cover, kBiomes> kGrown = {
    Cover{0.0F, 0.0F, 0.0F},  Cover{0.0F, 0.0F, 0.0F},   Cover{0.0F, 0.2F, 0.6F},  Cover{0.75F, 0.1F, 0.1F},
    Cover{0.8F, 0.1F, 0.1F},  Cover{0.05F, 0.1F, 0.8F},  Cover{0.05F, 0.5F, 0.3F}, Cover{0.0F, 0.1F, 0.1F},
    Cover{0.2F, 0.2F, 0.55F}, Cover{0.9F, 0.05F, 0.05F}, Cover{0.1F, 0.2F, 0.6F},  Cover{0.0F, 0.1F, 0.4F},
    Cover{0.05F, 0.2F, 0.5F},
};

constexpr int kYear = 60;  // game days (TIM-18)

}  // namespace

std::array<float, 3> grown_cover(Biome b) {
    const Cover& c = kGrown[static_cast<std::size_t>(b)];
    return {c.trees, c.bushes, c.grass};
}

double settle(World* w, int years, minds::Pool* pool) {
    const auto t0 = std::chrono::steady_clock::now();
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    const double cell_km = g.metres / 1000.0;

    // the cover at generation: each cell at some stage of regrowth since its last fire or flood (WLD-09 step 7)
    std::vector<Cover> cover(n);
    for (std::size_t c = 0; c < n; ++c) {
        if (w->sea(static_cast<int>(c))) {
            continue;
        }
        const Cover grown = kGrown[w->biome[c]];
        const auto stage = static_cast<float>(0.4 + (0.6 * samebits::chance(w->seed, c, 0, Draw::kFire, 0)));
        cover[c] = {grown.trees * stage, grown.bushes * stage, grown.grass};
    }
    // where each species may go, worked out once: a bit for each
    std::vector<std::uint16_t> habitat(n, 0);
    pool->run(g.height, [&](int y, int /*thread*/) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            if (w->sea(c)) {
                continue;
            }
            std::uint16_t bits = 0;
            for (int s = 0; s < kSpecies; ++s) {
                bits |= lives_in(static_cast<Species>(s), *w, c)
                            ? static_cast<std::uint16_t>(1U << static_cast<unsigned>(s))
                            : 0;
            }
            habitat[static_cast<std::size_t>(c)] = bits;
        }
    });
    // the land cells basin by basin, each from its mouth up, depth first, so a cell's donors lie close after it and
    // the day's routing reads memory nearly in order; at[c] is a cell's place in it
    std::vector<std::int32_t> basin;
    std::vector<std::int32_t> at(n, -1);
    {
        std::vector<std::int32_t> first(n + 1, 0);
        for (const std::int32_t c : w->order) {
            ++first[static_cast<std::size_t>(w->receiver[static_cast<std::size_t>(c)]) + 1];
        }
        for (std::size_t c = 0; c < n; ++c) {
            first[c + 1] += first[c];
        }
        std::vector<std::int32_t> donors(w->order.size());
        std::vector<std::int32_t> fill(first.begin(), first.end() - 1);
        for (const std::int32_t c : w->order) {
            const auto r = static_cast<std::size_t>(w->receiver[static_cast<std::size_t>(c)]);
            donors[static_cast<std::size_t>(fill[r]++)] = c;
        }
        basin.reserve(w->order.size());
        std::vector<std::int32_t> stack;
        for (std::size_t s = 0; s < n; ++s) {
            if (!w->sea(static_cast<int>(s))) {
                continue;
            }
            for (std::int32_t k = first[s]; k < first[s + 1]; ++k) {
                stack.push_back(donors[static_cast<std::size_t>(k)]);
                while (!stack.empty()) {
                    const std::int32_t c = stack.back();
                    stack.pop_back();
                    at[static_cast<std::size_t>(c)] = static_cast<std::int32_t>(basin.size());
                    basin.push_back(c);
                    for (std::int32_t j = first[static_cast<std::size_t>(c)];
                         j < first[static_cast<std::size_t>(c) + 1]; ++j) {
                        stack.push_back(donors[static_cast<std::size_t>(j)]);
                    }
                }
            }
        }
    }
    // each place's receiver's place, or -1 at the sea
    std::vector<std::int32_t> down(basin.size());
    for (std::size_t i = 0; i < basin.size(); ++i) {
        down[i] = at[static_cast<std::size_t>(w->receiver[static_cast<std::size_t>(basin[i])])];
    }
    std::vector<float> soil_water(n, 75.0F);
    std::vector<float> runoff(n, 0.0F);
    std::vector<float> flow(basin.size(), 0.0F);  // by place in the basins' order
    std::vector<float> grazed(n, 0.0F);

    // the weather cells, about 10 km across (WLD-16), each with its rain drawn by the day
    const int wc = std::max(1, static_cast<int>(std::lround(10.0 / cell_km)));
    const int wx = (g.width + wc - 1) / wc;
    std::array<double, 12> season{};
    for (int m = 0; m < 12; ++m) {
        season[static_cast<std::size_t>(m)] = -samebits::cosine(6.28318530717958623200 * (m + 0.5) / 12.0);
    }

    const int wy = (g.height + wc - 1) / wc;
    std::vector<std::uint8_t> raining(static_cast<std::size_t>(wx) * static_cast<std::size_t>(wy));
    for (int day = 0; day < years * kYear; ++day) {
        const int month = (day % kYear) / 5;
        // the day's weather, drawn once for each weather cell
        for (std::size_t cell = 0; cell < raining.size(); ++cell) {
            raining[cell] =
                samebits::chance(w->seed, cell, static_cast<std::uint64_t>(day), Draw::kWeather, 0) < 0.3 ? 1 : 0;
        }
        // water: each land cell's bucket takes the day's rain and loses what the warmth draws, the rest runs off
        pool->run(g.height, [&](int y, int /*thread*/) {
            const double half = g.latitude(y) >= 0.0 ? 1.0 : -1.0;
            for (int x = 0; x < g.width; ++x) {
                const int c = g.at(x, y);
                const auto cs = static_cast<std::size_t>(c);
                if (w->sea(c)) {
                    continue;
                }
                const int weather = ((y / wc) * wx) + (x / wc);
                const auto cell = static_cast<std::size_t>(weather);
                const double rain = raining[cell] != 0 ? (w->rain[cs] / kYear) / 0.3 : 0.0;
                const double swing = (w->warm[cs] - w->cold[cs]) / 2.0;
                const double t = w->temperature[cs] + (half * swing * season[static_cast<std::size_t>(month)]);
                const double draw = t > 0.0 ? ((4.0 * t) + (0.04 * t * t)) / 5.0 : 0.0;
                double bucket = soil_water[cs] + rain;
                const double over = std::max(0.0, bucket - 150.0);
                bucket = std::min(150.0, bucket);
                bucket -= std::min(draw, bucket);
                soil_water[cs] = static_cast<float>(bucket);
                runoff[cs] = static_cast<float>(over);
            }
        });
        // rivers: the day's runoff gathered from the far ends of every basin to the sea
        for (std::size_t i = 0; i < basin.size(); ++i) {
            flow[i] = runoff[static_cast<std::size_t>(basin[i])];
        }
        for (std::size_t i = basin.size(); i-- > 0;) {
            if (down[i] >= 0) {
                flow[static_cast<std::size_t>(down[i])] += flow[i];
            }
        }

        // herds: each moves a cell a day toward food, water and its season's range, eats, breeds in spring and dies
        // of hunger (WLD-32, WLD-18)
        const bool summer = (day % kYear) >= 15 && (day % kYear) < 45;
        for (Herd& h : w->herds) {
            const int home = summer ? h.summer : h.winter;
            const auto mine = static_cast<std::uint16_t>(1U << static_cast<unsigned>(h.species));
            int best = h.cell;
            double best_score = -1.0e9;
            for (int k = -1; k < 8; ++k) {
                const int c = k < 0 ? h.cell
                                    : g.at(g.x_of(h.cell) + kDx[static_cast<std::size_t>(k)],
                                           g.y_of(h.cell) + kDy[static_cast<std::size_t>(k)]);
                const auto cs = static_cast<std::size_t>(c);
                if ((habitat[cs] & mine) == 0) {
                    continue;
                }
                const double dx = std::abs(g.x_of(c) - g.x_of(home));
                const double dy = std::abs(g.y_of(c) - g.y_of(home));
                const double food = cover[cs].grass + cover[cs].bushes - grazed[cs];
                const double score =
                    food + (flow[static_cast<std::size_t>(at[cs])] > 0.0F ? 0.2 : 0.0) - (0.02 * (dx + dy));
                if (score > best_score) {
                    best_score = score;
                    best = c;
                }
            }
            h.cell = best;
            const auto cs = static_cast<std::size_t>(best);
            const double food = std::max(0.0, static_cast<double>(cover[cs].grass + cover[cs].bushes - grazed[cs]));
            grazed[cs] += h.count * 0.0005F;
            h.condition = static_cast<float>(std::clamp(h.condition + (0.05 * (food - 0.3)), 0.0, 1.0));
            if ((day % kYear) < 10 && h.condition > 0.6F) {
                h.count *= 1.004F;
            }
            h.count *= static_cast<float>(1.0 - (0.0006 * (1.5 - h.condition)));
        }

        // plant cover every five days: growing back toward its biome where the soil holds water, grazed, and burned
        // where lightning strikes dry land
        if (day % 5 == 4) {
            pool->run(g.height, [&](int y, int /*thread*/) {
                for (int x = 0; x < g.width; ++x) {
                    const int c = g.at(x, y);
                    const auto cs = static_cast<std::size_t>(c);
                    if (w->sea(c)) {
                        continue;
                    }
                    const Cover grown = kGrown[w->biome[cs]];
                    const float water = soil_water[cs] / 150.0F;
                    Cover& v = cover[cs];
                    v.grass += (grown.grass - v.grass) * 0.08F * water;
                    v.bushes += (grown.bushes - v.bushes) * 0.02F * water;
                    v.trees += (grown.trees - v.trees) * 0.004F * water;
                    v.grass = std::max(0.0F, v.grass - (grazed[cs] * 0.5F));
                    v.bushes = std::max(0.0F, v.bushes - (grazed[cs] * 0.2F));
                    grazed[cs] = 0.0F;
                    const double dry = 1.0 - w->wetness[cs];
                    const double strike = w->warm[cs] > 15.0F ? 0.0004 * dry * dry : 0.0;
                    if (strike > 0.0 &&
                        samebits::chance(w->seed, cs, static_cast<std::uint64_t>(day), Draw::kFire, 1) < strike) {
                        v.trees *= 0.2F;
                        v.bushes *= 0.2F;
                        v.grass *= 0.5F;
                    }
                }
            });
        }
    }
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
}

}  // namespace worldgen
