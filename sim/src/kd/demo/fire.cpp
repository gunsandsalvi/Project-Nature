#include "kd/demo/fire.hpp"
#include "kd/chance/chance.hpp"
#include "kd/demo/choice.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
namespace kd::demo {
namespace {
std::uint32_t required_entry(const kd::data::Catalogue& catalogue, std::string_view folder, std::string_view name) {
    const auto found = catalogue.find(folder, name);
    KD_CHECK(found.has_value(), "Fire requires its validated catalogue entry");
    return found.value_or(0);
}
using world::Fire;
using world::Item;
std::int64_t rate(const Fire& f) {
    return f.heat >= 3 ? 5000000 : f.heat == 2 ? 1000000 : 0;
}
time::Seconds next_fire(const Fire& f) {
    return f.deadline();
}
Item& item(world::World& w, ecs::Id id) {
    return w.things().raw().get<Item>(w.things().handle(id));
}
Fire& fire(world::World& w, ecs::Id id) {
    return w.things().raw().get<Fire>(w.things().handle(id));
}
num::Point at(const world::World& w, ecs::Id id, time::Seconds now) {
    const auto& i = w.things().raw().get<Item>(w.things().handle(id));
    if (i.owner.value) {
        const auto h = w.beings().handle(i.owner);
        return w.beings().raw().get<world::Activity>(h).at(w.torus(), now);
    }
    return w.things().raw().get<world::Place>(w.things().handle(id)).at;
}
// Dimensions and classes are visible; characteristic estimates come only from personal evidence.
data::FitInput perceived_input(const data::Catalogue& catalogue, const Item& item, const world::Familiar& memory) {
    const auto& kind = catalogue.kind<data::ItemKind>()[item.kind];
    const auto& material = catalogue.kind<data::ItemKind>()[item.material];
    data::FitInput out{kind.inherit ? std::string_view(material.material_class) : std::string_view(kind.material_class),
                       kind.form,
                       {},
                       item.length,
                       item.mass};
    for (std::size_t p = 0; p < out.values.size(); ++p)
        if ((memory.mask & (1U << p)) && memory.certainty[p]) out.values[p] = memory.values[p];
    return out;
}
std::uint8_t confidence(const world::Familiar& memory, std::initializer_list<std::size_t> properties) {
    std::uint8_t out = 100;
    for (const auto p : properties) {
        if (!(memory.mask & (1U << p))) return 0;
        out = std::min(out, memory.certainty[p]);
    }
    return out;
}
// Known maintenance is selected by the action/heat affordance, never a catalogue name.
std::optional<std::uint32_t> maintenance_recipe(const world::World& w, world::Beings::Handle person,
                                                std::uint8_t operation, const data::FitInput* input = nullptr) {
    const auto& know = w.beings().raw().get<world::Knowledge>(person);
    const auto& recipes = w.catalogue().kind<data::Blueprint>();
    for (std::uint32_t r = 0; r < recipes.size(); ++r) {
        const auto& b = recipes[r];
        if (b.heat == 1 && b.action == (operation == 3 ? 16 : 0) && b.inputs.size() == 1 && Learning::knows(know, r) &&
            (!input || data::fits(b.inputs[0], *input)))
            return r;
    }
    return {};
}
void clear_tend(world::Thermal& t) {
    t.tending = t.tending_phase = t.tending_shared = 0;
    t.tending_fire = t.tending_input = {};
    t.tending_mass = t.tending_started = 0;
    t.tending_choice = 0;
}
void release_input(world::Context& c, ecs::Id input, num::Point here) {
    auto& w = c.world();
    item(w, input).owner = {};
    w.things().raw().get<world::Place>(w.things().handle(input)).at = here;
    // A carried portion may have physically ignited while waiting beside the hearth.
    if (auto* f = w.things().raw().try_get<Fire>(w.things().handle(input))) {
        f->owner = {};
        f->at = here;
    }
    c.item_changed(input);
}
void schedule_world(world::World& w, ecs::Id camp) {
    auto next = w.beings().raw().get<world::Ambient>(w.beings().handle(camp)).next;
    w.things().each([&](ecs::Id, auto h) {
        const auto* f = w.things().raw().try_get<Fire>(h);
        if (f && f->hearth == camp && f->next) next = std::min(next, f->next);
    });
    w.schedule(camp, 2, next);
}
}  // namespace
bool FireRules::task_light(const world::World& w, ecs::Id camp, num::Point from, num::Point to, time::Seconds now) {
    for (const auto h : w.things().raw().view<Fire>()) {
        const auto& f = w.things().raw().get<Fire>(h);
        if (f.hearth != camp || f.heat < 2) continue;
        const auto here = at(w, w.things().id_of(h), now);
        if (w.torus().squared_distance(here, from) <= 200LL * 200 &&
            w.torus().squared_distance(here, to) <= 200LL * 200 && Living::visible(w, camp, here, from) &&
            Living::visible(w, camp, here, to))
            return true;
    }
    return false;
}
std::int64_t FireRules::ambient(time::Seconds at) {
    const auto clock = at % time::kDay;
    return clock >= 6 * time::kHour && clock < 20 * time::kHour ? 24000 : 18000;
}
time::Seconds FireRules::next_ambient(time::Seconds at) {
    const auto midnight = at - at % time::kDay;
    for (const auto candidate : {midnight + 6 * time::kHour, midnight + 20 * time::kHour})
        if (candidate > at) return candidate;
    return midnight + time::kDay + 6 * time::kHour;
}
void FireRules::start(world::World& w) {
    auto& raw = w.beings().raw();
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (raw.all_of<world::Camp, world::Lessons>(h) && !raw.all_of<world::Ambient>(h)) {
            const auto next = next_ambient(w.frontier());
            raw.emplace<world::Ambient>(h, ambient(w.frontier()), next);
            w.schedule(id, 2, next);
        }
        if (raw.all_of<world::Person, world::Knowledge>(h) && !raw.all_of<world::Thermal>(h)) {
            auto& t = raw.emplace<world::Thermal>(h);
            t.felt_milli_c = ambient(w.frontier());
            t.warmth = 100 - 5 * (24000 - t.felt_milli_c) / 1000;
            t.settled_at = w.frontier();
        }
    });
}
void FireRules::fresh_hearth(world::World& w, bool already_out) {
    w.beings().each([&](ecs::Id camp, auto ch) {
        if (!w.beings().raw().all_of<world::Ambient>(ch)) return;
        const auto here = w.beings().raw().get<world::Camp>(ch).shelter_at;
        std::int64_t wanted = 5000000;
        std::vector<world::Link> parents;
        std::uint32_t kind = 0;
        w.things().each([&](ecs::Id id, auto h) {
            auto& source = w.things().raw().get<Item>(h);
            const auto fit = Crafting::physical(w.catalogue(), source);
            if (!wanted || source.home != camp || source.mass < 625000 || fit.material_class != "wood" ||
                fit.values[9] > 1 || fit.values[6] < 2)
                return;
            const auto taken = std::min(wanted, source.mass);
            if (!taken) return;
            kind = source.kind;
            source.mass -= taken;
            if (!source.mass) source.state = 4;
            wanted -= taken;
            parents.push_back({id});
        });
        KD_CHECK(wanted == 0, "Fresh lightning fire draws five kilograms from finite dry wood");
        const auto h = w.make_thing();
        w.things().raw().emplace<world::Place>(h, here);
        auto& i = w.things().raw().emplace<Item>(h);
        i.home = camp;
        i.kind = i.material = kind;
        i.mass = 5000000;
        i.length = 500;
        i.parents = std::move(parents);
        i.made_at = w.frontier();
        i.state = 3;
        i.changed_mask = 1U << 12U;
        i.changed[12] = 3;
        auto& f = w.things().raw().emplace<Fire>(h);
        f.hearth = camp;
        f.at = here;
        f.heat = 3;
        f.fuel_mg = i.mass;
        f.settled_at = w.frontier();
        f.air_until = w.frontier() + 1;
        if (already_out) {
            f.heat = 0;
            f.fuel_mg = 0;
            f.ash_mg = i.mass;
            f.air_until = 0;
            i.kind = i.material = required_entry(w.catalogue(), "item", "base:ash");
            i.state = 0;
            i.changed[12] = 0;
        }
        f.next = next_fire(f);
        schedule_world(w, camp);
    });
}
void FireRules::ember(world::Context& c, ecs::Id id) {
    auto& w = c.world();
    const auto h = w.things().handle(id);
    if (w.things().raw().all_of<Fire>(h)) return;
    const auto i = item(w, id);
    auto& f = w.things().raw().emplace<Fire>(h);
    f.hearth = i.home;
    f.owner = i.owner;
    f.at = w.things().raw().get<world::Place>(h).at;
    f.origin = id;
    f.ignited_at = c.now();
    f.heat = 1;
    f.ring = 0;
    f.unblown_checked = 0;
    f.fuel_mg = i.mass;
    f.settled_at = c.now();
    f.air_until = c.now() + 180;
    f.embers_until = c.now() + 3 * time::kHour;
    f.next = next_fire(f);
    c.item_changed(id);
    deadlines(c, f.hearth);
}
void FireRules::settle_fire(world::Context& c, ecs::Id id) {
    auto& w = c.world();
    thermal_before(c, fire(w, id).hearth);
    auto& f = fire(w, id);
    const auto old_heat = f.heat;
    const auto elapsed = c.now() - f.settled_at;
    const auto units = rate(f) * elapsed + f.burn_remainder;
    const auto burned = std::min(f.fuel_mg, units / time::kHour);
    f.fuel_mg -= burned;
    f.ash_mg += burned;
    f.burn_remainder = f.fuel_mg ? units % time::kHour : 0;
    f.settled_at = c.now();
    if (f.owner.value) {
        f.at = at(w, id, c.now());
        w.things().raw().get<world::Place>(w.things().handle(id)).at = f.at;
    }
    if (f.heat >= 2 && !f.fuel_mg) {
        f.heat = 1;
        f.air_until = 0;
        f.embers_until = c.now() + 3 * time::kHour;
    }
    if (f.heat == 1 && f.air_until && f.air_until <= c.now()) {
        if (!f.unblown_checked) {
            const chance::Draws draws(w.seed(), chance::name("unblown ember"), id.value, item(w, id).made_at,
                                      chance::name("death"));
            if (draws.below(0, 2) == 0) f.heat = 0;
            f.unblown_checked = 1;
        } else if (f.fuel_mg > 1000) {
            f.heat = 2;
        }
        f.air_until = 0;
        if (f.heat == 2) {
            const auto values = Crafting::characteristics(w.catalogue(), item(w, id));
            if (values[7] >= 2 && values[6] <= 3) f.air_until = c.now() + 600;
        }
    }
    if (f.heat >= 2 && f.air_until && f.air_until <= c.now()) {
        f.heat = 3;
        f.air_until = 0;
    }
    if (f.heat >= 2 && f.owner.value) {
        f.owner = {};
        item(w, id).owner = {};
    }
    if (f.heat == 1 && std::max(f.embers_until, f.banked_until) <= c.now()) f.heat = 0;
    f.next = next_fire(f);
    if (!f.heat) f.next = 0;
    if (!f.heat && f.fuel_mg) {
        // Putting out leaves real charcoal. The spent hearth retains its conserved ash.
        auto charcoal = item(w, id);
        charcoal.kind = charcoal.material = required_entry(w.catalogue(), "item", "base:charcoal");
        charcoal.mass = f.fuel_mg;
        charcoal.parents = {{id}};
        charcoal.made_at = c.now();
        charcoal.maker = {};
        charcoal.changed_mask = 0;
        charcoal.changed.fill(0);
        charcoal.state = 0;
        f.fuel_mg = 0;
        item(w, id).mass = f.ash_mg;
        const auto h = w.make_thing();
        w.things().raw().emplace<Item>(h, charcoal);
        w.things().raw().emplace<world::Place>(h, f.at);
        c.item_changed(w.things().id_of(h));
    }
    if (!f.fuel_mg) {
        auto& remains = item(w, id);
        remains.kind = remains.material = required_entry(w.catalogue(), "item", "base:ash");
        remains.state = remains.mass ? 0 : 4;
        remains.changed_mask = 0;
        remains.changed.fill(0);
    }
    if (old_heat != f.heat) c.record(219, id.value, f.heat);
    thermal_after(c, f.hearth);
    c.item_changed(id);
}
bool FireRules::feed(world::Context& c, ecs::Id hearth, ecs::Id input, std::int64_t mass, ecs::Id person) {
    auto& w = c.world();
    const auto live_input = w.things().find(input);
    if (!live_input || input == hearth || mass <= 0 || w.things().raw().all_of<Fire>(*live_input) ||
        Crafting::available(w, input, person) < mass)
        return false;
    const auto source = item(w, input);
    if (source.owner.value && source.owner != person) return false;
    const auto fit = Crafting::physical(w.catalogue(), source);
    if (w.torus().squared_distance(at(w, input, c.now()), at(w, hearth, c.now())) > 10000) return false;
    settle_fire(c, hearth);
    auto& f = fire(w, hearth);
    if (!f.heat) return false;
    if (fit.values[9] >= 3) {
        f.damp_remainder += mass;
        f.evaporated_mg += mass;
        f.heat = static_cast<std::uint8_t>(std::max<std::int64_t>(0, f.heat - f.damp_remainder / 1000000));
        f.damp_remainder %= 1000000;
    } else {
        if (fit.values[9] > (f.heat == 1 ? 1 : 2) || fit.values[6] < (f.heat == 1 ? 4 : 2)) return false;
        f.fuel_mg += mass;
        item(w, hearth).mass += mass;
        if (f.heat >= 2 && fit.values[7] >= 2 && fit.values[6] <= 3) f.air_until = c.now() + 600;
    }
    item(w, input).mass -= mass;
    if (!item(w, input).mass) item(w, input).state = 4;
    f.next = f.heat ? next_fire(f) : 0;
    if (!f.heat) settle_fire(c, hearth);
    thermal_after(c, f.hearth);
    c.item_changed(input);
    c.item_changed(hearth);
    deadlines(c, f.hearth);
    return true;
}
bool FireRules::blow(world::Context& c, ecs::Id hearth) {
    settle_fire(c, hearth);
    auto& f = fire(c.world(), hearth);
    if (f.heat != 1 || f.fuel_mg <= 1000) return false;
    // The caller has physically fitted dry burn-4/5 tinder before blowing.
    f.unblown_checked = 1;
    f.banked_until = 0;
    f.embers_until = c.now() + 3 * time::kHour;
    f.air_until = c.now() + 60;
    f.next = next_fire(f);
    c.world().physical_changed(hearth);
    deadlines(c, f.hearth);
    return true;
}
bool FireRules::bank(world::Context& c, ecs::Id hearth) {
    settle_fire(c, hearth);
    auto& f = fire(c.world(), hearth);
    if (!f.heat || f.ash_mg < 100000 || f.banked_until) return false;
    f.heat = 1;
    f.unblown_checked = 1;
    f.air_until = 0;
    f.banked_until = c.now() + 12 * time::kHour;
    f.embers_until = f.banked_until;
    f.next = next_fire(f);
    c.world().physical_changed(hearth);
    thermal_after(c, f.hearth);
    deadlines(c, f.hearth);
    return true;
}
ecs::Id FireRules::carry(world::Context& c, ecs::Id hearth, ecs::Id input, ecs::Id person) {
    auto& w = c.world();
    const auto live_input = w.things().find(input);
    if (!live_input || w.things().raw().all_of<Fire>(*live_input)) return {};
    const auto source = item(w, input);
    const auto fit = Crafting::physical(w.catalogue(), source);
    if ((fit.material_class != "wood" && fit.material_class != "plant") || fit.values[6] < 4 || fit.values[9] > 1 ||
        fit.values[7] < 1 || fit.values[7] > 2 || Crafting::available(w, input, person) < 100000 ||
        (source.owner.value && source.owner != person) ||
        w.torus().squared_distance(at(w, input, c.now()), at(w, hearth, c.now())) > 10000)
        return {};
    settle_fire(c, hearth);
    auto f = fire(w, hearth);
    if (!f.heat || f.fuel_mg + f.ash_mg < 1000) return {};
    const auto fuel = std::min<std::int64_t>(1000, f.fuel_mg);
    fire(w, hearth).fuel_mg -= fuel;
    fire(w, hearth).ash_mg -= 1000 - fuel;
    item(w, hearth).mass -= 1000;
    item(w, input).mass -= 100000;
    if (!item(w, input).mass) item(w, input).state = 4;
    if (!item(w, hearth).mass) {
        item(w, hearth).state = 4;
        fire(w, hearth).heat = 0;
    }
    auto result = source;
    result.mass = 101000;
    result.owner = result.maker = person;
    result.made_at = c.now();
    result.parents = {{hearth}, {input}};
    const auto h = w.make_thing();
    const auto id = w.things().id_of(h);
    w.things().raw().emplace<Item>(h, result);
    w.things().raw().emplace<world::Place>(h, at(w, hearth, c.now()));
    f.source = hearth;
    f.ignited_at = c.now();
    f.owner = person;
    f.at = at(w, hearth, c.now());
    f.heat = 1;
    f.ring = 0;
    f.fuel_mg = 100000 + fuel;
    f.ash_mg = 1000 - fuel;
    f.unblown_checked = 1;
    f.air_until = 0;
    f.settled_at = c.now();
    f.burn_remainder = 0;
    f.embers_until = f.banked_until = c.now() + time::kDay;
    f.next = next_fire(f);
    f.evaporated_mg = f.damp_remainder = 0;
    w.things().raw().emplace<Fire>(h, f);
    settle_fire(c, hearth);
    c.item_changed(input);
    c.item_changed(hearth);
    c.item_changed(id);
    deadlines(c, f.hearth);
    return id;
}
void FireRules::deadlines(world::Context& c, ecs::Id camp) {
    auto& w = c.world();
    const auto ch = w.beings().handle(camp);
    const auto* ambient = w.beings().raw().try_get<world::Ambient>(ch);
    if (!ambient) return;
    auto heat = ambient->next;
    time::Seconds timer = 0;
    const auto& physical = std::as_const(w).physical_items(camp);
    if (!physical.fire_due.empty()) heat = std::min(heat, physical.fire_due.begin()->first);
    if (!physical.food_due.empty()) timer = physical.food_due.begin()->first;
    c.schedule(camp, 2, std::max(c.now() + 1, heat));
    if (timer)
        c.schedule(camp, 3, std::max(c.now() + 1, timer));
    else
        c.cancel(camp, 3);
}
void FireRules::handle(world::Context& c, ecs::Id camp, std::uint32_t slot) {
    auto& w = c.world();
    thermal_before(c, camp);
    auto& a = w.beings().raw().get<world::Ambient>(w.beings().handle(camp));
    if (slot == 2 && a.next <= c.now()) {
        a.milli_c = ambient(c.now());
        a.next = next_ambient(c.now());
    }
    std::vector<ecs::Id> due;
    for (const auto& [when, ids] : std::as_const(w).physical_items(camp).fire_due) {
        if (when > c.now()) break;
        due.insert(due.end(), ids.begin(), ids.end());
    }
    std::stable_sort(due.begin(), due.end());
    for (const auto id : due) settle_fire(c, id);
    std::vector<std::pair<ecs::Id, ecs::Id>> ignite;
    const auto candidates = std::as_const(w).physical_items(camp).fuel;
    for (const auto& entry : candidates) {
        const auto id = entry.id;
        const auto h = entry.handle;
        const auto& source = w.things().raw().get<Item>(h);
        if (source.home != camp || source.mass == 0 || w.things().raw().any_of<Fire, world::HeatTimer>(h)) continue;
        const auto fit = Crafting::physical(w.catalogue(), source);
        if (fit.values[6] < 2 || fit.values[9] >= 3) continue;
        for (const auto flame : due) {
            if (fire(w, flame).heat >= 2 &&
                w.torus().squared_distance(at(w, id, c.now()), at(w, flame, c.now())) <= 10000) {
                ignite.emplace_back(id, flame);
                break;
            }
        }
    }
    for (const auto& [id, source] : ignite) {
        const auto i = item(w, id);
        auto& f = w.things().raw().emplace<Fire>(w.things().handle(id));
        f.hearth = camp;
        f.origin = fire(w, source).origin;
        f.source = source;
        f.ignited_at = c.now();
        f.tended_at = fire(w, source).tended_at;
        f.owner = i.owner;
        f.at = w.things().raw().get<world::Place>(w.things().handle(id)).at;
        f.heat = 2;
        f.ring = 0;
        f.fuel_mg = i.mass;
        f.settled_at = c.now();
        f.next = next_fire(f);
        c.item_changed(id);
    }
    thermal_after(c, camp);
    food_refresh(c, camp);
    deadlines(c, camp);
}
bool FireRules::choose(Living& living, world::Context& c, world::Beings::Handle h, ChoiceSet* proposals) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* t = raw.try_get<world::Thermal>(h);
    if (!t || t->tending || raw.get<world::Work>(h).state) return false;
    const auto& l = raw.get<world::Life>(h);
    if (*std::min_element(l.decision_needs.begin(), l.decision_needs.end()) < 20) return false;
    const auto person = w.beings().id_of(h), camp = raw.get<Home>(h).camp;
    const auto here = raw.get<world::Place>(h).at;
    const auto sight_clock = c.now() % time::kDay;
    const std::int64_t sight_range = sight_clock >= 6 * time::kHour && sight_clock < 20 * time::kHour ? 3000 : 500;
    ecs::Id target{};
    for (const auto th : w.things().raw().view<Fire>()) {
        const auto& f = w.things().raw().get<Fire>(th);
        const auto id = w.things().id_of(th);
        if (!f.heat || f.hearth != camp || (f.owner.value && f.owner != person) || (target.value && target < id) ||
            w.torus().squared_distance(here, at(w, id, c.now())) > sight_range * sight_range ||
            !Living::visible(w, camp, here, at(w, id, c.now())))
            continue;
        target = id;
    }
    if (!target.value) return false;
    const auto f = fire(w, target);
    std::uint8_t operation = 0;
    ecs::Id input{};
    std::int64_t mass = 0, score = 0;
    const auto clock = c.now() % time::kDay;
    auto& know = raw.get<world::Knowledge>(h);
    bool hypothesis = false;
    if (clock >= 20 * time::kHour && f.heat >= 2 && f.ash_mg >= 100000 && maintenance_recipe(w, h, 3)) {
        operation = 3;
        score = 120;
    } else if ((f.heat == 1 || f.fuel_mg < 2500000) &&
               (f.banked_until <= c.now() || (clock >= 6 * time::kHour && clock < 20 * time::kHour))) {
        w.things().each([&](ecs::Id id, auto th) {
            const auto& source = w.things().raw().get<Item>(th);
            if (input.value || source.home != camp || (source.owner.value && source.owner != person) ||
                w.things().raw().all_of<Fire>(th) || Crafting::available(w, id, person) < 100000)
                return;
            const auto* remembered = Discovery::familiar(know, source);
            if (!remembered || !confidence(*remembered, {6, 9})) return;
            const auto fit = perceived_input(w.catalogue(), source, *remembered);
            if (fit.values[9] > (f.heat == 1 ? 1 : 2) || fit.values[6] < (f.heat == 1 ? 4 : 2) ||
                w.torus().squared_distance(here, at(w, id, c.now())) > 25000000 ||
                !Living::visible(w, camp, here, at(w, id, c.now())))
                return;
            input = id;
            mass = std::min<std::int64_t>(1000000, Crafting::available(w, id, person));
        });
        if (input.value) {
            operation = f.heat == 1 ? 2 : 1;
            const auto* evidence = Discovery::familiar(know, item(w, input));
            score = 150 * confidence(*evidence, {6, 9}) / 100;
        }
    }
    if (!operation && f.heat == 1 && maintenance_recipe(w, h, 4)) {
        const chance::Draws draws(w.seed(), chance::name("carry ember choice"), person.value, c.now() / 3600,
                                  chance::name("container"));
        if (draws.below(0, 24) == 0)
            w.things().each([&](ecs::Id id, auto th) {
                const auto& source = w.things().raw().get<Item>(th);
                const auto* remembered = Discovery::familiar(know, source);
                if (!remembered || !confidence(*remembered, {6, 7, 9})) return;
                const auto fit = perceived_input(w.catalogue(), source, *remembered);
                if (!input.value && source.home == camp && !w.things().raw().all_of<Fire>(th) &&
                    (!source.owner.value || source.owner == person) && maintenance_recipe(w, h, 4, &fit) &&
                    fit.values[6] >= 4 && fit.values[7] >= 1 && fit.values[7] <= 2 && fit.values[9] <= 1 &&
                    Crafting::available(w, id, person) >= 100000 &&
                    w.torus().squared_distance(here, at(w, id, c.now())) <= 25000000 &&
                    Living::visible(w, camp, here, at(w, id, c.now())))
                    input = id;
            });
        if (input.value) {
            operation = 4;
            mass = 100000;
            const auto* evidence = Discovery::familiar(know, item(w, input));
            score = 80 * confidence(*evidence, {6, 7, 9}) / 100;
        }
    }
    // A visible failing heat source gives a goal for a personally known maintenance action.
    // Trying an uncertain familiar input is a plan hypothesis, not another hourly random draw.
    // Actual contact below records its burn/wetness outcome; uncertainty never changes ignition.
    const auto hour = static_cast<std::uint64_t>(c.now() / time::kHour + 1);
    if (!operation && maintenance_recipe(w, h, 4) && (f.heat == 1 || f.fuel_mg < 2500000)) {
        std::int64_t closest = INT64_MAX;
        w.things().each([&](ecs::Id id, auto th) {
            const auto& source = w.things().raw().get<Item>(th);
            const auto* remembered = Discovery::familiar(know, source);
            if (!remembered || confidence(*remembered, {6, 9}) || source.home != camp ||
                (source.owner.value && source.owner != person) || w.things().raw().all_of<Fire>(th) ||
                Crafting::available(w, id, person) < 100000 || !(remembered->mask & (1U << 9U)) ||
                remembered->values[9] > (f.heat == 1 ? 1 : 2) ||
                w.torus().squared_distance(here, at(w, id, c.now())) > 25000000 ||
                !Living::visible(w, camp, here, at(w, id, c.now())))
                return;
            auto guess = perceived_input(w.catalogue(), source, *remembered);
            const auto recipe = maintenance_recipe(w, h, 4);
            for (const auto& range : w.catalogue().kind<data::Blueprint>()[recipe.value_or(0)].inputs[0].ranges)
                if (!(remembered->mask & (1U << static_cast<unsigned>(range.characteristic))))
                    guess.values[static_cast<std::size_t>(range.characteristic)] = range.minimum;
            if (!maintenance_recipe(w, h, 4, &guess)) return;
            const auto distance = w.torus().distance(here, at(w, id, c.now())) +
                                  w.torus().distance(at(w, id, c.now()), at(w, target, c.now()));
            if (distance < closest || (distance == closest && id < input)) {
                closest = distance;
                input = id;
            }
        });
        if (input.value) {
            mass = std::min<std::int64_t>(1000000, Crafting::available(w, input, person));
            operation = f.heat == 1 ? 2 : 1;
            hypothesis = true;
            score = 500;
        }
    }
    if (!operation) return false;
    world::CraftReason reason;
    reason.kind = 3;
    reason.action = operation;
    reason.need = hypothesis ? 3 : 4;
    reason.need_met = hypothesis ? know.curiosity_need : static_cast<std::uint8_t>(t->warmth);
    reason.parts = {0, score, 0};
    reason.observed_heat = f.heat;
    reason.observed_fuel_mg = f.fuel_mg;
    const auto distance = input.value ? w.torus().distance(here, at(w, input, c.now())) +
                                            w.torus().distance(at(w, input, c.now()), at(w, target, c.now()))
                                      : w.torus().distance(here, at(w, target, c.now()));
    reason.seconds = (operation == 3 ? 300 : 60) + distance * 10 / living.rules().speed;
    reason.inputs.push_back({target});
    if (input.value) {
        reason.inputs.push_back({input});
        reason.confidence = confidence(
            *Discovery::familiar(know, item(w, input)),
            operation == 4 ? std::initializer_list<std::size_t>{6, 7, 9} : std::initializer_list<std::size_t>{6, 9});
    }
    const auto uncertainty = hypothesis ? (100 - reason.confidence) / 2 : 0;
    reason.parts[2] = -reason.seconds / 60 - uncertainty;
    reason.score = reason.parts[1] + reason.parts[2];
    if (!proposals && reason.score <= l.scores[l.goal]) return false;
    auto commit = [&living, &c, h, target, input, operation, mass, reason, hypothesis, hour](std::uint64_t choice) {
        if (hypothesis) c.world().beings().raw().get<world::Knowledge>(h).hourly_draw = hour;
        auto& thermal = c.world().beings().raw().get<world::Thermal>(h);
        thermal.tending_choice = choice ? choice : Choices::keep(c, h, reason);
        thermal.tending = operation;
        thermal.tending_phase = input.value ? 1 : 2;
        thermal.tending_fire = target;
        thermal.tending_input = input;
        thermal.tending_mass = mass;
        thermal.tending_started = c.now();
        return continue_tending(living, c, h, false);
    };
    if (proposals) {
        proposals->add(reason, std::move(commit));
        return false;
    }
    return commit(0);
}
bool FireRules::continue_tending(Living& living, world::Context& c, world::Beings::Handle h, bool interrupted) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* t = raw.try_get<world::Thermal>(h);
    if (!t || !t->tending) return false;
    Choices::restore(w, h, t->tending_choice);
    const auto person = w.beings().id_of(h);
    const auto target = t->tending_fire, input = t->tending_input;
    if (interrupted || !w.things().find(target) || !fire(w, target).heat) {
        if (input.value && t->tending_shared && w.things().find(input)) {
            release_input(c, input, raw.get<world::Place>(h).at);
        }
        clear_tend(*t);
        return false;
    }
    const auto here = raw.get<world::Place>(h).at;
    if (t->tending_phase == 3) {
        if (t->tending <= 2) {
            // A completed real fuel trial reveals these properties even when it fails.
            Discovery::learn(c, h, input, (1U << 6U) | (1U << 9U), 3);
            const auto observed = *Discovery::familiar(raw.get<world::Knowledge>(h), item(w, input));
            Discovery::memory(c, h, 0, {observed}, 0, target);
            if (feed(c, target, input, t->tending_mass, person)) {
                if (t->tending == 2) (void)blow(c, target);
                fire(w, target).tended_at = c.now();
                c.item_changed(target);
                c.record(
                    218, target.value,
                    static_cast<std::uint64_t>(t->tending) | (static_cast<std::uint64_t>(fire(w, target).heat) << 8U));
            }
            t->tending_mass = 0;
        }
        if (t->tending >= 3) {
            const auto recipe = maintenance_recipe(w, h, t->tending);
            KD_CHECK(recipe.has_value(), "Tending uses a validated known recipe");
            const std::array roles{target};
            const chance::Draws draws(w.seed(), chance::name("tend result"), person.value, t->tending_started,
                                      w.catalogue().kind<data::Blueprint>().key(recipe.value_or(0)));
            const bool success = draws.below(0, 1000000) <
                                 static_cast<std::uint64_t>(Crafting::success(w, h, recipe.value_or(0), roles));
            if (success) {
                if (t->tending == 3)
                    (void)bank(c, target);
                else
                    (void)carry(c, target, input, person);
            }
            auto& know = raw.get<world::Knowledge>(h);
            for (auto& skill : know.skills)
                if (skill.recipe == recipe.value_or(0))
                    Learning::practice(skill.practice, c.now(), t->tending == 3 ? 300 : 60, success, know.learning_ppm);
        }
        if (input.value && t->tending_shared) release_input(c, input, here);
        clear_tend(*t);
        return false;
    }
    auto destination = t->tending_phase == 1 ? at(w, input, c.now()) : at(w, target, c.now());
    if (here != destination) {
        const auto path = Living::route(w, raw.get<Home>(h).camp, here, destination);
        if (path.empty()) {
            clear_tend(*t);
            return false;
        }
        raw.get<world::Life>(h).portion = 0;
        living.begin(c, h, world::LivingAct::walk,
                     (w.torus().distance(here, path.front()) * 10 + living.rules_.speed - 1) / living.rules_.speed,
                     path.front());
        return true;
    }
    if (t->tending_phase == 1) {
        // Split only the reserved portion. Other claims keep their shared source at its site.
        auto portion = item(w, input);
        t->tending_shared = portion.owner.value == 0;
        portion.mass = t->tending_mass;
        portion.owner = person;
        item(w, input).mass -= portion.mass;
        if (!item(w, input).mass) item(w, input).state = 4;
        const auto carried = w.make_thing();
        w.things().raw().emplace<Item>(carried, portion);
        w.things().raw().emplace<world::Place>(carried, here);
        t->tending_input = w.things().id_of(carried);
        c.item_changed(input);
        c.item_changed(t->tending_input);
        t->tending_phase = 2;
        return continue_tending(living, c, h, false);
    }
    if (input.value) {
        w.things().raw().get<world::Place>(w.things().handle(input)).at = here;
        c.item_changed(input);
    }
    t->tending_phase = 3;
    raw.get<world::Life>(h).portion = 0;
    living.begin(c, h, world::LivingAct::tend, t->tending == 3 ? 300 : 60, here);
    return true;
}
}  // namespace kd::demo
