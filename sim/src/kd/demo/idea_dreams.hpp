// Remembered-action hints, never recipe knowledge (GOD-03, MND-12).
#pragma once
#include "kd/world/world.hpp"
namespace kd::demo {
class IdeaDreams {
public:
    [[nodiscard]] static const world::Memory* memory(const world::Knowledge& know, std::uint64_t id);
    [[nodiscard]] static std::optional<world::IdeaFields> fit(const world::World& w, world::Beings::Handle h,
                                                              std::uint64_t memory);
    [[nodiscard]] static bool valid(const world::World& w, world::Beings::Handle h, const world::IdeaFields& idea);
    [[nodiscard]] static std::optional<world::IdeaFields> guess(const world::World& w, world::Beings::Handle h,
                                                                bool compatible, std::uint64_t pick);
    [[nodiscard]] static std::string problem(const world::World& w, ecs::Id person, std::uint64_t memory,
                                             time::Seconds at = -1);
    static void dream(world::Context& c, world::Beings::Handle h, world::IdeaFields idea);
    static void attempted(world::Context& c, world::Beings::Handle h, std::uint8_t action,
                          std::span<const ecs::Id> inputs);
};
}  // namespace kd::demo
