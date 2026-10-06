#include "kd/bench/scenarios.hpp"

#include <array>

#include "kd/demo/clockwork.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/world/world.hpp"

namespace kd::bench {

namespace {

constexpr time::Seconds kDay = time::kDay;
constexpr time::Seconds kMorning = demo::kMorning;

// 17 minutes of scenarios, about 19 with the worlds' making between them (A18.1). The pass lines are PLT-04's: at least
// 97% of frames on time and none more than 50 ms late, a gap of 66 ms at 60 frames a second, while the camera moves; a
// world reopened in 3 seconds; and every digest the cloud's (RES-05), which every scenario is held to.
constexpr std::array<Scenario, 7> kScenarios{{
    {"calendar", "the calendar alone at top speed, its stand-in's work each game hour", Ground::calendar, "top",
     Camera::still, false, false, 60, 60 * kDay, 0, 0, 0, 0, 0},
    {"real", "10,000 markers at real speed, the camera touring", Ground::crowd, "person", Camera::tour, false, false,
     120, kMorning + 60, 0, 0, 970, 66, 0},
    {"top", "10,000 markers at top speed, the camera touring", Ground::crowd, "top", Camera::tour, false, false, 240,
     10 * kDay, 0, 0, 970, 66, 0},
    {"pinned", "the same, the world's thread pinned to the middle cores", Ground::crowd, "top", Camera::tour, true,
     false, 240, 10 * kDay, 0, 0, 970, 66, 0},
    {"sweep", "10,000 markers through each zoom stop's speed and top, 20 seconds each, the camera touring",
     Ground::crowd, "sweep", Camera::tour, false, false, 120, 30 * kDay, 0, 0, 970, 66, 0},
    {"saves", "10,000 markers at the valley's speed, saved every 30 seconds, exported and reopened, a camp called home",
     Ground::crowd, "valley", Camera::still, false, true, 120, 20 * kDay, 2 * kDay + 12 * time::kHour, 3, 0, 0, 3000},
    {"still", "10,000 markers at real speed, the camera still, for the screen's own power", Ground::crowd, "person",
     Camera::still, false, false, 120, kMorning + 60, 0, 0, 970, 66, 0},
}};

}  // namespace

std::span<const Scenario> scenarios() {
    return kScenarios;
}

std::uint64_t headless_digest(const Scenario& s, const data::Catalogue& catalogue) {
    if (s.ground == Ground::calendar) {
        demo::Clockwork clock(demo::kCalendarWork);
        for (time::Seconds at = 0; at < s.mark;) {
            at = clock.advance(at, s.mark);
        }
        return clock.state();
    }
    // the crowd as the Crowd page makes it, from the tuning's seed and camps, begun on the first morning
    demo::CrowdWorld crowd(demo::kCrowdSeed, catalogue, std::nullopt);
    world::World& w = crowd.world();
    w.run_to(kMorning);
    if (s.call_at > 0) {
        w.run_to(s.call_at);
        (void)w.command(s.call_at, static_cast<std::uint32_t>(demo::Commanded::call_home),
                        crowd.camp_ids().at(static_cast<std::size_t>(s.call_camp)).value, 0);
    }
    w.run_to(s.mark);
    return w.digests().whole;
}

}  // namespace kd::bench
