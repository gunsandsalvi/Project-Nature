// The blind test (A5.5, PRE-01): ten pairs of one view drawn two ways, the better way on one of them, each asking
// "which is sharper?"; eight or more right means the difference shows, which guessing reaches about 5% of the time.
// Each pair's view, and which of its two pictures is drawn the better way, come from the test's seed by keyed
// chance, the same on the phone and in the cloud, so the answers' code holds only the comparison, the seed and the
// answers. Written once, for the app that asks and the cloud that reads the code.
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace kd::look {

/// The pairs a test asks about, and the right answers at which a difference shows.
inline constexpr std::int64_t kBlindPairs = 10;
inline constexpr std::int64_t kBlindShows = 8;

/// What a blind test compares: the better way first.
enum class Comparison : std::uint8_t {
    msaa = 1,  // MSAA 4x against 2x, on the meadow
};

/// One pair: where its view looks, and whether the better way is drawn first (on top).
struct BlindPair {
    bool better_first = false;
    double heading = 0.0;   // degrees clockwise from north, in steps of 5
    std::int64_t east = 0;  // the focus, in centimetres east and north of the world's centre
    std::int64_t north = 0;
};

/// Implements PRE-01, see A5.5: a test's pairs, from its seed.
[[nodiscard]] std::vector<BlindPair> blind_pairs(std::uint32_t seed);

/// A test taken: what it compared, its seed, and for each pair whether the first picture was chosen.
struct BlindTest {
    Comparison comparison = Comparison::msaa;
    std::uint32_t seed = 0;  // 16 bits
    std::vector<bool> chose_first;
};

/// The pairs answered right: the better way chosen.
[[nodiscard]] std::int64_t blind_right(const BlindTest& test);

/// Implements PRE-01, see A5.5: a taken test as a short code (kd/num/letters.hpp): the comparison in 4 bits, the
/// seed in 16, then a bit for each of the ten answers.
[[nodiscard]] std::string blind_code(const BlindTest& test);

/// A code read back: the test, or why it cannot be read.
struct BlindRead {
    BlindTest test;
    std::string why;
};

/// Implements PRE-01, see A5.5: a blind test's code read back.
[[nodiscard]] BlindRead read_blind_code(std::string_view code);

}  // namespace kd::look
