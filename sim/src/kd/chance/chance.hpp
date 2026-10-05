// Chance (A3.5): every draw is a pure function of its key, (world seed, system, being, moment, purpose, index), mixed
// by a chain of SplitMix64's finaliser, so any thread can draw any number in any order and get the same one, and a
// new kind of draw, keyed by its own name, never moves another's (TIM-16). Nothing in the simulation draws any
// other way.
#pragma once

#include <cstdint>
#include <string_view>

#include "kd/num/probability.hpp"

namespace kd::chance {

/// A system's or a purpose's name, as a stable 64-bit hash of its text (XXH3 through the canonical digest), the same
/// on every build and in every version. Implements TIM-16, see A3.5.
struct Name {
    std::uint64_t hash = 0;
    friend constexpr bool operator==(Name, Name) = default;
};

/// The name of a text, such as "weather" or "rain". Implements TIM-16, see A3.5.
Name name(std::string_view text);

/// The draws for one purpose of one being at one moment: the first five parts of the key, mixed once, then each draw
/// by its index. Implements TIM-16, see A3.5.
class Draws {
public:
    /// being is the being's never-reused id, or 0 for a draw that belongs to no one, such as the weather's; moment is
    /// the game second.
    Draws(std::uint64_t seed, Name system, std::uint64_t being, std::int64_t moment, Name purpose);

    /// 64 evenly spread bits: the index's draw.
    [[nodiscard]] std::uint64_t bits(std::uint64_t index) const;

    /// Whether the index's draw fires with probability p: its bits fall below p's threshold.
    [[nodiscard]] bool fires(std::uint64_t index, num::Probability p) const;

    /// A whole number from 0 to n - 1, n at least 1, by a 128-bit multiply: as even as 64 bits allow, off by at
    /// most n / 2^64.
    [[nodiscard]] std::uint64_t below(std::uint64_t index, std::uint64_t n) const;

    /// A whole number from lo to hi, both included.
    [[nodiscard]] std::int64_t between(std::uint64_t index, std::int64_t lo, std::int64_t hi) const;

    /// A fraction from 0 up to 1 (never 1), from the draw's top 53 bits.
    [[nodiscard]] double fraction(std::uint64_t index) const;

private:
    std::uint64_t prefix_;
};

}  // namespace kd::chance
