#include "kd/proof/learning_cases.hpp"
#include <algorithm>
#include "kd/data/craft.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/learning.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/snapshot.hpp"
namespace kd::proof {
void learning_reserves(demo::CrowdWorld& camp, bool non_flaking) {
    auto& w = camp.world();
    auto& raw = w.beings().raw();
    const auto home = camp.camp_ids().front();
    const auto ch = w.beings().handle(home);
    auto& environment = raw.get<world::Habitat>(ch);
    environment.crop_budget_mg = 40000000000LL;
    environment.root_water_ml = 40000000;
    environment.upstream_ml = 120000000;
    const auto site = raw.get<world::Camp>(ch).stone_at;
    for (int n = 0; n < 200; ++n) {
        const auto kind = w.catalogue().find("item", n < 75 ? "base:flint" : n < 150 ? "base:chert" : "base:granite");
        KD_CHECK(kind, "Learning scene uses recorded physical stone kinds");
        const auto h = w.make_thing();
        w.things().raw().emplace<world::Place>(h, site);
        auto& item = w.things().raw().emplace<world::Item>(h);
        item.home = home;
        item.kind = *kind;
        item.material = *kind;
        item.mass = 50000000;
        item.length = 300;
    }
    if (non_flaking)
        w.things().each([&](ecs::Id, world::Things::Handle h) {
            auto& item = w.things().raw().get<world::Item>(h);
            if (w.catalogue().kind<data::ItemKind>()[item.material].material_class != "stone") return;
            item.changed_mask |= 1U << 3U;
            item.changed[3] = 0;
        });
}
LearningRun sharp_stone(const data::Catalogue& catalogue, std::uint64_t seed, bool non_flaking, std::int64_t workers,
                        const LearningProgress& progress) {
    demo::CrowdWorld camp(seed, catalogue, 1, true, true);
    learning_reserves(camp, non_flaking);
    auto& w = camp.world();
    run::Workers pool(static_cast<int>(workers));
    const auto home = camp.camp_ids().front();
    const auto h = w.beings().handle(home);
    const auto& recipes = catalogue.kind<data::Blueprint>();
    LearningRun out;
    out.seed = seed;
    std::size_t cursor = 0;
    // Only the current snapshot is kept: proving a phase never retains the display's historical trail.
    const auto reopen = [&] {
        ++out.reopens;
        std::string why;
        const auto decoded = save::read_snapshot(save::write_snapshot(w.save()), why);
        const auto copy = decoded ? demo::CrowdWorld::open(catalogue, *decoded, why) : nullptr;
        if (!copy || copy->world().digests().whole != w.digests().whole) {
            ++out.reopen_failures;
            std::fprintf(stderr, "learning seed %llu frontier %lld reopen: %s\n", static_cast<unsigned long long>(seed),
                         static_cast<long long>(w.frontier()), copy ? "snapshot digest mismatch" : why.c_str());
        }
    };
    reopen();
    auto end = (non_flaking ? 4 : 3) * time::kYear;
    if (progress) progress(w, end);
    while (w.frontier() < end) {
        const auto next = std::min(end, w.frontier() + time::kDay);
        if (workers == 1)
            w.run_to(next);
        else
            w.run_islands(next, pool, 1);
        if (progress && (w.frontier() == time::kDay || w.frontier() % (5 * time::kDay) == 0)) progress(w, end);
        const auto& history = w.beings().raw().get<world::CraftHistory>(h);
        bool phase = false;
        for (; cursor < history.events.size(); ++cursor) {
            const auto& event = history.events[cursor];
            if (event.kind == 0 || event.kind == 1 || event.kind == 5) ++out.fitting_tries;
            if (recipes[event.recipe].edge_from < 0 || event.result.value == 0 || event.kind == 5) continue;
            if (event.kind == 2 && event.route == 4 && out.first_watch < 0) {
                out.first_watch = event.at;
                phase = true;
            }
            if (event.kind == 2 && event.route == 5 && out.first_taught < 0) {
                out.first_taught = event.at;
                phase = true;
            }
            if (event.kind == 0 || event.kind == 1) ++out.flakes;
            if (out.first < 0 && event.kind == 1 && event.noticed) {
                out.first = event.at;
                phase = true;
                out.route = event.route;
                if (!non_flaking) end = event.at + time::kYear;
            }
        }
        if (phase) reopen();
    }
    reopen();
    out.ended = w.frontier();
    w.beings().each([&](ecs::Id, world::Beings::Handle person) {
        const auto* body = w.beings().raw().try_get<world::Person>(person);
        if (!body || body->age_years < 18) return;
        ++out.adults;
        const auto& mind = w.beings().raw().get<world::Knowledge>(person);
        out.work_started += mind.next_work - 1;
        if (std::any_of(mind.skills.begin(), mind.skills.end(),
                        [&](const auto& skill) { return skill.known && recipes[skill.recipe].edge_from >= 0; }))
            ++out.holders;
    });
    const auto& env = w.beings().raw().get<world::Habitat>(h);
    out.crop_left = env.crop_budget_mg;
    out.root_water_left = env.root_water_ml;
    out.upstream_left = env.upstream_ml;
    out.stone_left = demo::Crafting::total(w, home, "stone");
    out.digest = num::to_hex(w.digests().whole);
    return out;
}
}  // namespace kd::proof
