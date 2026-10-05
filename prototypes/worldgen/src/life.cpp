// P7's living layers (A7.2 steps 6 and 7, WLD-09): biomes by BIOME1's numbers, soils by Jenny's factors, caves,
// deposits by geology, wild grains, and the big animals placed where their food and cover are. Pre-production code
// (research 00).
#include <algorithm>
#include <cmath>

#include "chance.hpp"
#include "draws.hpp"
#include "life.hpp"
#include "maths.hpp"
#include "world.hpp"

namespace worldgen {

namespace {

bool in(Rock r, std::initializer_list<Rock> kinds) {
    return std::find(kinds.begin(), kinds.end(), r) != kinds.end();
}

// A count of 0 to 3 drawn so that each is as likely as `per_km2` things in a cell of the grid's area would make it.
int draw_count(const World& w, int c, double per_km2, std::uint64_t index) {
    const double km2 = (w.grid.metres / 1000.0) * (w.grid.metres / 1000.0);
    const double p = 1.0 - samebits::exponent(-per_km2 * km2);
    int count = 0;
    for (std::uint64_t k = 0; k < 3; ++k) {
        if (samebits::chance(w.seed, static_cast<std::uint64_t>(c), index, Draw::kCave, k) < p) {
            ++count;
        }
    }
    return count;
}

}  // namespace

double slope_of(const World& w, int c) {
    const auto cs = static_cast<std::size_t>(c);
    const std::int32_t r = w.receiver[cs];
    if (r == c || r < 0) {
        return 0.0;
    }
    const Grid& g = w.grid;
    const int dx = std::abs(g.x_of(c) - g.x_of(r));
    const int dy = std::abs(g.y_of(c) - g.y_of(r));
    const double hr = w.sea(r) ? 0.0 : w.height[static_cast<std::size_t>(r)];
    return std::max(0.0, w.height[cs] - hr) / (((dx == 0 || dy == 0) ? 1.0 : kDiagonal) * g.metres);
}

bool lives_in(Species s, const World& w, int c) {
    const auto b = static_cast<Biome>(w.biome[static_cast<std::size_t>(c)]);
    const double slope = slope_of(w, c);
    switch (s) {
        case Species::kRedDeer:
            return b == Biome::kBroadleaf || b == Biome::kConifer || b == Biome::kGrassland;
        case Species::kReindeer:
            return b == Biome::kTundra || b == Biome::kConifer;
        case Species::kWildCattle:
            return b == Biome::kBroadleaf || b == Biome::kGrassland || b == Biome::kMarsh;
        case Species::kBison:
            return b == Biome::kGrassland || (b == Biome::kConifer && w.cold[static_cast<std::size_t>(c)] > -20.0F);
        case Species::kHorse:
            return b == Biome::kGrassland || b == Biome::kScrub || b == Biome::kTundra;
        case Species::kWildGoat:
            return b == Biome::kHeights || ((b == Biome::kScrub || b == Biome::kGrassland) && slope > 0.04);
        case Species::kWildSheep:
            return b == Biome::kHeights || b == Biome::kScrub || (b == Biome::kGrassland && slope > 0.02);
        case Species::kBoar:
            return b == Biome::kBroadleaf || b == Biome::kMarsh;
        case Species::kWolf:
            return b == Biome::kTundra || b == Biome::kConifer || b == Biome::kBroadleaf || b == Biome::kGrassland ||
                   b == Biome::kScrub || b == Biome::kHeights;
        case Species::kBear:
            return b == Biome::kConifer || b == Biome::kBroadleaf || b == Biome::kHeights;
    }
    return false;
}

void make_life(World* w, minds::Pool* pool) {
    const Grid& g = w->grid;
    const auto n = static_cast<std::size_t>(g.cells());
    const double cell_km = g.metres / 1000.0;

    // biomes: what each cell's climate and wetness would grow if left alone (WLD-31)
    w->biome.assign(n, 0);
    pool->run(g.height, [&](int y, int /*thread*/) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            const auto cs = static_cast<std::size_t>(c);
            Biome b = Biome::kSea;
            const float cold = w->cold[cs];
            const float warm = w->warm[cs];
            const float wet = w->wetness[cs];
            const double slope = slope_of(*w, c);
            if (w->sea(c)) {
                b = Biome::kSea;
            } else if (warm < 0.0F) {
                b = Biome::kIce;
            } else if (warm < 10.0F) {
                b = w->height[cs] > 1500.0F ? Biome::kHeights : Biome::kTundra;
            } else if (wet > 0.75F && (w->lake[cs] > 0.0F || (w->area[cs] > 1000.0F && slope < 0.001))) {
                b = Biome::kMarsh;
            } else if (cold >= 15.0F) {
                b = wet >= 0.8F ? Biome::kTropical
                                : (wet >= 0.45F ? Biome::kSavanna : (wet >= 0.25F ? Biome::kScrub : Biome::kDesert));
            } else if (cold < -8.0F) {
                b = wet >= 0.5F ? Biome::kConifer : (wet >= 0.3F ? Biome::kGrassland : Biome::kDesert);
            } else {
                b = wet >= 0.65F ? Biome::kBroadleaf
                                 : (wet >= 0.4F ? Biome::kGrassland : (wet >= 0.25F ? Biome::kScrub : Biome::kDesert));
            }
            if (b != Biome::kSea && b != Biome::kIce && w->height[cs] < 20.0F) {
                for (int k = 0; k < 8; ++k) {
                    if (w->sea(g.at(x + kDx[static_cast<std::size_t>(k)], y + kDy[static_cast<std::size_t>(k)]))) {
                        b = Biome::kShore;
                        break;
                    }
                }
            }
            w->biome[cs] = static_cast<std::uint8_t>(b);
        }
    });

    // soils and their fertility: from the rock beneath, the climate, the slope and the plants (WLD-27)
    w->soil.assign(n, 0);
    w->fertility.assign(n, 0);
    w->caves.assign(n, 0);
    w->grains.assign(n, 0);
    w->deposits.assign(n * kDeposits, 0);
    pool->run(g.height, [&](int y, int /*thread*/) {
        for (int x = 0; x < g.width; ++x) {
            const int c = g.at(x, y);
            const auto cs = static_cast<std::size_t>(c);
            if (w->sea(c)) {
                continue;
            }
            const auto b = static_cast<Biome>(w->biome[cs]);
            const Rock top = w->surface(c);
            const auto below = static_cast<Rock>(w->rock[(cs * 3) + 1]);
            const double slope = slope_of(*w, c);
            const float t = w->temperature[cs];
            const float wet = w->wetness[cs];
            Soil s = Soil::kLoam;
            if (b == Biome::kIce || b == Biome::kTundra || b == Biome::kHeights) {
                s = slope > 0.25 ? Soil::kThin : Soil::kFrozen;
            } else if (slope > 0.25) {
                s = Soil::kThin;
            } else if (top == Rock::kSilt) {
                s = Soil::kSilt;
            } else if (b == Biome::kMarsh) {
                s = Soil::kPeat;
            } else if (b == Biome::kDesert) {
                s = Soil::kDesert;
            } else if (in(top, {Rock::kLavaAsh, Rock::kGlassyLava})) {
                s = Soil::kVolcanic;
            } else if (b == Biome::kTropical || (b == Biome::kSavanna && wet > 0.6F)) {
                s = Soil::kWashed;
            } else if (b == Biome::kGrassland && !in(top, {Rock::kSandstone, Rock::kQuartzite})) {
                s = Soil::kBlack;
            } else if (in(top, {Rock::kSandstone, Rock::kQuartzite, Rock::kGravel}) ||
                       (top == Rock::kGranite && b == Biome::kConifer)) {
                s = Soil::kSandy;
            } else if (in(top, {Rock::kShale, Rock::kSlate})) {
                s = Soil::kClay;
            } else if (in(top, {Rock::kLimestone, Rock::kChalk})) {
                s = Soil::kChalky;
            }
            constexpr std::array<std::uint8_t, 13> kFertile = {0, 5, 4, 5, 3, 1, 2, 3, 1, 1, 0, 0, 0};
            std::uint8_t f = kFertile[static_cast<std::size_t>(s)];
            if (b == Biome::kScrub && f > 1) {
                --f;
            }
            w->soil[cs] = static_cast<std::uint8_t>(s);
            w->fertility[cs] = f;

            // dry caves and overhangs big enough for a band (WLD-09, WLD-24): in limestone, old lava and soft rock
            // under hard
            if (w->lake[cs] == 0.0F) {
                int caves = 0;
                if ((in(top, {Rock::kLimestone, Rock::kChalk}) || in(below, {Rock::kLimestone})) && slope > 0.01) {
                    caves += draw_count(*w, c, 0.3, 0);
                }
                if (in(top, {Rock::kLavaAsh, Rock::kBasalt}) && slope < 0.05) {
                    caves += draw_count(*w, c, 0.08, 1);
                }
                if (in(top, {Rock::kSandstone, Rock::kQuartzite, Rock::kGranite, Rock::kBasalt, Rock::kLimestone}) &&
                    in(below, {Rock::kShale, Rock::kChalk, Rock::kSilt, Rock::kLavaAsh}) && slope > 0.03) {
                    caves += draw_count(*w, c, 0.15, 2);
                }
                w->caves[cs] = static_cast<std::uint8_t>(std::min(3, caves));
            }

            // deposits where the rocks put them (WLD-14); flint carried downstream comes after
            std::uint8_t* d = &w->deposits[cs * kDeposits];
            const bool cut = slope > 0.02 || w->area[cs] > 100.0F;
            if (top == Rock::kChalk) {
                d[kFlint] = 3;
            } else if (below == Rock::kChalk && cut) {
                d[kFlint] = 2;
            }
            if ((top == Rock::kLimestone || (below == Rock::kLimestone && cut)) &&
                samebits::chance(w->seed, static_cast<std::uint64_t>(c), 0, Draw::kDeposit, 0) < 0.3) {
                d[kChert] = 1 + static_cast<std::uint8_t>(samebits::chance(w->seed, static_cast<std::uint64_t>(c), 0,
                                                                           Draw::kDeposit, 1) < 0.4);
            }
            if (w->volcano[cs] == 2 || top == Rock::kGlassyLava) {
                d[kObsidian] = w->volcano[cs] == 2 ? 3 : 2;
            }
            if ((in(top, {Rock::kQuartzite, Rock::kBasalt, Rock::kSandstone}) && cut) || top == Rock::kGravel) {
                d[kHammerStone] = 1;
            }
            if (top == Rock::kSilt) {
                d[kClay] = 2;
            } else if ((top == Rock::kGranite && t > 10.0F && wet > 0.6F) || top == Rock::kShale) {
                d[kClay] = 1;
            }
            if ((in(top, {Rock::kBasalt, Rock::kLavaAsh}) && t > 5.0F && wet > 0.4F) ||
                (top == Rock::kSandstone &&
                 samebits::chance(w->seed, static_cast<std::uint64_t>(c), 0, Draw::kDeposit, 2) < 0.2)) {
                d[kOchre] = 1;
            }
            if (static_cast<Setting>(w->setting[cs]) == Setting::kArc && in(top, {Rock::kLavaAsh, Rock::kGlassyLava}) &&
                (w->rock[(cs * 3) + 1] == static_cast<std::uint8_t>(Rock::kGranite) ||
                 w->rock[(cs * 3) + 2] == static_cast<std::uint8_t>(Rock::kGranite))) {
                const double km2 = cell_km * cell_km;
                const double p = 1.0 - samebits::exponent(-0.06 * km2);
                if (samebits::chance(w->seed, static_cast<std::uint64_t>(c), 0, Draw::kDeposit, 3) < p) {
                    d[kCopperOre] =
                        1 + static_cast<std::uint8_t>(
                                3.0 * samebits::chance(w->seed, static_cast<std::uint64_t>(c), 0, Draw::kDeposit, 4) *
                                0.99);
                    if (samebits::chance(w->seed, static_cast<std::uint64_t>(c), 0, Draw::kDeposit, 5) < 0.05) {
                        d[kNativeCopper] = 1;
                    }
                }
            }

            // wild grains: open, seasonal land with mild winters, as where wild wheat and barley grew
            const float cold = w->cold[cs];
            if ((b == Biome::kGrassland || b == Biome::kScrub || b == Biome::kBroadleaf) && cold > -6.0F &&
                cold < 12.0F && wet > 0.3F && wet < 0.75F && w->warm[cs] > 18.0F) {
                w->grains[cs] = 1;
            }
        }
    });

    // flint and chert carried downstream in the rivers' gravel, up to about 100 km from where they wash out
    const int reach = static_cast<int>(100.0 / cell_km);
    for (const std::int32_t source : w->order) {
        const auto ss = static_cast<std::size_t>(source);
        if (w->deposits[(ss * kDeposits) + kFlint] < 2 && w->deposits[(ss * kDeposits) + kChert] == 0) {
            continue;
        }
        std::int32_t c = source;
        for (int k = 0; k < reach; ++k) {
            const std::int32_t r = w->receiver[static_cast<std::size_t>(c)];
            if (r == c || w->sea(r)) {
                break;
            }
            c = r;
            if (w->surface(c) == Rock::kGravel) {
                std::uint8_t& f = w->deposits[(static_cast<std::size_t>(c) * kDeposits) + kFlint];
                f = std::max<std::uint8_t>(f, 1);
            }
        }
    }
}

