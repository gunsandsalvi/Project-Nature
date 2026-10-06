#include "kd/look/blind.hpp"

#include "kd/chance/chance.hpp"
#include "kd/core/check.hpp"
#include "kd/num/letters.hpp"

namespace kd::look {

namespace {

constexpr unsigned kComparisonBits = 4;
constexpr unsigned kSeedBits = 16;
constexpr std::int64_t kReach = 5'000;  // how far a pair's view may lie from the world's centre, in centimetres

}  // namespace

std::vector<BlindPair> blind_pairs(std::uint32_t seed) {
    const chance::Draws draws(seed, chance::name("look"), 0, 0, chance::name("blind test"));
    std::vector<BlindPair> out;
    for (std::uint64_t i = 0; i < static_cast<std::uint64_t>(kBlindPairs); ++i) {
        BlindPair p;
        p.better_first = draws.below(4 * i, 2) == 0;
        p.heading = 5.0 * static_cast<double>(draws.below(4 * i + 1, 72));
        p.east = draws.between(4 * i + 2, -kReach, kReach);
        p.north = draws.between(4 * i + 3, -kReach, kReach);
        out.push_back(p);
    }
    return out;
}

std::int64_t blind_right(const BlindTest& test) {
    const std::vector<BlindPair> pairs = blind_pairs(test.seed);
    KD_CHECK(test.chose_first.size() == pairs.size(), "look: a blind test has an answer for each pair");
    std::int64_t right = 0;
    for (std::size_t i = 0; i < pairs.size(); ++i) {
        right += test.chose_first[i] == pairs[i].better_first ? 1 : 0;
    }
    return right;
}

std::string blind_code(const BlindTest& test) {
    KD_CHECK(test.chose_first.size() == static_cast<std::size_t>(kBlindPairs) && test.seed < (1U << kSeedBits),
             "look: a blind test has ten answers and a 16-bit seed");
    std::vector<bool> bits;
    num::put_bits(bits, static_cast<std::uint64_t>(test.comparison), kComparisonBits);
    num::put_bits(bits, test.seed, kSeedBits);
    for (const bool first : test.chose_first) {
        bits.push_back(first);
    }
    return num::write_letters(std::move(bits));
}

BlindRead read_blind_code(std::string_view code) {
    BlindRead out;
    const num::Letters letters =
        num::read_letters(code, kComparisonBits + kSeedBits + static_cast<std::size_t>(kBlindPairs));
    if (!letters.why.empty()) {
        out.why = letters.why;
        return out;
    }
    std::size_t at = 0;
    const std::uint64_t comparison = num::take_bits(letters.body, at, kComparisonBits);
    if (comparison != static_cast<std::uint64_t>(Comparison::msaa)) {
        out.why = "it compares something this build does not know (" + std::to_string(comparison) + ")";
        return out;
    }
    out.test.comparison = Comparison::msaa;
    out.test.seed = static_cast<std::uint32_t>(num::take_bits(letters.body, at, kSeedBits));
    for (std::int64_t i = 0; i < kBlindPairs; ++i) {
        const bool first = letters.body[at];
        ++at;
        out.test.chose_first.push_back(first);
    }
    return out;
}

}  // namespace kd::look
