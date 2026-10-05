// P7's draws of keyed chance (A3.5, TIM-16), each its own purpose, so a new kind of draw never shifts the others.
// Pre-production code (research 00).
#pragma once

#include <cstdint>

namespace worldgen {

enum class Draw : std::uint8_t {
    kWorld = 1,    // a world's own numbers: its tilt, share of land and plates
    kNoise = 2,    // the lattices of the noise fields
    kVolcano = 3,  // where volcanoes stand
    kCave = 4,     // caves and overhangs
    kDeposit = 5,  // chert, copper, ochre and the rest
    kHerd = 6,     // where herds live
    kWeather = 7,  // a day's rain while settling
    kFire = 8,     // lightning fires while settling
    kDetail = 9,   // the ground's detail, made on demand (P8)
    kCloud = 10,   // the clouds' noise (P8)
};

}  // namespace worldgen
