// The saves' tuning (A3.7): base/tuning/saves.toml, how often a world is saved while it runs. Only the screen reads
// it, so it counts in the look, never in the world's rules.
#pragma once

#include <cstdint>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"

namespace kd::run {

/// Implements PLT-07, see A3.7: how often a running world is saved.
struct SaveTuning {
    std::int64_t every = 0;  // seconds of life

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        using data::Measure;
        v.quantity({"every", "how often a running world is saved, in real time", Affects::look}, s.every,
                   Measure::life_time, {5, 600});
    }
};

}  // namespace kd::run
