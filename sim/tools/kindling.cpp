// The kindling tool: the simulation without Godot, for the cloud (A2.2, A17).
//
//     kindling proof [--threads N] [suite...]    each suite's digest and time, one line each: "<suite> <digest> <ms>"
//     kindling suites                             the proof suites and what each computes
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#include "kd/proof/proof.hpp"
#include "kd/run/workers.hpp"

namespace {

int usage() {
    std::fprintf(stderr,
                 "usage: kindling proof [--threads N] [suite...]\n"
                 "       kindling suites\n");
    return 2;
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
    if (command == "suites") {
        for (const auto& s : kd::proof::suites()) {
            std::printf("%.*s: %.*s\n", static_cast<int>(s.name.size()), s.name.data(),
                        static_cast<int>(s.about.size()), s.about.data());
        }
        return 0;
    }
    return usage();
}
