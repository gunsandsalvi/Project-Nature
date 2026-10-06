// Scenes (A17, RES-21): a small setting built for one check, written in data/scenes/<name>.toml and stating, before its
// first run, what it checks, its world and size, its first seed and its runs, how long each runs in game time, its
// time limit and budget in real time, its test switches, and its pass rule in exact numbers (RES-09); and the ranges
// it expects and the rules never to break, so whatever falls outside them is flagged as an oddity (RES-12). A scene is
// read through the catalogue's loader, so each mistake is named at its file, line and column.
#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/toml.hpp"
#include "kd/world/world.hpp"

namespace kd::scene {

/// A measure of a run, or a rule never to break, as a kind of world offers it: its name, and what it means.
struct Named {
    std::string_view name;
    std::string_view about;
};

/// What a kind of world can be measured by and held to, for its scenes.
struct WorldKind {
    std::string_view name;
    std::span<const Named> measures;
    std::span<const Named> nevers;
};

/// The pass rule (RES-09): each run's measure at least, or at most, a number, in at least so many of the runs.
struct Rule {
    std::string measure;
    std::optional<std::int64_t> at_least;
    std::optional<std::int64_t> at_most;
    std::int64_t in = 0;

    /// Whether one run's measure meets it.
    [[nodiscard]] bool met(std::int64_t value) const {
        return (!at_least || value >= *at_least) && (!at_most || value <= *at_most);
    }
};

/// A range a measure is expected in (RES-12): a run outside it is an oddity.
struct Expect {
    std::string measure;
    std::int64_t from = 0;
    std::int64_t to = 0;
};

/// A rule never to break (RES-12), with its limit, in the rule's own units.
struct Never {
    std::string rule;
    std::int64_t limit = 0;
};

/// Implements RES-21 and RES-09, see A17: a scene as its file states it.
struct Scene {
    /// Its file's name, without .toml.
    std::string name;
    std::string about;
    /// The items it checks, by ID.
    std::vector<std::string> checks;
    /// The kind of world it runs, and its size in camps.
    std::string world;
    std::int64_t camps = 0;
    /// The first run's seed; each next run's is one more.
    std::int64_t seed = 0;
    std::int64_t runs = 0;
    /// Game seconds each run lasts, real seconds each run may take, and real seconds the whole scene may take.
    std::int64_t until = 0;
    std::int64_t limit = 0;
    std::int64_t budget = 0;
    /// The test switches each run takes (RES-10), or, with one_each, one each in turn, so each stands alone.
    std::vector<world::Switch> switches;
    bool one_each = false;
    Rule pass;
    std::vector<Expect> expects;
    std::vector<Never> nevers;

    /// The switches the run with this index takes.
    [[nodiscard]] std::vector<world::Switch> switches_of(std::int64_t run) const;
};

/// A scene read: it, and what is wrong with its file, empty when nothing is.
struct Read {
    Scene scene;
    std::vector<data::Problem> problems;
};

/// Implements RES-21 and RES-09, see A17: a scene from its file's text, its name from its path, its world one of these
/// kinds and its measures and rules ones that kind offers.
[[nodiscard]] Read read_scene(std::string_view text, const std::string& path, std::span<const WorldKind> kinds);

/// One run of a scene, as it ended.
struct RunResult {
    std::int64_t index = 0;
    std::int64_t seed = 0;
    /// Each of its kind's measures, by name, in the kind's order; none when the run gave none, as one that crashed.
    std::vector<std::pair<std::string, std::int64_t>> measures;
    /// Each oddity seen, in words (RES-12).
    std::vector<std::string> oddities;
    /// The world's whole digest at its end, and the game days it ran.
    std::uint64_t digest = 0;
    std::int64_t days = 0;
    std::vector<world::Switch> switches;

    /// A measure by name, if the run has it.
    [[nodiscard]] std::optional<std::int64_t> measure(const std::string& name) const;
};

/// The verdict on a scene's runs by its rule (RES-13).
struct Verdict {
    bool passed = false;
    /// The runs that met the rule, the runs judged, and how many had to.
    std::int64_t passes = 0;
    std::int64_t judged = 0;
    std::int64_t needed = 0;
    /// Judged on fewer runs than 20, so it counts but is named as provisional.
    bool provisional = false;
    /// Judged on twice its runs, after its rule failed on the first.
    bool reran = false;
};

/// How many of n runs must meet a rule stated for some runs: its count scaled, rounded against passing.
[[nodiscard]] std::int64_t needed(const Rule& rule, std::int64_t stated, std::int64_t n);

/// Implements RES-13, see A17: a scene's runs judged by its rule, each run's measure in order; a run that gave none, as
/// one that crashed, fails it. Judged on twice the scene's runs, it is marked as a rerun.
[[nodiscard]] Verdict judge(const Scene& s, std::span<const std::optional<std::int64_t>> measures);

/// Whether a failed verdict calls for the rerun on as many fresh seeds: when the scene ran 20 runs or more (RES-13).
[[nodiscard]] bool rerun_due(const Scene& s, const Verdict& v);

}  // namespace kd::scene
