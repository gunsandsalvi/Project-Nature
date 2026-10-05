// P7's run from the command line (WLD-11): "New world" from a seed, timed: the candidates at the coarse size, the
// best few at full size, the three offered, and settling the first; the maps, if a folder is given; and a digest of
// the offered worlds, last, for the cloud's comparison of x86-64, arm64 under qemu and thread counts.
//   worldgen_cli [threads] [small|full] [seed] [maps folder]
// Pre-production code (research 00).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "hash.hpp"
#include "world.hpp"

namespace {

worldgen::Settings settings_for(int argc, char** argv) {
    worldgen::Settings s;
    if (argc > 1) {
        s.threads = std::atoi(argv[1]);
    }
    if (argc > 2 && std::strcmp(argv[2], "small") == 0) {
        s = worldgen::small(s);
    }
    if (argc > 3) {
        s.seed = std::strtoull(argv[3], nullptr, 10);
    }
    return s;
}

void write_map(const worldgen::World& w, int scale, const std::string& path) {
    const std::vector<std::uint8_t> rgb = worldgen::map_rgb(w, scale);
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (f == nullptr) {
        return;
    }
    std::fprintf(f, "P6\n%d %d\n255\n", w.grid.width / scale, w.grid.height / scale);
    std::fwrite(rgb.data(), 1, rgb.size(), f);
    std::fclose(f);
}

}  // namespace

int main(int argc, char** argv) {
    const worldgen::Settings settings = settings_for(argc, argv);
    worldgen::Offer offer = worldgen::new_world(settings);
    const worldgen::Times& t = offer.times;
    std::printf("%d candidates at %d x %d in %.2f s on %d threads; %d qualified\n", offer.made, settings.coarse_width,
                settings.coarse_width / 2, offer.candidates_seconds, settings.threads, offer.qualified);
    for (const std::string& line : offer.log) {
        std::printf("  %s\n", line.c_str());
    }
    std::printf("the best few at %d x %d in %.2f s\n", settings.full_width, settings.full_width / 2,
                offer.best_seconds);
    std::printf(
        "each stage, summed over every world: plates and rock %.2f s, erosion %.2f s, climate %.2f s, life %.2f s, "
        "scoring %.2f s\n",
        t.plates, t.erosion, t.climate, t.life, t.score);
    std::vector<std::uint64_t> sums;
    for (std::size_t i = 0; i < offer.three.size(); ++i) {
        std::printf("offer %zu: %s\n", i + 1, offer.three[i].summary.c_str());
        sums.push_back(offer.three[i].checksum());
        if (argc > 4) {
            write_map(offer.three[i], settings.full_width / 512 > 0 ? settings.full_width / 512 : 1,
                      std::string(argv[4]) + "/map-" + std::to_string(i + 1) + ".ppm");
        }
    }
    if (!offer.three.empty()) {
        minds::Pool pool(settings.threads);
        const double settled = worldgen::settle(&offer.three[0], settings.settle_years, &pool);
        std::printf("settling the first for %d years: %.2f s, %zu herds\n", settings.settle_years, settled,
                    offer.three[0].herds.size());
        sums.push_back(offer.three[0].checksum());
    }
    std::printf("in all: %.2f s to three worlds\n", offer.candidates_seconds + offer.best_seconds);
    std::printf("digest %s\n", samebits::digest(sums).c_str());
    return 0;
}
