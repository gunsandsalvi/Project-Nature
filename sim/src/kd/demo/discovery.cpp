#include "kd/demo/discovery.hpp"
#include <map>
#include <tuple>
#include "kd/chance/chance.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery_scene.hpp"
#include "kd/demo/fire.hpp"
#include "kd/demo/idea_dreams.hpp"
#include "kd/demo/living.hpp"
#include "kd/num/sort.hpp"
namespace kd::demo {
namespace {
world::Familiar& fact(world::Knowledge& know, const world::Item& item) {
    for (auto& f : know.familiar)
        if (f.kind == item.kind && f.material == item.material && f.state == item.state) return f;
    KD_CHECK(know.familiar.size() < 128, "Scoped familiar catalogue fits its saved bound");
    world::Familiar f;
    f.kind = item.kind;
    f.material = item.material;
    f.state = item.state;
    know.familiar.push_back(f);
    return know.familiar.back();
}
void evidence(world::Familiar& f, const std::array<std::int64_t, 18>& values, std::uint32_t mask, std::uint8_t source,
              time::Seconds now, std::uint64_t event, ecs::Id person) {
    f.at = now;
    for (std::size_t i = 0; i < 18; ++i) {
        if ((mask & (1U << i)) == 0) continue;
        f.mask |= 1U << i;
        f.values[i] = static_cast<std::uint8_t>(values[i]);
        f.certainty[i] = 100;
        f.sources[i] = source;
        f.learned_at[i] = now;
        f.source_events[i] = event;
        f.source_people[i] = person;
    }
}
std::string word(const world::World& w, world::CraftHistory& history, std::uint32_t recipe) {
    const auto& recipes = w.catalogue().kind<data::Blueprint>();
    const auto kind = recipes[recipe].result.index;
    for (const auto& e : history.events) {
        if (!e.noticed || e.word.empty()) continue;
        if (recipes[e.recipe].result.index == kind) return e.word;
    }
    const auto& scenes = w.catalogue().kind<DiscoveryScene>();
    KD_CHECK(scenes.size() == 1 && !scenes[0].syllables.empty(), "Discovery words need recorded syllables");
    const auto& syllables = scenes[0].syllables;
    const chance::Draws draws(w.seed(), chance::name("coined word"), recipes.key(recipe).hash, 0,
                              chance::name("two syllables"));
    std::string made = syllables[draws.below(0, syllables.size())] + syllables[draws.below(1, syllables.size())];
    const auto base = made;
    std::uint64_t suffix = 1;
    while (std::any_of(history.events.begin(), history.events.end(), [&](const auto& e) { return e.word == made; }))
        made = base + std::to_string(++suffix);
    return made;
}
}  // namespace
void Discovery::starting(world::World& w, world::Beings::Handle h) {
    auto& know = w.beings().raw().get<world::Knowledge>(h);
    const auto& kinds = w.catalogue().kind<data::ItemKind>();
    // Labelled starting food familiarity is separate from observed memories.
    for (std::uint32_t n = 0; n < kinds.size(); ++n) {
        bool starting_food = kinds[n].edible;
        const auto& blueprints = w.catalogue().kind<data::Blueprint>();
        for (std::uint32_t r = 0; r < blueprints.size(); ++r) {
            const auto& b = blueprints[r];
            if (!b.starting || b.inputs.empty()) continue;
            const bool food_role = std::any_of(b.inputs[0].ranges.begin(), b.inputs[0].ranges.end(),
                                               [](const auto& r) { return r.characteristic == 8; });
            const data::FitInput input{kinds[n].material_class, kinds[n].form, kinds[n].characteristics,
                                       kinds[n].length, kinds[n].mass};
            if (food_role && data::fits(b.inputs[0], input)) starting_food = true;
        }
        if (!starting_food) continue;
        world::Item item;
        item.kind = n;
        item.material = n;
        auto& f = fact(know, item);
        evidence(f, kinds[n].characteristics, 1U << 8U, 2, w.frontier(), 0, {});
        f.edible = kinds[n].edible ? 1 : 0;
        f.edible_source = kinds[n].edible ? 2 : 0;
    }
}
const world::Familiar* Discovery::familiar(const world::Knowledge& know, const world::Item& item) {
    for (const auto& f : know.familiar)
        if (f.kind == item.kind && f.material == item.material && f.state == item.state) return &f;
    return nullptr;
}
void Discovery::learn(world::Context& c, world::Beings::Handle h, ecs::Id id, std::uint32_t mask, std::uint8_t source,
                      bool edible, std::uint64_t event, ecs::Id person) {
    const auto& physical_world = std::as_const(c.world());
    const auto& item = physical_world.things().raw().get<world::Item>(physical_world.things().handle(id));
    auto& know = c.world().beings().raw().get<world::Knowledge>(h);
    auto& f = fact(know, item);
    evidence(f, Crafting::characteristics(c.world().catalogue(), item), mask, source, c.now(), event, person);
    if (edible) {
        f.edible = 1;
        f.edible_source = source;
    }
}
void Discovery::see(world::Context& c, world::Beings::Handle h) {
    const auto& w = c.world();
    const auto& raw = w.beings().raw();
    const auto home = raw.get<Home>(h).camp;
    const auto here = raw.get<world::Activity>(h).at(w.torus(), c.now());
    const auto daylight = c.now() % time::kDay >= 6 * time::kHour && c.now() % time::kDay < 20 * time::kHour;
    const std::int64_t range = daylight ? 3000 : 500;
    struct Seen {
        ecs::Id first, last;
    };
    std::map<std::tuple<std::uint32_t, std::uint32_t, std::uint8_t>, Seen> kinds;
    for (const auto& site : w.item_sites()) {
        if (site.home != home || w.torus().squared_distance(here, site.at) > range * range ||
            !Living::visible(w, home, here, site.at))
            continue;
        for (const auto& seen : site.sight) {
            const auto [kind, fresh] =
                kinds.try_emplace({seen.kind, seen.material, seen.state}, Seen{seen.first, seen.last});
            if (!fresh) {
                kind->second.first = std::min(kind->second.first, seen.first);
                kind->second.last = std::max(kind->second.last, seen.last);
            }
        }
    }
    std::vector<Seen> ordered;
    ordered.reserve(kinds.size());
    for (const auto& entry : kinds) ordered.push_back(entry.second);
    num::sort_strict(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    // Preserve first insertion order and final actual masked sight evidence in canonical item order.
    for (const auto& seen : ordered) learn(c, h, seen.last, kSight, 1);
    FireRules::notice_food(c, h);
}
std::vector<world::Familiar> Discovery::handling(world::Context& c, world::Beings::Handle h, std::uint8_t action,
                                                 std::span<const ecs::Id> inputs) {
    auto& know = c.world().beings().raw().get<world::Knowledge>(h);
    IdeaDreams::attempted(c, h, action, inputs);
    know.performed |= 1U << action;
    std::vector<world::Familiar> out;
    for (const auto id : inputs) {
        auto mask = kSight;
        if (action == 2) mask |= (1U << 2U) | (1U << 3U);  // resistance/fracture experienced while striking
        learn(c, h, id, mask, 3);
        const auto& physical_world = std::as_const(c.world());
        const auto& item = physical_world.things().raw().get<world::Item>(physical_world.things().handle(id));
        const auto* f = familiar(know, item);
        KD_CHECK(f != nullptr, "Handling records its perceived input");
        out.push_back(*f);
    }
    return out;
}
void Discovery::memory(world::Context& c, world::Beings::Handle h, std::uint8_t action,
                       std::vector<world::Familiar> inputs, std::uint8_t sign, ecs::Id result, std::uint64_t event,
                       std::uint8_t strength) {
    const auto measured = c.world().measure(world::Cost::evidence);
    auto& know = c.world().beings().raw().get<world::Knowledge>(h);
    world::Memory m;
    m.id = know.next_memory++;
    m.event = event;
    m.at = c.now();
    m.place = c.world().beings().raw().get<world::Place>(h).at;
    m.action = action;
    m.sign = sign;
    m.strength = strength;
    m.certainty = 100;
    m.inputs = std::move(inputs);
    m.result = result;
    m.participants.push_back({c.world().beings().id_of(h)});
    know.memories.push_back(std::move(m));
    if (know.memories.size() > 200) {
        const auto weakest =
            std::min_element(know.memories.begin(), know.memories.end(), [](const auto& a, const auto& b) {
                if (a.strength != b.strength) return a.strength < b.strength;
                if (a.at != b.at) return a.at < b.at;
                return a.id < b.id;
            });
        know.memories.erase(weakest);
    }
}
std::uint64_t Discovery::result(world::Context& c, world::Beings::Handle h, std::uint32_t recipe,
                                std::span<const ecs::Id> inputs, std::vector<world::Familiar> perceived, ecs::Id result,
                                bool success, bool unknown, std::uint8_t route,
                                std::span<const world::HeatCredit> heat_sources, std::uint64_t choice, ecs::Id source) {
    auto& raw = c.world().beings().raw();
    const auto person = c.world().beings().id_of(h);
    auto& know = raw.get<world::Knowledge>(h);
    const auto& blueprint = c.world().catalogue().kind<data::Blueprint>()[recipe];
    const auto& life = raw.get<world::Life>(h);
    auto& history = raw.get<world::CraftHistory>(c.world().beings().handle(raw.get<Home>(h).camp));
    const chance::Draws draws(c.world().seed(), chance::name("surprise"), person.value, c.now(),
                              c.world().catalogue().kind<data::Blueprint>().key(recipe));
    auto noticing = 250000 + static_cast<std::int64_t>(know.curiosity) * 5000;
    if ((unknown && route == 1) || Living::needs(life)[2] < 20) noticing /= 2;
    const auto noticed = !unknown || draws.below(0, 1000000) < static_cast<std::uint64_t>(noticing);
    world::Result e;
    e.id = history.next++;
    e.choice = choice ? choice : raw.get<world::Work>(h).choice;
    if (choice) e.source = source;
    e.at = c.now();
    e.place = raw.get<world::Place>(h).at;
    e.actor = person;
    e.result = result;
    e.recipe = recipe;
    e.route = route;
    e.noticed = success && noticed && result.value != 0;
    e.kind = !success ? 5 : unknown && noticed ? 1 : 0;
    e.heat_sources.assign(heat_sources.begin(), heat_sources.end());
    for (const auto id : inputs) e.inputs.push_back({id});
    if (e.noticed) e.word = word(c.world(), history, recipe);
    bool returning = false;
    if (e.kind == 1) {
        const auto previous = std::find_if(history.events.rbegin(), history.events.rend(), [&](const auto& old) {
            return old.recipe == recipe && old.kind >= 1 && old.kind <= 4;
        });
        returning = previous != history.events.rend() && previous->kind == 3;
    }
    history.events.push_back(e);
    if (returning) {
        auto returned = e;
        returned.id = history.next++;
        returned.kind = 4;
        history.events.push_back(std::move(returned));
    }
    if (success && result.value != 0) {
        const auto& physical = c.world().things().raw().get<world::Item>(c.world().things().handle(result));
        const bool edible = c.world().catalogue().kind<data::ItemKind>()[physical.kind].edible && !unknown;
        learn(c, h, result, kSight | (edible ? 1U << 8U : 0), 4, edible, e.id, person);
    }
    // Smoke is a failure sign, never a physical ember or a heat source.
    const bool smoke = blueprint.hint != 13 || draws.below(1, 2) == 0;
    const auto sign = static_cast<std::uint8_t>(!success && noticed && smoke ? blueprint.hint : 0);
    memory(c, h, static_cast<std::uint8_t>(blueprint.action), std::move(perceived), sign, result, e.id,
           static_cast<std::uint8_t>(noticed && unknown ? 90 : 30));
    if (success && unknown && noticed && result.value != 0) {
        auto found =
            std::find_if(know.skills.begin(), know.skills.end(), [&](const auto& s) { return s.recipe == recipe; });
        if (found == know.skills.end()) {
            world::Skill skill;
            skill.recipe = recipe;
            know.skills.push_back(skill);
            found = std::prev(know.skills.end());
        }
        if (!found->known || found->practice.level == 0) {
            found->known = 1;
            found->practice = {1000, 1000, 0, c.now()};
            found->source = person;
            found->source_event = e.id;
            found->route = route;
        }
        know.curiosity_need = static_cast<std::uint8_t>(std::min<int>(100, know.curiosity_need + 10));
    } else if (sign != 0 && unknown) {
        const auto created = know.next_memory - 1;
        const auto found =
            std::find_if(know.memories.begin(), know.memories.end(), [&](const auto& m) { return m.id == created; });
        if (found == know.memories.end()) return e.id;
        const auto& m = *found;
        world::Hunch hunch;
        hunch.action = m.action;
        hunch.result_form = 14;  // actual crumb sign, not an undiscovered recipe/result name
        hunch.source_memory = m.id;
        hunch.last_use = c.now();
        for (std::size_t n = 0; n < std::min<std::size_t>(2, m.inputs.size()); ++n) hunch.inputs.push_back(m.inputs[n]);
        const auto same = [&](const auto& old) {
            return old.action == hunch.action && old.inputs.size() == hunch.inputs.size() && !old.inputs.empty() &&
                   old.inputs[0].kind == hunch.inputs[0].kind && old.inputs[0].material == hunch.inputs[0].material;
        };
        if (std::none_of(know.hunches.begin(), know.hunches.end(), same)) {
            if (know.hunches.size() == 5) know.hunches.erase(know.hunches.begin());
            know.hunches.push_back(std::move(hunch));
        }
    }
    c.record(success ? 200 : 201, person.value, result.value);
    return e.id;
}
}  // namespace kd::demo
