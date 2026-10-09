#include "kd/demo/fire.hpp"
#include "kd/chance/chance.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
namespace kd::demo {
namespace {
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
void clear_tend(world::Thermal& t) {
    t.tending = t.tending_phase = t.tending_shared = 0;
    t.tending_fire = t.tending_input = {};
    t.tending_mass = t.tending_started = 0;
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
void FireRules::fresh_hearth(world::World& w) {
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
        charcoal.kind = charcoal.material = *w.catalogue().find("item", "base:charcoal");
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
        remains.kind = remains.material = *w.catalogue().find("item", "base:ash");
        remains.state = remains.mass ? 0 : 4;
        remains.changed_mask = 0;
        remains.changed.fill(0);
    }
    thermal_after(c, f.hearth);
    c.item_changed(id);
}
bool FireRules::feed(world::Context& c, ecs::Id hearth, ecs::Id input, std::int64_t mass, ecs::Id person) {
    auto& w = c.world();
    if (input == hearth || mass <= 0 || w.things().raw().all_of<Fire>(w.things().handle(input)) ||
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
    thermal_after(c, f.hearth);
    deadlines(c, f.hearth);
    return true;
}
ecs::Id FireRules::carry(world::Context& c, ecs::Id hearth, ecs::Id input, ecs::Id person) {
    auto& w = c.world();
    if (w.things().raw().all_of<Fire>(w.things().handle(input))) return {};
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
    w.things().each([&](ecs::Id, world::Things::Handle h) {
        if (const auto* f = w.things().raw().try_get<Fire>(h); f && f->hearth == camp && f->next)
            heat = std::min(heat, f->next);
        if (const auto* t = w.things().raw().try_get<world::HeatTimer>(h);
            t && item(w, t->item).home == camp && t->next)
            timer = timer ? std::min(timer, t->next) : t->next;
    });
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
    w.things().each([&](ecs::Id id, auto h) {
        const auto* f = w.things().raw().try_get<Fire>(h);
        if (f && f->hearth == camp && f->next && f->next <= c.now()) due.push_back(id);
    });
    for (const auto id : due) settle_fire(c, id);
    std::vector<ecs::Id> ignite;
    w.things().each([&](ecs::Id id, auto h) {
        const auto& source = w.things().raw().get<Item>(h);
        if (source.home != camp || source.mass == 0 || w.things().raw().any_of<Fire, world::HeatTimer>(h)) return;
        const auto fit = Crafting::physical(w.catalogue(), source);
        if (fit.values[6] < 2 || fit.values[9] >= 3) return;
        for (const auto flame : due) {
            if (fire(w, flame).heat >= 2 &&
                w.torus().squared_distance(at(w, id, c.now()), at(w, flame, c.now())) <= 10000) {
                ignite.push_back(id);
                break;
            }
        }
    });
    for (const auto id : ignite) {
        const auto i = item(w, id);
        auto& f = w.things().raw().emplace<Fire>(w.things().handle(id));
        f.hearth = camp;
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
    deadlines(c, camp);
}
bool FireRules::choose(Living& living, world::Context& c, world::Beings::Handle h) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* t = raw.try_get<world::Thermal>(h);
    if (!t || t->tending || raw.get<world::Work>(h).state) return false;
    const auto& l = raw.get<world::Life>(h);
    if (*std::min_element(l.decision_needs.begin(), l.decision_needs.end()) < 20) return false;
    const auto person = w.beings().id_of(h), camp = raw.get<Home>(h).camp;
    const auto here = raw.get<world::Place>(h).at;
    ecs::Id target{};
    w.things().each([&](ecs::Id id, auto th) {
        const auto* f = w.things().raw().try_get<Fire>(th);
        if (!f || !f->heat || f->hearth != camp || (f->owner.value && f->owner != person) || target.value ||
            w.torus().squared_distance(here, at(w, id, c.now())) > 25000000 ||
            !Living::visible(w, camp, here, at(w, id, c.now())))
            return;
        target = id;
    });
    if (!target.value) return false;
    const auto f = fire(w, target);
    std::uint8_t operation = 0;
    ecs::Id input{};
    std::int64_t mass = 0, score = 0;
    const auto clock = c.now() % time::kDay;
    const auto& know = raw.get<world::Knowledge>(h);
    const auto has = [&](std::string_view name) {
        const auto r = w.catalogue().find("blueprint", name);
        return r && Learning::knows(know, *r);
    };
    if (clock >= 20 * time::kHour && f.heat >= 2 && f.ash_mg >= 100000 && has("base:bank_fire")) {
        operation = 3;
        score = 120;
    } else if ((f.heat == 1 || f.fuel_mg < 2500000) &&
               (f.banked_until <= c.now() || (clock >= 6 * time::kHour && clock < 20 * time::kHour))) {
        w.things().each([&](ecs::Id id, auto th) {
            const auto& source = w.things().raw().get<Item>(th);
            if (input.value || source.home != camp || (source.owner.value && source.owner != person) ||
                w.things().raw().all_of<Fire>(th) || Crafting::available(w, id, person) < 100000)
                return;
            const auto fit = Crafting::physical(w.catalogue(), source);
            const auto* remembered = Discovery::familiar(know, source);
            if (!remembered || fit.values[9] > (f.heat == 1 ? 1 : 2) || fit.values[6] < (f.heat == 1 ? 4 : 2) ||
                w.torus().squared_distance(here, at(w, id, c.now())) > 25000000 ||
                !Living::visible(w, camp, here, at(w, id, c.now())))
                return;
            input = id;
            mass = std::min<std::int64_t>(1000000, Crafting::available(w, id, person));
        });
        if (input.value) {
            operation = f.heat == 1 ? 2 : 1;
            score = 150;
        }
    }
    if (!operation && f.heat == 1 && has("base:carry_ember")) {
        const chance::Draws draws(w.seed(), chance::name("carry ember choice"), person.value, c.now() / 3600,
                                  chance::name("container"));
        if (draws.below(0, 24) == 0)
            w.things().each([&](ecs::Id id, auto th) {
                const auto& source = w.things().raw().get<Item>(th);
                const auto fit = Crafting::physical(w.catalogue(), source);
                if (!input.value && source.home == camp && !w.things().raw().all_of<Fire>(th) &&
                    (!source.owner.value || source.owner == person) && Discovery::familiar(know, source) &&
                    fit.values[6] >= 4 && fit.values[7] >= 1 && fit.values[7] <= 2 && fit.values[9] <= 1 &&
                    Crafting::available(w, id, person) >= 100000 && Living::visible(w, camp, here, at(w, id, c.now())))
                    input = id;
            });
        if (input.value) {
            operation = 4;
            mass = 100000;
            score = 80;
        }
    }
    if (!operation || score <= l.scores[l.goal]) return false;
    t->tending = operation;
    t->tending_phase = input.value ? 1 : 2;
    t->tending_fire = target;
    t->tending_input = input;
    t->tending_mass = mass;
    t->tending_started = c.now();
    return continue_tending(living, c, h, false);
}
bool FireRules::continue_tending(Living& living, world::Context& c, world::Beings::Handle h, bool interrupted) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* t = raw.try_get<world::Thermal>(h);
    if (!t || !t->tending) return false;
    const auto person = w.beings().id_of(h);
    const auto target = t->tending_fire, input = t->tending_input;
    if (interrupted || !w.things().find(target) || !fire(w, target).heat) {
        if (input.value && t->tending_shared && w.things().find(input)) {
            item(w, input).owner = {};
            w.things().raw().get<world::Place>(w.things().handle(input)).at = raw.get<world::Place>(h).at;
            c.item_changed(input);
        }
        clear_tend(*t);
        return false;
    }
    const auto here = raw.get<world::Place>(h).at;
    if (t->tending_phase == 3) {
        if (t->tending >= 3) {
            const auto recipe =
                w.catalogue().find("blueprint", t->tending == 3 ? "base:bank_fire" : "base:carry_ember");
            const std::array roles{target};
            const chance::Draws draws(w.seed(), chance::name("tend result"), person.value, t->tending_started,
                                      w.catalogue().kind<data::Blueprint>().key(*recipe));
            const bool success =
                draws.below(0, 1000000) < static_cast<std::uint64_t>(Crafting::success(w, h, *recipe, roles));
            if (success) {
                if (t->tending == 3)
                    (void)bank(c, target);
                else
                    (void)carry(c, target, input, person);
            }
            auto& know = raw.get<world::Knowledge>(h);
            for (auto& skill : know.skills)
                if (skill.recipe == *recipe)
                    Learning::practice(skill.practice, c.now(), t->tending == 3 ? 300 : 60, success, know.learning_ppm);
        }
        if (input.value && t->tending_shared) {
            item(w, input).owner = {};
            c.item_changed(input);
        }
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
    if (t->tending <= 2) {
        if (!feed(c, target, input, t->tending_mass, person)) {
            clear_tend(*t);
            return false;
        }
        t->tending_mass = 0;
        if (t->tending == 2) (void)blow(c, target);
    }
    t->tending_phase = 3;
    raw.get<world::Life>(h).portion = 0;
    living.begin(c, h, world::LivingAct::tend, t->tending == 3 ? 300 : 60, here);
    return true;
}
}  // namespace kd::demo
