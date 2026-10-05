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
#include "kd/num/digest.hpp"
#include "kd/proof/proof.hpp"
#include "kd/run/workers.hpp"

namespace {

int usage() {
    std::fprintf(stderr,
                 "usage: kindling proof [--threads N] [suite...]\n"
                 "       kindling suites\n"
                 "       kindling catalogue check|schema|fingerprint [data]\n"
                 "       kindling catalogue show <name> [data]\n");
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
    if (command == "suites") {
        for (const auto& s : kd::proof::suites()) {
            std::printf("%.*s: %.*s\n", static_cast<int>(s.name.size()), s.name.data(),
                        static_cast<int>(s.about.size()), s.about.data());
        }
        return 0;
    }
    return usage();
}
