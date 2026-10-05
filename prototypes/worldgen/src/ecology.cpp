// P9 Ecology (IMPLEMENTATION α0.6a, A9, research 08): nature on a world's cells with nobody in it, in steps of five
// game days. Each land cell keeps its plant cover (trees and bushes, which fire thins and which grow back over years),
// the grass, browse and mast standing on it, its soil's water and its snow; and each species' count there with its
// condition. The weather's years differ region by region: wet and dry years, hard and mild winters, good and poor
// mast. Plant eaters eat what they can reach, losing condition when they cannot, and die first of hunger in a hard
// winter; hunters kill the weakest most easily and no more than they eat; each breeds once a year in spring, as well
// as its condition lets it, up to what its cell holds (WLD-18, WLD-30, WLD-32). Pre-production code (research 00).
#include "ecology.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "chance.hpp"
#include "draws.hpp"
#include "hash.hpp"
#include "maths.hpp"

namespace worldgen {

namespace {

constexpr int kTicks = 12;  // a game year of 60 days (TIM-18), in steps of five
constexpr double kDays = 5.0;
constexpr double kRegionKm = 64.0;  // a region whose year's weather is its own, about storm-sized (WLD-30)
constexpr std::size_t kWide = 6;    // regions each way that share a wider pattern
constexpr double kTau = 6.28318530717958623200;

enum : std::uint8_t {
    kRed,
    kRoe,
    kReindeer,
    kMoose,
    kCattle,
    kBison,
    kHorse,
    kGoat,
    kSheep,
    kBoar,
    kHare,
    kGazelle,
    kZebra,
    kDuiker,
    kWolf,
    kLynx,
    kLion,
    kLeopard,
};
constexpr std::uint32_t bit(int k) {
    return 1U << static_cast<unsigned>(k);
}

// The catalogue: weights, diets, breeding and life spans from field guides, rounded. Homes by biome: sea, ice,
// tundra, conifer, broadleaf, grassland, scrub, desert, savanna, tropical forest, marsh, heights, shore.
// clang-format off
const std::array<Kind, kKinds> kTable = {{
    {"red deer", 110.0, false, {0.5, 0.45, 0.05}, 1.0, 0.45, 15.0, -30.0,
     {0, 0, 0.2F, 0.7F, 1, 0.6F, 0.3F, 0, 0, 0, 0.4F, 0.3F, 0.2F}, 0},
    {"roe deer", 25.0, false, {0.25, 0.65, 0.1}, 0.8, 0.8, 10.0, -25.0,
     {0, 0, 0, 0.6F, 1, 0.5F, 0.3F, 0, 0, 0, 0.4F, 0.2F, 0.2F}, 0},
    {"reindeer", 100.0, false, {0.65, 0.35, 0.0}, 5.0, 0.4, 12.0, -99.0,
     {0, 0.1F, 1, 0.6F, 0, 0, 0, 0, 0, 0, 0.2F, 0.4F, 0.1F}, 0},
    {"moose", 400.0, false, {0.1, 0.9, 0.0}, 4.0, 0.5, 15.0, -99.0,
     {0, 0, 0.3F, 1, 0.5F, 0, 0, 0, 0, 0, 0.8F, 0.1F, 0}, 0},
    {"wild cattle", 700.0, false, {0.8, 0.2, 0.0}, 1.2, 0.35, 20.0, -20.0,
     {0, 0, 0, 0.2F, 0.6F, 1, 0.3F, 0, 0.3F, 0, 0.8F, 0, 0.2F}, 0},
    {"bison", 600.0, false, {0.9, 0.1, 0.0}, 3.0, 0.35, 20.0, -35.0,
     {0, 0, 0.1F, 0.3F, 0.2F, 1, 0.4F, 0, 0, 0, 0.2F, 0, 0}, 0},
    {"wild horse", 350.0, false, {0.95, 0.05, 0.0}, 2.0, 0.35, 20.0, -35.0,
     {0, 0, 0.4F, 0.1F, 0, 1, 0.6F, 0.1F, 0.2F, 0, 0.1F, 0.2F, 0}, 0},
    {"wild goat", 60.0, false, {0.45, 0.55, 0.0}, 2.0, 0.45, 15.0, -20.0,
     {0, 0, 0, 0, 0, 0.1F, 0.4F, 0.1F, 0, 0, 0, 1, 0}, 0},
    // wild sheep winter on windswept slopes, where the snow lies thin
    {"wild sheep", 45.0, false, {0.85, 0.15, 0.0}, 3.0, 0.5, 12.0, -25.0,
     {0, 0, 0.1F, 0, 0, 0.3F, 0.6F, 0.1F, 0, 0, 0, 0.8F, 0}, 0},
    {"wild boar", 80.0, false, {0.35, 0.15, 0.5}, 1.0, 1.5, 10.0, -15.0,
     {0, 0, 0, 0.3F, 1, 0.2F, 0.3F, 0, 0.2F, 0.5F, 0.8F, 0, 0.1F}, 0},
    {"hare", 3.5, false, {0.6, 0.4, 0.0}, 3.0, 3.0, 4.0, -99.0,
     {0, 0, 0.8F, 0.4F, 0.4F, 1, 0.7F, 0.3F, 0.5F, 0, 0.3F, 0.4F, 0.2F}, 0},
    {"gazelle", 25.0, false, {0.6, 0.4, 0.0}, 1.0, 0.6, 10.0, 5.0,
     {0, 0, 0, 0, 0, 0.4F, 0.8F, 0.5F, 1, 0, 0, 0.1F, 0}, 0},
    {"zebra", 300.0, false, {0.95, 0.05, 0.0}, 1.0, 0.35, 20.0, 10.0,
     {0, 0, 0, 0, 0, 0.5F, 0.5F, 0.1F, 1, 0, 0.1F, 0, 0}, 0},
    {"forest antelope", 15.0, false, {0.0, 0.5, 0.5}, 1.0, 0.6, 10.0, 12.0,
     {0, 0, 0, 0, 0.3F, 0, 0, 0, 0.2F, 1, 0.2F, 0, 0}, 0},
    {"wolf", 40.0, true, {0, 0, 0}, 3.0, 0.8, 10.0, -99.0,
     {0, 0, 0.8F, 1, 0.8F, 0.8F, 0.6F, 0.1F, 0.1F, 0, 0.3F, 0.6F, 0.1F},
     bit(kRed) | bit(kRoe) | bit(kReindeer) | bit(kMoose) | bit(kCattle) | bit(kBison) | bit(kHorse) | bit(kGoat) |
         bit(kSheep) | bit(kBoar) | bit(kHare)},
    {"lynx", 20.0, true, {0, 0, 0}, 3.0, 0.8, 12.0, -99.0,
     {0, 0, 0.2F, 1, 0.8F, 0.1F, 0.3F, 0, 0, 0, 0.2F, 0.6F, 0},
     bit(kRoe) | bit(kReindeer) | bit(kGoat) | bit(kSheep) | bit(kHare)},
    {"lion", 160.0, true, {0, 0, 0}, 1.0, 0.6, 14.0, 8.0,
     {0, 0, 0, 0, 0, 0.4F, 0.6F, 0.1F, 1, 0.1F, 0.1F, 0, 0},
     bit(kZebra) | bit(kGazelle) | bit(kCattle) | bit(kBoar) | bit(kHorse) | bit(kDuiker)},
    {"leopard", 55.0, true, {0, 0, 0}, 1.0, 0.6, 12.0, 2.0,
     {0, 0, 0, 0, 0.3F, 0.1F, 0.6F, 0.1F, 0.7F, 1, 0.2F, 0.5F, 0},
     bit(kGazelle) | bit(kDuiker) | bit(kGoat) | bit(kSheep) | bit(kBoar) | bit(kHare) | bit(kRoe)},
}};
// clang-format on

// A plant eater finds all it asks only where there is this many times as much: it must search.
constexpr double kSearch = 3.0;
// And it eats more slowly where its food lies thin: kg a km² of grass, browse and mast where it eats at half its pace,
// so a failed mast starves the boar that live on it.
constexpr std::array<double, kFoods> kThin = {2000.0, 1000.0, 5000.0};
// How fast an animal eating less than it needs loses condition, each step: a game winter costs as a real one does.
constexpr double kLoss = 0.6;
// Snow, mm of water, that halves the reach to grass and mast, and to browse, for a red deer.
constexpr double kSnowGround = 40.0;
constexpr double kSnowBrowse = 200.0;

// A number drawn from the normal distribution, keyed (Box and Muller).
double normal(std::uint64_t seed, std::uint64_t region, std::uint64_t year, std::uint64_t index) {
    const double u = 1.0 - samebits::chance(seed, region, year, Draw::kYear, index * 2);
    const double v = samebits::chance(seed, region, year, Draw::kYear, (index * 2) + 1);
    return std::sqrt(-2.0 * samebits::logarithm(u)) * samebits::cosine(kTau * v);
}

}  // namespace

const std::array<Kind, kKinds>& kinds() {
    return kTable;
}

double damuth_density(double kg) {
    return samebits::power(10.0, 4.23) * samebits::power(kg * 1000.0, -0.75);
}

double hunters_per_prey(double kg) {
    return 89.1 * samebits::power(kg, -1.05);
}

EcologyRun run_ecology(const World& w, int settle, int years, minds::Pool* pool, int focus) {
    const auto t0 = std::chrono::steady_clock::now();
    const Grid& g = w.grid;
    const double km = g.metres / 1000.0;
    const double area = km * km;

    // the land cells, and each one's place among them
    std::vector<std::int32_t> land;
    for (int c = 0; c < g.cells(); ++c) {
        if (!w.sea(c)) {
            land.push_back(c);
        }
    }
    const std::size_t n = land.size();
    std::vector<std::int32_t> place(static_cast<std::size_t>(g.cells()), -1);
    for (std::size_t i = 0; i < n; ++i) {
        place[static_cast<std::size_t>(land[i])] = static_cast<std::int32_t>(i);
    }

    // what each cell is: its biome's grown cover, how much it grows, its climate and its weather region's corners
    const int region = std::max(1, static_cast<int>(std::lround(kRegionKm / km)));
    const int rx = (g.width + region - 1) / region;
    const int ry = (g.height + region - 1) / region;
    std::vector<std::uint8_t> biome(n);
    std::vector<float> grow(n);     // the land's growth, 0 to about 1.6, from its warmth and wetness
    std::vector<float> mean(n);     // the year's mean, °C
    std::vector<float> swing(n);    // half the year's swing, signed by the hemisphere
    std::vector<float> rain(n);     // mm a year
    std::vector<float> wetness(n);  // the moisture index the place's plants live by (BIOME1)
    std::vector<std::array<float, 3>> grown(n);
    std::vector<std::array<std::int32_t, 4>> corner(n);
    std::vector<std::array<float, 4>> weight(n);
    std::vector<bool> south(n);
    for (std::size_t i = 0; i < n; ++i) {
        const auto c = static_cast<std::size_t>(land[i]);
        biome[i] = w.biome[c];
        grown[i] = grown_cover(static_cast<Biome>(w.biome[c]));
        grow[i] = static_cast<float>(std::clamp(w.gdd5[c] / 1500.0, 0.05, 1.6) * std::clamp(w.wetness[c], 0.05F, 1.0F));
        mean[i] = w.temperature[c];
        south[i] = g.latitude(g.y_of(land[i])) < 0.0;
        swing[i] = (w.warm[c] - w.cold[c]) / 2.0F * (south[i] ? -1.0F : 1.0F);
        rain[i] = w.rain[c];
        wetness[i] = std::min(1.0F, w.wetness[c]);
        // the four regions round the cell, and how near each is, so a year's weather changes smoothly
        const double fx = ((g.x_of(land[i]) + 0.5) / region) - 0.5;
        const double fy = ((g.y_of(land[i]) + 0.5) / region) - 0.5;
        const int x0 = static_cast<int>(std::floor(fx));
        const int y0 = static_cast<int>(std::floor(fy));
        const auto ax = static_cast<float>(fx - x0);
        const auto ay = static_cast<float>(fy - y0);
        const auto wrap = [](int v, int m) { return ((v % m) + m) % m; };
        corner[i] = {(wrap(y0, ry) * rx) + wrap(x0, rx), (wrap(y0, ry) * rx) + wrap(x0 + 1, rx),
                     (wrap(y0 + 1, ry) * rx) + wrap(x0, rx), (wrap(y0 + 1, ry) * rx) + wrap(x0 + 1, rx)};
        weight[i] = {(1.0F - ax) * (1.0F - ay), ax * (1.0F - ay), (1.0F - ax) * ay, ax * ay};
    }
    // the cells of the weather region round the focus, if any
    std::vector<std::size_t> near;
    if (focus >= 0) {
        const int fx0 = g.x_of(focus) / region;
        const int fy0 = g.y_of(focus) / region;
        for (std::size_t i = 0; i < n; ++i) {
            if (g.x_of(land[i]) / region == fx0 && g.y_of(land[i]) / region == fy0) {
                near.push_back(i);
            }
        }
    }

    // what each cell holds of each species: a sixth of Damuth's density for plant eaters (WLD-30), in its home
    const auto& table = kinds();
    std::array<double, kKinds> eats{};  // a plant eater's dry food, or a hunter's meat, kg a day
    std::array<std::vector<float>, kKinds> cap;
    for (int k = 0; k < kKinds; ++k) {
        const Kind& kind = table[static_cast<std::size_t>(k)];
        eats[static_cast<std::size_t>(k)] = (kind.hunter ? 0.2 : 0.09) * samebits::power(kind.kg, 0.75);
        auto& kc = cap[static_cast<std::size_t>(k)];
        kc.assign(n, 0.0F);
        if (kind.hunter) {
            continue;
        }
        const double density = damuth_density(kind.kg) / 6.0;
        for (std::size_t i = 0; i < n; ++i) {
            const auto c = static_cast<std::size_t>(land[i]);
            if (w.cold[c] < kind.coldest) {
                continue;
            }
            kc[i] = static_cast<float>(density * area * kind.home[biome[i]]);
        }
    }

    // the state: cover, food, water and snow, and each species' count and condition
    std::vector<float> trees(n);
    std::vector<float> bushes(n);
    std::vector<float> grass(n);
    std::vector<float> browse(n);
    std::vector<float> mast(n, 0.0F);
    std::vector<float> water(n, 75.0F);
    std::vector<float> snow(n, 0.0F);
    std::array<std::vector<double>, kKinds> count;
    std::array<std::vector<float>, kKinds> condition;
    for (std::size_t i = 0; i < n; ++i) {
        // each cell at some stage of regrowth since its last fire or flood, as P7 makes it
        const auto stage = static_cast<float>(
            0.4 + (0.6 * samebits::chance(w.seed, static_cast<std::uint64_t>(land[i]), 0, Draw::kFire, 0)));
        trees[i] = grown[i][0] * stage;
        bushes[i] = grown[i][1] * stage;
    }
    // the grass's and browse's most and their fastest growth a day, kg a km², from the cover now
    const auto grass_most = [&](std::size_t i) {
        const float share = std::clamp(grown[i][2] + (0.6F * std::max(0.0F, grown[i][0] - trees[i])) +
                                           (0.3F * std::max(0.0F, grown[i][1] - bushes[i])),
                                       0.0F, 1.0F);
        return 150000.0F * share * grow[i];
    };
    const auto browse_most = [&](std::size_t i) { return 80000.0F * (bushes[i] + (0.25F * trees[i])) * grow[i]; };
    for (std::size_t i = 0; i < n; ++i) {
        grass[i] = 0.5F * grass_most(i);
        browse[i] = 0.5F * browse_most(i);
    }
    for (int k = 0; k < kKinds; ++k) {
        count[static_cast<std::size_t>(k)].assign(n, 0.0);
        condition[static_cast<std::size_t>(k)].assign(n, 0.8F);
        for (std::size_t i = 0; i < n; ++i) {
            count[static_cast<std::size_t>(k)][i] = 0.8 * cap[static_cast<std::size_t>(k)][i];
        }
    }
    // a hunter's cell holds as many as its prey there feeds (Carbone and Gittleman 2002)
    const auto hunter_caps = [&]() {
        for (int h = 0; h < kKinds; ++h) {
            const Kind& kind = table[static_cast<std::size_t>(h)];
            if (!kind.hunter) {
                continue;
            }
            const double per = hunters_per_prey(kind.kg) / 10000.0;
            auto& hc = cap[static_cast<std::size_t>(h)];
            pool->run(static_cast<int>((n + 255) / 256), [&](int chunk, int /*thread*/) {
                const std::size_t end = std::min(n, static_cast<std::size_t>(chunk + 1) * 256);
                for (std::size_t i = static_cast<std::size_t>(chunk) * 256; i < end; ++i) {
                    double prey = 0.0;
                    for (int p = 0; p < kKinds; ++p) {
                        if ((kind.prey & bit(p)) != 0) {
                            prey += count[static_cast<std::size_t>(p)][i] * table[static_cast<std::size_t>(p)].kg;
                        }
                    }
                    const bool warm_enough = w.cold[static_cast<std::size_t>(land[i])] >= kind.coldest;
                    hc[i] = warm_enough ? static_cast<float>(per * prey * kind.home[biome[i]]) : 0.0F;
                }
            });
        }
    };
    hunter_caps();
    for (int h = 0; h < kKinds; ++h) {
        if (table[static_cast<std::size_t>(h)].hunter) {
            for (std::size_t i = 0; i < n; ++i) {
                count[static_cast<std::size_t>(h)][i] = 0.8 * cap[static_cast<std::size_t>(h)][i];
            }
        }
    }

    // the seasons: -1 at midwinter in the north to 1 at midsummer
    std::array<double, kTicks> season{};
    for (int t = 0; t < kTicks; ++t) {
        season[static_cast<std::size_t>(t)] = -samebits::cosine(kTau * (t + 0.5) / kTicks);
    }
    const auto regions = static_cast<std::size_t>(rx) * static_cast<std::size_t>(ry);
    std::vector<std::array<float, 3>> year_of(regions);  // each region's year: its rain, its winter, its mast
    std::vector<std::array<float, 3>> cell_year(n);
    std::vector<double> before(n);
    std::vector<double> out(n);
    std::vector<std::uint8_t> targets(n);

    EcologyRun run;
    const auto record = [&]() {
        std::array<double, kKinds> totals{};
        std::array<double, kKinds> fed{};
        std::array<std::array<double, kBiomes>, kKinds> in{};
        double prey = 0.0;
        double hunters = 0.0;
        for (int k = 0; k < kKinds; ++k) {
            double sum = 0.0;
            double fit = 0.0;
            for (std::size_t i = 0; i < n; ++i) {
                sum += count[static_cast<std::size_t>(k)][i];
                fit += count[static_cast<std::size_t>(k)][i] * condition[static_cast<std::size_t>(k)][i];
                in[static_cast<std::size_t>(k)][biome[i]] += count[static_cast<std::size_t>(k)][i];
            }
            totals[static_cast<std::size_t>(k)] = sum;
            fed[static_cast<std::size_t>(k)] = sum > 0.0 ? fit / sum : 0.0;
            const Kind& kind = table[static_cast<std::size_t>(k)];
            if (kind.hunter) {
                hunters += sum;
            } else if (kind.kg >= 15.0) {
                prey += sum;
            }
        }
        run.totals.push_back(totals);
        run.fed.push_back(fed);
        run.by_biome.push_back(in);
        std::array<double, kKinds> here{};
        for (int k = 0; k < kKinds; ++k) {
            for (const std::size_t i : near) {
                here[static_cast<std::size_t>(k)] += count[static_cast<std::size_t>(k)][i];
            }
        }
        run.local.push_back(here);
        run.prey_per_hunter.push_back(hunters > 0.0 ? prey / hunters : 0.0);
    };
    // the biomes each species lives in: settled, those holding at least a twentieth of it and ten animals; at the
    // end, those holding any at all
    const auto homes = [&](bool settled) {
        std::array<std::uint32_t, kKinds> bits{};
        for (int k = 0; k < kKinds; ++k) {
            std::array<double, kBiomes> in{};
            double all = 0.0;
            for (std::size_t i = 0; i < n; ++i) {
                in[biome[i]] += count[static_cast<std::size_t>(k)][i];
                all += count[static_cast<std::size_t>(k)][i];
            }
            for (int b = 0; b < kBiomes; ++b) {
                const double here = in[static_cast<std::size_t>(b)];
                if (settled ? (here >= 0.05 * all && here >= 10.0) : here >= 1.0) {
                    bits[static_cast<std::size_t>(k)] |= bit(b);
                }
            }
        }
        return bits;
    };

    // the settled year, then `years` more, each counted at the same moment
    for (int year = 0; year <= settle + years; ++year) {
        // this year's weather in each region: its rain, how much colder or milder its winter, how good its mast;
        // each the region's own, a pattern shared by its neighbours for several hundred km, as a blocked jet stream
        // or a drought's high gives, and the whole world's, as after a great eruption
        const auto y = static_cast<std::uint64_t>(year);
        const std::uint64_t everywhere = regions + 1000000;
        for (std::size_t r = 0; r < regions; ++r) {
            const std::uint64_t wide =
                1000 + ((r / static_cast<std::size_t>(rx) / kWide) * 64) + (r % static_cast<std::size_t>(rx) / kWide);
            const auto at_scales = [&](std::uint64_t k) {
                return (0.6 * normal(w.seed, r, y, k)) + (0.65 * normal(w.seed, wide, y, k)) +
                       (0.45 * normal(w.seed, everywhere, y, k));
            };
            year_of[r] = {static_cast<float>(std::clamp(samebits::exponent(0.3 * at_scales(0)), 0.35, 2.0)),
                          static_cast<float>(std::clamp(2.5 * at_scales(1), -9.0, 6.0)),
                          static_cast<float>(std::clamp(samebits::exponent(0.5 * at_scales(2)), 0.2, 3.0))};
        }
        for (std::size_t i = 0; i < n; ++i) {
            std::array<float, 3> v{};
            for (std::size_t j = 0; j < 4; ++j) {
                const auto& r = year_of[static_cast<std::size_t>(corner[i][j])];
                for (std::size_t q = 0; q < 3; ++q) {
                    v[q] += weight[i][j] * r[q];
                }
            }
            cell_year[i] = v;
        }
        for (int tick = 0; tick < kTicks; ++tick) {
            const double s = season[static_cast<std::size_t>(tick)];
            pool->run(static_cast<int>((n + 255) / 256), [&](int chunk, int /*thread*/) {
                const std::size_t end = std::min(n, static_cast<std::size_t>(chunk + 1) * 256);
                for (std::size_t i = static_cast<std::size_t>(chunk) * 256; i < end; ++i) {
                    const auto& yr = cell_year[i];
                    const double here = s * (south[i] ? -1.0 : 1.0);  // -1 at this cell's midwinter
                    const int local = south[i] ? (tick + 6) % kTicks : tick;
                    // the weather: warmth with this winter's own cold, rain or snow, the soil's water
                    const double temp = mean[i] + (swing[i] * s) + (yr[1] * std::max(0.0, -here));
                    const double fall = rain[i] / kTicks * yr[0];
                    double wet = water[i];
                    if (temp < 0.0) {
                        snow[i] += static_cast<float>(fall);
                    } else {
                        const double melt = std::min(static_cast<double>(snow[i]), kDays * 3.0 * temp);
                        snow[i] -= static_cast<float>(melt);
                        wet += fall + melt;
                    }
                    const double draw = temp > 0.0 ? kDays * ((4.0 * temp) + (0.04 * temp * temp)) / 5.0 : 0.0;
                    wet = std::min(150.0, wet);
                    // the plants grow as far as the water they find meets what the warmth draws (BIOME1's moisture
                    // index), partly against the place's own, as dry country's plants live on less, so a normal year
                    // there feeds its animals about as much as it costs them and a drought costs more; and only in
                    // the warmth
                    const double usual = std::sqrt(std::max(0.15, static_cast<double>(wetness[i])));
                    const double moist = draw > 0.0 ? std::min(1.0, wet / draw / usual) : 1.0;
                    wet -= std::min(draw, wet);
                    water[i] = static_cast<float>(wet);
                    // the plants: grass and browse grow in the warmth where the soil holds water, wither in the cold;
                    // mast falls in autumn, more in a good year, and rots or is taken through the year
                    const double growing = std::clamp((temp - 5.0) / 10.0, 0.0, 1.0) * moist;
                    const double gm = grass_most(i);
                    const double bm = browse_most(i);
                    if (gm > 0.0) {
                        grass[i] +=
                            static_cast<float>(kDays * 0.02 * gm * growing * std::max(0.0, 1.0 - (grass[i] / gm)));
                    }
                    if (bm > 0.0) {
                        browse[i] +=
                            static_cast<float>(kDays * 0.01 * bm * growing * std::max(0.0, 1.0 - (browse[i] / bm)));
                    }
                    grass[i] *= temp < 0.0 ? 0.92F : 0.99F;
                    browse[i] *= temp < 0.0 ? 0.97F : 0.995F;
                    mast[i] *= 0.9F;
                    if (local == 8) {
                        mast[i] += static_cast<float>(30000.0 * (trees[i] + (0.5 * bushes[i])) * grow[i] * yr[2]);
                    }
                    // the plant eaters: each eats what it can reach under the snow, of food enough for all who eat
                    // it there; green food feeds best, dry grass least; and its condition follows how full it is
                    const std::array<double, kFoods> food = {grass[i], browse[i], mast[i]};
                    const std::array<double, kFoods> worth = {0.35 + (0.65 * growing), 0.5 + (0.5 * growing), 1.0};
                    std::array<double, kFoods> asked{};  // what all of them would eat, kg a km²
                    for (int k = 0; k < kKinds; ++k) {
                        const Kind& kind = table[static_cast<std::size_t>(k)];
                        const double head = count[static_cast<std::size_t>(k)][i];
                        if (!kind.hunter && head > 0.0) {
                            for (std::size_t f = 0; f < kFoods; ++f) {
                                asked[f] += head * eats[static_cast<std::size_t>(k)] * kDays * kind.diet[f] / area;
                            }
                        }
                    }
                    std::array<double, kFoods> enough{};  // the share of what they ask that they find
                    for (std::size_t f = 0; f < kFoods; ++f) {
                        enough[f] = (asked[f] > 0.0 ? std::min(1.0, food[f] / (kSearch * asked[f])) : 1.0) * food[f] /
                                    (food[f] + kThin[f]);
                    }
                    std::array<double, kFoods> eaten{};
                    for (int k = 0; k < kKinds; ++k) {
                        const Kind& kind = table[static_cast<std::size_t>(k)];
                        double& head = count[static_cast<std::size_t>(k)][i];
                        if (kind.hunter || head <= 0.0) {
                            continue;
                        }
                        const double ground = 1.0 / (1.0 + (snow[i] / (kSnowGround * kind.snow)));
                        const std::array<double, kFoods> reach = {
                            ground, 1.0 / (1.0 + (snow[i] / (kSnowBrowse * kind.snow))), ground};
                        double fill = 0.0;
                        for (std::size_t f = 0; f < kFoods; ++f) {
                            const double got = kind.diet[f] * reach[f] * enough[f];
                            fill += got * worth[f];
                            eaten[f] += head * eats[static_cast<std::size_t>(k)] * kDays * got / area;
                        }
                        if (temp < -10.0) {
                            fill *= std::max(0.5, 1.0 - (0.015 * (-10.0 - temp)));  // the cold's cost
                        }
                        float& fit = condition[static_cast<std::size_t>(k)][i];
                        fit =
                            static_cast<float>(std::clamp(fit + ((fill - 0.6) * (fill > 0.6 ? 0.5 : kLoss)), 0.0, 1.0));
                        const double age = (1.0 / (kind.years * kTicks)) * (1.0 + (2.0 * std::max(0.0, 0.5 - fit)));
                        const double hunger = fit < 0.3F ? 0.25 * (0.3 - fit) / 0.3 : 0.0;
                        head *= 1.0 - std::min(0.9, age + hunger);
                    }
                    grass[i] = static_cast<float>(std::max(0.0, grass[i] - eaten[0]));
                    browse[i] = static_cast<float>(std::max(0.0, browse[i] - eaten[1]));
                    mast[i] = static_cast<float>(std::max(0.0, mast[i] - eaten[2]));
                    // the hunters: each takes prey as a predator does (Holling's second type), the weak and those
                    // floundering in deep snow more easily, and no more than it and its young eat
                    for (int h = 0; h < kKinds; ++h) {
                        const Kind& kind = table[static_cast<std::size_t>(h)];
                        double& head = count[static_cast<std::size_t>(h)][i];
                        if (!kind.hunter || head <= 0.0) {
                            continue;
                        }
                        const double deep = 1.0 + std::min(1.0, snow[i] / 200.0);
                        std::array<double, kKinds> open{};
                        double weak = 0.0;  // the prey's weight within reach, kg a km², the weak counting more
                        for (int p = 0; p < kKinds; ++p) {
                            if ((kind.prey & bit(p)) == 0) {
                                continue;
                            }
                            const double v =
                                std::clamp(1.3 - condition[static_cast<std::size_t>(p)][i], 0.3, 1.3) * deep;
                            open[static_cast<std::size_t>(p)] =
                                count[static_cast<std::size_t>(p)][i] * table[static_cast<std::size_t>(p)].kg * v;
                            weak += open[static_cast<std::size_t>(p)];
                        }
                        weak /= area;
                        const double need = eats[static_cast<std::size_t>(h)] / 0.6;  // prey's weight a day
                        const double attack = 0.2 * samebits::power(kind.kg, 0.75);   // km² searched a day
                        const double handling = 1.0 / (2.0 * need);
                        const double kill = std::min(1.2 * need, attack * weak / (1.0 + (attack * handling * weak)));
                        const double taken = head * kill * kDays;
                        if (weak > 0.0) {
                            for (int p = 0; p < kKinds; ++p) {
                                const double o = open[static_cast<std::size_t>(p)];
                                if (o > 0.0) {
                                    double& prey = count[static_cast<std::size_t>(p)][i];
                                    prey -= std::min(0.5 * prey, taken * (o / (weak * area)) /
                                                                     table[static_cast<std::size_t>(p)].kg);
                                }
                            }
                        }
                        const double fill = kill / need;
                        float& fit = condition[static_cast<std::size_t>(h)][i];
                        fit =
                            static_cast<float>(std::clamp(fit + ((fill - 0.6) * (fill > 0.6 ? 0.5 : kLoss)), 0.0, 1.0));
                        const double age = (1.0 / (kind.years * kTicks)) * (1.0 + (2.0 * std::max(0.0, 0.5 - fit)));
                        const double hunger = fit < 0.3F ? 0.25 * (0.3 - fit) / 0.3 : 0.0;
                        head *= 1.0 - std::min(0.9, age + hunger);
                    }
                    // spring: young as many as the parents' condition allows, fewer as the cell fills toward what
                    // it holds, so a crash takes years to mend
                    if (local == 3) {
                        for (int k = 0; k < kKinds; ++k) {
                            double& head = count[static_cast<std::size_t>(k)][i];
                            const double most = cap[static_cast<std::size_t>(k)][i];
                            if (head > 0.0 && head < most) {
                                head += head * table[static_cast<std::size_t>(k)].births *
                                        std::min(1.0F, condition[static_cast<std::size_t>(k)][i] / 0.5F) *
                                        (1.0 - (head / most));
                            }
                            // too few to find each other, or nothing left to live on: gone from this cell
                            if (head < 0.5 && (most <= 0.0 || head < 0.05 * most)) {
                                head = 0.0;
                            }
                        }
                    }
                    // summer's end: lightning fires in dry country, worse in a dry year; the cover grows back
                    if (local == 7) {
                        const auto c = static_cast<std::size_t>(land[i]);
                        const double dry = 1.0 - std::min(1.0F, w.wetness[c]);
                        const double strike = w.warm[c] > 10.0F ? 0.04 * dry * dry * std::max(0.0, 2.0 - yr[0]) : 0.0;
                        if (strike > 0.0 &&
                            samebits::chance(w.seed, c, static_cast<std::uint64_t>(year), Draw::kFire, 2) < strike) {
                            trees[i] *= 0.3F;
                            bushes[i] *= 0.3F;
                            grass[i] *= 0.5F;
                            browse[i] *= 0.3F;
                            mast[i] = 0.0F;
                        }
                        trees[i] += 0.03F * (grown[i][0] - trees[i]);
                        bushes[i] += 0.12F * (grown[i][1] - bushes[i]);
                    }
                }
            });
            // the hunters' room follows their prey, set each spring
            if (tick == 3 || tick == 9) {
                hunter_caps();
            }
            // late summer: young animals leave for cells next door that suit them and are less crowded, from any
            // cell, more from crowded ones, so land emptied by a hard year is found again
            if (tick == 5) {
                for (int k = 0; k < kKinds; ++k) {
                    auto& head = count[static_cast<std::size_t>(k)];
                    const auto& most = cap[static_cast<std::size_t>(k)];
                    before = head;
                    const auto fullness = [&](std::size_t i) { return most[i] > 0.0F ? before[i] / most[i] : 1.0e9; };
                    const auto neighbour = [&](std::size_t i, std::size_t d) {
                        const int x = g.x_of(land[i]) + kDx[d];
                        const int y = g.y_of(land[i]) + kDy[d];
                        return place[static_cast<std::size_t>(g.at(x, y))];
                    };
                    pool->run(static_cast<int>((n + 255) / 256), [&](int chunk, int /*thread*/) {
                        const std::size_t end = std::min(n, static_cast<std::size_t>(chunk + 1) * 256);
                        for (std::size_t i = static_cast<std::size_t>(chunk) * 256; i < end; ++i) {
                            out[i] = 0.0;
                            targets[i] = 0;
                            if (before[i] <= 0.0 || most[i] <= 0.0F) {
                                continue;
                            }
                            for (std::size_t d = 0; d < 8; ++d) {
                                const std::int32_t j = neighbour(i, d);
                                if (j >= 0 && fullness(static_cast<std::size_t>(j)) < fullness(i) - 0.1) {
                                    ++targets[i];
                                }
                            }
                            out[i] = targets[i] > 0 ? before[i] * (0.02 + (0.08 * std::min(1.0, fullness(i)))) : 0.0;
                        }
                    });
                    pool->run(static_cast<int>((n + 255) / 256), [&](int chunk, int /*thread*/) {
                        const std::size_t end = std::min(n, static_cast<std::size_t>(chunk + 1) * 256);
                        for (std::size_t i = static_cast<std::size_t>(chunk) * 256; i < end; ++i) {
                            double in = 0.0;
                            if (most[i] > 0.0F) {
                                for (std::size_t d = 0; d < 8; ++d) {
                                    const std::int32_t j = neighbour(i, d);
                                    if (j >= 0 && targets[static_cast<std::size_t>(j)] > 0 &&
                                        fullness(i) < fullness(static_cast<std::size_t>(j)) - 0.1) {
                                        in += out[static_cast<std::size_t>(j)] / targets[static_cast<std::size_t>(j)];
                                    }
                                }
                            }
                            head[i] = before[i] - out[i] + in;
                        }
                    });
                }
            }
            // the totals, counted at the same moment each year: late winter in the north
            if (tick == 1 && year >= settle) {
                record();
                if (year == settle) {
                    run.biomes_settled = homes(true);
                }
            }
            if (year == settle + years && tick >= 1) {
                break;
            }
            if (tick == 7 && year >= settle) {
                std::array<double, 4> p{};
                for (std::size_t i = 0; i < n; ++i) {
                    p[0] += grass[i] * area / 1000.0;
                    p[1] += browse[i] * area / 1000.0;
                    p[2] += mast[i] * area / 1000.0;
                    p[3] += trees[i] * area;
                }
                run.plants.push_back(p);
            }
        }
    }
    run.biomes_end = homes(false);
    std::uint64_t h = w.seed;
    for (int k = 0; k < kKinds; ++k) {
        for (std::size_t i = 0; i < n; ++i) {
            samebits::hash_into(&h, count[static_cast<std::size_t>(k)][i]);
        }
    }
    run.checksum = h;
    run.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    return run;
}

}  // namespace worldgen