void place_herds(World* w) {
    const Grid& g = w->grid;
    const double km2 = (g.metres / 1000.0) * (g.metres / 1000.0);
    // adults a km² where each lives, a sixth of Earth's (WLD-18, WLD-30), and the size of a herd, pack or lone animal
    constexpr std::array<double, kSpecies> kDensity = {0.33, 0.25, 0.08, 0.17, 0.1, 0.13, 0.13, 0.25, 0.0025, 0.003};
    constexpr std::array<double, kSpecies> kGroup = {12, 50, 20, 40, 15, 10, 12, 8, 6, 1};
    w->herds.clear();
    for (int c = 0; c < g.cells(); ++c) {
        if (w->sea(c)) {
            continue;
        }
        for (int s = 0; s < kSpecies; ++s) {
            const auto species = static_cast<Species>(s);
            if (!lives_in(species, *w, c)) {
                continue;
            }
            const double expected = kDensity[static_cast<std::size_t>(s)] * km2 / kGroup[static_cast<std::size_t>(s)];
            const double roll =
                samebits::chance(w->seed, static_cast<std::uint64_t>(c), static_cast<std::uint64_t>(s), Draw::kHerd, 0);
            const int herds = static_cast<int>(expected) + (roll < expected - std::floor(expected) ? 1 : 0);
            for (int h = 0; h < herds; ++h) {
                Herd herd;
                herd.cell = c;
                herd.winter = c;
                herd.species = species;
                const int which = (s * 8) + h;
                const double size = samebits::chance(w->seed, static_cast<std::uint64_t>(c),
                                                     static_cast<std::uint64_t>(which), Draw::kHerd, 1);
                herd.count = static_cast<float>(kGroup[static_cast<std::size_t>(s)] * (0.5 + size));
                // reindeer, bison and horses move to summer ranges about 150 km poleward (WLD-32)
                int summer = c;
                if (species == Species::kReindeer || species == Species::kBison || species == Species::kHorse) {
                    const int dy = static_cast<int>(150.0 / (g.metres / 1000.0));
                    summer = g.at(g.x_of(c), g.y_of(c) + (g.latitude(g.y_of(c)) >= 0.0 ? dy : -dy));
                    if (w->sea(summer)) {
                        summer = c;
                    }
                }
                herd.summer = summer;
                w->herds.push_back(herd);
            }
        }
    }
}

}  // namespace worldgen
