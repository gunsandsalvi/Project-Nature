#include "kd/demo/choice.hpp"
// Generic, event-settled work and finite meals (MAT-04, MAT-09, TIM-17).
#include <bit>
#include <map>
#include "kd/chance/chance.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/fire.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
#include "kd/num/sort.hpp"
namespace kd::demo {
namespace {
using world::Item;
using world::LivingAct;
using world::Work;
const Item& value(const world::World& w, ecs::Id id) {
    return w.things().raw().get<Item>(w.things().handle(id));
}
Item& mutable_item(world::World& w, ecs::Id id) {
    return w.things().raw().get<Item>(w.things().handle(id));
}
num::Point place(const world::World& w, ecs::Id id) {
    return w.things().raw().get<world::Place>(w.things().handle(id)).at;
}
void spent(Item& item) {
    if (item.mass == 0) item.state = 4;
}
std::vector<ecs::Id> input_ids(const Work& work) {
    std::vector<ecs::Id> out;
    out.reserve(work.inputs.size());
    for (const auto& r : work.inputs) out.push_back(r.item);
    return out;
}
ecs::Id born(world::Context& c, Item item, num::Point at) {
    KD_CHECK(item.mass > 0, "A result conserves positive input mass");
    const auto h = c.world().make_thing();
    const auto id = c.world().things().id_of(h);
    c.world().things().raw().emplace<world::Place>(h, at);
    c.world().things().raw().emplace<Item>(h, std::move(item));
    c.item_changed(id);
    return id;
}
// Floor of length * cube-root(remaining/original), without floating point.
std::int64_t shorter(std::int64_t length, std::int64_t left, std::int64_t original) {
    std::int64_t low = 1, high = length;
    using Wide = __int128;
    const auto limit = static_cast<Wide>(length) * length * length * left;
    while (low < high) {
        const auto middle = (low + high + 1) / 2;
        if (static_cast<Wide>(middle) * middle * middle * original <= limit)
            low = middle;
        else
            high = middle - 1;
    }
    return low;
}
void release(world::Context& c, world::Beings::Handle h) {
    auto& raw = c.world().beings().raw();
    const auto person = c.world().beings().id_of(h);
    auto& work = raw.get<Work>(h);
    const auto here = raw.get<world::Place>(h).at;
    for (const auto& r : work.inputs) {
        auto& item = mutable_item(c.world(), r.item);
        if (item.owner == person && r.return_shared) item.owner = {};
        if (r.picked) c.world().things().raw().get<world::Place>(c.world().things().handle(r.item)).at = here;
        if (auto* fire = c.world().things().raw().try_get<world::Fire>(c.world().things().handle(r.item))) {
            fire->owner = item.owner;
            fire->at = place(c.world(), r.item);
        }
        c.item_changed(r.item);
    }
    Learning::forget_work(c, h);
    work = {};
    c.cancel(person, 2);
}
// A mind may compare dimensions and familiar evidence; absent properties remain uncertain.
bool perceived_fit(const data::InputRole& role, const data::FitInput& physical, const world::Familiar& f) {
    auto known = physical;
    known.values.fill(0);
    for (const auto& r : role.ranges) {
        const auto i = static_cast<std::size_t>(r.characteristic);
        known.values[i] = (f.mask & (1U << i)) != 0 ? f.values[i] : r.minimum;
    }
    return data::fits(role, known);
}
// A decision observes one immutable reservation state. Batch the same reads once, without a cache
// surviving a task, item change or save boundary.
class Supplies {
    struct Claim {
        std::int64_t mass = 0;
        bool retained = false;
    };
    std::map<ecs::Id, Claim> claims_;

public:
    Supplies(const world::World& w, ecs::Id self) {
        w.beings().each([&](ecs::Id id, world::Beings::Handle person) {
            if (id == self) return;
            if (const auto* life = w.beings().raw().try_get<world::Life>(person); life && life->meal_item.value != 0)
                claims_[life->meal_item].mass += life->carried_food;
            if (const auto* t = w.beings().raw().try_get<world::Thermal>(person); t && t->tending_input.value)
                claims_[t->tending_input].mass += t->tending_mass;
            if (const auto* work = w.beings().raw().try_get<Work>(person))
                for (const auto& r : work->inputs) {
                    auto& claim = claims_[r.item];
                    claim.mass += r.mass;
                    claim.retained = claim.retained || r.retained;
                }
        });
    }
    std::int64_t available(ecs::Id id, const Item& item) const {
        const auto found = claims_.find(id);
        return found == claims_.end()   ? item.mass
               : found->second.retained ? 0
                                        : std::max<std::int64_t>(0, item.mass - found->second.mass);
    }
    bool free(ecs::Id id) const { return !claims_.contains(id); }
};
struct SeenInput {
    ecs::Id id;
    const Item* item;
    const world::Familiar* familiar;
    std::int64_t available;
    bool free;
    std::uint64_t tie;
    std::uint8_t edge;
};
struct Inputs {
    std::vector<SeenInput> all, tools;
};
Inputs reachable(world::Context& c, world::Beings::Handle h, const Supplies& supply, ecs::Id participant = {}) {
    const auto measured = c.world().measure(world::Cost::inputs);
    const auto& w = c.world();
    const auto& raw = w.beings().raw();
    const auto person = w.beings().id_of(h);
    const auto home = raw.get<Home>(h).camp;
    const auto here = raw.get<world::Place>(h).at;
    const auto& know = raw.get<world::Knowledge>(h);
    const auto clock = c.now() % time::kDay;
    const std::int64_t range = clock >= 6 * time::kHour && clock < 20 * time::kHour ? 3000 : 500;
    const chance::Draws draws(w.seed(), chance::name("reachable input ties"), person.value,
                              std::bit_cast<std::int64_t>(raw.get<world::Knowledge>(h).next_work),
                              chance::name("item order"));
    Inputs out;
    for (const auto& site : w.item_sites()) {
        if (site.home != home || w.torus().squared_distance(here, site.at) > range * range ||
            !Living::visible(w, home, here, site.at) || Living::route(w, home, here, site.at).empty())
            continue;
        const std::array owners{std::uint64_t{0}, person.value, participant.value};
        for (std::size_t n = 0; n < owners.size(); ++n) {
            if (n == 2 && (participant.value == 0 || participant == person)) continue;
            const auto owner = owners[n];
            const auto group = site.owned.find(owner);
            if (group == site.owned.end()) continue;
            for (const auto& entry : group->second) {
                const auto id = entry.id;
                const auto& item = w.things().raw().get<Item>(entry.handle);
                w.visited_item(false, item);
                const auto* familiar = Discovery::familiar(know, item);
                const auto available = supply.available(id, item);
                if (available == 0 || !familiar || w.things().raw().all_of<world::Fire>(entry.handle)) continue;
                out.all.push_back({id, &item, familiar, available, supply.free(id), draws.bits(id.value),
                                   static_cast<std::uint8_t>((familiar->mask & (1U << 1U)) ? familiar->values[1] : 0)});
            }
        }
    }
    num::sort_strict(out.all.begin(), out.all.end(),
                     [](const auto& a, const auto& b) { return a.tie != b.tie ? a.tie < b.tie : a.id < b.id; });
    std::array<std::vector<SeenInput>, 6> edges;
    for (const auto& input : out.all) edges[input.edge].push_back(input);
    out.tools.reserve(out.all.size());
    for (std::size_t edge = edges.size(); edge-- > 0;)
        out.tools.insert(out.tools.end(), edges[edge].begin(), edges[edge].end());
    return out;
}
struct Candidate {
    bool dream_hunch = false;
    std::int64_t ordinary_hunch = -1;
    world::CraftReason reason;
    std::vector<world::Reservation> inputs;
    std::int64_t duration = 0, unit = 0, goal = 0;
};
std::optional<Candidate> known(world::Context& c, world::Beings::Handle h, std::uint32_t recipe, const Inputs& seen) {
    const auto& w = c.world();
    const auto& raw = w.beings().raw();
    const auto& know = raw.get<world::Knowledge>(h);
    const auto& b = w.catalogue().kind<data::Blueprint>()[recipe];
    if (b.heat > 0 && !FireRules::cooking_spot(w, h, c.now())) return std::nullopt;
    Candidate out;
    out.reason = {0, 1, static_cast<std::uint8_t>(b.action), static_cast<std::uint8_t>(b.need), recipe, 0, 0, 0, {}};
    for (std::size_t role = 0; role < b.inputs.size(); ++role) {
        const auto& requirement = b.inputs[role];
        ecs::Id selected{};
        const SeenInput* selected_input = nullptr;
        for (const auto& input : requirement.retained ? seen.tools : seen.all) {
            const auto id = input.id;
            if (std::any_of(out.inputs.begin(), out.inputs.end(), [&](const auto& r) { return r.item == id; }))
                continue;
            if (requirement.retained && !input.free) continue;
            const auto& item = *input.item;
            if (b.heat > 0 && item.state != 0) continue;
            const auto* f = input.familiar;
            if (!f) continue;
            // A known cooking use requires personal food evidence; missing food is not an edible guess.
            if (b.action == 12 &&
                std::any_of(requirement.ranges.begin(), requirement.ranges.end(),
                            [](const auto& r) { return r.characteristic == 8 && r.minimum > 0; }) &&
                (!(f->mask & (1U << 8U)) || !f->certainty[8] || f->values[8] == 0))
                continue;
            const auto& kind = w.catalogue().kind<data::ItemKind>()[item.kind];
            const auto& material = w.catalogue().kind<data::ItemKind>()[item.material];
            data::FitInput dimensions{
                kind.inherit ? std::string_view(material.material_class) : std::string_view(kind.material_class),
                kind.form,
                {},
                item.length,
                item.mass};
            dimensions.mass = input.available;
            if (!perceived_fit(requirement, dimensions, *f)) continue;
            selected = id;
            selected_input = &input;
            break;  // Same first input / strongest known tool edge, with original stable ties.
        }
        if (selected.value == 0) {
            if (requirement.optional) continue;
            return std::nullopt;
        }
        const auto& item = *selected_input->item;
        const auto mass = requirement.retained || b.result_mass > 0
                              ? item.mass
                              : std::min<std::int64_t>(selected_input->available, 1000000);
        out.inputs.push_back({selected, mass, static_cast<std::uint8_t>(role),
                              static_cast<std::uint8_t>(requirement.retained), 0,
                              static_cast<std::uint8_t>(item.owner.value == 0)});
        out.reason.inputs.push_back({selected});
        for (const auto& range : requirement.ranges) {
            const auto property = static_cast<std::size_t>(range.characteristic);
            out.reason.confidence = std::min(
                out.reason.confidence, static_cast<std::uint8_t>(selected_input->familiar->mask & (1U << property)
                                                                     ? selected_input->familiar->certainty[property]
                                                                     : 0));
        }
    }
    const auto main = std::find_if(out.inputs.begin(), out.inputs.end(), [](const auto& r) { return r.role == 0; });
    if (main == out.inputs.end()) return std::nullopt;
    out.unit = std::min(b.unit_mass, main->mass);
    out.goal = b.result_mass > 0 ? std::min(main->mass, b.unit_mass * (3600 / b.seconds)) : main->mass;
    std::int64_t edge = 0;
    bool tool = false;
    for (const auto& r : out.inputs) {
        if (!r.retained) continue;
        const auto* f = Discovery::familiar(know, value(w, r.item));
        if (f && (f->mask & (1U << 1U))) edge = std::max<std::int64_t>(edge, f->values[1]);
        if (b.inputs[r.role].optional) tool = true;
    }
    // Mandatory tools are neither a bare-hand fallback nor necessarily cutting tools.
    if (std::none_of(b.inputs.begin(), b.inputs.end(), [](const auto& r) { return r.optional; })) tool = true;
    out.duration = Crafting::time_cost(w, h, Crafting::duration(b, edge, tool));
    const auto& life = raw.get<world::Life>(h);
    const auto needs = Living::needs(life);
    const auto need = b.need < 3 ? needs[static_cast<std::size_t>(b.need)] : know.curiosity_need;
    const auto urgency = std::max<std::int64_t>(0, 80 - need);
    out.reason.benefit = std::min<std::int64_t>(b.benefit * out.goal / 1000000, 100 - need);
    out.reason.seconds = std::min<std::int64_t>(3600, out.duration * ((out.goal + out.unit - 1) / out.unit));
    out.reason.score = urgency * out.reason.benefit * 10 - out.reason.seconds / 60;
    out.reason.parts = {urgency * out.reason.benefit * 10, 0, -out.reason.seconds / 60};
    return out;
}
std::vector<ecs::Id> matching(const world::World& w, const data::Blueprint& b, std::span<const ecs::Id> inputs) {
    std::vector<ecs::Id> out;
    // At most eight inputs: bounded backtracking avoids a first-fit tool stealing the only core.
    const auto search = [&](auto&& self, std::size_t role) -> bool {
        if (role == b.inputs.size()) return true;
        for (const auto id : inputs) {
            if (std::find(out.begin(), out.end(), id) != out.end()) continue;
            if (!data::fits(b.inputs[role], Crafting::physical(w.catalogue(), value(w, id)))) continue;
            out.push_back(id);
            if (self(self, role + 1)) return true;
            out.pop_back();
        }
        if (b.inputs[role].optional) {
            out.push_back({});
            if (self(self, role + 1)) return true;
            out.pop_back();
        }
        return false;
    };
    if (!search(search, 0)) out.clear();
    return out;
}
std::int64_t quality(const world::World& w, world::Beings::Handle h, std::uint32_t recipe,
                     std::span<const ecs::Id> inputs, const chance::Draws& draws, std::uint64_t try_number) {
    const auto& know = w.beings().raw().get<world::Knowledge>(h);
    std::int64_t level = 0;
    for (const auto& skill : know.skills)
        if (skill.recipe == recipe) level = skill.practice.level;
    auto q = level / 2000;
    std::int64_t sum = 0, count = 0;
    for (const auto id : inputs) {
        if (id.value == 0) continue;
        sum += value(w, id).quality;
        ++count;
    }
    if (count > 0) q += sum < 2 * count ? -1 : sum >= 4 * count ? 1 : 0;
    const auto roll = draws.below(try_number * 4 + 1, 1000000);
    q += roll < 100000 ? -1 : roll >= 900000 ? 1 : 0;
    return std::clamp<std::int64_t>(q, 0, 5);
}
void resolve(world::Context& c, world::Beings::Handle h, std::uint32_t recipe, std::vector<ecs::Id> roles, bool unknown,
             std::uint8_t route, std::uint64_t try_number) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto& work = raw.get<Work>(h);
    const auto person = w.beings().id_of(h);
    const auto& b = w.catalogue().kind<data::Blueprint>()[recipe];
    const auto all = input_ids(work);
    const auto handled = Discovery::handling(c, h, static_cast<std::uint8_t>(b.action), all);
    std::vector<ecs::Id> actual;
    std::vector<world::Familiar> perceived;
    for (const auto id : roles) {
        if (id.value == 0) continue;
        actual.push_back(id);
        const auto found = std::find(all.begin(), all.end(), id);
        KD_CHECK(found != all.end(), "Actual fits use this work's held inputs");
        perceived.push_back(handled[static_cast<std::size_t>(found - all.begin())]);
    }
    const chance::Draws draws(w.seed(), chance::name("craft result"), person.value, work.start,
                              w.catalogue().kind<data::Blueprint>().key(recipe));
    auto chance = Crafting::success(w, h, recipe, roles);
    if (unknown)
        chance = chance * (route == 3 ? 500000 : route == 2 ? 200000 : 50000) / 1000000 * b.discovery / 1000000;
    // Each blueprint has one unknown roll for this entire work. Known rolls use the saved try index.
    const bool success = draws.below(try_number * 4, 1000000) < static_cast<std::uint64_t>(chance);
    auto source = value(w, roles[0]);
    const auto old_mass = source.mass;
    const auto worked = std::min(source.mass, b.result_mass > 0 ? b.result_mass : work.unit_mass);
    if (worked == 0) return;
    const auto here = raw.get<world::Place>(h).at;
    const auto q = static_cast<std::uint8_t>(quality(w, h, recipe, roles, draws, try_number));
    Item result = source;
    result.owner = person;
    result.maker = person;
    result.made_at = c.now();
    result.parents.clear();
    for (const auto id : roles)
        if (id.value != 0) result.parents.push_back({id});
    result.changed.fill(0);
    result.changed_mask = 0;
    result.wear = 0;
    result.wear_remainder = 0;
    result.state = 0;
    result.quality = q;
    result.kind = success ? b.result.index : b.failure.index;
    if (!w.catalogue().kind<data::ItemKind>()[result.kind].inherit) result.material = result.kind;
    result.length = success ? b.result_length : w.catalogue().kind<data::ItemKind>()[b.failure.index].length;
    bool bare = false;
    for (std::size_t i = 0; i < b.inputs.size(); ++i)
        if (b.inputs[i].optional && roles[i].value == 0) bare = true;
    const auto yield = bare ? b.bare_yield : b.yield;
    result.mass = success ? (b.result_mass > 0 ? worked : worked * yield / 1000000) : worked;
    // A failed flaking fit can shatter its whole reserved core; no other recipe consumes unrelated stock.
    const bool shatter =
        !success && b.shatter > 0 && draws.below(try_number * 4 + 2, 1000000) < static_cast<std::uint64_t>(b.shatter);
    if (shatter) result.mass = source.mass;
    if (success && b.edge_from >= 0) {
        result.changed_mask |= 1U << 1U;
        result.changed[1] = static_cast<std::uint8_t>(
            Crafting::characteristics(w.catalogue(), source)[static_cast<std::size_t>(b.edge_from)]);
    }
    if (b.toughness >= 0) {
        result.changed_mask |= 1U << 2U;
        result.changed[2] = static_cast<std::uint8_t>(b.toughness);
    }
    if (!success && b.hint == 2) {
        result.changed_mask |= 1U << 1U;
        result.changed[1] = 2;  // the sharp crumb is real physical evidence, not a recipe hint alone
    }
    // Copy all source values before births: an ECS pool may move when an item is inserted.
    const auto debit = shatter ? old_mass : worked;
    mutable_item(w, roles[0]).mass -= debit;
    spent(mutable_item(w, roles[0]));
    ecs::Id made{};
    if (result.mass > 0) made = born(c, result, here);
    const auto leftover = debit - result.mass;
    if (leftover > 0) {
        auto waste = result;
        waste.kind = b.leftover.index;
        if (!w.catalogue().kind<data::ItemKind>()[waste.kind].inherit) waste.material = waste.kind;
        waste.mass = leftover;
        waste.length = w.catalogue().kind<data::ItemKind>()[waste.kind].length;
        waste.changed_mask = 0;
        waste.changed.fill(0);
        (void)born(c, std::move(waste), here);
    }
    if (b.result_mass > 0 && mutable_item(w, roles[0]).mass > 0) {
        // The remaining core receives a new identity and a source link; the old core becomes spent.
        auto remainder = source;
        remainder.mass = old_mass - debit;
        remainder.length = shorter(source.length, remainder.mass, old_mass);
        remainder.kind = remainder.length >= b.inputs[0].min_length ? b.leftover.index : b.failure.index;
        remainder.made_at = c.now();
        remainder.maker = person;
        remainder.parents = {{roles[0]}};
        remainder.owner = person;
        mutable_item(w, roles[0]).mass = 0;
        spent(mutable_item(w, roles[0]));
        const auto next = born(c, remainder, here);
        for (auto& r : work.inputs) {
            if (r.item != roles[0]) continue;
            r.item = next;
            r.mass = remainder.mass;
        }
    } else {
        for (auto& r : work.inputs) {
            if (r.item != roles[0]) continue;
            r.mass = std::min(r.mass, mutable_item(w, roles[0]).mass);
        }
    }
    c.item_changed(roles[0]);
    for (std::size_t role = 1; role < roles.size(); ++role) {
        if (roles[role].value == 0) continue;
        Crafting::wear(c, roles[role], debit, b.inputs[role].wear);
        // Wear/breakage is applied once to each actual retained tool.
        for (auto& r : work.inputs)
            if (r.item == roles[role]) r.mass = std::min(r.mass, mutable_item(w, r.item).mass);
    }
    if (success && made.value && w.catalogue().find("item", "base:ember") == result.kind) FireRules::ember(c, made);
    auto& history = raw.get<world::CraftHistory>(w.beings().handle(raw.get<Home>(h).camp));
    history.routine.record({0, c.now() / time::kDay * time::kDay, person, 0, recipe, source.kind, result.kind, 1,
                            static_cast<std::uint64_t>(success), debit, result.mass, 0});
    const auto event = Discovery::result(c, h, recipe, actual, perceived, made, success, unknown, route);
    Learning::worked(c, h, event, success);
    Learning::demonstrated(c, h, event);
    auto& know = raw.get<world::Knowledge>(h);
    for (auto& skill : know.skills)
        if (skill.recipe == recipe) skill.practice.last_use = c.now();
    know.sectors[static_cast<std::size_t>(b.sector)].last_use = c.now();
}
}  // namespace
struct Crafting::Decision {
    Supplies supply;
    Inputs inputs;
    Decision(world::Context& c, world::Beings::Handle h) : Decision(c, h, c.world()) {}
    Decision(world::Context& c, world::Beings::Handle h, world::World& w)
        : supply(w, w.beings().id_of(h)), inputs(reachable(c, h, supply)) {}
};
std::int64_t Crafting::duration(const data::Blueprint& recipe, std::int64_t edge, bool tool) {
    auto seconds = tool ? recipe.seconds : recipe.bare_seconds;
    // Only actual edge constraints have this speed advantage.
    std::int64_t minimum = 0;
    for (const auto& role : recipe.inputs)
        if (role.retained)
            for (const auto& r : role.ranges)
                if (r.characteristic == 1) minimum = std::max(minimum, r.minimum);
    if (minimum > 0 && tool)
        seconds = seconds * (100 - std::min<std::int64_t>(33, std::max<std::int64_t>(0, edge - minimum) * 10)) / 100;
    return std::max<std::int64_t>(1, seconds);
}
std::int64_t Crafting::time_cost(const world::World& w, world::Beings::Handle h, std::int64_t seconds) {
    const auto& raw = w.beings().raw();
    const auto& life = w.beings().raw().get<world::Life>(h);
    if (Living::needs(life)[2] < 20) seconds = seconds * 5 / 4;
    const auto clock = life.settled % time::kDay;
    const auto here = raw.get<world::Activity>(h).at(w.torus(), life.settled);
    if ((clock < 6 * time::kHour || clock >= 20 * time::kHour) &&
        !FireRules::task_light(w, raw.get<Home>(h).camp, here, here, life.settled))
        seconds = seconds * 5 / 4;
    return std::clamp<std::int64_t>(seconds, 1, 3600);
}
std::int64_t Crafting::success(const world::World& w, world::Beings::Handle h, std::uint32_t recipe,
                               std::span<const ecs::Id> roles, time::Seconds at) {
    const auto& raw = w.beings().raw();
    const auto& know = raw.get<world::Knowledge>(h);
    const auto& b = w.catalogue().kind<data::Blueprint>()[recipe];
    std::int64_t skill = 0;
    for (const auto& s : know.skills)
        if (s.recipe == recipe) skill = s.practice.level;
    const auto mean = (skill + know.sectors[static_cast<std::size_t>(b.sector)].level) / 2;
    auto ppm = 500000 + (mean - b.difficulty * 1000) * 100;
    const auto& life = raw.get<world::Life>(h);
    auto fatigue = life.awake;
    if (at >= 0) {
        const auto& activity = raw.get<world::Activity>(h);
        const auto elapsed = std::max<time::Seconds>(0, std::clamp(at, activity.start, activity.end) - life.settled);
        const auto resting = activity.what == static_cast<std::uint8_t>(LivingAct::rest) ||
                             activity.what == static_cast<std::uint8_t>(LivingAct::warm);
        fatigue = std::clamp<std::int64_t>(fatigue + (resting ? -2 : 1) * elapsed, 0, 129600);
    }
    if (100 - fatigue * 100 / 129600 < 20) ppm -= 100000;
    const auto night = (at < 0 ? life.settled : at) % time::kDay;
    const auto moment = at < 0 ? life.settled : at;
    const auto here = raw.get<world::Activity>(h).at(w.torus(), moment);
    if ((night < 6 * time::kHour || night >= 20 * time::kHour) &&
        !FireRules::task_light(w, raw.get<Home>(h).camp, here, here, moment))
        ppm -= 100000;
    std::int64_t quality_sum = 0, count = 0;
    for (const auto id : roles) {
        if (id.value == 0) continue;
        quality_sum += value(w, id).quality;
        ++count;
    }
    if (count > 0) ppm += quality_sum < 2 * count ? -100000 : quality_sum >= 4 * count ? 100000 : 0;
    return std::clamp<std::int64_t>(ppm, 50000, 950000);
}
void Crafting::wear(world::Context& c, ecs::Id tool, std::int64_t worked, std::int64_t rate) {
    auto item = value(c.world(), tool);
    if (rate == 0 || worked == 0 || item.mass == 0) return;
    const auto tough = characteristics(c.world().catalogue(), item)[2];
    const auto numerator = (tough <= 1 ? 2 : 1) * (item.quality <= 1 ? 2 : 1);
    const auto denominator = (tough >= 4 ? 2 : 1) * (item.quality >= 4 ? 2 : 1);
    // Save a single denominator-four remainder so changing quality/toughness loses no fraction.
    const auto units = rate * worked * numerator * (4 / denominator) + item.wear_remainder;
    item.wear = std::min<std::int64_t>(5000000, item.wear + units / 4000000);
    item.wear_remainder = units % 4000000;
    mutable_item(c.world(), tool) = item;
    c.item_changed(tool);
    if (item.wear < 5000000) return;
    item.kind = c.world().catalogue().kind<data::ItemKind>()[item.kind].broken.index;
    item.made_at = c.now();
    item.parents = {{tool}};
    item.wear = 0;
    item.wear_remainder = 0;
    item.changed_mask = 0;
    item.changed.fill(0);
    const auto at = place(c.world(), tool);
    mutable_item(c.world(), tool).mass = 0;
    spent(mutable_item(c.world(), tool));
    c.item_changed(tool);
    (void)born(c, item, at);
}
bool Crafting::prepare_lesson(world::Context& c, world::Beings::Handle teacher, world::Beings::Handle learner,
                              std::uint32_t recipe, std::uint64_t session, Living* living) {
    const auto learner_id = c.world().beings().id_of(learner);
    const Supplies supply(c.world(), learner_id);
    auto inputs = reachable(c, teacher, supply, learner_id);
    // Shared practice cannot commandeer the teacher's personal stock or tools. Filter
    // those before selecting a recipe's first/best inputs, so accessible alternatives win.
    const auto inaccessible = [&](const auto& input) {
        return input.item->owner.value != 0 && input.item->owner != learner_id;
    };
    std::erase_if(inputs.all, inaccessible);
    std::erase_if(inputs.tools, inaccessible);
    auto candidate = known(c, teacher, recipe, inputs);
    if (!candidate) return false;
    auto& w = c.world();
    auto& raw = w.beings().raw();
    const auto person = w.beings().id_of(learner);
    for (const auto& r : candidate->inputs) {
        const auto& item = value(w, r.item);
        if (item.owner.value != 0 && item.owner != person) return false;
        if (available(w, r.item, person) < r.mass || !tool_free(w, r.item, person)) return false;
    }
    auto& work = raw.get<Work>(learner);
    if (work.state != 0) return false;
    candidate->reason.kind = 5;
    candidate->reason.need_met = raw.get<world::Knowledge>(learner).curiosity_need;
    std::optional<ChoiceSet> alternatives;
    if (living) {
        // The learner considers the invitation against their own options, without interrupting
        // their current watch or starting a different plan merely because someone asked.
        const auto saved_life = raw.get<world::Life>(learner);
        const auto saved_dream = raw.get<world::Dream>(learner);
        const auto saved_choice = raw.get<world::Knowledge>(learner).choice;
        const auto saved_reasons = raw.get<world::Knowledge>(learner).reasons;
        const auto saved_draw = raw.get<world::Knowledge>(learner).hourly_draw;
        const auto* thermal = raw.try_get<world::Thermal>(learner);
        raw.get<world::Life>(learner) =
            living->sample(saved_life, raw.get<world::Activity>(learner), c.now(), thermal ? thermal->water_due_ml : 0);
        alternatives.emplace();
        living->choose(c, learner, raw.get<Home>(learner).camp, &*alternatives);
        const auto curiosity = raw.get<world::Knowledge>(learner).curiosity_need;
        candidate->reason.need = 3;
        candidate->reason.need_met = curiosity;
        candidate->reason.benefit = 10;  // Existing expected learning benefit, evaluated by its learner.
        candidate->reason.parts = {std::max<std::int64_t>(0, 80 - curiosity) * 100, 0, -candidate->reason.seconds / 60};
        candidate->reason.score = candidate->reason.parts[0] + candidate->reason.parts[2];
        const auto best = std::max_element(alternatives->reasons().begin(), alternatives->reasons().end(),
                                           [](const auto& a, const auto& b) { return a.score < b.score; });
        const bool preferred = best == alternatives->reasons().end() || candidate->reason.score > best->score;
        raw.get<world::Life>(learner) = saved_life;
        raw.get<world::Dream>(learner) = saved_dream;
        auto& mind = raw.get<world::Knowledge>(learner);
        mind.choice = saved_choice;
        mind.reasons = saved_reasons;
        mind.hourly_draw = saved_draw;
        if (!preferred) return false;
    }
    const auto reason = candidate->reason;
    auto accept = [&c, learner, recipe, session, teacher,
                   chosen = std::move(*candidate)](std::uint64_t choice) mutable {
        auto& w = c.world();
        auto& raw = w.beings().raw();
        auto& work = raw.get<Work>(learner);
        work.choice = choice;
        work.state = 1;
        work.number = raw.get<world::Knowledge>(learner).next_work++;
        work.action = chosen.reason.action;
        work.intended = 1;
        work.recipe = recipe;
        work.route = 5;
        work.lesson = session;
        work.start = c.now();
        work.active_start = c.now();
        work.try_seconds = time_cost(w, learner, chosen.duration);
        work.unit_mass = chosen.unit;
        work.goal_mass = chosen.goal;
        work.target = raw.get<world::Place>(teacher).at;
        work.inputs = std::move(chosen.inputs);
        return true;
    };
    if (alternatives) {
        alternatives->add(reason, std::move(accept));
        return alternatives->commit(c, learner);
    }
    const auto choice = Choices::keep(c, learner, reason);
    return accept(choice);
}
bool Crafting::choose(Living& living, world::Context& c, world::Beings::Handle h, ChoiceSet* proposals) {
    auto& raw = c.world().beings().raw();
    auto* know = raw.try_get<world::Knowledge>(h);
    if (!know) return false;
    Discovery::see(c, h);
    auto& life = raw.get<world::Life>(h);
    const auto needs = Living::needs(life);
    const bool urgent = *std::min_element(needs.begin(), needs.end()) < 20 || life.awake >= 129600;
    const bool other_urgent = needs[1] < 20 || needs[2] < 20 || life.awake >= 129600;
    const auto& pending = raw.get<Work>(h);
    const bool preparing_food =
        pending.intended && c.world().catalogue().kind<data::Blueprint>()[pending.recipe].need == 0;
    if (pending.lesson != 0) return false;
    if (pending.state == 4 && (!urgent || (!other_urgent && preparing_food))) {
        if (!proposals) return continue_work(living, c, h);
        Choices::restore(c.world(), h, pending.choice);
        world::CraftReason reason;
        if (!know->reasons.empty())
            reason = know->reasons.front();
        else {
            reason.kind = pending.intended ? 0 : 1;
            reason.intended = pending.intended;
            reason.recipe = pending.intended ? pending.recipe : world::kNoRecipe;
            reason.action = pending.action;
            reason.need = 3;
            reason.need_met = know->curiosity_need;
            reason.seconds = pending.try_seconds;
            for (const auto& input : pending.inputs) reason.inputs.push_back({input.item});
        }
        reason.parts = {0, 90000, 0};
        reason.score = 90000;  // Protect an existing safe plan; urgent survival excludes it above.
        proposals->add(reason, [&living, &c, h](std::uint64_t choice) {
            c.world().beings().raw().get<Work>(h).choice = choice;
            return continue_work(living, c, h);
        });
        return false;
    }
    std::optional<Decision> decision;
    if (life.goal == 0 || (needs[0] < 60 && life.scores[0] == -1000000)) {
        decision.emplace(c, h);
        if (meal(living, c, h, *decision, proposals)) return true;
    }
    if (other_urgent || raw.get<Work>(h).state != 0) return false;
    if (!decision) decision.emplace(c, h);
    const auto& supply = decision->supply;
    const auto& seen = decision->inputs;
    std::vector<Candidate> options;
    for (const auto& skill : know->skills) {
        if (options.size() == 8) break;
        if (!skill.known || (urgent && c.world().catalogue().kind<data::Blueprint>()[skill.recipe].need != 0)) continue;
        auto candidate = known(c, h, skill.recipe, seen);
        if (candidate) options.push_back(std::move(*candidate));
    }
    auto& thought = raw.get<world::Dream>(h);
    if (!urgent && thought.kind == 1 && thought.at >= 0 && thought.until > c.now()) {
        const auto hint = std::find_if(know->hunches.begin(), know->hunches.end(), [&](const auto& hunch) {
            return hunch.origin == 2 && hunch.id == thought.hunch_id;
        });
        if (hint != know->hunches.end()) {
            Candidate idea;
            idea.dream_hunch = true;
            idea.reason.kind = 1;
            idea.reason.need = 3;
            idea.reason.action = hint->action;
            idea.duration = time_cost(c.world(), h, hint->action == 6 || hint->action == 11 ? 300 : 60);
            idea.reason.seconds = idea.duration;
            idea.reason.benefit = 0;
            idea.reason.score = Living::kDreamPull - idea.duration / 60;
            idea.unit = 20000;
            idea.goal = 20000;
            for (const auto& familiar : hint->inputs) {
                for (const auto& input : seen.all) {
                    const auto id = input.id;
                    const auto& item = value(c.world(), id);
                    if (item.kind != familiar.kind || item.material != familiar.material || !supply.free(id) ||
                        std::any_of(idea.inputs.begin(), idea.inputs.end(),
                                    [&](const auto& r) { return r.item == id; }))
                        continue;
                    idea.inputs.push_back({id, item.mass, static_cast<std::uint8_t>(idea.inputs.size()), 1, 0,
                                           static_cast<std::uint8_t>(item.owner.value == 0)});
                    idea.reason.inputs.push_back({id});
                    const auto distance = c.world().torus().distance(raw.get<world::Place>(h).at, place(c.world(), id));
                    idea.reason.score -= (distance * 10 / living.rules().loaded_speed) / 60;
                    break;
                }
            }
            idea.reason.parts = {0, Living::kDreamPull, idea.reason.score - Living::kDreamPull};
            if (!idea.inputs.empty() && idea.inputs.size() == hint->inputs.size()) options.push_back(std::move(idea));
        }
    }
    const auto hour = static_cast<std::uint64_t>(c.now() / 3600 + 1);
    if (!urgent && know->hourly_draw != hour && !seen.all.empty()) {
        know->hourly_draw = hour;
        const auto person = c.world().beings().id_of(h);
        const chance::Draws draws(c.world().seed(), chance::name("curious hour"), person.value,
                                  static_cast<std::int64_t>(hour), chance::name("attempt and familiar action"));
        if (draws.below(0, know->curiosity >= 75 ? 24 : 168) == 0) {
            Candidate experiment;
            experiment.reason.kind = 1;
            experiment.reason.need = 3;
            experiment.reason.benefit = 10;
            experiment.reason.score = 500;
            experiment.reason.parts = {0, 500, 0};
            experiment.duration = time_cost(c.world(), h, 60);
            experiment.reason.seconds = experiment.duration;
            experiment.reason.action = static_cast<std::uint8_t>(draws.below(1, 21));
            const auto first_input = draws.below(3, seen.all.size());
            for (std::size_t n = 0; n < seen.all.size(); ++n) {
                const auto id = seen.all[(first_input + n) % seen.all.size()].id;
                if (!supply.free(id)) continue;
                const auto& item = value(c.world(), id);
                experiment.inputs.push_back({id, item.mass, static_cast<std::uint8_t>(experiment.inputs.size()), 1, 0,
                                             static_cast<std::uint8_t>(item.owner.value == 0)});
                experiment.reason.inputs.push_back({id});
                if (experiment.inputs.size() == 2) break;
            }
            if (!know->hunches.empty()) {
                auto& hunch = know->hunches[draws.below(2, know->hunches.size())];
                std::vector<world::Reservation> guessed;
                for (const auto& familiar : hunch.inputs) {
                    for (const auto& input : seen.all) {
                        const auto id = input.id;
                        const auto& item = value(c.world(), id);
                        if (item.kind != familiar.kind || item.material != familiar.material || !supply.free(id) ||
                            std::any_of(guessed.begin(), guessed.end(), [&](const auto& r) { return r.item == id; }))
                            continue;
                        guessed.push_back({id, item.mass, static_cast<std::uint8_t>(guessed.size()), 1, 0,
                                           static_cast<std::uint8_t>(item.owner.value == 0)});
                        break;
                    }
                }
                if (guessed.size() == hunch.inputs.size() && !guessed.empty()) {
                    experiment.inputs = std::move(guessed);
                    experiment.reason.action = hunch.action;
                    experiment.reason.inputs.clear();
                    for (const auto& r : experiment.inputs) experiment.reason.inputs.push_back({r.item});
                    experiment.ordinary_hunch = static_cast<std::int64_t>(&hunch - know->hunches.data());
                }
            }
            if (experiment.reason.action == 6 || experiment.reason.action == 11)
                experiment.duration = time_cost(c.world(), h, 300);
            experiment.reason.seconds = experiment.duration;
            experiment.unit = 20000;
            experiment.goal = 20000;
            if (!experiment.inputs.empty()) options.push_back(std::move(experiment));
        }
    }
    std::stable_sort(options.begin(), options.end(),
                     [](const auto& a, const auto& b) { return a.reason.score > b.reason.score; });
    if (options.empty() || (!proposals && options[0].reason.score <= life.scores[life.goal])) return false;
    std::vector<world::CraftReason> rejected;
    for (std::size_t n = 1; n < options.size(); ++n) rejected.push_back(options[n].reason);
    auto make_commit = [&living, &c, h](Candidate chosen) {
        return [&living, &c, h, chosen = std::move(chosen)](std::uint64_t choice) mutable {
            auto& raw = c.world().beings().raw();
            auto& know = raw.get<world::Knowledge>(h);
            auto& thought = raw.get<world::Dream>(h);
            if (chosen.ordinary_hunch >= 0)
                know.hunches[static_cast<std::size_t>(chosen.ordinary_hunch)].last_use = c.now();
            if (chosen.dream_hunch) {
                thought.decision_pull = Living::kDreamPull;
                for (auto& hint : know.hunches)
                    if (hint.origin == 2 && hint.id == thought.hunch_id) hint.last_use = c.now();
            }
            auto& work = raw.get<Work>(h);
            work.state = 1;
            work.choice = choice;
            work.number = know.next_work++;
            work.action = chosen.reason.action;
            work.intended = chosen.reason.intended;
            work.recipe = chosen.reason.recipe;
            work.route = work.intended ? 0 : 2;
            if (!work.intended && std::any_of(know.hunches.begin(), know.hunches.end(), [&](const auto& hunch) {
                    return hunch.last_use == c.now() && hunch.action == work.action;
                }))
                work.route = 3;
            work.start = c.now();
            work.active_start = c.now();
            work.try_seconds = chosen.duration;
            work.unit_mass = chosen.unit;
            work.goal_mass = chosen.goal;
            work.target = raw.get<world::Place>(h).at;
            work.inputs = std::move(chosen.inputs);
            if (work.action == 12)
                if (const auto spot = FireRules::cooking_spot(c.world(), h, c.now())) work.target = *spot;
            return continue_work(living, c, h);
        };
    };
    for (auto& option : options)
        option.reason.need_met = static_cast<std::uint8_t>(
            option.reason.need < 3 ? life.decision_needs[option.reason.need] : know->curiosity_need);
    if (proposals) {
        for (auto& option : options) {
            const auto reason = option.reason;
            proposals->add(reason, make_commit(std::move(option)));
        }
        return false;
    }
    const auto choice = Choices::keep(c, h, options[0].reason, std::move(rejected));
    return make_commit(std::move(options[0]))(choice);
}
bool Crafting::continue_work(Living& living, world::Context& c, world::Beings::Handle h) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* pending = raw.try_get<Work>(h);
    if (!pending || pending->state == 0 || pending->state == 3) return false;
    auto& work = *pending;
    const auto person = w.beings().id_of(h);
    const auto home = raw.get<Home>(h).camp;
    Choices::restore(w, h, work.choice);
    // Paused shared practice must return through Learning::choose, which revalidates both people.
    // A learner finishing another activity cannot restart the teacher's unfinished drink or rest.
    if (work.lesson != 0) {
        const auto& lessons = raw.get<world::Lessons>(w.beings().handle(home));
        const auto session = std::find_if(lessons.sessions.begin(), lessons.sessions.end(),
                                          [&](const auto& s) { return s.id == work.lesson; });
        if (session == lessons.sessions.end() || session->state == 2) return false;
    }
    const auto here = raw.get<world::Place>(h).at;
    auto& life = raw.get<world::Life>(h);
    life.portion = 0;
    for (auto& r : work.inputs) {
        if (r.picked) continue;
        const auto at = place(w, r.item);
        const auto path = Living::route(w, home, here, at);
        if (path.empty()) {
            release(c, h);
            return false;
        }
        if (here != at) {
            const auto length = w.torus().distance(here, path.front()) * 10;
            living.begin(c, h, LivingAct::carry, (length + living.rules_.loaded_speed - 1) / living.rules_.loaded_speed,
                         path.front());
            return true;
        }
        auto item = value(w, r.item);
        if (item.mass < r.mass || (item.owner.value != 0 && item.owner != person)) {
            release(c, h);
            return false;
        }
        if (!r.retained && r.mass < item.mass) {
            // Physically collected portions get their own identity; other reservations stay on the source stock.
            mutable_item(w, r.item).mass -= r.mass;
            c.item_changed(r.item);
            item.mass = r.mass;
            item.parents = {{r.item}};
            item.made_at = c.now();
            item.maker = person;
            item.owner = person;
            r.item = born(c, item, here);
        } else
            mutable_item(w, r.item).owner = person;
        r.picked = 1;
        c.item_changed(r.item);
    }
    if (here != work.target) {
        const auto path = Living::route(w, home, here, work.target);
        if (path.empty()) {
            release(c, h);
            return false;
        }
        const auto length = w.torus().distance(here, path.front()) * 10;
        living.begin(c, h, LivingAct::carry, (length + living.rules_.loaded_speed - 1) / living.rules_.loaded_speed,
                     path.front());
        return true;
    }
    if (work.action == 12)
        for (const auto& input : work.inputs) FireRules::food_intent(c, input.item, person, work.intended != 0);
    work.state = 2;
    work.active_start = c.now();
    const auto remaining = std::max<std::int64_t>(1, work.try_seconds - work.retained_progress);
    const auto tries = std::max<std::int64_t>(
        1, (work.goal_mass + work.unit_mass - 1) / work.unit_mass - static_cast<std::int64_t>(work.completed_tries));
    work.end = c.now() + std::min<std::int64_t>(3600, remaining + (tries - 1) * work.try_seconds);
    work.next_try = c.now() + remaining;
    if (work.intended && work.next_try < work.end) c.schedule(person, 2, work.next_try);
    if (work.lesson != 0) Learning::started(living, c, h);
    living.begin(c, h, LivingAct::craft, work.end - c.now(), here);
    return true;
}
void Crafting::settle(Living& living, world::Context& c, world::Beings::Handle h, bool interrupted, bool try_event) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto& work = raw.get<Work>(h);
    const auto person = w.beings().id_of(h);
    if (work.state != 2) {
        if (interrupted && work.state == 1) release(c, h);
        return;
    }
    Learning::observe_maker(c, h);
    if (std::any_of(work.inputs.begin(), work.inputs.end(),
                    [&](const auto& r) { return w.things().raw().all_of<world::Fire>(w.things().handle(r.item)); })) {
        release(c, h);
        return;
    }
    if (interrupted) {
        c.cancel(person, 2);
        if (work.action == 2)
            release(c, h);  // cancelled strikes have no physical result or hidden roll
        else {
            work.retained_progress = std::min(work.try_seconds, work.retained_progress + c.now() - work.active_start);
            work.state = 4;
        }
        return;
    }
    if (work.intended && w.catalogue().kind<data::Blueprint>()[work.recipe].heat > 0) {
        for (const auto& input : work.inputs) FireRules::food_changed(c, input.item);
        release(c, h);
        return;
    }
    if (work.intended && work.applied_marker == work.completed_tries && c.now() >= work.next_try &&
        work.retained_progress + c.now() - work.active_start >= work.try_seconds) {
        const auto ids = input_ids(work);
        const auto& b = w.catalogue().kind<data::Blueprint>()[work.recipe];
        auto roles = matching(w, b, ids);
        if (roles.empty()) {
            Discovery::memory(c, h, work.action, Discovery::handling(c, h, work.action, ids), 0);
            release(c, h);
            return;
        }
        ++work.completed_tries;
        work.applied_marker = work.completed_tries;
        resolve(c, h, work.recipe, std::move(roles), false, work.route, work.completed_tries);
        work.retained_progress = 0;
        work.active_start = c.now();
        work.next_try = std::min(work.end, c.now() + work.try_seconds);
    }
    const bool exhausted =
        std::any_of(work.inputs.begin(), work.inputs.end(), [](const auto& r) { return r.mass == 0; });
    if (try_event && !exhausted && c.now() < work.end) {
        if (work.next_try < work.end)
            c.schedule(person, 2, work.next_try);
        else
            c.cancel(person, 2);
        // Body settlement advanced LIFE; restart the remaining interval without another recipe/unknown draw.
        living.begin(c, h, LivingAct::craft, work.end - c.now(), raw.get<world::Place>(h).at);
        return;
    }
    if (work.lesson != 0 && !exhausted && work.retained_progress + c.now() - work.active_start < work.try_seconds) {
        work.retained_progress += c.now() - work.active_start;
        work.state = 4;
        c.cancel(person, 2);
        return;
    }
    if (!work.rolled) {
        work.rolled = 1;
        const auto ids = input_ids(work);
        const auto& recipes = w.catalogue().kind<data::Blueprint>();
        const auto& know = raw.get<world::Knowledge>(h);
        const auto memories_before = know.next_memory;
        bool noticed_success = false;
        for (std::uint32_t r = 0; r < recipes.size(); ++r) {
            if ((work.lesson != 0 && r == work.recipe) || recipes[r].action != work.action || recipes[r].heat > 0 ||
                std::any_of(know.skills.begin(), know.skills.end(),
                            [&](const auto& s) { return s.recipe == r && s.known; }))
                continue;
            auto roles = matching(w, recipes[r], ids);
            if (roles.empty()) continue;
            bool tool = true;
            std::int64_t edge = 0;
            for (std::size_t role = 0; role < roles.size(); ++role) {
                if (recipes[r].inputs[role].optional && !roles[role].value) tool = false;
                if (role && roles[role].value)
                    edge = std::max(edge, characteristics(w.catalogue(), value(w, roles[role]))[1]);
            }
            if (static_cast<std::int64_t>(work.completed_tries) * work.try_seconds + work.retained_progress + c.now() -
                    work.active_start <
                duration(recipes[r], edge, tool))
                continue;
            resolve(c, h, r, std::move(roles), true, work.intended ? 1 : work.route, 0);
            if (std::any_of(know.skills.begin(), know.skills.end(),
                            [&](const auto& s) { return s.recipe == r && s.known; }))
                noticed_success = true;
        }
        if (work.route == 3 && !noticed_success) {
            auto& hunches = raw.get<world::Knowledge>(h).hunches;
            for (auto& hunch : hunches)
                if (hunch.action == work.action && hunch.last_use == work.start) ++hunch.failures;
            std::erase_if(hunches, [](const auto& hunch) { return hunch.failures >= 10; });
        }
        if (!work.intended && know.next_memory == memories_before)
            Discovery::memory(c, h, work.action, Discovery::handling(c, h, work.action, ids), 0);
    }
    release(c, h);
}
bool Crafting::meal(Living& living, world::Context& c, world::Beings::Handle h) {
    const auto& life = c.world().beings().raw().get<world::Life>(h);
    if (life.carried_food != 0 || life.meal_item.value != 0) return false;
    const Decision decision(c, h);
    return meal(living, c, h, decision);
}
bool Crafting::meal(Living& living, world::Context& c, world::Beings::Handle h, const Decision& decision,
                    ChoiceSet* proposals) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto& life = raw.get<world::Life>(h);
    if (life.carried_food != 0 || life.meal_item.value != 0) return false;
    const auto& work = raw.get<Work>(h);
    for (const auto& input : decision.inputs.all) {
        const auto id = input.id;
        if (std::any_of(work.inputs.begin(), work.inputs.end(),
                        [&](const auto& reserved) { return reserved.item == id; }))
            continue;
        const auto& item = value(w, id);
        const auto* f = Discovery::familiar(raw.get<world::Knowledge>(h), item);
        auto raw_food = item;
        raw_food.state = 0;
        const auto* raw_familiar = Discovery::familiar(raw.get<world::Knowledge>(h), raw_food);
        const bool cooked_familiar = item.state == 1 && raw_familiar && raw_familiar->edible;
        if ((!f || !f->edible || (f->mask & (1U << 8U)) == 0 || f->values[8] == 0) && !cooked_familiar) continue;
        const auto portion = std::min<std::int64_t>(input.available, 1000000);
        if (proposals) {
            auto reason = Choices::body(life, 0);
            reason.inputs = {{id}};
            reason.unavailable = 0;
            reason.seconds = living.uses_[0].use.game +
                             w.torus().distance(raw.get<world::Place>(h).at, place(w, id)) * 10 / living.rules_.speed;
            const auto food_value = f && f->edible ? f->values[8] : raw_familiar->values[8];
            reason.benefit = std::min<std::int64_t>(100 - life.decision_needs[0], portion * food_value * 50 / 4000000);
            reason.score =
                std::max<std::int64_t>(0, 80 - life.decision_needs[0]) * reason.benefit * 10 - reason.seconds / 60;
            reason.parts = {std::max<std::int64_t>(0, 80 - life.decision_needs[0]) * reason.benefit * 10, 0,
                            -reason.seconds / 60};
            proposals->add(
                reason, [&living, &c, h, id, portion](std::uint64_t) { return start_meal(living, c, h, id, portion); });
            return false;
        }
        return start_meal(living, c, h, id, portion);
    }
    return false;
}
bool Crafting::start_meal(Living& living, world::Context& c, world::Beings::Handle h, ecs::Id id,
                          std::int64_t portion) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto& life = raw.get<world::Life>(h);
    const auto here = raw.get<world::Place>(h).at;
    life.goal = 0;
    life.use_at = place(w, id);
    life.meal_item = id;
    life.carried_food = portion;
    if (life.use_at != here) {
        const auto path = Living::route(w, raw.get<Home>(h).camp, here, life.use_at);
        const auto length = w.torus().distance(here, path.front()) * 10;
        life.portion = 0;
        living.begin(c, h, LivingAct::walk, (length + living.rules_.speed - 1) / living.rules_.speed, path.front());
        return true;
    }
    const auto actual = characteristics(w.catalogue(), value(w, id));
    life.food_factor_ppm = actual[8] * 500000;
    life.water_ml_per_kg = actual[9] * 200;
    life.portion = portion;
    living.begin(c, h, LivingAct::eat, living.uses_[0].use.game, here);
    return true;
}

void Crafting::settle_meal(world::Context& c, world::Beings::Handle h, std::int64_t eaten, bool finished) {
    auto& raw = c.world().beings().raw();
    auto& life = raw.get<world::Life>(h);
    if (life.meal_item.value == 0) return;
    auto& item = mutable_item(c.world(), life.meal_item);
    KD_CHECK(eaten <= item.mass, "A reserved finite meal cannot be eaten twice");
    if (eaten > 0) Discovery::learn(c, h, life.meal_item, (1U << 8U) | (1U << 9U), 3, true);
    item.mass -= eaten;
    spent(item);
    c.item_changed(life.meal_item);
    if (eaten > 0) {
        raw.get<world::CraftHistory>(c.world().beings().handle(raw.get<Home>(h).camp))
            .routine.record({0, c.now() / time::kDay * time::kDay, c.world().beings().id_of(h), 1, 0, item.kind,
                             item.kind, 0, 0, 0, 0, eaten});
        c.record(202, c.world().beings().id_of(h).value, static_cast<std::uint64_t>(eaten));
    }
    if (!finished) return;
    life.carried_food = 0;
    life.meal_item = {};
    life.food_factor_ppm = 1000000;
    life.water_ml_per_kg = 800;
}
}  // namespace kd::demo
