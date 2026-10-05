// P7 World generation (IMPLEMENTATION α0.5a, A7.2): a world made from its seed in the order of real causes, at a
// coarse size for the candidates and at full size for the best few (WLD-08, WLD-09, WLD-10). Each stage is a function
// over the torus's cells, worked in fixed chunks on one thread or several, with P5's own maths and keyed chance, so a
// seed makes the same world everywhere (A7.4). Pre-production code (research 00).
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "pool.hpp"

namespace worldgen {

// A torus of cells: x wraps east to west, y runs from pole to pole and wraps there, the equator across the middle
// (WLD-01).
struct Grid {
    int width = 0;
    int height = 0;
    double metres = 0.0;  // a cell's side

    [[nodiscard]] int cells() const { return width * height; }
    [[nodiscard]] int x_of(int c) const { return c % width; }
    [[nodiscard]] int y_of(int c) const { return c / width; }
    [[nodiscard]] int at(int x, int y) const {
        x %= width;
        y %= height;
        return ((y < 0 ? y + height : y) * width) + (x < 0 ? x + width : x);
    }
    // Row y's latitude in degrees: -90 at the seam's one side, 0 at the equator, 90 at the seam's other.
    [[nodiscard]] double latitude(int y) const { return (((y + 0.5) / height) * 180.0) - 90.0; }
    // A cell's centre as fractions of the world's width and height, the same place at any size.
    [[nodiscard]] double u_of(int c) const { return (x_of(c) + 0.5) / width; }
    [[nodiscard]] double v_of(int c) const { return (y_of(c) + 0.5) / height; }
};

// The eight neighbours, as steps, and each step's length in cells.
constexpr std::array<int, 8> kDx = {1, 1, 0, -1, -1, -1, 0, 1};
constexpr std::array<int, 8> kDy = {0, 1, 1, 1, 0, -1, -1, -1};
constexpr double kDiagonal = 1.4142135623730951;
constexpr std::array<double, 8> kStep = {1.0, kDiagonal, 1.0, kDiagonal, 1.0, kDiagonal, 1.0, kDiagonal};

// The world's size: about 2,000 km around and 1,000 from pole to pole (WLD-03).
constexpr double kAroundMetres = 2.0e6;

// About 12 kinds of rock (WLD-09).
enum class Rock : std::uint8_t {
    kGranite,
    kBasalt,
    kLavaAsh,
    kGlassyLava,  // young lava from volcanoes with thick, sticky lava, which gives obsidian
    kSandstone,
    kShale,
    kLimestone,
    kChalk,
    kQuartzite,
    kSlate,
    kGravel,
    kSilt,
};
constexpr int kRocks = 12;

// A cell's place among the plates (WLD-09): old worn-down land, old sea basins, folded ranges, volcanoes, rifts or
// sea floor.
enum class Setting : std::uint8_t { kSeaFloor, kShield, kBasin, kRange, kArc, kRift };

// What a cell's climate, soil and wetness would grow if left alone (WLD-31), plus shores and seas.
enum class Biome : std::uint8_t {
    kSea,
    kIce,
    kTundra,
    kConifer,
    kBroadleaf,
    kGrassland,
    kScrub,
    kDesert,
    kSavanna,
    kTropical,
    kMarsh,
    kHeights,
    kShore,
};
constexpr int kBiomes = 13;

// A cell's soil (WLD-27).
enum class Soil : std::uint8_t {
    kNone,
    kSilt,
    kVolcanic,
    kBlack,
    kLoam,
    kSandy,
    kClay,
    kChalky,
    kPeat,
    kWashed,
    kThin,
    kDesert,
    kFrozen,
};

// What lies in a cell (WLD-14), each with how rich it is, 0 to 3.
enum Deposit : std::uint8_t {
    kFlint,
    kChert,
    kObsidian,
    kHammerStone,
    kClay,
    kOchre,
    kCopperOre,
    kNativeCopper,
};
constexpr int kDeposits = 8;

// The big animals P7 places (WLD-32), the five with a domestic kind among them (WLD-33).
enum class Species : std::uint8_t {
    kRedDeer,
    kReindeer,
    kWildCattle,
    kBison,
    kHorse,
    kWildGoat,
    kWildSheep,
    kBoar,
    kWolf,
    kBear,
};
constexpr int kSpecies = 10;

struct Herd {
    std::int32_t cell = 0;    // where it is
    std::int32_t summer = 0;  // the middles of its seasonal ranges (WLD-32)
    std::int32_t winter = 0;
    Species species = Species::kRedDeer;
    float count = 0.0F;      // adults and young
    float condition = 1.0F;  // 0 starving to 1 well fed
};

// The seconds each stage took, summed over the worlds it ran for.
struct Times {
    double plates = 0.0;
    double erosion = 0.0;
    double climate = 0.0;
    double life = 0.0;  // biomes, soils, caves, deposits, plants and animals
    double score = 0.0;
    double settle = 0.0;
};

// Where history may begin (WLD-24): a region's middle and what it was judged by.
struct Start {
    std::int32_t cell = -1;
    int shelters = 0;     // dry caves or overhangs with water and food in reach
    int food_kinds = 0;   // kinds of food in reach
    double margin = 0.0;  // food in reach in the leanest season over what the bands need
    double score = 0.0;
};

struct World {
    Grid grid;
    std::uint64_t seed = 0;
    bool full = false;        // made at full size, every stage
    double tilt = 0.0;        // degrees (WLD-06)
    double land_share = 0.0;  // the share of land the seed asked for (WLD-06)

