// A scene's report (A17, RES-06): what it checked, its rule and the verdict, each measure's range over its runs ("in
// 18 of 20 worlds"), each run with its seed, switches, measures, oddities and digest, and the real time it took
// against its budget. Written as JSON, which the app's Reports page reads as it is and the cloud keeps beside the
// scene's worlds.
#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "kd/scene/scene.hpp"

namespace kd::scene {

/// What a scene's runs came to, beyond the runs themselves.
struct Outcome {
    Verdict verdict;
    /// The real seconds the scene took, and whether it stopped short of its runs, its budget spent.
    std::int64_t seconds = 0;
    bool over_budget = false;
    /// The version its worlds were saved under, the app's for worlds the phone opens.
    std::string build;
};

/// The rule in words: "greetings_per_camp_day at least 10, in at least 16 of 20 runs".
[[nodiscard]] std::string rule_words(const Scene& s);

/// Implements RES-06, see A17: the scene's report as JSON.
[[nodiscard]] std::string report_json(const Scene& s, const WorldKind& kind, std::span<const RunResult> runs,
                                      const Outcome& outcome);

}  // namespace kd::scene
