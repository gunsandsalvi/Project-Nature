// P9 Ecology (IMPLEMENTATION α0.6a, A9, research 08): nature on a world's cells with nobody in it. Plant cover grows
// by season and weather; plant eaters live at a sixth of the density Damuth's law gives their size (WLD-30); hunters
// take them by a predator and prey rule; numbers boom and crash with the weather and with each other, never by script
// (WLD-18). Pre-production code (research 00).
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "pool.hpp"
#include "world.hpp"

namespace worldgen {

// P9's species: plant eaters of every biome and the hunters that take them (WLD-32).
constexpr int kKinds = 18;
// What plant eaters eat: grass and herbs; browse, the leaves and twigs of bushes and young trees; mast, the fruit,
// nuts and roots (WLD-31).
constexpr int kFoods = 3;

// One species as its catalogue entry would hold it (A3.6).
struct Kind {
    const char* name;
    double kg;                        // an adult's weight
    bool hunter;                      // lives by killing the plant eaters in `prey`
    std::array<double, kFoods> diet;  // a plant eater's shares of grass, browse and mast
    double snow;     // how well it reaches food under snow: 1 as a red deer, more for the northern kinds
    double births;   // young each adult raises a year when well fed
    double years;    // its life span
    double coldest;  // the coldest month's mean it needs at least, °C
    std::array<float, kBiomes> home;  // how well each biome suits it, 0 to 1
    std::uint32_t prey;               // a bit for each species it hunts
};
const std::array<Kind, kKinds>& kinds();

// Damuth's law (1981) for plant-eating mammals: individuals a km² on Earth for an adult of `kg`.
double damuth_density(double kg);
// Carbone and Gittleman's rule (2002): a hunter of `kg` per 10,000 kg of its prey.
double hunters_per_prey(double kg);

// A run of the world's nature: each year's totals, from the settled year on.
struct EcologyRun {
    std::vector<std::array<double, kKinds>> totals;  // each species' adults and young, year 0 the settled world
    std::vector<std::array<double, 4>> plants;       // grass, browse and mast in tonnes, and trees in km² of crowns
    std::vector<double> prey_per_hunter;             // big plant eaters (15 kg and more) for each hunter
    std::vector<std::array<double, kKinds>> fed;     // each species' mean condition, 0 starving to 1 well fed
    std::vector<std::array<std::array<double, kBiomes>, kKinds>> by_biome;  // each species' total in each biome
    std::vector<std::array<double, kKinds>> local;       // each species' number in the weather region round `focus`
    std::array<std::uint32_t, kKinds> biomes_settled{};  // a bit for each biome a species lives in when settled
    std::array<std::uint32_t, kKinds> biomes_end{};      // and at the end
    double seconds = 0.0;
    std::uint64_t checksum = 0;  // of the last state, for the same bits on any thread count (RES-05)
};

// The world's nature for `settle` years (WLD-08) then `years` more, with nobody in it; with the numbers in the
// weather region round the cell `focus`, if one is given, where booms and crashes show that the world's totals hide.
EcologyRun run_ecology(const World& w, int settle, int years, minds::Pool* pool, int focus = -1);

}  // namespace worldgen
