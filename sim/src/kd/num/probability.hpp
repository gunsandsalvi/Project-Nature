// Probabilities (A3.4, A3.5): whole numbers a 64-bit draw must fall below to fire, read exactly from a ratio or a
// catalogue's text, never from a floating number, so the phone and the cloud fire on exactly the same draws.
#pragma once

#include <cstdint>

#include "kd/core/check.hpp"

namespace kd::num {

/// A probability p as the draws below p × 2^64, rounded down, with certainty its own case, since 2^64 does not fit
/// in 64 bits. Implements TIM-16 and RES-05, see A3.4 and A3.5.
class Probability {
public:
    static constexpr Probability never() { return {0, false}; }
    static constexpr Probability always() { return {0, true}; }

    /// numerator / denominator, from 0 to 1, rounded down to the 64-bit draw.
    static constexpr Probability ratio(std::uint64_t numerator, std::uint64_t denominator) {
        KD_CHECK(denominator > 0 && numerator <= denominator, "a probability is a ratio from 0 to 1");
        if (numerator == denominator) {
            return always();
        }
        using Wide = unsigned __int128;
        return {static_cast<std::uint64_t>((static_cast<Wide>(numerator) << 64U) / denominator), false};
    }

    /// Whether a draw of 64 evenly spread bits fires.
    [[nodiscard]] constexpr bool fires(std::uint64_t draw) const { return certain_ || draw < below_; }

    /// The draws that fire: those below this, or all of them when certain.
    [[nodiscard]] constexpr std::uint64_t threshold() const { return below_; }
    [[nodiscard]] constexpr bool certain() const { return certain_; }

    friend constexpr bool operator==(Probability, Probability) = default;

private:
    constexpr Probability(std::uint64_t below, bool certain) : below_(below), certain_(certain) {}

    std::uint64_t below_;
    bool certain_;
};

}  // namespace kd::num
