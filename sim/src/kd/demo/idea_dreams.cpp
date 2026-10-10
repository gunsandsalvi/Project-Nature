#include "kd/demo/idea_dreams.hpp"
#include <tuple>
#include "kd/demo/crafting.hpp"
#include "kd/demo/living.hpp"
namespace kd::demo {
namespace {
bool performed(const world::Knowledge& know, const world::Memory& m) {
    if (m.action > 20 || !(know.performed & (1U << m.action)) || m.inputs.empty()) return false;
    for (const auto& input : m.inputs) {
        bool handled_here = false;
        for (std::size_t p = 0; p < input.sources.size(); ++p)
            if ((input.sources[p] == 3 || input.sources[p] == 4) && input.learned_at[p] == m.at) handled_here = true;
        if (!handled_here) return false;
    }
    return true;
}
bool experienced(const world::Knowledge& know, std::size_t property) {
    for (const auto& m : know.memories)
        for (const auto& input : m.inputs)
            if ((input.mask & (1U << property)) && input.values[property] > 0 &&
                (input.sources[property] == 3 || input.sources[property] == 4))
                return true;
    return false;
}
std::optional<world::IdeaFields> route(const world::World& w, const world::Knowledge& know, const world::Memory& m,
                                       std::uint32_t recipe) {
    const auto& b = w.catalogue().kind<data::Blueprint>()[recipe];
    if (b.action != m.action || !performed(know, m) ||
        std::any_of(know.skills.begin(), know.skills.end(),
                    [&](const auto& skill) { return skill.known && skill.recipe == recipe; }))
        return {};
    world::IdeaFields idea;
    idea.kind = 1;
    idea.memory = m.id;
    idea.action = m.action;
    idea.recipe = recipe;
    std::vector<std::size_t> used;
    const auto& kinds = w.catalogue().kind<data::ItemKind>();
    for (const auto& role : b.inputs) {
        bool found = false;
        for (std::size_t i = 0; i < m.inputs.size(); ++i) {
            if (std::find(used.begin(), used.end(), i) != used.end()) continue;
            const auto& input = m.inputs[i];
            const auto& kind = kinds[input.kind];
            const auto& material = kinds[input.material];
            const data::FitInput physical{
                kind.inherit ? std::string_view(material.material_class) : std::string_view(kind.material_class),
                kind.form, kind.inherit ? material.characteristics : kind.characteristics, kind.length, kind.mass};
            if (!data::fits(role, physical)) continue;
            used.push_back(i);
            idea.inputs.push_back({input.kind, input.material});
            found = true;
            break;
        }
        if (!found && !role.optional) return {};
    }
    if (idea.inputs.empty() || idea.inputs.size() > 2) return {};
    const auto& result = kinds[b.result.index];
    for (const std::size_t p : {std::size_t{1}, 8UL, 9UL, 11UL, 12UL, 13UL, 16UL}) {
        const auto benefit = p == 1 && b.edge_from >= 0 ? 1 : result.characteristics[p];
        if (benefit > 0 && experienced(know, p)) {
            idea.desired_property = static_cast<std::uint8_t>(p);
            return idea;
        }
    }
    return {};
}
}  // namespace
const world::Memory* IdeaDreams::memory(const world::Knowledge& know, std::uint64_t id) {
    const auto found =
        std::find_if(know.memories.begin(), know.memories.end(), [&](const auto& m) { return m.id == id; });
    return found == know.memories.end() ? nullptr : &*found;
}
std::optional<world::IdeaFields> IdeaDreams::fit(const world::World& w, world::Beings::Handle h, std::uint64_t id) {
    const auto* know = w.beings().raw().try_get<world::Knowledge>(h);
    if (!know) return {};
    const auto* m = memory(*know, id);
    if (!m) return {};
    const auto& recipes = w.catalogue().kind<data::Blueprint>();
    std::optional<world::IdeaFields> best;
    std::tuple<std::int64_t, std::int64_t, std::uint64_t> rank{};
    for (std::uint32_t r = 0; r < recipes.size(); ++r) {
        auto candidate = route(w, *know, *m, r);
        if (!candidate) continue;
        const auto order =
            std::tuple{know->sectors[static_cast<std::size_t>(recipes[r].sector)].level, Crafting::success(w, h, r, {}),
                       std::numeric_limits<std::uint64_t>::max() - recipes.key(r).hash};
        if (!best || order > rank) {
            best = std::move(candidate);
            rank = order;
        }
    }
    return best;
}
bool IdeaDreams::valid(const world::World& w, world::Beings::Handle h, const world::IdeaFields& idea) {
    const auto* know = w.beings().raw().try_get<world::Knowledge>(h);
    if (!know) return false;
    const auto* m = memory(*know, idea.memory);
    if (!m || idea.recipe == world::kNoRecipe) return false;
    const auto candidate = route(w, *know, *m, idea.recipe);
    if (!candidate || candidate->desired_property != idea.desired_property ||
        candidate->inputs.size() != idea.inputs.size())
        return false;
    for (std::size_t i = 0; i < idea.inputs.size(); ++i)
        if (candidate->inputs[i].kind != idea.inputs[i].kind ||
            candidate->inputs[i].material != idea.inputs[i].material)
            return false;
    return true;
}
std::optional<world::IdeaFields> IdeaDreams::guess(const world::World& w, world::Beings::Handle h, bool compatible,
                                                   std::uint64_t pick) {
    const auto* know = w.beings().raw().try_get<world::Knowledge>(h);
    if (!know) return {};
    std::vector<world::IdeaFields> choices;
    for (const auto& m : know->memories) {
        if (!performed(*know, m) || m.inputs.size() > 2) continue;
        auto idea = fit(w, h, m.id);
        if (compatible && idea) choices.push_back(*idea);
        if (!compatible && !idea) {
            world::IdeaFields guess;
            guess.kind = 1;
            guess.memory = m.id;
            guess.action = m.action;
            for (const auto& input : m.inputs) guess.inputs.push_back({input.kind, input.material});
            for (std::size_t p : {12UL, 8UL, 9UL, 1UL, 11UL, 13UL, 16UL})
                if (experienced(*know, p)) {
                    guess.desired_property = static_cast<std::uint8_t>(p);
                    choices.push_back(guess);
                    break;
                }
        }
    }
    if (choices.empty()) return {};
    return choices[static_cast<std::size_t>(pick % choices.size())];
}
std::string IdeaDreams::problem(const world::World& w, ecs::Id person, std::uint64_t id, time::Seconds at) {
    auto limit = Living::dream_limit(w, person, at);
    if (!limit.empty()) return limit;
    const auto h = w.beings().handle(person);
    if (!fit(w, h, id)) return "No handled action and experienced benefit fit that memory";
    return {};
}
void IdeaDreams::dream(world::Context& c, world::Beings::Handle h, world::IdeaFields idea) {
    auto& raw = c.world().beings().raw();
    auto& know = raw.get<world::Knowledge>(h);
    const auto* m = memory(know, idea.memory);
    KD_CHECK(m != nullptr, "Dream remembers an existing experience");
    auto& thought = raw.get<world::Dream>(h);
    thought = {};
    idea.hunch_id = idea.memory;
    idea.first_attempt_at = -1;
    static_cast<world::IdeaFields&>(thought) = idea;
    thought.night = Living::night(c.now());
    thought.at = c.now();
    thought.until = c.now() + Living::kDreamLife;
    thought.place = m->place;
    const auto old = std::find_if(know.hunches.begin(), know.hunches.end(),
                                  [&](const auto& hint) { return hint.origin == 2 && hint.id == idea.hunch_id; });
    world::Hunch hint;
    hint.id = idea.hunch_id;
    hint.origin = 2;
    hint.source_memory = idea.memory;
    hint.action = idea.action;
    hint.last_use = c.now();
    for (const auto& input : idea.inputs)
        for (const auto& familiar : m->inputs)
            if (input.kind == familiar.kind && input.material == familiar.material) {
                hint.inputs.push_back(familiar);
                break;
            }
    if (old != know.hunches.end()) {
        hint.failures = old->failures;
        *old = std::move(hint);
    } else {
        if (know.hunches.size() == 5) know.hunches.erase(know.hunches.begin());
        know.hunches.push_back(std::move(hint));
    }
    c.record(150, c.world().beings().id_of(h).value, idea.action);
}
void IdeaDreams::attempted(world::Context& c, world::Beings::Handle h, std::uint8_t action,
                           std::span<const ecs::Id> inputs) {
    auto& raw = c.world().beings().raw();
    const auto* work = raw.try_get<world::Work>(h);
    if (!work || work->state != 2 || work->route != 3 ||
        (c.now() <= work->active_start && work->retained_progress == 0))
        return;
    const auto& know = raw.get<world::Knowledge>(h);
    auto& thought = raw.get<world::Dream>(h);
    const auto person = c.world().beings().id_of(h);
    auto& acts = raw.get<world::Dreams>(c.world().beings().handle(raw.get<Home>(h).camp)).acts;
    for (const auto& hint : know.hunches) {
        if (hint.origin != 2 || hint.action != action || hint.last_use != work->start) continue;
        bool matches = !hint.inputs.empty();
        for (const auto& required : hint.inputs) {
            bool found = false;
            for (auto id : inputs) {
                const auto& item =
                    std::as_const(c.world()).things().raw().get<world::Item>(c.world().things().handle(id));
                if (item.kind == required.kind && item.material == required.material && item.mass > 0) found = true;
            }
            matches = matches && found;
        }
        if (!matches) continue;
        if (thought.kind == 1 && thought.hunch_id == hint.id && thought.first_attempt_at < 0)
            thought.first_attempt_at = c.now();
        for (auto it = acts.rbegin(); it != acts.rend(); ++it)
            if (it->person == person.value && it->status == 2 && it->hunch_id == hint.id &&
                it->executed <= work->start) {
                if (it->first_attempt_at < 0) it->first_attempt_at = c.now();
                break;
            }
    }
}
}  // namespace kd::demo
