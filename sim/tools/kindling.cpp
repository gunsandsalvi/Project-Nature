// The kindling tool: the simulation without Godot, for the cloud (A2.2, A17).
//
//     kindling proof [--threads N] [suite...]    each suite's digest and time, one line each: "<suite> <digest> <ms>"
//     kindling suites                             the proof suites and what each computes
//     kindling catalogue check [data]            loads every source under the data folder (default: data), runs
//                                                 the catalogue's checks (MAT-17) and names each problem by file,
//                                                 line and column
//     kindling catalogue show <name> [data]      an entry's values, such as demo:walker
//     kindling catalogue schema [data]           every kind's fields, for whoever writes entries
//     kindling catalogue fingerprint [data]      the world-making version, each source's rules, world and look
//                                                 digests, and each entry's
//     kindling run [--days N] [--camps N] [--seed N] [--fuzz N] [--islands WINDOW --threads N] [data]
//                                                 the demonstration's crowd from the data folder, run one event at a
//                                                 time, with each game day's digests at midnight: the whole state's
//                                                 and each part's (TIM-16, A3.3)
//     kindling keep <world> [--camps N] [--seed N] [--until SECONDS] [--every SECONDS] [--call SECOND:CAMP]...
//                  [--data FOLDER]
//                                                 the crowd's world kept in a folder (A3.7): opened from its newest
//                                                 snapshot and journal, or made new; run to a game second with a
//                                                 snapshot every so many game seconds, calling camps home at their
//                                                 seconds unless the journal has; for the kill test (PLT-07)
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/num/digest.hpp"
#include "kd/proof/proof.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
#include "kd/world/world.hpp"

