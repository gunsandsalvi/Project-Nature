// Knowledge comes from starting facts and actual handling/results, never camera visibility.
#pragma once
#include "kd/world/world.hpp"
namespace kd::demo {
class Discovery {
public:
    static constexpr std::uint32_t kSight = (1U << 0U) | (1U << 1U) | (1U << 4U) | (1U << 5U) | (1U << 9U) |
                                            (1U << 13U) | (1U << 14U) | (1U << 15U) | (1U << 17U);
    static void starting(world::World& w, world::Beings::Handle h);
    static void see(world::Context& c, world::Beings::Handle h);
    [[nodiscard]] static const world::Familiar* familiar(const world::Knowledge& know, const world::Item& item);
    static void learn(world::Context& c, world::Beings::Handle h, ecs::Id item, std::uint32_t mask, std::uint8_t source,
                      bool edible = false, std::uint64_t event = 0, ecs::Id person = {});
    static std::vector<world::Familiar> handling(world::Context& c, world::Beings::Handle h, std::uint8_t action,
                                                 std::span<const ecs::Id> inputs);
    static std::uint64_t result(world::Context& c, world::Beings::Handle h, std::uint32_t recipe,
                                std::span<const ecs::Id> inputs, std::vector<world::Familiar> perceived, ecs::Id result,
                                bool success, bool unknown, std::uint8_t route,
                                std::span<const world::HeatCredit> heat_sources = {});
    static void memory(world::Context& c, world::Beings::Handle h, std::uint8_t action,
                       std::vector<world::Familiar> inputs, std::uint8_t sign, ecs::Id result = {},
                       std::uint64_t event = 0, std::uint8_t strength = 30);
};
}  // namespace kd::demo
