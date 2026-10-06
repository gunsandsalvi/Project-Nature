#include "kd/demo/kept.hpp"

#include <algorithm>

namespace kd::demo {

std::string crowd_about(std::uint64_t seed, std::int64_t camps) {
    return "# A world of the demonstration's crowd (MAT-16), made again from these if its snapshots are lost.\n"
           "kind = \"crowd\"\n"
           "seed = " +
           std::to_string(seed) + "\ncamps = " + std::to_string(camps) + "\n";
}

Kept keep_crowd(save::Keeper& keeper, const data::Catalogue& catalogue, std::uint64_t seed, std::int64_t camps) {
    Kept out;
    save::Found found = keeper.open();
    out.damaged = found.damaged;
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
        if (found.about) {
            seed = static_cast<std::uint64_t>(
                save::about_number(*found.about, "seed").value_or(static_cast<std::int64_t>(seed)));
            camps = save::about_number(*found.about, "camps").value_or(camps);
        }
        out.crowd = std::make_unique<CrowdWorld>(seed, catalogue,
                                                 camps > 0 ? std::optional<std::int64_t>(camps) : std::nullopt);
        out.made = true;
        if (!found.about) {
            keeper.about(crowd_about(seed, camps));
        }
    }
    world::World& w = out.crowd->world();
    keeper.expect(w.history_count());

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
