// P9's run from the command line (IMPLEMENTATION α0.6a, WLD-18): worlds made by P7 from seeds 1, 2, 3 and on, each
// run headless for 30 years standing in for a world made in its present-day state, 10 settling years and 100 more
// with nobody in it; each species' total, the plants' and the big
// prey for each hunter, year by year; whether every species stayed within half and twice its settled total and in
// every biome it lived in, and the hunters within 1 to 50-200 of their prey. Writes the report the app's Reports page
// shows, and prints a line a world.
//   ecology_cli [threads] [worlds] [coarse|full] [report.json]
// Pre-production code (research 00).
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "ecology.hpp"
#include "hash.hpp"
#include "world.hpp"

namespace {

// 30 years stand in for a world made directly in its present-day state, then WLD-08's 10 settling years
constexpr int kMade = 30;
constexpr int kSettle = 10;
constexpr int kYears = 100;  // WLD-18
constexpr double kLow = 0.5;
constexpr double kHigh = 2.0;
constexpr double kFewest = 50.0;  // big prey a hunter, at fewest and most (WLD-18)
constexpr double kMost = 200.0;

std::string number(double v) {
    char text[32];
    std::snprintf(text, sizeof text, "%.3g", v);
    return text;
}

}  // namespace

int main(int argc, char** argv) {
    const int threads = argc > 1 ? std::atoi(argv[1]) : 4;
    const int worlds = argc > 2 ? std::atoi(argv[2]) : 20;
    const bool full = argc > 3 && std::strcmp(argv[3], "full") == 0;
    const std::string path = argc > 4 ? argv[4] : "";
    worldgen::Settings settings;
    settings.threads = threads;
    const int width = full ? settings.full_width : settings.coarse_width;
    minds::Pool pool(threads);
    const auto& kinds = worldgen::kinds();

    std::string json = "{\"title\": \"P9 Ecology\", ";
    json +=
        "\"question\": \"Do the totals of plants and animals stay believable for 100 years with nobody in the "
        "world?\", ";
    char head[256];
    std::snprintf(head, sizeof head, "\"cells\": [%d, %d], \"km\": %.2f, \"made\": %d, \"settle\": %d, \"years\": %d, ",
                  width, width / 2, worldgen::kAroundMetres / width / 1000.0, kMade, kSettle, kYears);
    json += head;
    json += "\"bounds\": [" + number(kLow) + ", " + number(kHigh) + "], \"ratio\": [" + number(kFewest) + ", " +
            number(kMost) + "], \"species\": [";
    for (int k = 0; k < worldgen::kKinds; ++k) {
        const worldgen::Kind& kind = kinds[static_cast<std::size_t>(k)];
        json += std::string(k > 0 ? ", " : "") + "{\"name\": \"" + kind.name + "\", \"kg\": " + number(kind.kg) +
                ", \"hunter\": " + (kind.hunter ? "true" : "false") +
                ", \"per_km2\": " + number(kind.hunter ? 0.0 : worldgen::damuth_density(kind.kg) / 6.0) + "}";
    }
    json += "], \"plants\": [\"grass\", \"browse\", \"mast\", \"trees\"], \"runs\": [";

    std::vector<std::uint64_t> sums;
    bool all_pass = true;
    for (int i = 0; i < worlds; ++i) {
        worldgen::Times times;
        const std::uint64_t seed = static_cast<std::uint64_t>(i) + 1;
        worldgen::World w = worldgen::make_world(seed, width, full, settings, &pool, &times);
        // the land round where history would begin (WLD-24), where a hard year shows that the world's totals hide
        worldgen::judge(&w);
        const worldgen::EcologyRun run = worldgen::run_ecology(w, kMade + kSettle, kYears, &pool, w.start.cell);
        sums.push_back(run.checksum);
        bool pass = true;
        std::string failed;
        std::string species;
        for (int k = 0; k < worldgen::kKinds; ++k) {
            const double settled = run.totals[0][static_cast<std::size_t>(k)];
            std::string line = "[";
            double low = 1.0;
            double high = 1.0;
            for (std::size_t y = 0; y < run.totals.size(); ++y) {
                const double r = settled > 0.0 ? run.totals[y][static_cast<std::size_t>(k)] / settled : 0.0;
                low = std::min(low, r);
                high = std::max(high, r);
                line += std::string(y > 0 ? ", " : "") + number(r);
            }
            line += "]";
            const std::uint32_t lost =
                run.biomes_settled[static_cast<std::size_t>(k)] & ~run.biomes_end[static_cast<std::size_t>(k)];
            const bool here = settled >= 1.0;
            const bool ok = !here || (low >= kLow && high <= kHigh && lost == 0);
            if (!ok) {
                pass = false;
                failed += std::string(failed.empty() ? "" : ", ") + kinds[static_cast<std::size_t>(k)].name + " " +
                          number(low) + "-" + number(high) + (lost != 0 ? " lost a biome" : "");
            }
            char row[160];
            std::snprintf(row, sizeof row, "{\"settled\": %.0f, \"low\": %s, \"high\": %s, \"lost\": %u, \"ok\": %s, ",
                          settled, number(low).c_str(), number(high).c_str(), lost, ok ? "true" : "false");
            std::string fed = "[";
            for (std::size_t y = 0; y < run.fed.size(); ++y) {
                fed += std::string(y > 0 ? ", " : "") + number(run.fed[y][static_cast<std::size_t>(k)]);
            }
            fed += "]";
            species += std::string(k > 0 ? ", " : "") + row + "\"years\": " + (here ? line : "[]") +
                       ", \"fed\": " + (here ? fed : "[]") + "}";
        }
        std::string plants;
        for (std::size_t p = 0; p < 4; ++p) {
            const double settled = run.plants[0][p];
            std::string line = "[";
            double low = 1.0;
            double high = 1.0;
            for (std::size_t y = 0; y < run.plants.size(); ++y) {
                const double r = settled > 0.0 ? run.plants[y][p] / settled : 0.0;
                low = std::min(low, r);
                high = std::max(high, r);
                line += std::string(y > 0 ? ", " : "") + number(r);
            }
            line += "]";
            // mast is a crop that fails and floods by the year, as oaks' do; the cover itself must hold
            const bool ok = p == 2 || (low >= kLow && high <= kHigh);
            if (!ok) {
                pass = false;
                const char* names[] = {"grass", "browse", "mast", "trees"};
                failed += std::string(failed.empty() ? "" : ", ") + names[p] + " " + number(low) + "-" + number(high);
            }
            plants += std::string(p > 0 ? ", " : "") + "{\"settled\": " + number(settled) +
                      ", \"low\": " + number(low) + ", \"high\": " + number(high) +
                      ", \"ok\": " + (ok ? "true" : "false") + ", \"years\": " + line + "}";
        }
        std::string local;
        double local_low = 1.0;
        double local_high = 1.0;
        for (int k = 0; k < worldgen::kKinds && !run.local.empty(); ++k) {
            const double settled = run.local[0][static_cast<std::size_t>(k)];
            if (settled < 10.0) {
                continue;
            }
            std::string line = "[";
            double low = 1.0;
            double high = 1.0;
            for (std::size_t y = 0; y < run.local.size(); ++y) {
                const double r = run.local[y][static_cast<std::size_t>(k)] / settled;
                low = std::min(low, r);
                high = std::max(high, r);
                line += std::string(y > 0 ? ", " : "") + number(r);
            }
            local_low = std::min(local_low, low);
            local_high = std::max(local_high, high);
            local += std::string(local.empty() ? "" : ", ") + "{\"kind\": " + std::to_string(k) +
                     ", \"settled\": " + number(settled) + ", \"low\": " + number(low) + ", \"high\": " + number(high) +
                     ", \"years\": " + line + "]}";
        }
        double ratio = 0.0;
        double fewest = 1.0e9;
        double most = 0.0;
        for (const double r : run.prey_per_hunter) {
            ratio += r / static_cast<double>(run.prey_per_hunter.size());
            fewest = std::min(fewest, r);
            most = std::max(most, r);
        }
        const bool ratio_ok = ratio >= kFewest && ratio <= kMost;
        if (!ratio_ok) {
            pass = false;
            failed += std::string(failed.empty() ? "" : ", ") + "prey a hunter " + number(ratio);
        }
        all_pass = all_pass && pass;
        std::printf(
            "world %d: %s in %.1f s, %d x %d cells, big prey a hunter %.0f (%.0f to %.0f), round the start %.2f to "
            "%.2f%s%s\n",
            i + 1, pass ? "passes" : "fails", run.seconds, w.grid.width, w.grid.height, ratio, fewest, most, local_low,
            local_high, failed.empty() ? "" : ": ", failed.c_str());
        char tail[200];
        std::snprintf(tail, sizeof tail,
                      "], \"prey_per_hunter\": {\"mean\": %.1f, \"low\": %.1f, \"high\": %.1f, \"ok\": %s}, "
                      "\"pass\": %s, \"seconds\": %.1f}",
                      ratio, fewest, most, ratio_ok ? "true" : "false", pass ? "true" : "false", run.seconds);
        json += std::string(i > 0 ? ", " : "") + "{\"seed\": " + std::to_string(seed) + ", \"species\": [";
        json += species;
        json +=
            "], \"local\": {\"low\": " + number(local_low) + ", \"high\": " + number(local_high) + ", \"species\": [";
        json += local;
        json += "]}, \"plants\": [";
        json += plants;
        json += tail;
    }
    json += "], \"pass\": " + std::string(all_pass ? "true" : "false") + ", \"digest\": \"" + samebits::digest(sums) +
            "\"}\n";
    std::printf("%s, digest %s\n", all_pass ? "PASS" : "FAIL", samebits::digest(sums).c_str());
    if (!path.empty()) {
        std::FILE* f = std::fopen(path.c_str(), "wb");
        if (f != nullptr) {
            std::fwrite(json.data(), 1, json.size(), f);
            std::fclose(f);
        }
    }
    return 0;
}
