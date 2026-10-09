// Event-owned hearth deadlines. Physical operations arrive in T3.13c.2–5.
#pragma once
#include "kd/world/world.hpp"
namespace kd::demo {
class Living;
class FireRules {
public:
    static void start(world::World& w);
    static void deadlines(world::Context& c, ecs::Id camp);
    static void handle(world::Context& c, ecs::Id camp, std::uint32_t slot);
    [[nodiscard]] static std::int64_t ambient(time::Seconds at);
    [[nodiscard]] static time::Seconds next_ambient(time::Seconds at);
};
}  // namespace kd::demo
