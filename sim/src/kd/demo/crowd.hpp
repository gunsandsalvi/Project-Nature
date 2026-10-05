// The demonstration's crowd (MAT-16): how many camps and markers the foundations' crowd has, and how long two markers
// greet when they meet, as data/demo/tuning/crowd.toml.
#pragma once

#include <cstdint>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
#include "kd/time/duration.hpp"

namespace kd::demo {

/// Implements MAT-16, see A3.6: the crowd's tuning.
struct Crowd {
    std::int64_t camps = 0;
    std::int64_t per_camp = 0;
    time::Duration greeting{};

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        using data::Affects;
        v.whole({"camps", "how many camps the crowd lives in", Affects::rules}, c.camps, {1, 10'000});
        v.whole({"per_camp", "how many markers each camp holds", Affects::rules}, c.per_camp, {1, 1'000});
        v.duration({"greeting", "how long two markers who meet stand and greet", Affects::rules}, c.greeting);
    }
};

}  // namespace kd::demo
