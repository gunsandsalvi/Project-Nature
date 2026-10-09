// Event-owned hearth deadlines. Physical operations arrive in T3.13c.2–5.
#pragma once
#include "kd/world/world.hpp"
namespace kd::demo {
class Living;
class FireRules {
public:
    static void start(world::World& w);
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
