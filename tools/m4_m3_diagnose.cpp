// Reproduces already judged M3 seeds to diagnose them. This is never an acceptance gate.
#include <cstdio>
#include <map>
#include <string>
#include "kd/data/folder.hpp"
#include "kd/demo/living.hpp"
#include "kd/proof/fire_cases.hpp"
#include "kd/save/snapshot.hpp"
#include "m4_m3_counters.hpp"
int main(int argc, char** argv) {
    if (argc != 5) return 2;
    const std::string kind = argv[1], expected = argv[3];
    const auto seed = std::strtoull(argv[2], nullptr, 10);
    kd::data::Catalogue catalogue;
    if (!catalogue.load(kd::data::read_catalogue(argv[4])).empty()) return 2;
    std::vector<kd::world::Record> trace;
    std::map<std::uint32_t, std::uint64_t> records;
    std::size_t trace_peak = 0, items = 0, spent = 0, choices = 0, snapshot = 0;
    std::uint64_t choice_bytes = 0;
    const auto drain = [&] {
        trace_peak = std::max(trace_peak, trace.size());
        for (const auto& event : trace) ++records[event.what];
        trace.clear();
    };
    const auto progress = [&](const kd::world::World& w, kd::time::Seconds end) {
        drain();
        const_cast<kd::world::World&>(w).keep_history(&trace);
        std::fprintf(stderr, "diagnosis %s %llu day %.2f / %.2f\n", kind.c_str(), seed,
                     static_cast<double>(w.frontier()) / kd::time::kDay, static_cast<double>(end) / kd::time::kDay);
        if (w.frontier() != end || snapshot) return;
        items = w.things().size();
        w.things().each([&](kd::ecs::Id, auto h) {
            if (!w.things().raw().get<kd::world::Item>(h).mass) ++spent;
        });
        const auto home = w.beings().raw().view<kd::world::Camp>().front();
        const auto& history = w.beings().raw().get<kd::world::CraftHistory>(home);
        choices = history.choices.size();
        for (const auto& choice : history.choices) {
            kd::ByteWriter bytes;
            kd::ecs::write_component(choice, bytes);
            choice_bytes += bytes.take().size();
        }
        snapshot = kd::save::write_snapshot(w.save()).size();
    };
    const auto start = std::chrono::steady_clock::now();
    std::string digest;
    std::int64_t ended = 0;
    if (kind == "spread") {
        const auto run = kd::proof::sharp_stone(catalogue, seed, false, 1, progress);
        digest = run.digest;
        ended = run.ended;
    } else if (kind == "fire") {
        const auto run = kd::proof::fire_chain(catalogue, seed, false, 3 * kd::time::kYear, progress);
        digest = run.digest;
        ended = run.ended;
    } else
        return 2;
    drain();
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const double years = static_cast<double>(ended) / kd::time::kYear;
    std::printf(
        "{\"diagnostic\":true,\"kind\":\"%s\",\"seed\":%llu,\"digest\":\"%s\","
        "\"original_digest\":\"%s\",\"unchanged\":%s,\"ended\":%lld,\"seconds\":%.6f,"
        "\"years_per_minute\":%.6f,\"items\":%zu,\"spent\":%zu,\"choices\":%zu,"
        "\"choice_bytes\":%llu,\"snapshot_bytes\":%zu,\"peak_trace\":%zu,\"counts\":{",
        kind.c_str(), seed, digest.c_str(), expected.c_str(), digest == expected ? "true" : "false",
        static_cast<long long>(ended), seconds, years * 60 / seconds, items, spent, choices,
        static_cast<unsigned long long>(choice_bytes), snapshot, trace_peak);
    for (std::size_t n = 0; n < kd::diagnostic::count; ++n)
        std::printf("%s\"%s\":%llu", n ? "," : "", kd::diagnostic::names[n],
                    static_cast<unsigned long long>(kd::diagnostic::counts[n]));
    std::printf("},\"records\":{");
    bool first = true;
    for (const auto& [what, number] : records) {
        std::printf("%s\"%u\":%llu", first ? "" : ",", what, static_cast<unsigned long long>(number));
        first = false;
    }
    std::printf("},\"cpu_seconds_per_year\":{");
    for (std::size_t n = 0; n < kd::diagnostic::cpu_count; ++n)
        std::printf("%s\"%s\":%.6f", n ? "," : "", kd::diagnostic::cpu_names[n],
                    static_cast<double>(kd::diagnostic::nanoseconds[n]) / 1e9 / years);
    std::printf("}}\n");
    return digest == expected ? 0 : 1;
}
