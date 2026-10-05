// P7's living layers: what the stages after them and the tests read. Pre-production code (research 00).
#pragma once

#include "world.hpp"

namespace worldgen {

// The slope from a cell down to where its water goes, as a rise over a run; 0 at sea.
double slope_of(const World& w, int c);
// Whether a species lives in a cell's biome and ground (WLD-32).
bool lives_in(Species s, const World& w, int c);
// The big animals placed where their food and cover are, as herds, packs and lone animals (WLD-09 step 8, WLD-32).
void place_herds(World* w);

}  // namespace worldgen
