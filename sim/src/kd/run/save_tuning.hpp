// The saves' tuning (A3.7): base/tuning/saves.toml, how often a world is saved while it runs, and how little free
// space makes the game warn that the phone is nearly full. Only the screen reads it, so it counts in the look, never
// in the world's rules.
#pragma once

#include <cstdint>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"

namespace kd::run {

/// Implements PLT-07 and PLT-10, see A3.7: how often a running world is saved, and when the game warns of space.
struct SaveTuning {
    std::int64_t every = 0;       // seconds of life
    std::int64_t warn_below = 0;  // megabytes

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        using data::Measure;
        v.quantity({"every", "how often a running world is saved, in real time", Affects::look}, s.every,
                   Measure::life_time, {5, 600});
        v.whole({"warn_below", "the free space, in megabytes, below which the game warns that the phone is nearly full",
                 Affects::look},
                s.warn_below, {1, 1'000'000});
    }
};

}  // namespace kd::run
