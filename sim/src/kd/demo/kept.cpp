#include "kd/demo/kept.hpp"

#include <algorithm>
#include <cstdio>

#include "kd/data/toml.hpp"
#include "kd/save/snapshot.hpp"

namespace kd::demo {

namespace {

// A text as a TOML basic string, its quotes, backslashes and control characters escaped.
std::string quoted(const std::string& text) {
    std::string out = "\"";
    for (const char c : text) {
        const auto u = static_cast<unsigned char>(c);
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (u < 0x20 || u == 0x7f) {
            char escaped[8];
            std::snprintf(escaped, sizeof escaped, "\\u%04x", u);
            out += escaped;
        } else {
            out += c;
        }
    }
    return out + "\"";
}

}  // namespace

std::string about_text(const About& a) {
    std::string out =
        save::metadata_format() +
        "# A world of the demonstration's crowd (MAT-16): its name, and what makes it again if its snapshots are "
        "lost.\nname = " +
        quoted(a.name) + "\nkind = " + quoted(a.camp_alpha ? "camp_alpha" : "crowd") +
        "\nseed = " + std::to_string(a.seed) + "\ncamps = " + std::to_string(a.camps) + "\n";
    if (a.discovery) out += "discovery = true\n";
    if (a.fire_already_out) out += "fire_already_out = true\n";
    if (a.test) {
        out += "# A test's world (PLT-05), with the test switches it runs with (RES-10).\ntest = true\nswitches = [";
        for (std::size_t i = 0; i < a.switches.size(); ++i) {
            out += (i == 0 ? "" : ", ") + quoted(a.switches[i]);
        }
        out += "]\n";
    }
    return out;
}

std::optional<About> read_about(const std::string& text) {
    const data::Parsed p = data::parse_toml(text, "world.toml");
    const data::Value* kind = p.root.find("kind");
    const data::Value* seed = p.root.find("seed");
    const data::Value* camps = p.root.find("camps");
    const data::Value* name = p.root.find("name");
    const data::Value* test = p.root.find("test");
    const data::Value* discovery = p.root.find("discovery");
    const data::Value* switches = p.root.find("switches");
    const data::Value* cold_fire = p.root.find("fire_already_out");
    if (!p.problems.empty() ||
        (kind != nullptr &&
         (kind->kind != data::Value::Kind::text || (kind->text != "crowd" && kind->text != "camp_alpha"))) ||
        seed == nullptr || seed->kind != data::Value::Kind::whole || seed->whole < 0 || camps == nullptr ||
        camps->kind != data::Value::Kind::whole || camps->whole < 0 ||
        (name != nullptr && name->kind != data::Value::Kind::text) ||
        (test != nullptr && test->kind != data::Value::Kind::truth) ||
        (discovery != nullptr && discovery->kind != data::Value::Kind::truth) ||
        (cold_fire != nullptr && cold_fire->kind != data::Value::Kind::truth) ||
        (switches != nullptr && switches->kind != data::Value::Kind::array)) {
        return std::nullopt;
    }
    About a;
    a.camp_alpha = kind != nullptr && kind->text == "camp_alpha";
    a.name = name != nullptr ? name->text : "";
    a.seed = static_cast<std::uint64_t>(seed->whole);
    a.camps = camps->whole;
    a.test = test != nullptr && test->truth;
    a.discovery = discovery != nullptr && discovery->truth;
    a.fire_already_out = cold_fire && cold_fire->truth;
    if (a.fire_already_out && !a.discovery) return std::nullopt;
    if (a.discovery && !a.camp_alpha) return std::nullopt;
    if (switches != nullptr) {
        for (const data::Value& v : switches->items) {
            if (v.kind != data::Value::Kind::text) {
                return std::nullopt;
            }
            a.switches.push_back(v.text);
        }
    }
    return a;
}

Kept keep_crowd(save::Keeper& keeper, const data::Catalogue& catalogue, std::uint64_t seed, std::int64_t camps,
                bool camp_alpha) {
    Kept out;
    save::Found found = keeper.open();
    out.damaged = found.damaged;
    if (!found.problem.empty()) {
        out.problem = found.problem;
        return out;
    }
    out.update = keeper.begin(found, catalogue);
    if (out.update == save::Update::older) {
        out.problem = save::kOlderSave;
        return out;
    }
    if (out.update == save::Update::big) {
        out.problem = "this version makes worlds differently, so it cannot carry on; its history is kept";
        return out;
    }
    if (found.snapshot) {
        std::string why;
        out.crowd = CrowdWorld::open(catalogue, *found.snapshot, why);
        if (!out.crowd) {
            out.problem = "its snapshot " + found.snapshot_name + " cannot be opened: " + why;
            return out;
        }
        out.snapshot = found.snapshot_name;
    } else {
        // no whole snapshot: made new, from the seed and size the folder's world.toml keeps, if it is there
        const std::optional<About> about = found.about ? read_about(*found.about) : std::nullopt;
        if (about) {
            seed = about->seed;
            camps = about->camps;
            camp_alpha = about->camp_alpha;
        }
        out.crowd =
            std::make_unique<CrowdWorld>(seed, catalogue, camps > 0 ? std::optional<std::int64_t>(camps) : std::nullopt,
                                         camp_alpha, about && about->discovery, about && about->fire_already_out);
        out.made = true;
        // a test's world takes its switches before it runs, where the build has them (RES-10)
        if (about && !about->switches.empty()) {
            std::vector<world::Switch> switches;
            for (const std::string& name : about->switches) {
                const std::optional<world::Switch> s = world::switch_named(name);
                if (!s || !world::kSwitches) {
                    out.problem = "its test switch " + name + " is not in this build";
                    return out;
                }
                switches.push_back(*s);
            }
            out.crowd->world().set_switches(std::move(switches));
        }
        if (!about) {
            About made;
            made.seed = seed;
            made.camps = camps;
            made.camp_alpha = camp_alpha;
            keeper.about(about_text(made));
        }
    }
    world::World& w = out.crowd->world();
    keeper.keep_kinds([&w](const world::Record& r) { return w.keeps(r); });
    keeper.expect(w.frontier());

    // your later commands act again, each at its own second
    out.journaled = found.commands.size();
    for (const world::Command& c : found.commands) {
        if (c.number <= w.commands_made()) {
            continue;
        }
        if (c.number != w.commands_made() + 1 || c.at < w.frontier()) {
            out.problem = "its journal's commands do not follow on from its snapshot";
            return out;
        }
        w.replay(c);
        ++out.replayed;
    }

    out.was_at = w.frontier();
    if (found.paused_at) {
        out.was_at = std::max(out.was_at, *found.paused_at);
    }
    if (!found.commands.empty()) {
        out.was_at = std::max(out.was_at, found.commands.back().at);
    }
    if (!found.history.empty()) {
        out.was_at = std::max(out.was_at, found.history.back().key.second + 1);
    }
    if (out.made) {
        // a new world's first snapshot is written at once, so the folder always holds one
        keeper.snapshot(w);
        keeper.flush();
    }
    return out;
}

}  // namespace kd::demo