    std::vector<float> height;        // metres above sea level
    std::vector<float> uplift;        // metres a year
    std::vector<std::uint8_t> plate;  // each cell's plate
    std::vector<std::uint8_t> setting;
    std::vector<std::uint8_t> edge;     // on a plate's edge: 0 not, 1 a range, 2 a trench and arc, 3 a rift, 4 a fault
    std::vector<std::uint8_t> rock;     // three a cell: the surface and two layers below
    std::vector<std::uint8_t> volcano;  // a volcano stands here: 1 runny lava, 2 thick and sticky

    std::vector<std::int32_t> receiver;  // where each cell's water goes; a sea cell's is itself
    std::vector<std::int32_t> order;     // the land cells, every cell after the one its water goes to
    std::vector<float> area;             // the land draining through each cell, km²
    std::vector<float> lake;             // a lake's depth, metres

    std::vector<float> temperature;  // the year's mean, °C
    std::vector<float> cold;         // the coldest month's mean, °C
    std::vector<float> warm;         // the warmest month's mean, °C
    std::vector<float> rain;         // mm a year
    std::vector<float> wetness;      // the moisture index, what the plants could use over what they would (BIOME1)
    std::vector<float> gdd5;         // growing degree days above 5 °C

    std::vector<std::uint8_t> biome;
    std::vector<std::uint8_t> soil;
    std::vector<std::uint8_t> fertility;  // 0 to 5 (WLD-27)
    std::vector<std::uint8_t> caves;      // band-sized dry caves and overhangs, 0 to 3
    std::vector<std::uint8_t> deposits;   // kDeposits a cell
    std::vector<std::uint8_t> grains;     // wild grains grow here
    std::vector<Herd> herds;

    Start start;
    double score = 0.0;
    bool qualifies = false;
    std::string reasons;  // why it qualified or was rejected (WLD-10)
    std::string summary;  // one line for the offer

    [[nodiscard]] Rock surface(int c) const { return static_cast<Rock>(rock[static_cast<std::size_t>(c) * 3]); }
    [[nodiscard]] std::uint8_t deposit(int c, Deposit d) const {
        return deposits[(static_cast<std::size_t>(c) * kDeposits) + d];
    }
    [[nodiscard]] bool sea(int c) const { return height[static_cast<std::size_t>(c)] <= 0.0F; }
    // The checksum of every layer (RES-05).
    [[nodiscard]] std::uint64_t checksum() const;
};

struct Settings {
    std::uint64_t seed = 1;
    int threads = 1;
    int candidates = 20;     // made at the coarse size (WLD-10)
    int most = 40;           // made at most when fewer than three qualify
    int best = 4;            // made again at full size
    int full_width = 2048;   // cells around at full size: about 1 km each (WLD-03)
    int coarse_width = 512;  // and at the coarse size
    int coarse_steps = 40;   // steps of uplift and erosion, and their years
    double coarse_years = 50000.0;
    int full_steps = 10;
    double full_years = 25000.0;
    int settle_years = 10;  // WLD-08
};

// The same run at a sixteenth of the cells, for tests and for arm64 under qemu: 6 candidates, the best 2 at 256 × 128,
// 1 year of settling.
inline Settings small(Settings s) {
    s.full_width = 256;
    s.coarse_width = 128;
    s.candidates = 6;
    s.most = 12;
    s.best = 2;
    s.coarse_steps = 20;
    s.full_steps = 4;
    s.settle_years = 1;
    return s;
}

// The stages, each over the whole grid (WLD-09, A7.2).
void make_plates(World* w, minds::Pool* pool);   // plates, their edges, heights, uplift and volcanoes
void make_rock(World* w, minds::Pool* pool);     // the rock of each cell, from its place among the plates
void erode(World* w, int steps, double years);   // uplift and stream-power erosion, then lakes and rivers
void make_climate(World* w, minds::Pool* pool);  // temperature, rain with rain shadows, wetness
void make_life(World* w, minds::Pool* pool);     // biomes, soils, caves, deposits, wild grains and herds

// A world of the given size from its seed, through every stage; at the coarse size the stages are the rough ones of
// WLD-10's first pass. The times of its stages are added to times.
World make_world(std::uint64_t seed, int width, bool full, const Settings& settings, minds::Pool* pool, Times* times);
// The same world made again at full size: its land from the coarse one's, then every stage at full size.
World refine(const World& coarse, const Settings& settings, minds::Pool* pool, Times* times);

// Finds the start region and checks the must-haves, setting start, qualifies, score, reasons and summary (WLD-10,
// WLD-24).
void judge(World* w);

// The whole of "New world": the candidates, the best few at full size, and the best three offered (WLD-10).
struct Offer {
    std::vector<World> three;
    std::vector<std::string> log;  // every candidate's reasons
    int made = 0;                  // candidates made
    int qualified = 0;
    Times times;
    double candidates_seconds = 0.0;  // making and judging the candidates
    double best_seconds = 0.0;        // the best few at full size
};
Offer new_world(const Settings& settings);

// Settling (WLD-08): plant cover, water and herds run for the given years with no people; the seconds it took.
double settle(World* w, int years, minds::Pool* pool);

// The world's map as RGB bytes, a pixel for every scale × scale cells: relief, water, ice and plant cover, and the big
// rivers unless they are drawn as lines over it (P8, A8.5).
std::vector<std::uint8_t> map_rgb(const World& w, int scale, bool rivers = true);
// A biome's colour on the map, from the art book's.
std::array<std::uint8_t, 3> cover_colour(Biome b);
// A land biome's cover once grown (WLD-31): the shares under trees, bushes, and grass and herbs.
std::array<float, 3> grown_cover(Biome b);

// The seed of candidate number i.
std::uint64_t candidate_seed(std::uint64_t seed, int i);

}  // namespace worldgen
