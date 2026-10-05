// P5's keyed chance (A3.5, TIM-16): every draw is a hash of who draws it, when and why, so any thread can draw any
// number in any order and get the same one, and a new kind of draw never shifts the others. Pre-production code
// (research 00).
#pragma once

#include <cstdint>

namespace samebits {

// The draws the toy world makes, each its own purpose.
enum class Purpose : std::uint8_t { kTurn = 1, kWait = 2, kEat = 3, kStart = 4 };

// SplitMix64's finaliser: every bit of the input moves every bit of the output.
inline std::uint64_t mix(std::uint64_t z) {
    z += 0x9e3779b97f4a7c15ULL;
    z = (z ^ (z >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27U)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31U);
}

// A number from 0 up to 1, keyed by the world's seed, the being, the tick, the purpose and an index. The purpose is
// any enumeration of draws, this toy world's or another prototype's.
template <typename P>
inline double chance(std::uint64_t seed, std::uint64_t being, std::uint64_t tick, P purpose, std::uint64_t index = 0) {
    std::uint64_t h = mix(seed);
    h = mix(h ^ being);
    h = mix(h ^ tick);
    h = mix(h ^ static_cast<std::uint64_t>(purpose));
    h = mix(h ^ index);
    return static_cast<double>(h >> 11U) * 0x1.0p-53;
}

}  // namespace samebits
