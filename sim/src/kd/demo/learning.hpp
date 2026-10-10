// Personal, event-backed learning and conserved fractional practice (MND-06 MND-13 MND-23).
#pragma once
#include "kd/time/calendar.hpp"
#include "kd/world/world.hpp"
namespace kd::demo {
class Living;
class Learning {
public:
    static void settle_mind(world::Context& c, world::Beings::Handle h);
    // A proposal crosses into the learner's own decision; callers receive only a recorded yes/no reply.
    static bool reply_to_lesson(Living& living, world::Context& c, world::Beings::Handle listener, ecs::Id proposer,
                                bool resume = false);
    static bool choose(Living& living, world::Context& c, world::Beings::Handle h);
    static bool handle(Living& living, world::Context& c, world::Beings::Handle h, bool interrupted, bool try_event);
    static bool exchange(world::Context& c, world::Beings::Handle speaker, world::Beings::Handle listener,
                         std::uint32_t recipe);
    static void started(Living& living, world::Context& c, world::Beings::Handle learner);
    static void worked(world::Context& c, world::Beings::Handle maker, std::uint64_t event, bool success);
    static void observe(world::Context& c, world::Beings::Handle observer);
    static void observe_maker(world::Context& c, world::Beings::Handle maker);
    static void demonstrated(world::Context& c, world::Beings::Handle maker, std::uint64_t event);
    static void learned(world::Context& c, world::Beings::Handle learner, const world::Result& evidence, ecs::Id source,
                        std::uint8_t route);
    static void lost(world::Context& c, world::Beings::Handle last_holder, std::uint32_t recipe);
    static void forget_work(world::Context& c, world::Beings::Handle maker);
    [[nodiscard]] static bool can_watch(const world::World& w, ecs::Id camp, num::Point from, num::Point to,
                                        time::Seconds at);
    [[nodiscard]] static bool knows(const world::Knowledge& knowledge, std::uint32_t recipe);
    [[nodiscard]] static std::int64_t taught_multiplier(const world::Practice& teacher);
    static void fade(world::Practice& skill, time::Seconds now);
    static void practice(world::Practice& skill, time::Seconds now, time::Seconds seconds, bool success,
                         std::int64_t learning_ppm = 1000000, std::int64_t multiplier_ppm = 1000000);
};
}  // namespace kd::demo
