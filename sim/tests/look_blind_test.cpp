#include <cstdint>
#include <string>
#include <vector>

#include "doctest.h"
#include "kd/look/blind.hpp"
#include "kd/num/letters.hpp"

namespace look = kd::look;

// checks: PRE-01
TEST_CASE("a blind test's pairs come from its seed, the better way first about half the time") {
    const std::vector<look::BlindPair> a = look::blind_pairs(look::Comparison::msaa, 7);
    const std::vector<look::BlindPair> b = look::blind_pairs(look::Comparison::msaa, 7);
    REQUIRE(a.size() == 10);
    std::int64_t first = 0;
    std::int64_t differ = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        CHECK(a[i].better_first == b[i].better_first);
        CHECK(a[i].heading == b[i].heading);
        CHECK(a[i].east == b[i].east);
        CHECK(a[i].heading >= 0.0);
        CHECK(a[i].heading < 360.0);
        CHECK(a[i].east >= -5'000);
        CHECK(a[i].north <= 5'000);
        differ += a[i].east != look::blind_pairs(look::Comparison::msaa, 8)[i].east ? 1 : 0;
    }
    CHECK(differ >= 9);
    // over many seeds, the better way comes first half the time
    for (std::uint32_t seed = 0; seed < 200; ++seed) {
        for (const look::BlindPair& p : look::blind_pairs(look::Comparison::msaa, seed)) {
            first += p.better_first ? 1 : 0;
        }
    }
    CHECK(first > 900);
    CHECK(first < 1'100);
}

// checks: PRE-01
TEST_CASE("a blind test's code reads back its comparison, seed and answers, and counts the right ones") {
    const std::vector<look::BlindPair> pairs = look::blind_pairs(look::Comparison::msaa, 40'000);
    look::BlindTest all_right{look::Comparison::msaa, 40'000, {}};
    for (const look::BlindPair& p : pairs) {
        all_right.chose_first.push_back(p.better_first);
    }
    CHECK(look::blind_right(all_right) == 10);
    look::BlindTest two_wrong = all_right;
    two_wrong.chose_first[3] = !two_wrong.chose_first[3];
    two_wrong.chose_first[9] = !two_wrong.chose_first[9];
    CHECK(look::blind_right(two_wrong) == 8);
    const std::string code = look::blind_code(two_wrong);
    CHECK(code.size() == 13);  // 54 bits in 11 letters, in groups of five
    const look::BlindRead read = look::read_blind_code(code);
    REQUIRE(read.why.empty());
    CHECK(read.test.seed == 40'000);
    CHECK(read.test.chose_first == two_wrong.chose_first);
    CHECK(look::blind_right(read.test) == 8);
}

// checks: PRE-01
TEST_CASE("a blind test's code with a wrong letter or the wrong length is refused in words") {
    look::BlindTest t{look::Comparison::msaa, 123, std::vector<bool>(10, true)};
    std::string code = look::blind_code(t);
    std::string wrong = code;
    wrong[2] = wrong[2] == 'A' ? 'B' : 'A';
    CHECK(look::read_blind_code(wrong).why.find("checksum") != std::string::npos);
    CHECK(look::read_blind_code(code.substr(0, 8)).why.find("letters") != std::string::npos);
    CHECK(look::read_blind_code("U").why.find("no code has") != std::string::npos);
}

// checks: PRE-01, PRE-46
TEST_CASE("each comparison asks its own question about views of its own scene, and its code says which it was") {
    CHECK(look::comparison_words(look::Comparison::msaa).asks == "which is sharper?");
    CHECK(look::comparison_words(look::Comparison::leaves).asks == "which has smoother leaf edges?");
    CHECK(look::comparison_words(look::Comparison::fire_shadows).name == "Fire shadows");
    // MSAA's pairs are α2.1b's, so its codes read as they did; the leaves keep to the plants' bank and path, and the
    // fires' shadows to the camp
    const std::vector<look::BlindPair> meadow = look::blind_pairs(look::Comparison::msaa, 7);
    const std::vector<look::BlindPair> plants = look::blind_pairs(look::Comparison::leaves, 7);
    const std::vector<look::BlindPair> camp = look::blind_pairs(look::Comparison::fire_shadows, 7);
    for (std::size_t i = 0; i < meadow.size(); ++i) {
        CHECK(plants[i].better_first == meadow[i].better_first);
        CHECK(plants[i].east >= -300);
        CHECK(plants[i].east <= 300);
        CHECK(camp[i].north >= -150);
        CHECK(camp[i].north <= 150);
    }
    for (const look::Comparison c : look::kComparisons) {
        look::BlindTest t{c, 99, std::vector<bool>(10, false)};
        const look::BlindRead read = look::read_blind_code(look::blind_code(t));
        REQUIRE(read.why.empty());
        CHECK(read.test.comparison == c);
    }
    // a comparison this build does not know: its number, the seed and ten answers, as a code writes them
    std::vector<bool> bits;
    kd::num::put_bits(bits, 4, 4);
    kd::num::put_bits(bits, 99, 16);
    bits.resize(bits.size() + 10, false);
    CHECK(look::read_blind_code(kd::num::write_letters(bits)).why.find("does not know (4)") != std::string::npos);
}
