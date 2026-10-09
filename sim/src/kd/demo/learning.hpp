// Personal, event-backed learning and conserved fractional practice (MND-06 MND-13 MND-23).
#pragma once
#include "kd/time/calendar.hpp"
#include "kd/world/knowledge.hpp"
namespace kd::demo {
class Learning {
public:
    [[nodiscard]] static bool knows(const world::Knowledge& knowledge, std::uint32_t recipe);
    [[nodiscard]] static std::int64_t taught_multiplier(const world::Practice& teacher);
    static void fade(world::Practice& skill, time::Seconds now);
    static void practice(world::Practice& skill, time::Seconds now, time::Seconds seconds, bool success,
                         std::int64_t learning_ppm = 1000000, std::int64_t multiplier_ppm = 1000000);
};
}  // namespace kd::demo
