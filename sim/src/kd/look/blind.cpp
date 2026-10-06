#include "kd/look/blind.hpp"

#include <algorithm>
#include <iterator>

#include "kd/chance/chance.hpp"
#include "kd/core/check.hpp"
#include "kd/num/letters.hpp"

namespace kd::look {

namespace {

constexpr unsigned kComparisonBits = 4;
constexpr unsigned kSeedBits = 16;

// How far a pair's view may lie from the world's centre east and north, in centimetres: the meadow anywhere; the
// plants' bank and path, which run north and south a few metres east and west of it; the camp's fires, round it.
struct Reach {
    std::int64_t east;
    std::int64_t north;
};

Reach reach_of(Comparison comparison) {
    switch (comparison) {
        case Comparison::leaves:
            return {300, 5'000};
        case Comparison::fire_shadows:
            return {150, 150};
        case Comparison::msaa:
            break;
    }
    return {5'000, 5'000};
}

}  // namespace

ComparisonWords comparison_words(Comparison comparison) {
    switch (comparison) {
        case Comparison::leaves:
            return {"Leaf edges", "Leaves smoothed by alpha to coverage against plain cut-outs, on the plants of C2",
                    "which has smoother leaf edges?"};
        case Comparison::fire_shadows:
            return {"Fire shadows", "Fire shadows by a walk at every pixel against by a map for each fire, at night",
                    "which has sharper shadows?"};
        case Comparison::msaa:
            break;
    }
    return {"Sharpness", "MSAA 4x against 2x on the meadow", "which is sharper?"};
}

std::vector<BlindPair> blind_pairs(Comparison comparison, std::uint32_t seed) {
    const chance::Draws draws(seed, chance::name("look"), 0, 0, chance::name("blind test"));
    const Reach reach = reach_of(comparison);
    std::vector<BlindPair> out;
    for (std::uint64_t i = 0; i < static_cast<std::uint64_t>(kBlindPairs); ++i) {
        BlindPair p;
        p.better_first = draws.below(4 * i, 2) == 0;
        p.heading = 5.0 * static_cast<double>(draws.below(4 * i + 1, 72));
        p.east = draws.between(4 * i + 2, -reach.east, reach.east);
        p.north = draws.between(4 * i + 3, -reach.north, reach.north);
        out.push_back(p);
    }
    return out;
}

std::int64_t blind_right(const BlindTest& test) {
    const std::vector<BlindPair> pairs = blind_pairs(test.comparison, test.seed);
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
    const auto known = std::find_if(std::begin(kComparisons), std::end(kComparisons),
                                    [&](Comparison c) { return static_cast<std::uint64_t>(c) == comparison; });
    if (known == std::end(kComparisons)) {
        out.why = "it compares something this build does not know (" + std::to_string(comparison) + ")";
        return out;
    }
    out.test.comparison = *known;
    out.test.seed = static_cast<std::uint32_t>(num::take_bits(letters.body, at, kSeedBits));
    for (std::int64_t i = 0; i < kBlindPairs; ++i) {
        const bool first = letters.body[at];
        ++at;
        out.test.chose_first.push_back(first);
    }
    return out;
}

}  // namespace kd::look
