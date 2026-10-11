// A19.3: a fixed law over this person's own recorded experience.
#pragma once
#include "kd/world/world.hpp"
namespace kd::demo {
class Motivation {
public:
    static world::MotivationDay& day(world::Motivation&, time::Seconds);
    static void dawn(world::Motivation&, time::Seconds);
    [[nodiscard]] static bool valid(const world::Motivation&, time::Seconds);
    static void value(const world::Motivation&, world::CraftReason&, std::uint8_t failures, bool enabled = true);
    static void choose(world::Context&, world::Beings::Handle, std::vector<world::CraftReason>&);
    static world::MotivationAction& feedback(world::Motivation&, const world::CraftReason&);
    static void trial(world::Context&, world::Beings::Handle, const world::CraftReason&);
    static void relief(world::Context&, world::Beings::Handle, std::size_t channel, bool progress = false);
    static void request(world::Context&, world::Beings::Handle);
};
}  // namespace kd::demo
