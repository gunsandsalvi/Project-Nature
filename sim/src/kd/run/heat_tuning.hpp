// The heat governor's tuning (A3.9): base/tuning/heat.toml, the numbers by which time slows before the phone
// throttles. Only the screen reads it, so it counts in the look, never in the world's rules.
#pragma once

#include <cstdint>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"

namespace kd::run {

/// Implements PLT-01, see A3.9: when to cut the simulation's working share for heat, and how to give it back.
struct HeatTuning {
    std::int64_t near = 0;       // parts per million of severe throttling, on a phone that gives no thresholds
    std::int64_t margin = 0;     // parts per million of severe throttling below the phone's light threshold
    std::int64_t cut = 0;        // parts per million kept
    std::int64_t floor = 0;      // parts per million
    std::int64_t reading = 0;    // seconds of life between readings
    std::int64_t calm = 0;       // seconds of life
    std::int64_t give_back = 0;  // parts per million a reading

    template <typename V, typename Self>
    static void visit(V& v, Self& h) {
        using data::Affects;
        using data::Measure;
        v.quantity({"near",
                    "the heat forecast, as a share of severe throttling (Android's headroom of 1), at which time "
                    "slows on a phone that gives no thresholds of its own",
                    Affects::look},
                   h.near, Measure::ratio, {100'000, 1'000'000});
        v.quantity({"margin",
                    "how far below the phone's own light throttling threshold (Android 15 and later) time begins "
                    "to slow, as a share of severe throttling",
                    Affects::look},
                   h.margin, Measure::ratio, {0, 300'000});
        v.quantity({"cut", "what is kept of the working share at each reading that near", Affects::look}, h.cut,
                   Measure::ratio, {100'000, 1'000'000});
        v.quantity({"floor", "the least working share", Affects::look}, h.floor, Measure::ratio, {50'000, 1'000'000});
        v.quantity({"reading", "how often the forecast is read; Android forecasts only if asked at least every 10 s",
                    Affects::look},
                   h.reading, Measure::life_time, {1, 10});
        v.quantity({"calm", "how long the forecast stays below near before any share comes back", Affects::look},
                   h.calm, Measure::life_time, {1, 3'600});
        v.quantity({"give_back", "the share given back at each calm reading after that", Affects::look}, h.give_back,
                   Measure::ratio, {1'000, 1'000'000});
    }
};

}  // namespace kd::run