namespace {

int usage() {
    std::fprintf(
        stderr,
        "usage: kindling proof [--threads N] [suite...]\n"
        "       kindling suites\n"
        "       kindling catalogue check|schema|fingerprint [data]\n"
        "       kindling catalogue show <name> [data]\n"
        "       kindling run [--days N] [--camps N] [--seed N] [--fuzz N] [--islands WINDOW --threads N] [data]\n"
        "       kindling keep <world> [--camps N] [--seed N] [--until SECONDS] [--every SECONDS] "
        "[--call SECOND:CAMP]... [--data FOLDER]\n");
    return 2;
}

// Every .toml file under the data folder, with its path from there, as the catalogue takes them.
std::vector<kd::data::SourceFile> read_sources(const std::string& folder) {
    std::vector<kd::data::SourceFile> files;
    for (const auto& item : std::filesystem::recursive_directory_iterator(folder)) {
        if (item.is_regular_file() && item.path().extension() == ".toml") {
            std::ifstream in(item.path(), std::ios::binary);
            std::stringstream text;
            text << in.rdbuf();
            files.push_back({std::filesystem::relative(item.path(), folder).generic_string(), text.str()});
        }
    }
    return files;
}

int catalogue(const std::vector<std::string_view>& args) {
    if (args.empty()) {
        return usage();
    }
    const std::string_view command = args[0];
    const bool named = command == "show";
    if (named && args.size() < 2) {
        return usage();
    }
    const std::size_t folder_at = named ? 2 : 1;
    const std::string folder = args.size() > folder_at ? std::string(args[folder_at]) : "data";
    if (!std::filesystem::is_directory(folder)) {
        std::fprintf(stderr, "kindling: no data folder at %s\n", folder.c_str());
        return 1;
    }
    const std::vector<kd::data::SourceFile> files = read_sources(folder);
    kd::data::Catalogue cat;
    std::vector<kd::data::Problem> problems = cat.load(files);
    // the checks on the whole catalogue (MAT-17) run once it loads, since their faults would echo the loader's
    const bool loaded = problems.empty();
    if (command == "check" && loaded) {
        problems = kd::data::run_checks(cat);
    }
    for (const kd::data::Problem& p : problems) {
        std::printf("%s\n", kd::data::problem_text(p).c_str());
    }
    if (command == "check") {
        std::size_t entries = 0;
        for (const auto& k : cat.kinds()) {
            entries += k->size();
        }
        std::printf("Catalogue: %zu files, %zu kinds, %zu entries, %s, %zu problem%s\n", files.size(),
                    cat.kinds().size(), entries,
                    loaded ? (std::to_string(kd::data::checks().size()) + " checks run").c_str()
                           : "the checks wait until it loads",
                    problems.size(), problems.size() == 1 ? "" : "s");
        return problems.empty() ? 0 : 1;
    }
    if (!problems.empty()) {
        return 1;
    }
    if (command == "schema") {
        for (const auto& k : cat.kinds()) {
            std::printf("%s%s: %s\n%s\n", k->folder().c_str(), k->single() ? ".toml" : "/<name>.toml",
                        k->about().c_str(), k->schema().c_str());
        }
        return 0;
    }
    if (command == "fingerprint") {
        std::printf("world-making version %lld\n", static_cast<long long>(kd::data::kWorldMakingVersion));
        for (const kd::data::Source& s : cat.sources()) {
            std::printf("source %s version %lld rules %s world %s look %s\n", s.id.c_str(),
                        static_cast<long long>(s.version), kd::num::to_hex(s.digests[0]).c_str(),
                        kd::num::to_hex(s.digests[1]).c_str(), kd::num::to_hex(s.digests[2]).c_str());
        }
        for (const auto& k : cat.kinds()) {
            for (std::size_t i = 0; i < k->size(); ++i) {
                const kd::data::EntryDigests d = k->digests(i);
                std::printf("%s %s %s rules %s world %s look %s\n", k->folder().c_str(), k->name(i).c_str(),
                            kd::num::to_hex(d.all).c_str(), kd::num::to_hex(d.by_affects[0]).c_str(),
                            kd::num::to_hex(d.by_affects[1]).c_str(), kd::num::to_hex(d.by_affects[2]).c_str());
            }
        }
        return 0;
    }
    if (named) {
        for (const auto& k : cat.kinds()) {
            for (std::size_t i = 0; i < k->size(); ++i) {
                if (k->name(i) == args[1]) {
                    std::printf("%s, a %s from %s\n%s", k->name(i).c_str(), k->folder().c_str(), k->file(i).c_str(),
                                k->display(i).c_str());
                    return 0;
                }
            }
        }
        std::fprintf(stderr, "kindling: no entry named %.*s\n", static_cast<int>(args[1].size()), args[1].data());
        return 1;
    }
    return usage();
}

int run_world(const std::vector<std::string_view>& args) {
    long long days = 60;
    long long camps = -1;
    unsigned long long seed = 1;
    bool fuzz = false;
    unsigned long long fuzz_key = 0;
    long long window = 0;
    int threads = 1;
    std::string folder = "data";
    for (std::size_t i = 0; i < args.size(); ++i) {
        const bool more = i + 1 < args.size();
        if (args[i] == "--days" && more) {
            days = std::atoll(std::string(args[++i]).c_str());
        } else if (args[i] == "--camps" && more) {
            camps = std::atoll(std::string(args[++i]).c_str());
        } else if (args[i] == "--seed" && more) {
            seed = std::strtoull(std::string(args[++i]).c_str(), nullptr, 10);
        } else if (args[i] == "--islands" && more) {
            window = std::atoll(std::string(args[++i]).c_str());
        } else if (args[i] == "--threads" && more) {
            threads = std::atoi(std::string(args[++i]).c_str());
        } else if (args[i] == "--fuzz" && more) {
            fuzz = true;
            fuzz_key = std::strtoull(std::string(args[++i]).c_str(), nullptr, 10);
        } else {
            folder = std::string(args[i]);
        }
    }
    if (days < 1 || camps == 0 || camps < -1 || window < 0 || threads < 1 || threads > 16) {
        return usage();
    }
    const std::vector<kd::data::SourceFile> files = read_sources(folder);
    kd::data::Catalogue cat;
    const std::vector<kd::data::Problem> problems = cat.load(files);
    for (const kd::data::Problem& p : problems) {
        std::printf("%s\n", kd::data::problem_text(p).c_str());
    }
    if (!problems.empty()) {
        return 1;
    }
    kd::demo::CrowdWorld crowd(seed, cat, camps > 0 ? std::optional<std::int64_t>(camps) : std::nullopt);
    kd::world::World& w = crowd.world();
    if (fuzz) {
        w.set_fuzz(fuzz_key);
    }
    kd::run::Workers workers(threads);
    if (window > 0) {
        w.set_islands(&workers, window);
    }
    std::printf("seed %llu, %zu beings\n", seed, w.beings().size());
    for (long long day = 1; day <= days; ++day) {
        const kd::time::Seconds goal = day * kd::time::kDay;
        while (w.frontier() < goal) {
            w.advance(w.frontier(), goal);
        }
        const kd::world::Digests d = w.digests();
        std::printf("day %lld whole %s clock %s queue %s beings %s things %s systems %s events %llu\n", day,
                    kd::num::to_hex(d.whole).c_str(), kd::num::to_hex(d.clock).c_str(),
                    kd::num::to_hex(d.queue).c_str(), kd::num::to_hex(d.beings).c_str(),
                    kd::num::to_hex(d.things).c_str(), kd::num::to_hex(d.systems).c_str(),
                    static_cast<unsigned long long>(w.events_run()));
    }
    const kd::world::World::IslandCounts& n = w.island_counts();
    if (n.windows > 0) {
        std::printf(
            "islands: %llu windows, %.1f islands and %.0f owners a window, %.1f%% of events in each window's "
            "largest island\n",
            static_cast<unsigned long long>(n.windows), static_cast<double>(n.islands) / static_cast<double>(n.windows),
            static_cast<double>(n.owners) / static_cast<double>(n.windows),
            100.0 * static_cast<double>(n.largest_events) / static_cast<double>(std::max<std::uint64_t>(1, n.events)));
    }
    return 0;
}

int keep(const std::vector<std::string_view>& args) {
    if (args.empty()) {
        return usage();
    }
    const std::string folder(args[0]);
    long long camps = 0;
    unsigned long long seed = 1;
    long long until = kd::time::kDay;
    long long every = kd::time::kHour;
    std::string data = "data";
    std::vector<std::pair<kd::time::Seconds, std::size_t>> calls;
    for (std::size_t i = 1; i < args.size(); i += 2) {
        const bool more = i + 1 < args.size();
        const std::string value = more ? std::string(args[i + 1]) : "";
        if (args[i] == "--camps" && more) {
            camps = std::atoll(value.c_str());
        } else if (args[i] == "--seed" && more) {
            seed = std::strtoull(value.c_str(), nullptr, 10);
        } else if (args[i] == "--until" && more) {
            until = std::atoll(value.c_str());
        } else if (args[i] == "--every" && more) {
            every = std::atoll(value.c_str());
        } else if (args[i] == "--data" && more) {
            data = value;
        } else if (args[i] == "--call" && more && value.find(':') != std::string::npos) {
            calls.emplace_back(std::atoll(value.c_str()),
                               static_cast<std::size_t>(std::atoll(value.c_str() + value.find(':') + 1)));
        } else {
            return usage();
        }
    }
    if (camps < 0 || until < 1 || every < 1) {
        return usage();
    }
    std::sort(calls.begin(), calls.end());
    const std::vector<kd::data::SourceFile> files = read_sources(data);
    kd::data::Catalogue cat;
    if (!cat.load(files).empty()) {
        std::fprintf(stderr, "kindling: the catalogue under %s does not load\n", data.c_str());
        return 1;
    }
    std::filesystem::create_directories(folder);
    kd::save::DiskFiles disk(folder);
    kd::save::Keeper keeper(disk);
    kd::demo::Kept kept = kd::demo::keep_crowd(keeper, cat, seed, camps);
    for (const std::string& d : kept.damaged) {
        std::printf("damaged: %s\n", d.c_str());
    }
    if (!kept.crowd || !kept.problem.empty()) {
        std::printf("failed: %s\n", kept.problem.c_str());
        return 1;
    }
    kd::world::World& w = kept.crowd->world();
    std::printf("%s at %lld, %llu commands acted again, catching up to %lld\n",
                kept.made ? "made" : ("opened " + kept.snapshot).c_str(), static_cast<long long>(w.frontier()),
                static_cast<unsigned long long>(kept.replayed), static_cast<long long>(kept.was_at));
    std::fflush(stdout);

    std::vector<kd::ecs::Id> camp_ids;
    w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle /*h*/) {
        if (id.family() == kd::ecs::Family::place) {
            camp_ids.push_back(id);
        }
    });
    std::vector<kd::world::Record> records;
    w.keep_history(&records);
    // the calls the journal holds were given already, and a person never gives them twice
    std::size_t made = kept.journaled;
    while (w.frontier() < until) {
        const bool calling = made < calls.size();
        kd::time::Seconds stop = std::min<kd::time::Seconds>(until, (w.frontier() / every + 1) * every);
        if (calling) {
            stop = std::min(stop, std::max(w.frontier(), calls[made].first));
        }
        if (stop > w.frontier()) {
            w.run_to(stop);
        }
        keeper.history(records);
        records.clear();
        if (calling && w.frontier() == calls[made].first) {
            const kd::world::Command c =
                w.command(w.frontier(), static_cast<std::uint32_t>(kd::demo::Commanded::call_home),
                          camp_ids.at(calls[made].second).value, 0);
            keeper.command(c);
            ++made;
        }
        if (w.frontier() % every == 0) {
            keeper.snapshot(w);
        }
    }
    keeper.flush();
    std::printf("at %lld whole %s history %llu commands %llu mismatches %llu snapshots %llu\n",
                static_cast<long long>(w.frontier()), kd::num::to_hex(w.digests().whole).c_str(),
                static_cast<unsigned long long>(w.history_count()), static_cast<unsigned long long>(w.commands_made()),
                static_cast<unsigned long long>(keeper.mismatches()),
                static_cast<unsigned long long>(keeper.snapshots()));
    return keeper.mismatches() == 0 ? 0 : 1;
}

