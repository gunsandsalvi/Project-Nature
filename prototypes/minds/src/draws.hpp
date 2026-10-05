// P6's draws of keyed chance (A3.5, TIM-16), each its own purpose, so a new kind of draw never shifts the others.
// Pre-production code (research 00).
#pragma once

#include <cstdint>

namespace minds {

enum class Draw : std::uint8_t {
    kLand = 1,    // the land's noise
    kPlace = 2,   // where a spot or camp lies
    kBirth = 3,   // a person's traits, age and needs at the start
    kPick = 4,    // taking the best option or one close behind
    kLength = 5,  // how long an activity lasts
    kFar = 6,     // where exploring heads
    kTalk = 7,    // whether a person talks, and what they pass on
    kFind = 8,    // whether work at a spot yields
};

}  // namespace minds
