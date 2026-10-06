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
//                  [--data FOLDER] [--build NAME] [--islands WINDOW --threads N]
//                                                 the crowd's world kept in a folder (A3.7): opened from its newest
//                                                 snapshot and journal, or made new; run to a game second with a
//                                                 snapshot every so many game seconds, calling camps home at their
//                                                 seconds unless the journal has; for the kill test (PLT-07) and the
//                                                 corpus (PLT-09), the build naming the version that saves it
//     kindling scene <scene.toml> [--out FOLDER] [--jobs N] [--data FOLDER] [--build NAME] [--fresh]
//                                                 a scene's runs (A17, RES-21), each world in a process of its own and
//                                                 kept in its own folder under the output folder (build/scenes/<name>
//                                                 by default), so a crash or creeping memory is caught (RES-12) and a
//                                                 scene stopped and run again resumes where it was (PLT-05); saved
//                                                 under the build's name, the app's version for worlds the phone opens;
//                                                 judged by its rule, rerun on 20 fresh seeds if it fails (RES-13), and
//                                                 reported in report.json (RES-06); exit 0 when it passes with no
//                                                 oddity
//     kindling bench [data]                      the benchmark's scenarios (A18.1): each one's mark and the digest its
//                                                 world must reach there, run headless here (RES-05), one line each:
//                                                 "<scenario> <mark> <digest>"
//     kindling bench decode <code> [data]        a phone's code in words: each measure against its pass line, each
//                                                 digest against the cloud's (PLT-04)
//     kindling export <world> <file>             a world's folder as one .kindling file (PLT-08)
//     kindling import <file> <world>             a .kindling file into a new world's folder, refused with words
//                                                 naming any damage
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

#include "kd/bench/code.hpp"
#include "kd/bench/scenarios.hpp"
#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/crowd_scene.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/num/digest.hpp"
#include "kd/proof/proof.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/archive.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
#include "kd/scene/report.hpp"
#include "kd/scene/scene.hpp"
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
        "[--call SECOND:CAMP]... [--data FOLDER] [--build NAME] [--islands WINDOW --threads N]\n"
        "       kindling scene <scene.toml> [--out FOLDER] [--jobs N] [--data FOLDER] [--build NAME] [--fresh]\n"
        "       kindling bench [data]\n"
        "       kindling bench decode <code> [data]\n"
        "       kindling export <world> <file>\n"
        "       kindling import <file> <world>\n");
    return 2;
}

