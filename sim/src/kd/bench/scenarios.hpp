// The benchmark's scenarios (A18.1, PLT-04): what the phone runs, one after another, in about 19 minutes, each with
// the game second its world's digest is taken at and the pass lines it is held to, stated here before the first run
// (RES-09). The phone runs them on its screen; the cloud runs their worlds headless and must reach the same digests
// (RES-05). Written once, for both.
#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include "kd/data/catalogue.hpp"
#include "kd/time/calendar.hpp"

namespace kd::bench {

/// What a scenario's world is: the calendar's stand-in, or the demonstration's crowd of 10,000 markers.
enum class Ground : std::uint8_t { calendar, crowd };

/// How the camera moves: touring the crowd, panning and zooming, or still.
enum class Camera : std::uint8_t { tour, still };

/// Implements PLT-04, see A18.1: one scenario as the benchmark runs it.
struct Scenario {
    /// Its name in the code and the words that say what it is.
    std::string_view name;
    std::string_view about;
    Ground ground = Ground::crowd;
    /// The speed asked: a zoom stop's name from the time tuning (TIM-01), "top", or "sweep" through them all.
    std::string_view speed;
    Camera camera = Camera::tour;
    /// Whether the world's thread is pinned to the middle cores (A3.9).
    bool pinned = false;
    /// Whether it saves every 30 real seconds, exports the world and reopens it (A3.7).
    bool saves = false;
    /// The real seconds it lasts; its speed is read over its last minute, after 3 minutes at top speed (PLT-04).
    std::int64_t seconds = 0;
    /// The game second its world's digest is taken at, as its world passes it.
    time::Seconds mark = 0;
    /// A camp called home at a game second, by its number, or none when the second is 0.
    time::Seconds call_at = 0;
    std::int64_t call_camp = 0;
    /// Its pass lines (RES-09): frames on time at least, in thousandths; the slowest frame at most, in
    /// milliseconds; reopening at most, in milliseconds; 0 where it has none.
    std::int64_t on_time = 0;
    std::int64_t slowest = 0;
    std::int64_t open = 0;
};

/// Implements PLT-04, see A18.1: the scenarios, in the order they run.
[[nodiscard]] std::span<const Scenario> scenarios();

/// Implements RES-05, see A18.1: a scenario's world run headless to its mark, with its call, from the catalogue: the
/// digest the phone's must match. The crowd's is its whole state's; the calendar's, its work's.
[[nodiscard]] std::uint64_t headless_digest(const Scenario& s, const data::Catalogue& catalogue);

}  // namespace kd::bench
