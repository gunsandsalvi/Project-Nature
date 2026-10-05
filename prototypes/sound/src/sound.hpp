// P14 Sound (IMPLEMENTATION α0.7c, research 15): the camp's base sounds made by code from shaped noise and ringing
// modes (SND-06), varied by each play's seed and by what is involved: harder is brighter, bigger deeper and longer,
// wetter duller; the fire and the wind made live as they play, following their heat and speed (A16); and the murmur
// (SND-03): syllables from a bank the cloud renders once for a woman's and a man's voice (bank.py), strung into talk
// and shifted for age, build and feeling. Samples are floats in -1..1 at kRate. Pre-production code (research 00).
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sound {

constexpr int kRate = 22050;  // samples a second, the bank's own: a camp's sounds need little above 10 kHz

// What a sound's thing is (SND-06), each 0 to 1.
struct Stuff {
    float hard = 0.5f;  // soft (hide, earth, flesh) to hard (stone, bone)
    float size = 0.5f;  // small to big; for the fire its heat, for the wind its speed, for rain how heavy
    float wet = 0.0f;   // dry to soaked
};

// The base sounds a camp needs, of SND-06's set: work one-shots, the fire, the wind, the river and the rain, which
// loop, thunder and a drum; and stand-ins for the recorded calls of birds, dogs and a wolf.
enum class Base : std::uint8_t {
    kStrike,
    kScrape,
    kChop,
    kStep,
    kFire,
    kWind,
    kRiver,
    kRain,
    kThunder,
    kBird,
    kBark,
    kHowl,
    kDrum,
};
constexpr int kBases = 13;

// Whether a base sound loops, its end joined to its start.
bool loops(Base base);

// A base sound, varied by its seed and its stuff.
std::vector<float> make(Base base, const Stuff& stuff, std::uint32_t seed);

// A sound made as it plays, for what follows the world continuously (A16): the fire by its heat, the wind by its
// speed (Base::kFire or Base::kWind). set() may be called from any thread, render() only from the one that plays it,
// which it never makes wait or allocate.
class Live {
public:
    Live(Base base, std::uint32_t seed, float amount);
    ~Live();
    Live(const Live&) = delete;
    Live& operator=(const Live&) = delete;
    void set(float amount);
    void render(float* out, int frames);

private:
    struct State;
    std::unique_ptr<State> state_;
    std::atomic<float> target_;
};

// One syllable of the murmur's bank (SND-03), spoken once and marked: the middle of each grain the phone cuts from
// it, a pulse of the voice where it is voiced and a steady step where it is not, and where its vowel begins.
struct Syllable {
    std::string text;
    std::vector<float> samples;
    std::vector<int> marks;
    std::vector<std::uint8_t> voiced;
    int vowel = 0;
};

// A base voice's bank: its middle pitch and its syllables.
struct Bank {
    float pitch = 0.0f;
    std::vector<Syllable> syllables;
    const Syllable* find(const std::string& text) const;
};

// A bank from its file's bytes (bank.py's layout); empty if they are not one.
Bank read_bank(const std::uint8_t* data, std::size_t size);

// A speaker's shift of their base voice for age and build (SND-03, BIO-08): pitch and formants as ratios (a child's
// shorter throat raises them), breath, and the wobble of age.
struct Speaker {
    float pitch = 1.0f;
    float formants = 1.0f;
    float breath = 0.0f;
    float tremor = 0.0f;
};

// A feeling's shift of the voice (SND-03, MND-19): speed, the tune's swing, loudness, how far the tune falls by the
// phrase's end, and the pitch's own lift.
struct Feeling {
    float tempo = 1.0f;
    float range = 1.0f;
    float loud = 1.0f;
    float fall = 0.15f;
    float lift = 1.0f;
};

// A stand-in language's syllables (CUL-17): some of the bank's consonants before some of its vowels, drawn by the
// seed; at most count of them.
std::vector<std::string> language(const Bank& bank, std::uint32_t seed, int count);

// A phrase of the murmur: words of one to three of the syllables given, cut from the bank and joined, at the
// speaker's pitch and tune (TD-PSOLA: each pulse's grain moved to the new pitch, its formants shifted by stretching
// it).
std::vector<float> phrase(const Bank& bank, const std::vector<std::string>& syllables, const Speaker& speaker,
                          const Feeling& feeling, std::uint32_t seed);

// A blend (SND-07): copies of the sounds given scattered over a loop of `seconds`, about `per_second` of them a
// second, each at its own loudness; one that runs past the end carries on from the start, so it loops. The hum that
// a share's quietest sounds join when it is full (SND-01).
std::vector<float> blend(const std::vector<std::vector<float>>& sounds, float seconds, float per_second,
                         std::uint32_t seed);

// A drummer's loop of `seconds`: a hit from those given on most beats of `beat` seconds, the first of each bar of
// four the loudest, in a pattern drawn by the seed; a stand-in for SND-02's music.
std::vector<float> rhythm(const std::vector<std::vector<float>>& hits, float seconds, float beat, std::uint32_t seed);

// Stand-ins, made from the bank, for the few recordings SND-03 takes for laughing, crying, calling and screaming.
enum class Cry : std::uint8_t { kLaugh, kCry, kCall, kScream };
std::vector<float> cry(const Bank& bank, Cry kind, const Speaker& speaker, std::uint32_t seed);

// Measures for the tests: its brightness, the RMS of its spectrum's frequencies in Hz (from how fast it changes
// against how loud it is); its loudness (RMS); and its middle pitch between two bounds, by autocorrelation over its
// loud parts (0 if it has none).
float brightness(const std::vector<float>& s);
float loudness(const std::vector<float>& s);
float pitch(const std::vector<float>& s, float lowest, float highest);

}  // namespace sound
