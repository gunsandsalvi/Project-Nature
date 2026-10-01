// B78 (X10): stand-in simulation core in C++, shared by the phone and a headless Linux build.
#pragma once
#include <cstdint>

// SplitMix64 mixing, `rounds` times from `seed`. Pure and deterministic (X11).
inline uint64_t kcore_mix(uint64_t seed, uint32_t rounds) {
    uint64_t z = seed;
    for (uint32_t i = 0; i < rounds; ++i) {
        z += 0x9E3779B97F4A7C15ull;
        uint64_t x = z;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        z ^= x ^ (x >> 31);
    }
    return z;
}
