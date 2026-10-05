// P14's tests (IMPLEMENTATION α0.7c): every base sound made and within range, no two strikes alike, harder brighter,
// bigger deeper and longer, wetter duller, loops joined without a click, the live fire following its heat; the bank
// read back, and the murmur shifted for age, build and feeling. Pre-production code (research 00).
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "sound.hpp"

namespace {

using sound::Base;

std::vector<float> make(Base base, float hard, float size, float wet, std::uint32_t seed) {
    sound::Stuff stuff;
    stuff.hard = hard;
    stuff.size = size;
    stuff.wet = wet;
    return sound::make(base, stuff, seed);
}

// A measure averaged over eight seeds, so one play's chance does not decide a rule.
template <class Measure>
float average(Base base, float hard, float size, float wet, Measure measure) {
    float sum = 0.0f;
    for (std::uint32_t seed = 1; seed <= 8; ++seed) {
        sum += measure(make(base, hard, size, wet, seed));
    }
    return sum / 8.0f;
}

float seconds(const std::vector<float>& s) {
    return static_cast<float>(s.size()) / static_cast<float>(sound::kRate);
}

const sound::Bank& bank(const char* voice) {
    static const sound::Bank woman = [] {
        std::ifstream f(std::string(KD_BANK) + "/woman.bank", std::ios::binary);
        const std::vector<std::uint8_t> b((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        return sound::read_bank(b.data(), b.size());
    }();
    static const sound::Bank man = [] {
        std::ifstream f(std::string(KD_BANK) + "/man.bank", std::ios::binary);
        const std::vector<std::uint8_t> b((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        return sound::read_bank(b.data(), b.size());
    }();
    return std::strcmp(voice, "man") == 0 ? man : woman;
}

// Talk: four phrases of one stand-in language, joined.
std::vector<float> talk(const sound::Bank& b, const sound::Speaker& speaker, const sound::Feeling& feeling) {
    const auto words = sound::language(bank("woman"), 7, 14);
    std::vector<float> out;
    for (std::uint32_t p = 1; p <= 4; ++p) {
        const auto s = sound::phrase(b, words, speaker, feeling, p);
        out.insert(out.end(), s.begin(), s.end());
    }
    return out;
}

const sound::Feeling kAnger{1.25f, 1.6f, 1.6f, 0.05f, 1.15f};
const sound::Feeling kGrief{0.75f, 0.6f, 0.6f, 0.3f, 0.95f};

}  // namespace

// checks: SND-06
TEST_CASE("every base sound is made, within -1..1, and the same seed makes the same sound") {
    for (int b = 0; b < sound::kBases; ++b) {
        const auto base = static_cast<Base>(b);
        for (std::uint32_t seed = 1; seed <= 3; ++seed) {
            const auto s = make(base, 0.5f, 0.5f, 0.2f, seed);
            CAPTURE(b);
            REQUIRE(s.size() > 1000);
            // inside loops, checks take named truths: the analyzer misreads doctest's comparisons there
            const bool within =
                std::all_of(s.begin(), s.end(), [](float v) { return std::isfinite(v) && std::abs(v) <= 1.0f; });
            const bool heard = sound::loudness(s) > 0.01f;
            CHECK(within);
            CHECK(heard);
            const bool same = s == make(base, 0.5f, 0.5f, 0.2f, seed);
            CHECK(same);
        }
    }
}

// checks: SND-06
TEST_CASE("20 flint strikes in a row all differ") {
    std::vector<std::vector<float>> strikes;
    for (std::uint32_t seed = 1; seed <= 20; ++seed) {
        strikes.push_back(make(Base::kStrike, 0.9f, 0.3f, 0.0f, seed));
    }
    for (std::size_t a = 0; a < strikes.size(); ++a) {
        for (std::size_t b = a + 1; b < strikes.size(); ++b) {
            // differing in more than a few samples: a different length, or a mean difference of 2% of their loudness
            const auto& x = strikes[a];
            const auto& y = strikes[b];
            const std::size_t n = std::min(x.size(), y.size());
            double diff = 0.0;
            for (std::size_t i = 0; i < n; ++i) {
                diff += std::abs(static_cast<double>(x[i]) - static_cast<double>(y[i]));
            }
            const bool differ = x.size() != y.size() || diff / static_cast<double>(n) > 0.02 * sound::loudness(x);
            CHECK(differ);
        }
    }
}

// checks: SND-06
TEST_CASE("harder is brighter, bigger is deeper and longer, wetter is duller") {
    for (const Base base : {Base::kStrike, Base::kScrape, Base::kChop, Base::kStep, Base::kDrum}) {
        CAPTURE(static_cast<int>(base));
        const float hard = average(base, 0.9f, 0.5f, 0.0f, sound::brightness);
        const float soft = average(base, 0.1f, 0.5f, 0.0f, sound::brightness);
        const float big = average(base, 0.5f, 0.9f, 0.0f, sound::brightness);
        const float small = average(base, 0.5f, 0.1f, 0.0f, sound::brightness);
        const float wet = average(base, 0.5f, 0.5f, 0.9f, sound::brightness);
        const float dry = average(base, 0.5f, 0.5f, 0.0f, sound::brightness);
        CAPTURE(hard);
        CAPTURE(soft);
        CAPTURE(big);
        CAPTURE(small);
        CAPTURE(wet);
        CAPTURE(dry);
        const bool brighter = hard > soft;
        const bool deeper = big < small;
        const bool longer = average(base, 0.5f, 0.9f, 0.0f, seconds) > average(base, 0.5f, 0.1f, 0.0f, seconds);
        const bool duller = wet < dry;
        CHECK(brighter);
        CHECK(deeper);
        CHECK(longer);
        CHECK(duller);
    }
}

// checks: SND-11
TEST_CASE("the fire, the wind, the river and the rain loop without a click") {
    for (const Base base : {Base::kFire, Base::kWind, Base::kRiver, Base::kRain}) {
        CAPTURE(static_cast<int>(base));
        REQUIRE(sound::loops(base));
        const auto s = make(base, 0.5f, 0.6f, 0.3f, 4);
        // the jump from its last sample to its first is no bigger than its own steps nearly ever are
        std::vector<float> steps;
        for (std::size_t i = 1; i < s.size(); ++i) {
            steps.push_back(std::abs(s[i] - s[i - 1]));
        }
        std::nth_element(steps.begin(), steps.begin() + static_cast<std::ptrdiff_t>(steps.size() * 999 / 1000),
                         steps.end());
        const bool smooth = std::abs(s.front() - s.back()) <= steps[steps.size() * 999 / 1000];
        CHECK(smooth);
    }
    CHECK_FALSE(sound::loops(Base::kStrike));
}

// checks: SND-01
TEST_CASE("the live fire follows its heat: hotter is louder, with more crackles") {
    sound::Live fire(Base::kFire, 3, 0.1f);
    std::vector<float> low(static_cast<std::size_t>(sound::kRate * 3));
    fire.render(low.data(), static_cast<int>(low.size()));
    fire.set(0.9f);
    std::vector<float> rise(static_cast<std::size_t>(sound::kRate * 2));
    fire.render(rise.data(), static_cast<int>(rise.size()));  // the change glides in
    std::vector<float> high(low.size());
    fire.render(high.data(), static_cast<int>(high.size()));
    CHECK(sound::loudness(high) > 1.5f * sound::loudness(low));
    // crackles are sharp: the loudness of its sample-to-sample changes
    auto sharp = [](const std::vector<float>& s) {
        std::vector<float> d;
        for (std::size_t i = 1; i < s.size(); ++i) {
            d.push_back(s[i] - s[i - 1]);
        }
        return sound::loudness(d);
    };
    CHECK(sharp(high) > 2.0f * sharp(low));
}

// checks: SND-03
TEST_CASE("a bank reads back as written, and bytes that are not one read as empty") {
    std::vector<std::uint8_t> b = {'K', 'D', 'B', 'A', 'N', 'K', '1', '\0'};
    auto put = [&b](const void* p, std::size_t n) {
        const auto* c = static_cast<const std::uint8_t*>(p);
        b.insert(b.end(), c, c + n);
    };
    const std::uint32_t rate = sound::kRate;
    const float pitch = 120.0f;
    const std::uint32_t count = 1;
    put(&rate, 4);
    put(&pitch, 4);
    put(&count, 4);
    b.push_back(2);
    put("ka", 2);
    const std::uint32_t vowel = 2;
    const std::uint32_t n = 4;
    put(&vowel, 4);
    put(&n, 4);
    const std::int16_t pcm[4] = {0, 16384, -16384, 32767};
    put(pcm, 8);
    const std::uint32_t m = 2;
    put(&m, 4);
    const std::int32_t marks[2] = {1, 3};
    put(marks, 8);
    b.push_back(0);
    b.push_back(1);
    const sound::Bank read = sound::read_bank(b.data(), b.size());
    REQUIRE(read.syllables.size() == 1);
    CHECK(read.pitch == doctest::Approx(120.0f));
    const sound::Syllable* ka = read.find("ka");
    REQUIRE(ka != nullptr);
    CHECK(ka->vowel == 2);
    CHECK(ka->samples[3] == doctest::Approx(1.0f));
    const bool marked = ka->marks == std::vector<int>{1, 3};
    const bool voiced = ka->voiced == std::vector<std::uint8_t>{0, 1};
    CHECK(marked);
    CHECK(voiced);
    CHECK(sound::read_bank(b.data(), b.size() - 1).syllables.empty());
    b[0] = 'X';
    CHECK(sound::read_bank(b.data(), b.size()).syllables.empty());
}

// checks: SND-03
TEST_CASE("the bank holds both voices' syllables, marked") {
    for (const char* voice : {"woman", "man"}) {
        const sound::Bank& b = bank(voice);
        CAPTURE(voice);
        REQUIRE(b.syllables.size() == 60);
        for (const sound::Syllable& s : b.syllables) {
            const bool marked = std::count(s.voiced.begin(), s.voiced.end(), 1) >= 5;
            CHECK(marked);
        }
    }
    CHECK(bank("woman").pitch > 1.6f * bank("man").pitch);
}

// checks: SND-03
TEST_CASE("a language's syllables come from the bank, differ by seed and keep a") {
    const auto one = sound::language(bank("woman"), 1, 0);
    const auto two = sound::language(bank("woman"), 2, 0);
    const bool differ = one != two;
    CHECK(differ);
    CHECK(std::find(one.begin(), one.end(), "a") != one.end());
    for (const std::string& s : one) {
        CHECK(bank("man").find(s) != nullptr);
    }
    CHECK(sound::language(bank("woman"), 1, 5).size() == 5);
}

// checks: SND-03
TEST_CASE("a child, a woman, a man and an elder are shifted apart in pitch") {
    const float child = sound::pitch(talk(bank("woman"), {1.4f, 1.15f, 0.05f, 0.0f}, {}), 60.0f, 600.0f);
    const float woman = sound::pitch(talk(bank("woman"), {}, {}), 60.0f, 600.0f);
    const float man = sound::pitch(talk(bank("man"), {}, {}), 60.0f, 600.0f);
    const float elder = sound::pitch(talk(bank("man"), {0.9f, 0.97f, 0.2f, 0.035f}, {}), 60.0f, 600.0f);
    MESSAGE("child ", child, " Hz, woman ", woman, ", man ", man, ", elder ", elder);
    CHECK(child > 1.2f * woman);
    CHECK(woman > 1.6f * man);
    CHECK(man > elder);
    // each near what it was asked for
    CHECK(woman == doctest::Approx(bank("woman").pitch).epsilon(0.2));
    CHECK(man == doctest::Approx(bank("man").pitch).epsilon(0.2));
}

// checks: SND-03
TEST_CASE("anger is quicker, louder and higher; grief slower, softer and lower") {
    for (const char* voice : {"woman", "man"}) {
        CAPTURE(voice);
        const auto calm = talk(bank(voice), {}, {});
        const auto anger = talk(bank(voice), {}, kAnger);
        const auto grief = talk(bank(voice), {}, kGrief);
        const bool quicker = seconds(anger) < seconds(calm);
        const bool slower = seconds(grief) > seconds(calm);
        const bool louder = sound::loudness(anger) > 1.3f * sound::loudness(calm);
        const bool softer = sound::loudness(grief) < 0.8f * sound::loudness(calm);
        const bool higher = sound::pitch(anger, 60.0f, 600.0f) > sound::pitch(calm, 60.0f, 600.0f);
        const bool lower = sound::pitch(grief, 60.0f, 600.0f) < sound::pitch(calm, 60.0f, 600.0f);
        CHECK(quicker);
        CHECK(slower);
        CHECK(louder);
        CHECK(softer);
        CHECK(higher);
        CHECK(lower);
    }
}

// checks: SND-03
TEST_CASE("the murmur's phrases differ, and its cries are made") {
    const auto words = sound::language(bank("woman"), 7, 14);
    const bool differ = sound::phrase(bank("man"), words, {}, {}, 1) != sound::phrase(bank("man"), words, {}, {}, 2);
    CHECK(differ);
    CHECK(sound::phrase(bank("man"), {"zz"}, {}, {}, 1).empty());
    for (int c = 0; c < 4; ++c) {
        const auto s = sound::cry(bank("woman"), static_cast<sound::Cry>(c), {1.4f, 1.15f, 0.05f, 0.0f}, 2);
        CAPTURE(c);
        const bool long_enough = seconds(s) > 0.2f;
        const bool within = std::all_of(s.begin(), s.end(), [](float v) { return std::abs(v) <= 1.0f; });
        CHECK(long_enough);
        CHECK(within);
    }
    // a scream is far higher than talk in the same voice
    const auto scream = sound::cry(bank("man"), sound::Cry::kScream, {}, 2);
    CHECK(sound::pitch(scream, 60.0f, 600.0f) > 2.0f * bank("man").pitch);
}

// checks: SND-07
TEST_CASE("a blend loops: copies that run past its end carry on from its start") {
    const std::vector<std::vector<float>> sounds = {make(Base::kStrike, 0.9f, 0.3f, 0.0f, 1),
                                                    make(Base::kScrape, 0.3f, 0.5f, 0.0f, 2)};
    const auto hum = sound::blend(sounds, 3.0f, 8.0f, 5);
    CHECK(hum.size() == static_cast<std::size_t>(3 * sound::kRate));
    CHECK(sound::loudness(hum) > 0.05f);
    CHECK(std::all_of(hum.begin(), hum.end(), [](float v) { return std::abs(v) <= 1.0f; }));
    // a sound placed at the very end wraps: one long sound, blended once into a short loop, fills its start
    const auto wrapped =
        sound::blend({std::vector<float>(static_cast<std::size_t>(sound::kRate), 0.5f)}, 0.5f, 2.0f, 1);
    CHECK(std::all_of(wrapped.begin(), wrapped.end(), [](float v) { return v > 0.0f; }));
    CHECK(sound::blend({}, 1.0f, 4.0f, 1).size() == static_cast<std::size_t>(sound::kRate));
}

// checks: SND-01
TEST_CASE("a drummer's loop keeps its beat") {
    std::vector<std::vector<float>> hits;
    for (std::uint32_t seed = 1; seed <= 3; ++seed) {
        hits.push_back(make(Base::kDrum, 0.6f, 0.6f, 0.0f, seed));
    }
    const auto loop = sound::rhythm(hits, 4.0f, 0.5f, 3);
    REQUIRE(loop.size() == static_cast<std::size_t>(4 * sound::kRate));
    // the first beat of every bar is struck: loud just after it, quiet just before it
    for (int bar = 0; bar < 2; ++bar) {
        const auto at = static_cast<std::size_t>(bar) * 2 * static_cast<std::size_t>(sound::kRate);
        const std::vector<float> after(loop.begin() + static_cast<std::ptrdiff_t>(at),
                                       loop.begin() + static_cast<std::ptrdiff_t>(at + 2000));
        const bool struck = sound::loudness(after) > 0.1f;
        CHECK(struck);
    }
    CHECK(sound::rhythm({}, 1.0f, 0.5f, 1).size() == static_cast<std::size_t>(sound::kRate));
}