int proof(const std::vector<std::string_view>& args) {
    int threads = 1;
    std::vector<std::string_view> names;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--threads" && i + 1 < args.size()) {
            threads = std::atoi(std::string(args[++i]).c_str());
        } else {
            names.push_back(args[i]);
        }
    }
    if (threads < 1 || threads > 16) {
        return usage();
    }
    if (names.empty()) {
        for (const auto& s : kd::proof::suites()) {
            names.push_back(s.name);
        }
    }
    kd::run::Workers workers(threads);
    for (std::string_view name : names) {
        const auto start = std::chrono::steady_clock::now();
        const std::string digest = kd::proof::run(name, workers);
        const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        if (digest.empty()) {
            std::fprintf(stderr, "kindling: no proof suite named %.*s\n", static_cast<int>(name.size()), name.data());
            return 1;
        }
        std::printf("%.*s %s %.1f\n", static_cast<int>(name.size()), name.data(), digest.c_str(), ms);
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    if (args.empty()) {
        return usage();
    }
    const std::string_view command = args.front();
    args.erase(args.begin());
    if (command == "proof") {
        return proof(args);
    }
    if (command == "catalogue") {
        return catalogue(args);
    }
    if (command == "run") {
        return run_world(args);
    }
    if (command == "keep") {
        return keep(args);
    }
    if (command == "suites") {
        for (const auto& s : kd::proof::suites()) {
            std::printf("%.*s: %.*s\n", static_cast<int>(s.name.size()), s.name.data(),
                        static_cast<int>(s.about.size()), s.about.data());
        }
        return 0;
    }
    return usage();
}
