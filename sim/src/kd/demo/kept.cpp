#include "kd/demo/kept.hpp"

#include <algorithm>
#include <cstdio>

#include "kd/data/toml.hpp"

namespace kd::demo {

std::string about_text(const About& a) {
    // the name as a TOML basic string, its quotes, backslashes and control characters escaped
    std::string name;
    for (const char c : a.name) {
        const auto u = static_cast<unsigned char>(c);
        if (c == '"' || c == '\\') {
            name += '\\';
            name += c;
        } else if (u < 0x20 || u == 0x7f) {
            char escaped[8];
            std::snprintf(escaped, sizeof escaped, "\\u%04x", u);
            name += escaped;
        } else {
            name += c;
        }
    }
    return "# A world of the demonstration's crowd (MAT-16): its name, and what makes it again if its snapshots are "
           "lost.\n"
           "name = \"" +
           name + "\"\nkind = \"crowd\"\nseed = " + std::to_string(a.seed) + "\ncamps = " + std::to_string(a.camps) +
           "\n";
}

std::optional<About> read_about(const std::string& text) {
    const data::Parsed p = data::parse_toml(text, "world.toml");
    const data::Value* kind = p.root.find("kind");
    const data::Value* seed = p.root.find("seed");
    const data::Value* camps = p.root.find("camps");
    const data::Value* name = p.root.find("name");
    if (!p.problems.empty() || (kind != nullptr && (kind->kind != data::Value::Kind::text || kind->text != "crowd")) ||
        seed == nullptr || seed->kind != data::Value::Kind::whole || seed->whole < 0 || camps == nullptr ||
        camps->kind != data::Value::Kind::whole || camps->whole < 0 ||
        (name != nullptr && name->kind != data::Value::Kind::text)) {
        return std::nullopt;
    }
    About a;
    a.name = name != nullptr ? name->text : "";
    a.seed = static_cast<std::uint64_t>(seed->whole);
    a.camps = camps->whole;
    return a;
}

Kept keep_crowd(save::Keeper& keeper, const data::Catalogue& catalogue, std::uint64_t seed, std::int64_t camps,
                std::span<const world::Migration> migrations) {
    Kept out;
    save::Found found = keeper.open();
    out.damaged = found.damaged;
    out.update = keeper.begin(found, catalogue);
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
        // the migrations it has not had, made once now, before your later commands act again
        world::World& opened = out.crowd->world();
        std::vector<std::string>& had = keeper.versions().migrations;
        out.migrated = world::migrate(opened, had, migrations);
    } else {
        // no whole snapshot: made new, from the seed and size the folder's world.toml keeps, if it is there
        const std::optional<About> about = found.about ? read_about(*found.about) : std::nullopt;
        if (about) {
            seed = about->seed;
            camps = about->camps;
        }
        out.crowd = std::make_unique<CrowdWorld>(seed, catalogue,
                                                 camps > 0 ? std::optional<std::int64_t>(camps) : std::nullopt);
        out.made = true;
        if (!about) {
            keeper.about(about_text({"", seed, camps}));
        }
        // made as this version makes worlds, it needs none of the migrations
        for (const world::Migration& m : migrations) {
            keeper.versions().migrations.emplace_back(m.name);
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
