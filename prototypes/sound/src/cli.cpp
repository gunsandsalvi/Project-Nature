// P14 Sound's command line (IMPLEMENTATION α0.7c): writes every base sound, talk in each kind of voice and feeling,
// and the cries, as 16-bit WAV files into a folder, for listening and for drawing.
//     sound_cli <bank folder> <out folder>
// Pre-production code (research 00).
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "sound.hpp"

namespace {

void put32(std::ofstream& f, std::uint32_t v) {
    f.write(reinterpret_cast<const char*>(&v), 4);
}
void put16(std::ofstream& f, std::uint16_t v) {
    f.write(reinterpret_cast<const char*>(&v), 2);
}

bool write_wav(const std::string& path, const std::vector<float>& s) {
    std::ofstream f(path, std::ios::binary);
    const auto bytes = static_cast<std::uint32_t>(s.size() * 2);
    f.write("RIFF", 4);
    put32(f, 36 + bytes);
    f.write("WAVEfmt ", 8);
    put32(f, 16);
    put16(f, 1);
    put16(f, 1);
    put32(f, static_cast<std::uint32_t>(sound::kRate));
    put32(f, static_cast<std::uint32_t>(sound::kRate) * 2);
    put16(f, 2);
    put16(f, 16);
    f.write("data", 4);
    put32(f, bytes);
    for (const float v : s) {
        const float c = v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v);
        put16(f, static_cast<std::uint16_t>(static_cast<std::int16_t>(c * 32767.0f)));
    }
    return static_cast<bool>(f);
}

sound::Bank load(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return sound::read_bank(bytes.data(), bytes.size());
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: sound_cli <bank folder> <out folder>\n");
        return 2;
    }
    const std::string banks = argv[1];
    const std::string out = std::string(argv[2]) + "/";
    static const char* const kAll[sound::kBases] = {"strike", "scrape",  "chop", "step", "fire", "wind", "river",
                                                    "rain",   "thunder", "bird", "bark", "howl", "drum"};
    bool ok = true;
    for (int b = 0; b < sound::kBases; ++b) {
        for (int v = 0; v < 3; ++v) {
            sound::Stuff stuff;
            stuff.hard = 0.2f + 0.3f * static_cast<float>(v);
            stuff.size = 0.2f + 0.3f * static_cast<float>(v);
            const auto s = sound::make(static_cast<sound::Base>(b), stuff, static_cast<std::uint32_t>(v + 1));
            ok = write_wav(out + kAll[b] + "-" + std::to_string(v) + ".wav", s) && ok;
        }
    }
    const sound::Bank woman = load(banks + "/woman.bank");
    const sound::Bank man = load(banks + "/man.bank");
    if (woman.syllables.empty() || man.syllables.empty()) {
        std::fprintf(stderr, "sound_cli: no bank in %s\n", banks.c_str());
        return 1;
    }
    const auto words = sound::language(woman, 7, 14);
    struct Kind {
        const char* name;
        const sound::Bank* bank;
        sound::Speaker speaker;
    };
    const Kind kinds[] = {{"child", &woman, {1.4f, 1.15f, 0.05f, 0.0f}},
                          {"woman", &woman, {1.0f, 1.0f, 0.0f, 0.0f}},
                          {"man", &man, {1.0f, 1.0f, 0.0f, 0.0f}},
                          {"elder", &man, {0.9f, 0.97f, 0.2f, 0.035f}}};
    struct Mood {
        const char* name;
        sound::Feeling feeling;
    };
    const Mood moods[] = {
        {"calm", {}}, {"anger", {1.25f, 1.6f, 1.6f, 0.05f, 1.15f}}, {"grief", {0.75f, 0.6f, 0.6f, 0.3f, 0.95f}}};
    for (const Kind& k : kinds) {
        for (const Mood& m : moods) {
            std::vector<float> talk;
            for (std::uint32_t p = 0; p < 3; ++p) {
                const auto s = sound::phrase(*k.bank, words, k.speaker, m.feeling, p + 1);
                talk.insert(talk.end(), s.begin(), s.end());
                talk.insert(talk.end(), static_cast<std::size_t>(sound::kRate / 3), 0.0f);
            }
            ok = write_wav(out + "talk-" + k.name + "-" + m.name + ".wav", talk) && ok;
        }
        for (int c = 0; c < 4; ++c) {
            const auto s = sound::cry(*k.bank, static_cast<sound::Cry>(c), k.speaker, 3);
            static const char* const kCries[] = {"laugh", "cry", "call", "scream"};
            ok = write_wav(out + "cry-" + k.name + "-" + kCries[c] + ".wav", s) && ok;
        }
    }
    return ok ? 0 : 1;
}
