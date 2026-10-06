// The demonstration's crowd (MAT-16): how many camps and markers the foundations' crowd has, and how long two markers
// greet when they meet, as data/demo/tuning/crowd.toml.
#pragma once

#include <cstdint>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
#include "kd/num/probability.hpp"
#include "kd/time/duration.hpp"

namespace kd::demo {

/// Implements MAT-16, see A3.6: the crowd's tuning.
struct Crowd {
    std::int64_t camps = 0;
    std::int64_t per_camp = 0;
    std::int64_t area = 0;    // millimetres
    std::int64_t wander = 0;  // millimetres
    std::int64_t dawn = 0;    // game seconds after midnight
    std::int64_t dusk = 0;    // game seconds after midnight
    num::Probability homeward = num::Probability::never();
    time::Duration greeting{};

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        using data::Affects;
        using data::Measure;
        v.whole({"camps", "how many camps the crowd lives in", Affects::rules}, c.camps, {1, 10'000});
        v.whole({"per_camp", "how many markers each camp holds", Affects::rules}, c.per_camp, {1, 1'000});
        v.quantity({"area", "the side of the square the camps are placed in", Affects::rules}, c.area, Measure::length,
                   {100'000, 1'000'000'000});
        v.quantity({"wander", "how far from its camp, east or north, a marker walks to", Affects::rules}, c.wander,
                   Measure::length, {10'000, 100'000'000});
        v.quantity(
            {"dawn", "when the day begins, after midnight, until the world's sky sets it (WLD-07)", Affects::rules},
            c.dawn, Measure::game_time, {0, 86'399});
        v.quantity({"dusk", "when the night begins, after midnight", Affects::rules}, c.dusk, Measure::game_time,
                   {0, 86'399});
        v.chance(
            {"homeward", "the chance a marker's next walk is back to its camp, where others gather", Affects::rules},
            c.homeward);
        v.duration({"greeting", "how long two markers who meet stand and greet", Affects::rules}, c.greeting);
    }
};

}  // namespace kd::demo
