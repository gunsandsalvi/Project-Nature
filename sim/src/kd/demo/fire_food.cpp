#include "kd/chance/chance.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/fire.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
#include "kd/num/sort.hpp"
namespace kd::demo {
namespace {
using world::HeatTimer;
using world::Item;
num::Point position(const world::World& w, ecs::Id id, time::Seconds at) {
    const auto h = w.things().handle(id);
    const auto& i = w.things().raw().get<Item>(h);
    return i.owner.value ? w.beings().raw().get<world::Activity>(w.beings().handle(i.owner)).at(w.torus(), at)
                         : w.things().raw().get<world::Place>(h).at;
}
struct HeatSource {
    std::uint8_t heat = 0;
    ecs::Id fire{}, origin{};
    std::int64_t tended_at = -1;
};
HeatSource source(const world::World& w, ecs::Id id, time::Seconds now) {
    const auto& i = w.things().raw().get<Item>(w.things().handle(id));
    HeatSource out;
    const auto here = position(w, id, now);
    for (const auto fh : w.things().raw().view<world::Fire>()) {
        const auto& f = w.things().raw().get<world::Fire>(fh);
        const auto fire = w.things().id_of(fh);
        if (f.hearth != i.home || w.torus().squared_distance(here, position(w, fire, now)) > 10000) continue;
        if (f.heat > out.heat || (f.heat && f.heat == out.heat && fire < out.fire))
            out = {f.heat, fire, f.origin, f.tended_at};
    }
    return out;
}
std::uint8_t heat(const world::World& w, ecs::Id id, time::Seconds now) {
    return source(w, id, now).heat;
}
time::Seconds boundary(const world::World& w, ecs::Id id, time::Seconds now, HeatSource band) {
    const auto& i = w.things().raw().get<Item>(w.things().handle(id));
    auto end = now;
    if (i.owner.value) {
        const auto& a = w.beings().raw().get<world::Activity>(w.beings().handle(i.owner));
        if (a.from != a.to) end = std::max(end, a.end);
    }
    for (const auto fh : w.things().raw().view<world::Fire>()) {
        const auto& f = w.things().raw().get<world::Fire>(fh);
        if (f.hearth != i.home || !f.owner.value || f.heat < 2) continue;
        const auto& a = w.beings().raw().get<world::Activity>(w.beings().handle(f.owner));
        if (a.from != a.to) end = std::max(end, a.end);
    }
    for (auto second = now + 1; second <= end; ++second)
        if (const auto current = source(w, id, second); current.heat != band.heat || current.fire != band.fire)
            return second;
    return 0;
}
void settle(world::Context& c, ecs::Id id) {
    auto& w = c.world();
    const auto h = w.things().handle(id);
    auto& t = w.things().raw().get<HeatTimer>(h);
    auto& i = w.things().raw().get<Item>(h);
    const auto old_heat = t.exposure_heat;
    const auto old_next = t.next;
    const auto old_source = t.exposure_fire;
    const auto old_tending = t.exposure_tended_at;
    const auto duration = c.now() - t.settled_at;
    if (t.exposure_heat >= 2 && duration > 0 && t.elapsed < time::kHour && t.exposure_fire.value) {
        const auto credited = std::min<time::Seconds>(duration, time::kHour - t.elapsed);
        const auto old = std::find_if(t.heat_sources.begin(), t.heat_sources.end(), [&](const auto& x) {
            return x.fire == t.exposure_fire && x.origin == t.exposure_origin && x.tended_at == t.exposure_tended_at;
        });
        if (old == t.heat_sources.end())
            t.heat_sources.push_back(
                {t.exposure_fire, t.exposure_origin, t.exposure_tended_at, t.settled_at, credited});
        else
            old->seconds += credited;
    }
    if (!t.completed && t.exposure_heat >= 2) {
        t.elapsed = std::min(2 * time::kHour, t.elapsed + duration);
        if (t.exposure_heat >= 4) t.hot_elapsed = std::min(time::kHour, t.hot_elapsed + duration);
    }
    t.settled_at = c.now();
    const auto recipe = FireRules::cooking_recipe(w.catalogue(), i);
    const auto roast = recipe.value_or(world::kNoRecipe);
    if (!t.completed && (t.elapsed >= 2 * time::kHour || t.hot_elapsed >= time::kHour)) {
        i.state = 2;
        i.changed_mask |= 1U << 8U;
        i.changed[8] = 0;
        t.completed = 1;
        t.notices.clear();
        c.item_changed(id);
    } else if (recipe && !t.tried && t.elapsed >= time::kHour &&
               ((t.exposure_heat >= 2 && t.exposure_heat <= 3) ||
                (heat(w, id, c.now()) >= 2 && heat(w, id, c.now()) <= 3))) {
        t.tried = 1;
        const std::array roles{id};
        auto ppm = 400000LL;
        const auto clock = c.now() % time::kDay;
        const auto here = position(w, id, c.now());
        if ((clock < 6 * time::kHour || clock >= 20 * time::kHour) &&
            !FireRules::task_light(w, i.home, here, here, c.now()))
            ppm -= 100000;
        if (i.quality < 2) ppm -= 100000;
        if (i.quality >= 4) ppm += 100000;
        if (t.intended) ppm = Crafting::success(w, w.beings().handle(t.maker), roast, roles, c.now());
        const chance::Draws draws(
            w.seed(), chance::name("cooking timer"), (t.chance_source.value ? t.chance_source : id).value,
            w.things().raw().get<Item>(w.things().handle(t.chance_source.value ? t.chance_source : id)).made_at,
            w.catalogue().kind<data::Blueprint>().key(roast));
        if (draws.below(0, 1000000) < static_cast<std::uint64_t>(ppm)) {
            const auto nourishment = std::min<std::int64_t>(5, Crafting::characteristics(w.catalogue(), i)[8] + 1);
            i.state = 1;
            i.changed_mask |= 1U << 8U;
            i.changed[8] = static_cast<std::uint8_t>(nourishment);
            // Catalogue state changes supply exactly the raw food value plus one.
            c.item_changed(id);
        }
        if (t.intended) {
            const auto person = w.beings().handle(t.maker);
            const auto& work = w.beings().raw().get<world::Work>(person);
            const auto demonstrating = work.state == 2 && work.intended && work.recipe == roast;
            if (demonstrating) Learning::observe_maker(c, person);
            auto perceived = Discovery::handling(c, person, 12, roles);
            const auto event = Discovery::result(c, person, roast, roles, std::move(perceived), id, i.state == 1, false,
                                                 0, t.heat_sources);
            auto& know = w.beings().raw().get<world::Knowledge>(person);
            if (demonstrating) {
                Learning::worked(c, person, event, i.state == 1);
                Learning::demonstrated(c, person, event);
            } else {
                for (auto& skill : know.skills)
                    if (skill.recipe == roast)
                        Learning::practice(skill.practice, c.now(), time::kHour, i.state == 1, know.learning_ppm);
                Learning::practice(know.sectors[5], c.now(), time::kHour, i.state == 1, know.learning_ppm);
            }
            t.notices.push_back({t.maker});
            c.moved(t.maker);
        }
    }
    const auto present = i.mass && !t.completed ? source(w, id, c.now()) : HeatSource{};
    t.exposure_heat = present.heat;
    t.exposure_fire = present.fire;
    t.exposure_origin = present.origin;
    t.exposure_tended_at = present.tended_at;
    t.next = 0;
    const auto take = [&](time::Seconds at) {
        if (at > c.now()) t.next = t.next ? std::min(t.next, at) : at;
    };
    if (!t.completed && i.mass) {
        if (t.exposure_heat >= 2) {
            if (!t.tried && t.exposure_heat <= 3) take(c.now() + std::max<time::Seconds>(1, time::kHour - t.elapsed));
            take(c.now() + 2 * time::kHour - t.elapsed);
            if (t.exposure_heat >= 4) take(c.now() + time::kHour - t.hot_elapsed);
        }
        take(boundary(w, id, c.now(), present));
    }
    if (old_heat != t.exposure_heat || old_next != t.next || old_source != t.exposure_fire ||
        old_tending != t.exposure_tended_at)
        c.item_changed(id);
}
}  // namespace
std::optional<std::uint32_t> FireRules::cooking_recipe(const data::Catalogue& catalogue, const Item& item) {
    const auto physical = Crafting::physical(catalogue, item);
    const auto& recipes = catalogue.kind<data::Blueprint>();
    for (std::uint32_t r = 0; r < recipes.size(); ++r) {
        const auto& b = recipes[r];
        if (b.action == 12 && b.heat >= 2 && b.inputs.size() == 1 && data::fits(b.inputs[0], physical)) return r;
    }
    return {};
}
void FireRules::food_changed(world::Context& c, ecs::Id id) {
    auto& w = c.world();
    const auto h = w.things().handle(id);
    const auto i = w.things().raw().get<Item>(h);
    if (!w.beings().raw().all_of<world::Ambient>(w.beings().handle(i.home))) return;
    if (w.things().raw().all_of<world::Fire>(h)) {
        food_refresh(c, i.home);
        return;
    }
    if (!w.things().raw().all_of<HeatTimer>(h) && !cooking_recipe(w.catalogue(), i)) return;
    auto* t = w.things().raw().try_get<HeatTimer>(h);
    const HeatTimer* inherited = nullptr;
    if (i.parents.size() == 1) {
        const auto source = w.things().find(i.parents[0].id);
        if (source) {
            const auto& parent = w.things().raw().get<Item>(*source);
            if (parent.kind == i.kind && parent.material == i.material && parent.state == i.state)
                inherited = w.things().raw().try_get<HeatTimer>(*source);
        }
    }
    const auto present_heat = !t && i.state == 0 ? heat(w, id, c.now()) : 0;
    const auto approaching = !t && i.state == 0 && i.mass ? boundary(w, id, c.now(), source(w, id, c.now())) : 0;
    if (!t && i.mass && (inherited || (i.state == 0 && (present_heat >= 2 || approaching)))) {
        const auto retained = inherited ? *inherited : HeatTimer{};
        t = &w.things().raw().emplace<HeatTimer>(h, retained);
        t->item = id;
        if (!t->chance_source.value) t->chance_source = id;
        t->settled_at = c.now();
        if (!inherited && i.owner.value) {
            const auto* work = w.beings().raw().try_get<world::Work>(w.beings().handle(i.owner));
            if (work && work->action == 12 && work->state) {
                t->maker = i.owner;
                t->intended = work->intended;
                t->placement_choice = work->choice;
            }
        }
    }
    if (!t) return;
    settle(c, id);
    deadlines(c, i.home);
}
void FireRules::food_refresh(world::Context& c, ecs::Id camp) {
    std::vector<ecs::Id> foods;
    c.world().things().each([&](ecs::Id id, auto h) {
        const auto& i = c.world().things().raw().get<Item>(h);
        if (i.home == camp &&
            (c.world().things().raw().all_of<HeatTimer>(h) || cooking_recipe(c.world().catalogue(), i)))
            foods.push_back(id);
    });
    for (const auto id : foods) food_changed(c, id);
    deadlines(c, camp);
}
void FireRules::carried_food(world::Context& c, ecs::Id person) {
    const auto& w = std::as_const(c.world());
    std::vector<ecs::Id> carried;
    for (std::uint32_t kind = 0; kind < w.catalogue().kind<data::ItemKind>().size(); ++kind)
        for (const auto& entry : w.items_owned(person, kind)) carried.push_back(entry.id);
    // Preserve whole-world ID order before callbacks mutate the live carried-item index.
    num::sort_strict(carried.begin(), carried.end(), [](auto a, auto b) { return a < b; });
    for (const auto id : carried) food_changed(c, id);
}

void FireRules::food_intent(world::Context& c, ecs::Id id, ecs::Id maker, bool intended) {
    food_changed(c, id);
    auto* timer = c.world().things().raw().try_get<HeatTimer>(c.world().things().handle(id));
    if (timer && !timer->tried && !timer->elapsed) {
        timer->maker = maker;
        timer->intended = static_cast<std::uint8_t>(intended);
    }
}
std::optional<num::Point> FireRules::cooking_spot(const world::World& w, world::Beings::Handle person,
                                                  time::Seconds at) {
    const auto home = w.beings().raw().get<Home>(person).camp;
    const auto here = w.beings().raw().get<world::Activity>(person).at(w.torus(), at);
    const auto clock = at % time::kDay;
    const std::int64_t range = clock >= 6 * time::kHour && clock < 20 * time::kHour ? 3000 : 500;
    for (const auto fh : w.things().raw().view<world::Fire>()) {
        const auto& f = w.things().raw().get<world::Fire>(fh);
        if (f.hearth == home && !f.owner.value && f.heat >= 2 &&
            w.torus().squared_distance(here, f.at) <= range * range && Living::visible(w, home, here, f.at)) {
            const auto spot = w.torus().moved(f.at, {50, 0});
            if (!Living::route(w, home, here, spot).empty()) return spot;
        }
    }
    return {};
}
void FireRules::notice_food(world::Context& c, world::Beings::Handle person) {
    auto& w = c.world();
    auto& know = w.beings().raw().get<world::Knowledge>(person);
    const auto& activity = w.beings().raw().get<world::Activity>(person);
    if (activity.what == static_cast<std::uint8_t>(world::LivingAct::rest) && c.now() < activity.end) return;
    const auto home = w.beings().raw().get<Home>(person).camp;
    const auto viewer = w.beings().id_of(person);
    const auto here = w.beings().raw().get<world::Activity>(person).at(w.torus(), c.now());
    const auto clock = c.now() % time::kDay;
    const std::int64_t range = clock >= 6 * time::kHour && clock < 20 * time::kHour ? 3000 : 500;
    std::vector<ecs::Id> noticed;
    for (const auto fh : w.things().raw().view<HeatTimer>()) {
        auto& t = w.things().raw().get<HeatTimer>(fh);
        const auto& i = w.things().raw().get<Item>(fh);
        if (i.home != home || i.state != 1 || !i.mass || (i.owner.value && i.owner != viewer) ||
            std::any_of(t.notices.begin(), t.notices.end(), [&](auto n) { return n.id == viewer; }) ||
            w.torus().squared_distance(here, position(w, t.item, c.now())) > range * range ||
            !Living::visible(w, home, here, position(w, t.item, c.now())))
            continue;
        t.notices.push_back({viewer});
        noticed.push_back(t.item);
    }
    for (const auto id : noticed) {
        Discovery::learn(c, person, id, Discovery::kSight, 1);
        const auto& i = w.things().raw().get<Item>(w.things().handle(id));
        const auto recipe = cooking_recipe(w.catalogue(), i);
        if (!recipe) continue;
        const auto roast = *recipe;
        const auto* familiar = Discovery::familiar(know, i);
        const std::array inputs{id};
        auto& history = w.beings().raw().get<world::CraftHistory>(w.beings().handle(home));
        const auto before = history.events.size();
        const auto& timer = w.things().raw().get<HeatTimer>(w.things().handle(id));
        Discovery::result(c, person, roast, inputs, {*familiar}, id, true, !Learning::knows(know, roast), 1,
                          timer.heat_sources);
        if (timer.placement_choice)
            for (auto n = before; n < history.events.size(); ++n) {
                history.events[n].choice = timer.placement_choice;
                history.events[n].source = timer.maker;
            }
        c.moved(viewer);
    }
}
}  // namespace kd::demo
