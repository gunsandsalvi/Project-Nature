// MND-23 RES-16: bounded personal experience, never a world success target.
#pragma once
#include <array>
#include "kd/ecs/component.hpp"
namespace kd::world {
struct MotivationDay {
    friend bool operator==(const MotivationDay&, const MotivationDay&) = default;
    std::int64_t day = -1;
    std::array<std::uint64_t, 2> opportunities{}, deficits{}, relief{}, requests{}, progress{};
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"day", "personal dawn interval"}, c.day);
        for (std::size_t n = 0; n < 2; ++n) {
            v.u64({"opportunities", "personally available options"}, c.opportunities[n]);
            v.u64({"deficits", "experienced unmet needs or observed inability"}, c.deficits[n]);
            v.u64({"relief", "experienced relief or answered practice"}, c.relief[n]);
            v.u64({"requests", "actual replies showing inability"}, c.requests[n]);
            v.u64({"progress", "observed learning progress"}, c.progress[n]);
        }
    }
};
struct MotivationAction {
    friend bool operator==(const MotivationAction&, const MotivationAction&) = default;
    std::uint64_t evidence = 0;
    std::int64_t progress = 0;
    std::uint8_t failures = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"evidence", "fingerprint of own relevant estimates"}, c.evidence);
        v.i64({"progress", "own best learning for this action"}, c.progress);
        v.u8({"failures", "trials without new evidence or progress"}, c.failures);
    }
};
struct Motivation {
    friend bool operator==(const Motivation&, const Motivation&) = default;
    static constexpr std::int64_t kUnit = 65536;
    std::int64_t dawn = -1, last_satisfaction = -1;
    std::array<std::int64_t, 2> pressure{}, update_remainder{}, deficit_remainder{}, relief_remainder{}, denominator{};
    std::array<MotivationDay, 7> window{};
    std::array<MotivationAction, 21> actions{};
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"dawn", "last actual dawn update"}, c.dawn);
        v.i64({"last_satisfaction", "own last experienced satisfaction"}, c.last_satisfaction);
        for (std::size_t n = 0; n < 2; ++n) {
            v.i64({"pressure", "bounded exploration or teaching pressure"}, c.pressure[n]);
            v.i64({"update_remainder", "fractional daily pressure change"}, c.update_remainder[n]);
            v.i64({"deficit_remainder", "fractional normalised deficit"}, c.deficit_remainder[n]);
            v.i64({"relief_remainder", "fractional normalised relief"}, c.relief_remainder[n]);
            v.i64({"denominator", "previous normalisation denominator"}, c.denominator[n]);
        }
        for (auto& day : c.window) MotivationDay::visit(v, day);
        for (auto& action : c.actions) MotivationAction::visit(v, action);
    }
};
}  // namespace kd::world