// Every .toml file under the data folder, with its path from there, as the catalogue takes them.
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
    const std::vector<kd::data::SourceFile> files = kd::data::read_folder(folder);
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
    const std::vector<kd::data::SourceFile> files = kd::data::read_folder(folder);
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
    std::string build = "kindling";
    long long window = 0;
    long long threads = 1;
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
        } else if (args[i] == "--build" && more) {
            build = value;
        } else if (args[i] == "--islands" && more) {
            window = std::atoll(value.c_str());
        } else if (args[i] == "--threads" && more) {
            threads = std::atoll(value.c_str());
        } else if (args[i] == "--call" && more && value.find(':') != std::string::npos) {
            calls.emplace_back(std::atoll(value.c_str()),
                               static_cast<std::size_t>(std::atoll(value.c_str() + value.find(':') + 1)));
        } else {
            return usage();
        }
    }
    if (camps < 0 || until < 1 || every < 1 || window < 0 || threads < 1 || threads > 16) {
        return usage();
    }
    std::sort(calls.begin(), calls.end());
    const std::vector<kd::data::SourceFile> files = kd::data::read_folder(data);
    kd::data::Catalogue cat;
    if (!cat.load(files).empty()) {
        std::fprintf(stderr, "kindling: the catalogue under %s does not load\n", data.c_str());
        return 1;
    }
    std::filesystem::create_directories(folder);
    kd::save::DiskFiles disk(folder);
    kd::save::Keeper keeper(disk, build);
    kd::demo::Kept kept = kd::demo::keep_crowd(keeper, cat, seed, camps);
    for (const std::string& d : kept.damaged) {
        std::printf("damaged: %s\n", d.c_str());
    }
    if (!kept.crowd || !kept.problem.empty()) {
        std::printf("failed: %s\n", kept.problem.c_str());
        return 1;
    }
    kd::world::World& w = kept.crowd->world();
    kd::run::Workers workers(static_cast<int>(threads));
    std::printf("%s at %lld, %llu commands acted again, catching up to %lld\n",
                kept.made ? "made" : ("opened " + kept.snapshot).c_str(), static_cast<long long>(w.frontier()),
                static_cast<unsigned long long>(kept.replayed), static_cast<long long>(kept.was_at));
    std::fflush(stdout);

    const std::vector<kd::ecs::Id> camp_ids = kept.crowd->camp_ids();
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
        if (stop > w.frontier() && window > 0) {
            w.run_islands(stop, workers, window);
        } else if (stop > w.frontier()) {
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

// The resident memory of this process, in bytes.
std::int64_t resident_bytes() {
    std::ifstream statm("/proc/self/statm");
    long long pages = 0;
    long long resident = 0;
    statm >> pages >> resident;
    return resident * static_cast<std::int64_t>(sysconf(_SC_PAGESIZE));
}

// A run's result as its process leaves it in its folder, a line a fact.
void write_result(const std::string& folder, const kd::scene::RunResult& r) {
    std::string text = "seed " + std::to_string(r.seed) + "\ndays " + std::to_string(r.days) + "\ndigest " +
                       std::to_string(r.digest) + "\n";
    for (const kd::world::Switch s : r.switches) {
        text += "switch " + std::string(kd::world::kSwitchNames[static_cast<std::size_t>(s)]) + "\n";
    }
    for (const auto& [name, value] : r.measures) {
        text += "measure " + name + " " + std::to_string(value) + "\n";
    }
    for (const std::string& o : r.oddities) {
        text += "oddity " + o + "\n";
    }
    {
        std::ofstream out(folder + "/result.tmp", std::ios::binary);
        out << text;
    }
    std::filesystem::rename(folder + "/result.tmp", folder + "/result.txt");
}

std::optional<kd::scene::RunResult> read_result(const std::string& folder, std::int64_t index) {
    std::ifstream in(folder + "/result.txt", std::ios::binary);
    if (!in) {
        return std::nullopt;
    }
    kd::scene::RunResult r;
    r.index = index;
    std::string line;
    while (std::getline(in, line)) {
        const std::size_t space = line.find(' ');
        const std::string key = line.substr(0, space);
        const std::string rest = space == std::string::npos ? "" : line.substr(space + 1);
        if (key == "seed") {
            r.seed = std::atoll(rest.c_str());
        } else if (key == "days") {
            r.days = std::atoll(rest.c_str());
        } else if (key == "digest") {
            r.digest = std::strtoull(rest.c_str(), nullptr, 10);
        } else if (key == "switch") {
            if (const auto s = kd::world::switch_named(rest)) {
                r.switches.push_back(*s);
            }
        } else if (key == "measure") {
            const std::size_t at = rest.find(' ');
            r.measures.emplace_back(rest.substr(0, at), std::atoll(rest.c_str() + at + 1));
        } else if (key == "oddity") {
            r.oddities.push_back(rest);
        }
    }
    return r;
}

int scene_command(const std::vector<std::string_view>& args) {
    if (args.empty()) {
        return usage();
    }
    const std::string file(args[0]);
    std::string out;
    std::string data = "data";
    std::string build = "kindling";
    long long jobs = std::max(1U, std::thread::hardware_concurrency());
    bool fresh = false;
    for (std::size_t i = 1; i < args.size(); ++i) {
        const bool more = i + 1 < args.size();
        if (args[i] == "--out" && more) {
            out = std::string(args[++i]);
        } else if (args[i] == "--jobs" && more) {
            jobs = std::atoll(std::string(args[++i]).c_str());
        } else if (args[i] == "--data" && more) {
            data = std::string(args[++i]);
        } else if (args[i] == "--build" && more) {
            build = std::string(args[++i]);
        } else if (args[i] == "--fresh") {
            fresh = true;
        } else {
            return usage();
        }
    }
    std::ifstream in(file, std::ios::binary);
    std::stringstream text;
    text << in.rdbuf();
    const std::array<kd::scene::WorldKind, 1> kinds{kd::demo::crowd_kind()};
    const kd::scene::Read read = kd::scene::read_scene(text.str(), file, kinds);
    for (const kd::data::Problem& p : read.problems) {
        std::printf("%s\n", kd::data::problem_text(p).c_str());
    }
    if (!in || !read.problems.empty() || jobs < 1) {
        return read.problems.empty() ? usage() : 1;
    }
    const kd::scene::Scene& s = read.scene;
    if (out.empty()) {
        out = "build/scenes/" + s.name;
    }
    std::error_code error;
    if (fresh) {
        std::filesystem::remove_all(out, error);
    }
    std::filesystem::create_directories(out);
    kd::data::Catalogue cat;
    if (!cat.load(kd::data::read_folder(data)).empty()) {
        std::fprintf(stderr, "kindling: the catalogue under %s does not load\n", data.c_str());
        return 1;
    }
    std::fflush(nullptr);

    using Clock = std::chrono::steady_clock;
    const Clock::time_point started = Clock::now();
    const auto seconds_since = [](Clock::time_point t) {
        return std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - t).count();
    };
    std::vector<kd::scene::RunResult> results;
    bool over_budget = false;
    // the runs [from, to), as many at once as there are jobs, each world in a process of its own
    const auto run_batch = [&](std::int64_t from, std::int64_t to) {
        struct Child {
            std::int64_t index = 0;
            Clock::time_point started;
            bool timed_out = false;
        };
        std::map<pid_t, Child> running;
        std::int64_t next = from;
        while (next < to || !running.empty()) {
            while (next < to && static_cast<long long>(running.size()) < jobs) {
                const std::int64_t index = next++;
                char name[32];
                std::snprintf(name, sizeof name, "/run-%03lld", static_cast<long long>(index));
                const std::string folder = out + name;
                if (const std::optional<kd::scene::RunResult> done = read_result(folder, index)) {
                    results.push_back(*done);
                    continue;
                }
                if (seconds_since(started) > s.budget) {
                    over_budget = true;
                    next = to;
                    break;
                }
                std::filesystem::create_directories(folder);
                std::fflush(nullptr);
                const pid_t pid = fork();
                if (pid == 0) {
                    kd::save::DiskFiles disk(folder);
                    std::vector<std::int64_t> resident;
                    kd::scene::RunResult r = kd::demo::run_crowd(s, index, disk, cat, build, [&](std::int64_t /*day*/) {
                        resident.push_back(resident_bytes());
                    });
                    // memory that creeps up over the run is a leak (RES-12)
                    constexpr std::int64_t kCreep = std::int64_t{32} << 20U;
                    if (resident.size() >= 2 && resident.back() - resident.front() > kCreep) {
                        r.oddities.push_back("its memory crept up by " +
                                             std::to_string((resident.back() - resident.front()) >> 20U) +
                                             " MB over its run");
                    }
                    write_result(folder, r);
                    std::fflush(nullptr);
                    _exit(0);
                }
                running[pid] = {index, Clock::now(), false};
            }
            int status = 0;
            const pid_t ended = waitpid(-1, &status, WNOHANG);
            if (ended > 0) {
                const Child child = running[ended];
                running.erase(ended);
                char name[32];
                std::snprintf(name, sizeof name, "/run-%03lld", static_cast<long long>(child.index));
                std::optional<kd::scene::RunResult> r = read_result(out + name, child.index);
                if (!r || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                    kd::scene::RunResult failed;
                    failed.index = child.index;
                    failed.seed = s.seed + child.index;
                    failed.switches = s.switches_of(child.index);
                    if (child.timed_out) {
                        failed.oddities.push_back("it took longer than its limit of " + std::to_string(s.limit) +
                                                  " s, and was stopped");
                    } else if (WIFSIGNALED(status)) {
                        failed.oddities.push_back("it crashed, stopped by signal " + std::to_string(WTERMSIG(status)));
                    } else {
                        failed.oddities.push_back("it ended with no result");
                    }
                    r = failed;
                }
                results.push_back(*r);
                continue;
            }
            for (auto& [pid, child] : running) {
                if (!child.timed_out && seconds_since(child.started) > s.limit) {
                    child.timed_out = true;
                    kill(pid, SIGKILL);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };
    const auto measures = [&] {
        std::stable_sort(results.begin(), results.end(),
                         [](const auto& a, const auto& b) { return a.index < b.index; });
        std::vector<std::optional<std::int64_t>> m;
        m.reserve(results.size());
        for (const kd::scene::RunResult& r : results) {
            m.push_back(r.measure(s.pass.measure));
        }
        return m;
    };
    run_batch(0, s.runs);
    kd::scene::Verdict verdict = kd::scene::judge(s, measures());
    if (kd::scene::rerun_due(s, verdict) && !over_budget) {
        // failed on its runs, it runs as many on fresh seeds and is judged on all (RES-13)
        run_batch(s.runs, 2 * s.runs);
        verdict = kd::scene::judge(s, measures());
    }
    kd::scene::Outcome outcome;
    outcome.verdict = verdict;
    outcome.seconds = seconds_since(started);
    outcome.over_budget = over_budget;
    outcome.build = build;
    const std::string report = kd::scene::report_json(s, kinds[0], results, outcome);
    {
        std::ofstream o(out + "/report.json", std::ios::binary);
        o << report;
    }
    std::size_t oddities = 0;
    for (const kd::scene::RunResult& r : results) {
        oddities += r.oddities.size();
        for (const std::string& o : r.oddities) {
            std::printf("oddity: run %lld: %s\n", static_cast<long long>(r.index), o.c_str());
        }
    }
    std::printf("%s: %s, %lld of %lld runs met %s (%lld needed)%s%s; %zu oddities; %lld s of a %lld s budget\n",
                s.name.c_str(), verdict.passed ? "pass" : "fail", static_cast<long long>(verdict.passes),
                static_cast<long long>(verdict.judged), kd::scene::rule_words(s).c_str(),
                static_cast<long long>(verdict.needed), verdict.reran ? ", judged again on fresh seeds" : "",
                verdict.provisional ? ", provisional" : "", oddities, static_cast<long long>(outcome.seconds),
                static_cast<long long>(s.budget));
    return verdict.passed && oddities == 0 ? 0 : 1;
}

// A count with its thousands set apart: 10,000.
std::string grouped(long long n) {
    const std::string digits = std::to_string(n < 0 ? -n : n);
    std::string out = n < 0 ? "-" : "";
    for (std::size_t i = 0; i < digits.size(); ++i) {
        if (i > 0 && (digits.size() - i) % 3 == 0) {
            out += ',';
        }
        out += digits[i];
    }
    return out;
}

// A speed in words: "2.3 game days a real second".
std::string speed_words(double game_per_real) {
    char text[64];
    if (game_per_real >= 86'400.0) {
        std::snprintf(text, sizeof text, "%.2f game days a real second", game_per_real / 86'400.0);
    } else if (game_per_real >= 3'600.0) {
        std::snprintf(text, sizeof text, "%.1f game hours a real second", game_per_real / 3'600.0);
    } else {
        std::snprintf(text, sizeof text, "%.1f game seconds a real second", game_per_real);
    }
    return text;
}

int bench_command(const std::vector<std::string_view>& args) {
    const bool decoding = !args.empty() && args[0] == "decode";
    if (decoding && args.size() < 2) {
        return usage();
    }
    const std::size_t data_at = decoding ? 2 : 0;
    const std::string data = args.size() > data_at ? std::string(args[data_at]) : "data";
    kd::data::Catalogue cat;
    if (!cat.load(kd::data::read_folder(data)).empty()) {
        std::fprintf(stderr, "kindling: the catalogue under %s does not load\n", data.c_str());
        return 1;
    }
    // each scenario's world run once, those that differ only in how the phone runs them, such as pinned, shared
    std::map<std::string, std::uint64_t> cloud;
    std::map<std::tuple<int, kd::time::Seconds, kd::time::Seconds, std::int64_t>, std::uint64_t> worlds;
    for (const kd::bench::Scenario& s : kd::bench::scenarios()) {
        const auto key = std::tuple(static_cast<int>(s.ground), s.mark, s.call_at, s.call_camp);
        if (!worlds.contains(key)) {
            worlds[key] = kd::bench::headless_digest(s, cat);
        }
        cloud[std::string(s.name)] = worlds[key];
    }
    if (!decoding) {
        for (const kd::bench::Scenario& s : kd::bench::scenarios()) {
            std::printf("%.*s %lld %s\n", static_cast<int>(s.name.size()), s.name.data(),
                        static_cast<long long>(s.mark), kd::num::to_hex(cloud[std::string(s.name)]).c_str());
        }
        return 0;
    }
    const kd::bench::Read read = kd::bench::decode(args[1]);
    if (!read.why.empty()) {
        std::printf("The code cannot be read: %s\n", read.why.c_str());
        return 1;
    }
    const auto value = [&](const std::string& name) -> std::optional<double> {
        const auto it = read.values.find(name);
        return it == read.values.end() ? std::nullopt : std::optional<double>(it->second);
    };
    const auto whole = [&](const std::string& name) { return static_cast<long long>(value(name).value_or(-1)); };
    // the phone's own lines, each one only if the phone could tell it
    std::vector<std::string> phone;
    const auto known = [&](const std::string& name, const std::string& before, const std::string& after) {
        if (value(name)) {
            phone.push_back(before + grouped(whole(name)) + after);
        }
    };
    if (value("build")) {
        phone.push_back("build " + std::to_string(whole("build")));
    }
    known("cores", "", " cores");
    known("big_mhz", "the fastest at ", " MHz");
    known("refresh_hz", "a ", " Hz screen");
    known("android", "Android ", "");
    known("battery", "battery ", "% at the start");
    if (whole("plugged") > 0) {
        phone.emplace_back(whole("plugged") == 2 ? "plugged in, which the run asks not to be" : "on battery");
    }
    if (whole("thermal") > 0) {
        phone.emplace_back(whole("thermal") == 1 ? "its heat forecast working" : "no heat forecast");
    }
    if (value("seconds")) {
        phone.push_back("the run took " + std::to_string(whole("seconds") / 60) + " min " +
                        std::to_string(whole("seconds") % 60) + " s");
    }
    std::string joined;
    for (const std::string& part : phone) {
        joined += (joined.empty() ? "" : ", ") + part;
    }
    std::printf("The phone: %s\n", joined.c_str());
    int lines = 0;
    int met = 0;
    const auto line = [&](bool held) {
        ++lines;
        met += held ? 1 : 0;
        return held ? "met" : "MISSED";
    };
    for (const kd::bench::Scenario& s : kd::bench::scenarios()) {
        const std::string n(s.name);
        std::printf("\n%s: %.*s\n", n.c_str(), static_cast<int>(s.about.size()), s.about.data());
        std::string out;
        if (const auto on_time = value(n + ".on_time")) {
            char text[96];
            std::snprintf(text, sizeof text, "  frames on time %.1f%%", *on_time * 100.0);
            out += text;
            if (s.on_time > 0) {
                out += std::string(" (at least ") + std::to_string(s.on_time / 10) +
                       "%: " + line(*on_time * 1000.0 >= static_cast<double>(s.on_time)) + ")";
            }
            out += "\n";
        }
        if (const auto slowest = value(n + ".slowest")) {
            out += "  slowest frame " + std::to_string(static_cast<long long>(*slowest)) + " ms";
            if (s.slowest > 0) {
                out += std::string(" (at most ") + std::to_string(s.slowest) + ": " +
                       line(*slowest <= static_cast<double>(s.slowest)) + ")";
            }
            out += ", " + std::to_string(whole(n + ".stalls")) + " frame periods skipped\n";
        }
        if (const auto draw = value(n + ".draw_ms")) {
            char text[96];
            std::snprintf(text, sizeof text, "  the crowd drawn in %.2f ms of the main thread a frame\n", *draw);
            out += text;
        }
        if (const auto speed = value(n + ".speed")) {
            out += "  held " + speed_words(*speed) + "\n";
        }
        if (const auto heat = value(n + ".heat")) {
            char text[128];
            std::snprintf(text, sizeof text,
                          "  heat forecast at most %.2f of the first throttling level; the work "
                          "share at least %lld%%\n",
                          *heat, whole(n + ".share"));
            out += text;
        }
        if (const auto clock = value(n + ".clock")) {
            out += "  the fastest core at " + grouped(static_cast<long long>(*clock)) + " MHz on average\n";
        }
        if (value(n + ".current")) {
            out += "  battery " + std::to_string(whole(n + ".current")) + " mA, about " +
                   std::to_string(whole(n + ".current") * 385 / 100) + " mW at 3.85 V; memory " +
                   std::to_string(whole(n + ".memory")) + " MB; the world's thread " +
                   std::to_string(whole(n + ".cpu")) + "% of a core\n";
        }
        if (s.saves) {
            out += "  slowest save's pause " + std::to_string(whole(n + ".save_ms")) + " ms, export " +
                   std::to_string(whole(n + ".export_ms")) + " ms, reopened in " +
                   std::to_string(whole(n + ".open_ms")) + " ms";
            if (const auto open = value(n + ".open_ms")) {
                out += std::string(" (at most ") + std::to_string(s.open) + ": " +
                       line(*open <= static_cast<double>(s.open)) + ")";
            }
            out += "\n";
        }
        // the digest: the phone compared it with the cloud's as built; the cloud compares its top bits again here
        const long long flag = whole(n + ".digest");
        const auto bits = static_cast<std::uint64_t>(value(n + ".digest_bits").value_or(0));
        const bool same_bits = bits == (cloud[n] >> 44U);
        if (flag == 0) {
            out += "  digest: not taken\n";
            line(false);
        } else {
            out += std::string("  digest at game second ") + std::to_string(static_cast<long long>(s.mark)) +
                   ": the phone found it " + (flag == 1 ? "the cloud's" : "different") + ", and its top bits are " +
                   (same_bits ? "the cloud's" : "not the cloud's") + ": " + line(flag == 1 && same_bits) + "\n";
        }
        std::printf("%s", out.c_str());
    }
    std::printf("\n%d of %d pass lines met\n", met, lines);
    return met == lines ? 0 : 1;
}

int export_world(const std::vector<std::string_view>& args) {
    if (args.size() != 2) {
        return usage();
    }
    kd::save::DiskFiles disk{std::string(args[0])};
    kd::save::ArchiveWriter writer(disk);
    std::FILE* out = std::fopen(std::string(args[1]).c_str(), "wb");
    if (out == nullptr) {
        std::fprintf(stderr, "kindling: cannot write %s\n", std::string(args[1]).c_str());
        return 1;
    }
    for (;;) {
        const kd::save::Bytes piece = writer.next(std::size_t{1} << 20U);
        if (piece.empty()) {
            break;
        }
        std::fwrite(piece.data(), 1, piece.size(), out);
    }
    return std::fclose(out) == 0 ? 0 : 1;
}

int import_world(const std::vector<std::string_view>& args) {
    if (args.size() != 2) {
        return usage();
    }
    const std::string folder(args[1]);
    std::error_code error;
    if (std::filesystem::exists(folder) && !std::filesystem::is_empty(folder, error)) {
        std::fprintf(stderr, "kindling: %s already holds something\n", folder.c_str());
        return 1;
    }
    std::FILE* in = std::fopen(std::string(args[0]).c_str(), "rb");
    if (in == nullptr) {
        std::fprintf(stderr, "kindling: cannot read %s\n", std::string(args[0]).c_str());
        return 1;
    }
    std::filesystem::create_directories(folder);
    kd::save::DiskFiles disk(folder);
    kd::save::ArchiveReader reader(disk);
    kd::save::Bytes piece(std::size_t{1} << 20U);
    for (;;) {
        const std::size_t n = std::fread(piece.data(), 1, piece.size(), in);
        if (n == 0 || !reader.feed(std::span(piece).first(n))) {
            break;
        }
    }
    std::fclose(in);
    if (!reader.finish()) {
        std::printf("refused: %s\n", reader.why().c_str());
        std::filesystem::remove_all(folder, error);
        return 1;
    }
    std::printf("imported into %s\n", folder.c_str());
    return 0;
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
    if (command == "scene") {
        return scene_command(args);
    }
    if (command == "bench") {
        return bench_command(args);
    }
    if (command == "export") {
        return export_world(args);
    }
    if (command == "import") {
        return import_world(args);
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
