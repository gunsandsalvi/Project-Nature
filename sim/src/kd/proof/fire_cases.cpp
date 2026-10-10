#include "kd/proof/fire_cases.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/save/snapshot.hpp"
namespace kd::proof {
void FireRun::observe_result(const world::World& w, const world::Result& event) {
    if ((event.kind != 0 && event.kind != 1) || !event.result.value) return;
    const auto& recipe = w.catalogue().kind<data::Blueprint>()[event.recipe];
    if ((recipe.action == 6 || recipe.action == 11) &&
        w.things().raw().all_of<world::Fire>(w.things().handle(event.result))) {
        ++friction_results;
        const auto& fire = w.things().raw().get<world::Fire>(w.things().handle(event.result));
        if (fire.origin == event.result && !fire.source.value) embers.try_emplace(event.result, event.at);
        if (ember_at < 0) ember_at = event.at;
    }
    if (recipe.action == 12 && recipe.heat >= 2) {
        ++cooked;
        if (cooked_at < 0) cooked_at = event.at;
        pending_cooking.push_back(event);
    }
}
void FireRun::observe_record(const world::World& w, const world::Record& record) {
    if (record.what == 218) {
        const ecs::Id id{record.a};
        const auto h = w.things().find(id);
        if (!h || !w.things().raw().all_of<world::Fire>(*h)) return;
        ++tended;
        tending.emplace(id, record.key.second);
        if (tend_at < 0) tend_at = record.key.second;
    }
    if (record.what == 219 && record.b >= 2) {
        flames.emplace(ecs::Id{record.a}, record.key.second);
        if (flame_at < 0) flame_at = record.key.second;
    }
}
void FireRun::finish_interval(const world::World& w) {
    for (const auto& event : pending_cooking)
        for (const auto& credit : event.heat_sources) {
            const auto ember = embers.find(credit.origin);
            if (ember == embers.end() || credit.seconds <= 0 || credit.tended_at < ember->second ||
                credit.from < credit.tended_at || event.at < credit.from + credit.seconds)
                continue;
            auto id = credit.fire;
            std::set<ecs::Id> seen;
            bool linked = false;
            time::Seconds actual_flame = -1;
            while (id.value && seen.insert(id).second) {
                const auto h = w.things().find(id);
                const auto* fire = h ? w.things().raw().try_get<world::Fire>(*h) : nullptr;
                if (!fire || fire->origin != credit.origin) break;
                const auto lit = flames.lower_bound({id, credit.tended_at});
                if (lit != flames.end() && lit->first == id && lit->second <= credit.from)
                    actual_flame = actual_flame < 0 ? lit->second : std::min(actual_flame, lit->second);
                if (tending.contains({id, credit.tended_at})) {
                    tended_fire = id;
                    linked = true;
                }
                id = fire->source;
            }
            if (!linked || actual_flame < 0 || completed_at >= 0) continue;
            completed_at = event.at;
            completed_origin = credit.origin;
            cooked_item = event.result;
            ember_at = ember->second;
            tend_at = credit.tended_at;
            flame_at = actual_flame;
            cooked_at = event.at;
            complete = true;
        }
    pending_cooking.clear();
}

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
    std::uint64_t cursor = 0;
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
        auto unseen = std::upper_bound(history.events.begin(), history.events.end(), cursor,
                                       [](auto id, const auto& event) { return id < event.id; });
        for (; unseen != history.events.end(); ++unseen) {
            cursor = unseen->id;
            out.observe_result(w, *unseen);
        }
        for (const auto& record : trace) out.observe_record(w, record);
        out.finish_interval(w);
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
