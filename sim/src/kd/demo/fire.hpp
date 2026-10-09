// Event-owned hearth deadlines. Physical operations arrive in T3.13c.2–5.
#pragma once
#include "kd/world/world.hpp"
namespace kd::demo {
class Living;
class FireRules {
public:
    static void start(world::World& w);
    static void food_changed(world::Context& c, ecs::Id id);
    static void food_refresh(world::Context& c, ecs::Id camp);
    static void carried_food(world::Context& c, ecs::Id person);
    static void notice_food(world::Context& c, world::Beings::Handle person);
    static void food_intent(world::Context& c, ecs::Id food, ecs::Id maker, bool intended);
    static std::optional<num::Point> cooking_spot(const world::World& w, world::Beings::Handle person,
                                                  time::Seconds at);
    static world::Thermal sample_thermal(const world::World& w, world::Beings::Handle person, time::Seconds at);
    static void thermal_before(world::Context& c, ecs::Id camp);
    static void thermal_after(world::Context& c, ecs::Id camp);
    static void experience(world::Context& c, world::Beings::Handle person);
    static bool choose_warm(Living& living, world::Context& c, world::Beings::Handle person);
    static bool continue_warm(Living& living, world::Context& c, world::Beings::Handle person, bool interrupted);
    static std::int64_t warmth(std::int64_t milli_c, world::LivingAct action);
    static void fresh_hearth(world::World& w);
    static void ember(world::Context& c, ecs::Id item);
    static void settle_fire(world::Context& c, ecs::Id item);
    static bool feed(world::Context& c, ecs::Id hearth, ecs::Id input, std::int64_t mass, ecs::Id person = {});
    static bool blow(world::Context& c, ecs::Id hearth);
    static bool bank(world::Context& c, ecs::Id hearth);
    static ecs::Id carry(world::Context& c, ecs::Id hearth, ecs::Id input, ecs::Id person);
    static bool choose(Living& living, world::Context& c, world::Beings::Handle person);
    static bool continue_tending(Living& living, world::Context& c, world::Beings::Handle person, bool interrupted);
    static void deadlines(world::Context& c, ecs::Id camp);
    static void handle(world::Context& c, ecs::Id camp, std::uint32_t slot);
    [[nodiscard]] static std::int64_t ambient(time::Seconds at);
    [[nodiscard]] static time::Seconds next_ambient(time::Seconds at);
};
}  // namespace kd::demo
