#include "kd/proof/idea_cases.hpp"
#include "kd/data/craft.hpp"
#include "kd/demo/idea_dreams.hpp"
#include "kd/num/digest.hpp"
#include "kd/save/snapshot.hpp"
namespace kd::proof {
namespace {
std::uint32_t entry(const data::Catalogue& cat, std::string_view name) {
    const auto id = cat.find("item", name);
    KD_CHECK(id, "Idea scenes require their validated physical catalogue");
    return id.value_or(0);
}
bool reopen(IdeaScene& scene, const data::Catalogue& catalogue) {
    const auto& w = scene.camp->world();
    const auto digest = w.digests().whole;
    std::string why;
    const auto decoded = save::read_snapshot(save::write_snapshot(w.save()), why);
    if (!decoded) return false;
    auto copy = demo::CrowdWorld::open(catalogue, *decoded, why);
    if (!copy || copy->world().digests().whole != digest) {
        std::fprintf(stderr, "idea scene seed %llu frontier %lld reopen: %s\n",
                     static_cast<unsigned long long>(w.seed()), static_cast<long long>(w.frontier()),
                     copy ? "snapshot digest mismatch" : why.c_str());
        return false;
    }
    scene.camp = std::move(copy);
    return true;
}
const world::DreamAct& act(const IdeaScene& scene) {
    const auto& w = std::as_const(*scene.camp).world();
    return w.beings().raw().get<world::Dreams>(w.beings().handle(scene.home)).acts.front();
}
std::int64_t first_try(const IdeaScene& scene, std::int64_t after) {
    const auto& w = std::as_const(*scene.camp).world();
    const auto& know = w.beings().raw().get<world::Knowledge>(w.beings().handle(scene.person));
    const auto rod = entry(w.catalogue(), "base:dry_stick"), board = entry(w.catalogue(), "base:dry_board");
    std::int64_t at = -1;
    for (const auto& m : know.memories) {
        if (m.action != 11 || m.at < after) continue;
        bool r = false, b = false;
        for (const auto& input : m.inputs) {
            bool actual = false;
            for (std::size_t p = 0; p < input.sources.size(); ++p)
                if (input.sources[p] == 3 && input.learned_at[p] == m.at) actual = true;
            r = r || (actual && input.kind == rod);
            b = b || (actual && input.kind == board);
        }
        if (r && b && (at < 0 || m.at < at)) at = m.at;
    }
    return at;
}
std::int64_t successes(const IdeaScene& scene, std::int64_t after) {
    const auto& w = std::as_const(*scene.camp).world();
    const auto& history = w.beings().raw().get<world::CraftHistory>(w.beings().handle(scene.home));
    std::int64_t successes = 0;
    for (const auto& e : history.events)
        if (e.at >= after && e.actor == scene.person && (e.kind == 0 || e.kind == 1) &&
            w.catalogue().kind<data::Blueprint>()[e.recipe].action == 11 && e.result.value != 0)
            ++successes;
    return successes;
}
}  // namespace
IdeaScene remembered_wood(const data::Catalogue& catalogue, std::uint64_t seed) {
    IdeaScene scene;
    scene.camp = std::make_unique<demo::CrowdWorld>(seed, catalogue, 1, true, true);
    auto& w = scene.camp->world();
    auto& raw = w.beings().raw();
    scene.home = scene.camp->camp_ids().front();
    w.beings().each([&](ecs::Id id, auto h) {
        if (!scene.person.value && raw.all_of<world::Person>(h)) scene.person = id;
    });
    const auto ph = w.beings().handle(scene.person);
    num::Point at{};
    w.things().each([&](ecs::Id, auto h) {
        if (w.things().raw().all_of<world::Fire>(h)) at = w.things().raw().get<world::Place>(h).at;
    });
    at = w.torus().moved(at, {150, 0});
    auto& work = raw.get<world::Work>(ph);
    auto& know = raw.get<world::Knowledge>(ph);
    work.state = 2;
    work.number = know.next_work++;
    work.action = 11;
    work.start = work.active_start = 0;
    work.end = 100;
    work.next_try = work.try_seconds = 300;
    work.unit_mass = work.goal_mass = 20000;
    work.target = at;
    for (const auto name : {"base:dry_stick", "base:dry_board"}) {
        const auto kind = entry(catalogue, name);
        ecs::Id chosen{};
        w.things().each([&](ecs::Id id, auto h) {
            const auto& things = std::as_const(w).things().raw();
            if (!chosen.value && things.all_of<world::Item>(h) && !things.all_of<world::Fire>(h) &&
                things.get<world::Item>(h).kind == kind && things.get<world::Item>(h).mass > 0)
                chosen = id;
        });
        KD_CHECK(chosen.value, "Conditional scene requires actual unspent dry wood");
        const auto ih = w.things().handle(chosen);
        auto& item = w.things().raw().get<world::Item>(ih);
        item.owner = scene.person;
        w.things().raw().get<world::Place>(ih).at = at;
        work.inputs.push_back({chosen, item.mass, static_cast<std::uint8_t>(work.inputs.size()), 1, 1, 0});
    }
    raw.get<world::Place>(ph).at = at;
    raw.get<world::Activity>(ph) = {8, 0, 100, at, at};
    for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(scene.person, slot);
    w.schedule(scene.person, world::kActivitySlot, 100);
    w.run_to(101);
    for (const auto& m : know.memories)
        if (m.action == 11 && m.inputs.size() == 2) scene.memory = m.id;
    KD_CHECK(scene.memory != 0, "Conditional scene records a real shorter-than-fit twirl");
    return scene;
}
IdeaPair idea_pair(const data::Catalogue& catalogue, std::uint64_t seed) {
    auto sent = remembered_wood(catalogue, seed);
    auto control = remembered_wood(catalogue, seed);
    auto missing = remembered_wood(catalogue, seed);
    IdeaPair out;
    out.seed = seed;
    const auto board = entry(catalogue, "base:dry_board");
    missing.camp->world().things().each([&](ecs::Id, auto h) {
        auto& item = missing.camp->world().things().raw().get<world::Item>(h);
        if (item.kind != board || missing.camp->world().things().raw().all_of<world::Fire>(h)) return;
        out.removed_mg += item.mass;
        item.mass = 0;
        item.state = 4;
    });
    for (auto* scene : {&sent, &missing}) {
        auto& w = scene->camp->world();
        KD_CHECK(demo::IdeaDreams::problem(w, scene->person, scene->memory).empty(),
                 "The declared conditional memory is eligible");
        w.command(101, demo::Living::kIdeaDream, scene->person.value, scene->memory);
        w.run_to(102);
    }
    out.pending_reopen = reopen(sent, catalogue) && reopen(missing, catalogue);
    while (sent.camp->world().frontier() < 3 * time::kDay && act(sent).status == 1)
        sent.camp->world().run_to(sent.camp->world().frontier() + time::kHour);
    out.status = act(sent).status;
    out.dream_at = act(sent).executed;
    out.delivered_reopen = reopen(sent, catalogue);
    const auto after = out.dream_at >= 0 ? out.dream_at : 101;
    const auto end = after + demo::Living::kDreamLife;
    for (auto* scene : {&sent, &control, &missing})
        scene->camp->world().run_to(std::max(end, scene->camp->world().frontier()));
    out.attempt_at = act(sent).first_attempt_at;
    out.control_try_at = first_try(control, after);
    out.missing_attempt_at = act(missing).first_attempt_at;
    out.success = successes(sent, after);
    out.control_success = successes(control, after);
    out.missing_success = successes(missing, after);
    out.absent_conserved = true;
    const auto& mw = std::as_const(*missing.camp).world();
    mw.things().each([&](ecs::Id, auto h) {
        const auto& item = mw.things().raw().get<world::Item>(h);
        if (item.kind == board && item.mass != 0 && !mw.things().raw().all_of<world::Fire>(h))
            out.absent_conserved = false;
    });
    out.final_reopen = reopen(sent, catalogue) && reopen(control, catalogue) && reopen(missing, catalogue);
    out.sent_digest = num::to_hex(sent.camp->world().digests().whole);
    out.control_digest = num::to_hex(control.camp->world().digests().whole);
    out.missing_digest = num::to_hex(missing.camp->world().digests().whole);
    return out;
}
}  // namespace kd::proof
