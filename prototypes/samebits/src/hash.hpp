// P5's hashes (RES-05): FNV-1a over a value's bytes, for the checksum of a whole state, a run's checksums joined in
// one digest, and a hash as text. P6 hashes its people the same way. Pre-production code (research 00).
#pragma once

#include <bit>
#include <cstdint>
#include <string>
#include <vector>

#include "chance.hpp"

namespace samebits {

// The hash of nothing: FNV-1a's offset basis.
constexpr std::uint64_t kHashStart = 0xcbf29ce484222325ULL;

inline void hash_into(std::uint64_t* h, std::uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        *h ^= (v >> (8U * static_cast<unsigned>(i))) & 0xffU;
        *h *= 0x100000001b3ULL;
    }
}

inline void hash_into(std::uint64_t* h, double v) {
    hash_into(h, std::bit_cast<std::uint64_t>(v));
}

inline void hash_into(std::uint64_t* h, float v) {
    hash_into(h, static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(v)));
}

// A hash as 16 hexadecimal digits.
inline std::string hex(std::uint64_t value) {
    std::string text(16, '0');
    for (int i = 15; i >= 0; --i) {
        text[static_cast<std::size_t>(i)] = "0123456789abcdef"[value & 0xfU];
        value >>= 4U;
    }
    return text;
}

// A run's checksums in one hash, as 16 hexadecimal digits.
inline std::string digest(const std::vector<std::uint64_t>& sums) {
    std::uint64_t h = 0;
    for (const std::uint64_t s : sums) {
        h = mix(h ^ s);
    }
    return hex(h);
}

}  // namespace samebits
