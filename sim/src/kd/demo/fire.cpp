#include "kd/demo/fire.hpp"
namespace kd::demo {
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
void FireRules::deadlines(world::Context& c, ecs::Id camp) {
    auto& w = c.world();
    const auto ch = w.beings().handle(camp);
    const auto* ambient = w.beings().raw().try_get<world::Ambient>(ch);
    if (!ambient) return;
    auto heat = ambient->next;
    time::Seconds timer = 0;
    w.things().each([&](ecs::Id, world::Things::Handle h) {
        if (const auto* f = w.things().raw().try_get<world::Fire>(h); f && f->hearth == camp && f->next)
            heat = std::min(heat, f->next);
        if (const auto* t = w.things().raw().try_get<world::HeatTimer>(h);
            t && w.things().raw().get<world::Item>(h).home == camp && t->next)
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
    const auto h = w.beings().handle(camp);
    auto& ambient = w.beings().raw().get<world::Ambient>(h);
    if (slot == 2 && ambient.next <= c.now()) {
        ambient.milli_c = FireRules::ambient(c.now());
        ambient.next = next_ambient(c.now());
    }
    deadlines(c, camp);
}
}  // namespace kd::demo
