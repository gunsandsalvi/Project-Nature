#include "kd/proof/fire_cases.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/save/snapshot.hpp"
namespace kd::proof {
FireRun fire_chain(const data::Catalogue& catalogue, std::uint64_t seed, bool wet_control, time::Seconds duration,
                   const LearningProgress& progress) {
    demo::CrowdWorld camp(seed, catalogue, 1, true, true, true);
    learning_reserves(camp, false);
    auto& w = camp.world();
    const auto home = camp.camp_ids().front();
    if (wet_control)
        w.things().each([&](ecs::Id, auto h) {
            auto& item = w.things().raw().get<world::Item>(h);
            const auto fit = demo::Crafting::physical(catalogue, item);
            if (w.things().raw().all_of<world::Fire>(h) || fit.values[6] < 2) return;
            item.changed_mask |= 1U << 9U;
            item.changed[9] = 3;
        });
    FireRun out;
    out.seed = seed;
    out.wet_control = wet_control;
    out.ordinary_setup = w.commands_made() == 0;
    w.beings().each([&](ecs::Id, auto h) {
        const auto* work = w.beings().raw().try_get<world::Work>(h);
        const auto* mind = w.beings().raw().try_get<world::Knowledge>(h);
        if ((work && work->state) || (mind && (!mind->memories.empty() || mind->next_work != 1)))
            out.ordinary_setup = false;
    });
    std::vector<world::Record> trace;
    w.keep_history(&trace);
    std::size_t cursor = 0;
    const auto reopen = [&] {
        std::string why;
        const auto encoded = save::write_snapshot(w.save());
        out.snapshot_bytes = encoded.size();
        const auto snapshot = save::read_snapshot(encoded, why);
        const auto copy = snapshot ? demo::CrowdWorld::open(catalogue, *snapshot, why) : nullptr;
        if (!copy || copy->world().digests().whole != w.digests().whole) {
            ++out.reopen_failures;
            const auto& kept = w.beings().raw().get<world::CraftHistory>(w.beings().handle(home));
            std::fprintf(stderr, "history events=%zu choices=%zu\n", kept.events.size(), kept.choices.size());
            std::fprintf(stderr, "fire seed %llu frontier %lld reopen: %s\n", static_cast<unsigned long long>(seed),
                         static_cast<long long>(w.frontier()), copy ? "digest mismatch" : why.c_str());
        }
    };
    reopen();
    if (progress) progress(w, duration);
    while (w.frontier() < duration && !out.complete) {
        w.run_to(std::min(duration, w.frontier() + time::kHour));
        out.peak_trace_records = std::max(out.peak_trace_records, trace.size());
        const auto& history = w.beings().raw().get<world::CraftHistory>(w.beings().handle(home));
        for (const auto& record : trace) {
            if (record.what == 218) {
                ++out.tended;
                if (out.tend_at < 0) out.tend_at = record.key.second;
            }
            if (record.what == 219 && record.b >= 2 && out.flame_at < 0) out.flame_at = record.key.second;
        }
        for (; cursor < history.events.size(); ++cursor) {
            const auto& event = history.events[cursor];
            if ((event.kind != 0 && event.kind != 1) || !event.result.value) continue;
            const auto& recipe = catalogue.kind<data::Blueprint>()[event.recipe];
            if (recipe.action == 11 && w.things().raw().all_of<world::Fire>(w.things().handle(event.result))) {
                ++out.friction_results;
                if (out.ember_at < 0) out.ember_at = event.at;
            }
            if (recipe.action == 12 && recipe.heat >= 2) {
                ++out.cooked;
                if (out.cooked_at < 0) out.cooked_at = event.at;
                if (out.ember_at >= 0 && out.tend_at >= out.ember_at && out.flame_at >= out.ember_at &&
                    event.at >= out.flame_at && event.at >= out.tend_at && out.completed_at < 0)
                    out.completed_at = event.at;
            }
        }
        // Keep at most one hour of trace, never the world's accumulated routine history.
        trace.clear();
        out.complete = out.completed_at >= 0;
        if (progress && w.frontier() % (5 * time::kDay) == 0) progress(w, duration);
    }
    w.keep_history(nullptr);
    out.ended = w.frontier();
    const auto& kept = w.beings().raw().get<world::CraftHistory>(w.beings().handle(home));
    out.choice_count = kept.choices.size();
    for (const auto& choice : kept.choices) {
        ByteWriter encoded;
        ecs::write_component(choice, encoded);
        out.choice_wire_bytes += encoded.take().size();
        const auto& reason = choice.reasons.front();
        if (reason.kind != 1 || (reason.action != 6 && reason.action != 11)) continue;
        ++out.friction_choices;
        bool rod = false, sheet = false;
        for (const auto& input : reason.inputs) {
            const auto& source = w.things().raw().get<world::Item>(w.things().handle(input.id));
            const auto fit = demo::Crafting::physical(catalogue, source);
            rod = rod || (fit.material_class == "wood" && fit.form == "rod");
            sheet = sheet || (fit.material_class == "wood" && fit.form == "sheet");
        }
        if (rod && sheet) ++out.wood_pairs;
    }
    reopen();
    out.digest = num::to_hex(w.digests().whole);
    return out;
}
}  // namespace kd::proof
