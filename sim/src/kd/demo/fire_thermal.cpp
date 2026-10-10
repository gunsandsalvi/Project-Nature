#include "kd/demo/choice.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/fire.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
#include "kd/world/motion.hpp"
namespace kd::demo {
namespace {
struct Felt {
    std::int64_t temperature = 18000;
    ecs::Id source{};
};
Felt felt(const world::World& w, world::Beings::Handle h, time::Seconds at) {
    const auto& raw = w.beings().raw();
    const auto camp = raw.get<Home>(h).camp;
    const auto* ambient = raw.try_get<world::Ambient>(w.beings().handle(camp));
    Felt out{ambient ? ambient->milli_c : FireRules::ambient(at), {}};
    const auto here = raw.get<world::Activity>(h).at(w.torus(), at);
    const auto& things = w.things().raw();
    for (const auto th : things.view<world::Fire>()) {
        const auto& f = things.get<world::Fire>(th);
        if (f.hearth != camp || f.heat < 2) continue;
        const auto place =
            f.owner.value ? raw.get<world::Activity>(w.beings().handle(f.owner)).at(w.torus(), at) : f.at;
        if (w.torus().squared_distance(here, place) > 200LL * 200) continue;
        const auto id = w.things().id_of(th);
        if (!out.source.value || id < out.source) out.source = id;
    }
    if (out.source.value) out.temperature += 15000;
    return out;
}
void clear_warm(world::Thermal& t) {
    t.warm_phase = 0;
    t.warm_choice = 0;
    t.warm_fire = {};
    t.warm_at = {};
}
}  // namespace
std::int64_t FireRules::warmth(std::int64_t milli_c, world::LivingAct action) {
    const bool work = action == world::LivingAct::craft || action == world::LivingAct::tend ||
                      action == world::LivingAct::gather || action == world::LivingAct::carry ||
                      action == world::LivingAct::walk;
    const auto comfort = work ? 14000 : 24000;
    return std::clamp<std::int64_t>(100 - 5 * std::max<std::int64_t>(0, comfort - milli_c) / 1000, 0, 100);
}
world::Thermal FireRules::sample_thermal(world::Thermal t, const world::Activity& a, const num::Torus& torus,
                                         time::Seconds at, std::int64_t water, std::int64_t ambient_temperature,
                                         std::span<const HeatField> fires, bool scalar) {
    at = std::clamp(at, a.start, a.end);
    const auto feeling = [&](time::Seconds second) {
        Felt out{ambient_temperature, {}};
        const auto here = a.at(torus, second);
        for (const auto& f : fires) {
            if (torus.squared_distance(here, f.activity.at(torus, second)) <= 200LL * 200 &&
                (!out.source.value || f.id < out.source))
                out.source = f.id;
        }
        if (out.source.value) out.temperature += 15000;
        return out;
    };
    const auto charge = [&](std::int64_t temperature, bool heated, time::Seconds seconds) {
        const auto numerator = seconds * water * std::max<std::int64_t>(0, temperature - 32000) * 2 + t.water_remainder;
        const auto used = numerator / (time::kDay * 100000);
        t.water_remainder = numerator % (time::kDay * 100000);
        t.water_used_ml += used;
        t.water_due_ml += used;
        if (heated) t.warming_progress += seconds;
    };
    const auto begin = std::max(t.settled_at, a.start);
    if (at > begin) {
        if (!scalar && fires.empty())
            charge(ambient_temperature, false, at - begin);
        else if (a.from == a.to && std::none_of(fires.begin(), fires.end(),
                                                [](const auto& f) { return f.activity.from != f.activity.to; }))
            charge(feeling(begin).temperature, feeling(begin).source.value != 0, at - begin);
        else if (scalar)
            for (auto second = begin; second < at; ++second) {
                const auto felt = feeling(second);
                charge(felt.temperature, felt.source.value != 0, 1);
            }
        else {
            std::vector<world::MotionSpan> heated;
            for (const auto& f : fires) {
                const auto spans = world::proximity_spans(a, f.activity, torus, begin, at, 200);
                heated.insert(heated.end(), spans.begin(), spans.end());
            }
            std::stable_sort(heated.begin(), heated.end(),
                             [](const auto& left, const auto& right) { return left.begin < right.begin; });
            time::Seconds warm_seconds = 0, through = begin;
            for (const auto& span : heated) {
                warm_seconds += std::max<time::Seconds>(0, span.end - std::max(through, span.begin));
                through = std::max(through, span.end);
            }
            // Both rates are nonnegative and carry the same integer remainder;
            // adding their numerators commutes without changing any rounding.
            charge(ambient_temperature + 15000, true, warm_seconds);
            charge(ambient_temperature, false, at - begin - warm_seconds);
        }
    }
    t.felt_milli_c = feeling(at).temperature;
    t.warmth = warmth(t.felt_milli_c, static_cast<world::LivingAct>(a.what));
    t.settled_at = std::max(t.settled_at, at);
    return t;
}
world::Thermal FireRules::sample_thermal(const world::World& w, world::Beings::Handle h, time::Seconds at) {
    const auto measured = w.measure(world::Cost::thermal);
    const auto& raw = w.beings().raw();
    const auto home = raw.get<Home>(h).camp;
    const auto* ambient = raw.try_get<world::Ambient>(w.beings().handle(home));
    std::vector<HeatField> fires;
    for (const auto fh : w.things().raw().view<world::Fire>()) {
        const auto& f = w.things().raw().get<world::Fire>(fh);
        if (f.hearth != home || f.heat < 2) continue;
        auto activity = world::Activity{0, 0, std::numeric_limits<time::Seconds>::max(), f.at, f.at};
        if (f.owner.value) activity = raw.get<world::Activity>(w.beings().handle(f.owner));
        fires.push_back({w.things().id_of(fh), activity});
    }
    return sample_thermal(raw.get<world::Thermal>(h), raw.get<world::Activity>(h), w.torus(), at,
                          w.catalogue().kind<LivingRules>()[0].water_day,
                          ambient ? ambient->milli_c : FireRules::ambient(at), fires, w.scalar_work());
}
void FireRules::thermal_before(world::Context& c, ecs::Id camp) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    w.beings().each([&](ecs::Id id, auto h) {
        if (raw.all_of<world::Thermal, Home>(h) && raw.get<Home>(h).camp == camp) {
            if (raw.all_of<world::Knowledge>(h)) Learning::observe(c, h);
            raw.get<world::Thermal>(h) = sample_thermal(w, h, c.now());
            c.moved(id);
        }
    });
}
void FireRules::thermal_after(world::Context& c, ecs::Id camp) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    for (const auto th : w.things().raw().view<world::Fire>()) {
        const auto& f = w.things().raw().get<world::Fire>(th);
        if (f.hearth != camp) continue;
        auto& physical = w.things().raw().get<world::Item>(th);
        const auto state = static_cast<std::uint8_t>(physical.mass ? (f.heat >= 2 ? 3 : 0) : 4);
        const auto warmth = static_cast<std::uint8_t>(f.heat >= 2 ? 3 : f.heat == 1 ? 1 : 0);
        if (physical.state == state && physical.changed[12] == warmth) continue;
        physical.state = state;
        physical.changed_mask |= 1U << 12U;
        physical.changed[12] = warmth;
        c.item_changed(w.things().id_of(th));
    }
    w.beings().each([&](ecs::Id id, auto h) {
        if (!raw.all_of<world::Thermal, Home>(h) || raw.get<Home>(h).camp != camp) return;
        auto& t = raw.get<world::Thermal>(h);
        const auto current = felt(w, h, c.now());
        t.felt_milli_c = current.temperature;
        t.warmth = warmth(current.temperature, static_cast<world::LivingAct>(raw.get<world::Activity>(h).what));
        c.moved(id);
    });
}
void FireRules::experience(world::Context& c, world::Beings::Handle h) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    if (!raw.all_of<world::Thermal>(h)) return;
    auto& t = raw.get<world::Thermal>(h);
    t = sample_thermal(w, h, c.now());
    const auto current = felt(w, h, c.now());
    if (!current.source.value || !t.warming_progress || raw.get<world::Activity>(h).what == 2) return;
    auto& know = raw.get<world::Knowledge>(h);
    if (std::any_of(know.memories.begin(), know.memories.end(),
                    [&](const auto& m) { return m.action == 12 && m.sign == 13 && m.result == current.source; }))
        return;
    Discovery::learn(c, h, current.source, 1U << 12U, 3);
    const auto& physical = w.things().raw().get<world::Item>(w.things().handle(current.source));
    const auto* remembered = Discovery::familiar(know, physical);
    KD_CHECK(remembered, "Actual experienced fire has personal evidence");
    Discovery::memory(c, h, 12, {*remembered}, 13, current.source, 0, 70);
}
bool FireRules::choose_warm(Living& living, world::Context& c, world::Beings::Handle h, ChoiceSet* proposals) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* t = raw.try_get<world::Thermal>(h);
    if (!t || t->warm_phase || t->tending || t->warm_blocked_until > c.now()) return false;
    const auto wanted = warmth(t->felt_milli_c, world::LivingAct::rest);
    if (wanted >= 100 || (raw.get<world::Work>(h).state && wanted >= 20)) return false;
    const auto& l = raw.get<world::Life>(h);
    if (*std::min_element(l.decision_needs.begin(), l.decision_needs.end()) < 20) return false;
    const auto here = raw.get<world::Place>(h).at;
    const auto camp = raw.get<Home>(h).camp;
    ecs::Id source{};
    num::Point spot{};
    const auto clock = c.now() % time::kDay;
    const std::int64_t range = clock >= 6 * time::kHour && clock < 20 * time::kHour ? 3000 : 500;
    for (const auto th : w.things().raw().view<world::Fire>()) {
        const auto& f = w.things().raw().get<world::Fire>(th);
        const auto id = w.things().id_of(th);
        if (f.hearth != camp || f.heat < 2 || f.owner.value || w.torus().squared_distance(here, f.at) > range * range ||
            !Living::visible(w, camp, here, f.at) || (source.value && source < id))
            continue;
        const auto proposed = w.torus().moved(f.at, {160, 0});
        if (Living::route(w, camp, here, proposed).empty()) continue;
        source = id;
        spot = proposed;
    }
    if (!source.value) {
        const auto& know = raw.get<world::Knowledge>(h);
        for (const auto& memory : know.memories) {
            if (memory.action != 12 || memory.sign != 13 || !memory.result.value || !w.things().find(memory.result))
                continue;
            const auto th = w.things().handle(memory.result);
            const auto* f = w.things().raw().try_get<world::Fire>(th);
            if (!f || f->hearth != camp) continue;
            if (w.torus().squared_distance(here, f->at) <= range * range && Living::visible(w, camp, here, f->at) &&
                f->heat < 2)
                continue;
            if (!Living::route(w, camp, here, memory.place).empty()) {
                source = memory.result;
                spot = memory.place;
                break;
            }
        }
    }
    if (!source.value) return false;
    const auto score = (wanted < 20 ? 200 : std::max<std::int64_t>(0, 80 - wanted)) * (100 - wanted) * 10 -
                       w.torus().distance(here, spot) / 100;
    if (!proposals && score <= l.scores[l.goal]) return false;
    world::CraftReason reason;
    reason.kind = 4;
    reason.action = 11;
    reason.need = 4;
    reason.need_met = static_cast<std::uint8_t>(wanted);
    reason.score = score;
    reason.parts[0] = (wanted < 20 ? 200 : std::max<std::int64_t>(0, 80 - wanted)) * (100 - wanted) * 10;
    reason.parts[2] = -w.torus().distance(here, spot) / 100;
    reason.benefit = 100 - wanted;
    reason.seconds = time::kHour + w.torus().distance(here, spot) * 10 / living.rules().speed;
    reason.inputs.push_back({source});
    auto commit = [&living, &c, h, source, spot, reason](std::uint64_t choice) {
        auto& thermal = c.world().beings().raw().get<world::Thermal>(h);
        thermal.warm_choice = choice ? choice : Choices::keep(c, h, reason);
        thermal.warm_fire = source;
        thermal.warm_at = spot;
        thermal.warm_phase = 1;
        return continue_warm(living, c, h, false);
    };
    if (proposals) {
        proposals->add(reason, std::move(commit));
        return false;
    }
    return commit(0);
}
bool FireRules::continue_warm(Living& living, world::Context& c, world::Beings::Handle h, bool interrupted) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* t = raw.try_get<world::Thermal>(h);
    if (!t || !t->warm_phase) return false;
    Choices::restore(w, h, t->warm_choice);
    if (interrupted || t->warm_phase == 2) {
        clear_warm(*t);
        return false;
    }
    const auto here = raw.get<world::Place>(h).at;
    if (here != t->warm_at) {
        const auto path = Living::route(w, raw.get<Home>(h).camp, here, t->warm_at);
        if (path.empty()) {
            clear_warm(*t);
            return false;
        }
        raw.get<world::Life>(h).portion = 0;
        living.begin(c, h, world::LivingAct::walk,
                     (w.torus().distance(here, path.front()) * 10 + living.rules_.speed - 1) / living.rules_.speed,
                     path.front());
        return true;
    }
    if (!felt(w, h, c.now()).source.value) {
        t->warm_blocked_until = c.now() + time::kHour;
        clear_warm(*t);
        return false;
    }
    t->warm_phase = 2;
    raw.get<world::Life>(h).portion = 0;
    living.begin(c, h, world::LivingAct::warm, 1800, here);
    return true;
}
}  // namespace kd::demo
