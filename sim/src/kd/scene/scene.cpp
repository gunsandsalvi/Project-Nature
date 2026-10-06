#include "kd/scene/scene.hpp"

#include <algorithm>

#include "kd/data/loader.hpp"
#include "kd/data/units.hpp"

namespace kd::scene {

namespace {

using data::Measure;
using data::Value;

std::vector<std::string_view> names_of(std::span<const Named> named) {
    std::vector<std::string_view> out;
    for (const Named& n : named) {
        out.push_back(n.name);
    }
    return out;
}

}  // namespace

bool item_id(const std::string& id) {
    const std::size_t dash = id.find('-');
    return dash >= 2 && dash <= 4 && id.size() == dash + 3 &&
           std::all_of(id.begin(), id.begin() + static_cast<std::ptrdiff_t>(dash),
                       [](char c) { return c >= 'A' && c <= 'Z'; }) &&
           std::all_of(id.begin() + static_cast<std::ptrdiff_t>(dash) + 1, id.end(),
                       [](char c) { return c >= '0' && c <= '9'; });
}

std::string name_of(const std::string& path) {
    const std::size_t slash = path.find_last_of('/');
    std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    if (name.size() > 5 && name.compare(name.size() - 5, 5, ".toml") == 0) {
        name.resize(name.size() - 5);
    }
    return name;
}

std::vector<world::Switch> Scene::switches_of(std::int64_t run) const {
    if (!one_each || switches.empty()) {
        return switches;
    }
    return {switches[static_cast<std::size_t>(run) % switches.size()]};
}

Read read_scene(std::string_view text, const std::string& path, std::span<const WorldKind> kinds) {
    Read out;
    Scene& s = out.scene;
    s.name = name_of(path);
    data::Parsed parsed = data::parse_toml(text, path);
    out.problems = std::move(parsed.problems);
    if (!out.problems.empty()) {
        return out;
    }
    data::Loader l(parsed.root, path, out.problems);
    l.text({"about", "what the scene shows, in a sentence"}, s.about);
    l.names({"checks", "the items it checks, by ID, such as WLD-13"}, s.checks);
    std::vector<std::string_view> kind_names;
    for (const WorldKind& k : kinds) {
        kind_names.push_back(k.name);
    }
    l.choice({"world", "the kind of world it runs"}, s.world, kind_names);
    l.whole({"camps", "the world's size, in camps"}, s.camps, {1, 10'000});
    l.whole({"seed", "the first run's seed; each next run's is one more"}, s.seed, {0, INT64_MAX / 2});
    l.whole({"runs", "how many worlds it runs: about 20 where chance matters (RES-13)"}, s.runs, {1, 200});
    l.quantity({"until", "how long each run lasts, in game time"}, s.until, Measure::game_time,
               {3600, std::int64_t{1000} * 60 * 86'400});
    l.quantity({"limit", "how long one run may take, in real time, before it counts as hung"}, s.limit,
               Measure::life_time, {1, 86'400});
    l.quantity({"budget", "how long the whole scene may take, in real time (RES-09)"}, s.budget, Measure::life_time,
               {1, std::int64_t{7} * 86'400});
    std::vector<std::string> switches;
    l.names({"switches", "the test switches each run takes (RES-10)", data::Affects::rules, false}, switches);
    l.truth(
        {"one_each", "each run takes one of the switches in turn, so each stands alone", data::Affects::rules, false},
        s.one_each);
    const Value* pass = l.table({"pass", "the pass rule, in exact numbers (RES-09)"});
    const std::vector<const Value*> expects =
        l.tables({"expect", "the ranges each measure is expected in (RES-12)", data::Affects::rules, false});
    const std::vector<const Value*> nevers =
        l.tables({"never", "the rules never to break (RES-12)", data::Affects::rules, false});
    l.finish();

    for (const std::string& id : s.checks) {
        if (!item_id(id)) {
            l.refuse(*parsed.root.find("checks"), "checks: \"" + id + "\" is not an item's ID, such as WLD-13");
        }
    }
    for (const std::string& name : switches) {
        if (const std::optional<world::Switch> sw = world::switch_named(name)) {
            s.switches.push_back(*sw);
        } else {
            l.refuse(*parsed.root.find("switches"), "switches: \"" + name + "\" is not a test switch");
        }
    }
    const auto kind = std::find_if(kinds.begin(), kinds.end(), [&](const WorldKind& k) { return k.name == s.world; });
    if (kind == kinds.end()) {
        return out;
    }
    const std::vector<std::string_view> measures = names_of(kind->measures);
    const std::vector<std::string_view> rules = names_of(kind->nevers);

    if (pass != nullptr) {
        data::Loader p(*pass, path, out.problems);
        p.choice({"measure", "what each run is measured by"}, s.pass.measure, measures);
        std::int64_t least = 0;
        std::int64_t most = 0;
        p.whole({"at_least", "each run's measure at least this", data::Affects::rules, false}, least, {});
        p.whole({"at_most", "each run's measure at most this", data::Affects::rules, false}, most, {});
        p.whole({"in", "in at least this many of the runs"}, s.pass.in, {1, std::max<std::int64_t>(1, s.runs)});
        p.finish();
        if (pass->find("at_least") != nullptr) {
            s.pass.at_least = least;
        }
        if (pass->find("at_most") != nullptr) {
            s.pass.at_most = most;
        }
        if (!s.pass.at_least && !s.pass.at_most) {
            p.refuse(*pass, "pass: a rule needs at_least, at_most or both");
        }
    }
    for (const Value* e : expects) {
        Expect x;
        data::Loader ex(*e, path, out.problems);
        ex.choice({"measure", "the measure expected in the range"}, x.measure, measures);
        ex.whole({"from", "the range's least"}, x.from, {});
        ex.whole({"to", "the range's most"}, x.to, {});
        ex.finish();
        if (x.from > x.to) {
            ex.refuse(*e, "expect: its range runs from " + std::to_string(x.from) + " back to " + std::to_string(x.to));
        }
        s.expects.push_back(std::move(x));
    }
    for (const Value* n : nevers) {
        Never x;
        data::Loader nv(*n, path, out.problems);
        nv.choice({"rule", "the rule never to break"}, x.rule, rules);
        nv.whole({"limit", "the rule's limit, in its own units"}, x.limit, {0, INT64_MAX});
        nv.finish();
        s.nevers.push_back(std::move(x));
    }
    return out;
}

std::optional<std::int64_t> RunResult::measure(const std::string& name) const {
    for (const auto& [n, v] : measures) {
        if (n == name) {
            return v;
        }
    }
    return std::nullopt;
}

std::int64_t needed(const Rule& rule, std::int64_t stated, std::int64_t n) {
    // in × n / stated, rounded up: against passing
    return (rule.in * n + stated - 1) / stated;
}

Verdict judge(const Scene& s, std::span<const std::optional<std::int64_t>> measures) {
    Verdict v;
    v.judged = static_cast<std::int64_t>(measures.size());
    for (const std::optional<std::int64_t>& m : measures) {
        v.passes += m && s.pass.met(*m) ? 1 : 0;
    }
    v.needed = needed(s.pass, s.runs, v.judged);
    v.passed = v.passes >= v.needed;
    v.provisional = v.judged < 20;
    v.reran = v.judged > s.runs;
    return v;
}

bool rerun_due(const Scene& s, const Verdict& v) {
    return !v.passed && !v.reran && s.runs >= 20;
}

}  // namespace kd::scene
