// The winning option and two real alternatives, kept before a timed plan begins (PRN-13).
#pragma once
#include "kd/world/world.hpp"
namespace kd::demo {
class Choices {
public:
    static std::uint64_t keep(world::Context& c, world::Beings::Handle h, world::CraftReason winner,
                              std::vector<world::CraftReason> alternatives = {});
    static void restore(world::World& w, world::Beings::Handle h, std::uint64_t choice);
};
}  // namespace kd::demo
