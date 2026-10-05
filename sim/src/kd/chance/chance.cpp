#include "kd/chance/chance.hpp"

#include "kd/core/check.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/mix.hpp"

namespace kd::chance {

namespace {

// SplitMix64's step: the golden ratio's odd 64-bit multiple.
constexpr std::uint64_t kGamma = 0x9E3779B97F4A7C15ULL;

// One link of the chain: a part of the key, spread over all 64 bits as SplitMix64 spreads its counter (and never
// zero, so a part of 0 still moves the chain), joined to what came before and mixed.
constexpr std::uint64_t link(std::uint64_t chain, std::uint64_t part) {
    return num::mix64(chain ^ ((part + 1) * kGamma));
}

}  // namespace

Name name(std::string_view text) {
    num::Digest digest;
    digest.text(text);
    return {digest.value()};
}

Draws::Draws(std::uint64_t seed, Name system, std::uint64_t being, std::int64_t moment, Name purpose)
    : prefix_(link(link(link(link(link(0, seed), system.hash), being), static_cast<std::uint64_t>(moment)),
                   purpose.hash)) {}

std::uint64_t Draws::bits(std::uint64_t index) const {
    return link(prefix_, index);
}

bool Draws::fires(std::uint64_t index, num::Probability p) const {
    return p.fires(bits(index));
}

std::uint64_t Draws::below(std::uint64_t index, std::uint64_t n) const {
    KD_CHECK(n > 0, "chance::below needs at least one number to choose from");
    using Wide = unsigned __int128;
    return static_cast<std::uint64_t>((static_cast<Wide>(bits(index)) * n) >> 64U);
}

std::int64_t Draws::between(std::uint64_t index, std::int64_t lo, std::int64_t hi) const {
    KD_CHECK(lo <= hi, "chance::between needs lo to be at most hi");
    // The span in 64 unsigned bits, which wraps to 0 only for the whole range of 64-bit numbers.
    const std::uint64_t span = static_cast<std::uint64_t>(hi) - static_cast<std::uint64_t>(lo) + 1;
    const std::uint64_t offset = span == 0 ? bits(index) : below(index, span);
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(lo) + offset);
}

double Draws::fraction(std::uint64_t index) const {
    return static_cast<double>(bits(index) >> 11U) * 0x1p-53;
}

}  // namespace kd::chance
