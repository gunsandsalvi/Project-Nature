// The speeds of time at the zoom stops (TIM-01): the tuning file tuning/time.toml, as game time a real minute.
// They only set how fast the screen asks time to run, never what happens in it (TIM-16), so they count in the look.
#pragma once

#include <cstdint>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"

namespace kd::time {

/// Implements TIM-01, see A3.6 and A3.9: how much game time each zoom stop asks for in a real minute.
struct ZoomSpeeds {
    std::int64_t person = 0;
    std::int64_t close_camp = 0;
    std::int64_t camp = 0;
    std::int64_t valley = 0;
    std::int64_t region = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        using data::Measure;
        const data::Range minute_to_decade{60, std::int64_t{10} * 5'184'000};
        v.quantity({"person", "real speed: a game minute a real minute (TIM-10)", Affects::look}, s.person,
                   Measure::game_time, {60, 60});
        v.quantity({"close_camp", "about an hour a minute", Affects::look}, s.close_camp, Measure::game_time,
                   minute_to_decade);
        v.quantity({"camp", "a day in a few minutes", Affects::look}, s.camp, Measure::game_time, minute_to_decade);
        v.quantity({"valley", "a season in about a minute", Affects::look}, s.valley, Measure::game_time,
                   minute_to_decade);
        v.quantity({"region", "a few years a minute", Affects::look}, s.region, Measure::game_time, minute_to_decade);
    }
};

}  // namespace kd::time
