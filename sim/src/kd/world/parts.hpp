// The world's own components (A3.2, A3.3): where a thing is, what a doer is doing, and the events an owner waits for.
// Each is a plain struct with its descriptor (kd/ecs/component.hpp).
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "kd/num/torus.hpp"
#include "kd/time/calendar.hpp"

namespace kd::world {

/// Implements WLD-01, see A3.4: where something that stays put is, such as a camp.
struct Place {
    static constexpr std::string_view name = "place";
    static constexpr std::uint32_t version = 1;
    num::Point at;

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.point({"at", "where it is, in whole centimetres on the torus"}, c.at);
    }
};

/// Implements TIM-17, see A3.3: what a doer is doing, from its start to its end, and its way, so it can be seen or
/// met at any second between; it ends at its owner's event.
struct Activity {
    static constexpr std::string_view name = "activity";
    static constexpr std::uint32_t version = 1;
    /// What it is, as the doer's system numbers its activities, such as rest, walk and sleep.
    std::uint8_t what = 0;
    time::Seconds start = 0;
    time::Seconds end = 0;
    num::Point from;
    num::Point to;

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u8({"what", "what the doer is doing, as its system numbers it"}, c.what);
        v.i64({"start", "the game second it began"}, c.start);
        v.i64({"end", "the game second it ends, at its owner's event"}, c.end);
        v.point({"from", "where it began"}, c.from);
        v.point({"to", "where it ends"}, c.to);
    }

    /// Where the doer is at a second: along the straight way, the short way round the torus, in whole centimetres
    /// rounded toward the start; before the start it is at the start, after the end at the end.
    [[nodiscard]] num::Point at(const num::Torus& torus, time::Seconds t) const;
};

/// Implements TIM-17, see A3.3: an owner's events, the sequence number its next event takes, and for each of its
/// slots the sequence it waits for, or 0 when it waits for none; an event whose sequence no longer matches is skipped.
struct Schedule {
    static constexpr std::string_view name = "schedule";
    static constexpr std::uint32_t version = 1;
    static constexpr std::uint32_t kSlots = 4;
    std::uint64_t next = 1;
    std::array<std::uint64_t, kSlots> expected{};

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"next", "the sequence number the owner's next event takes"}, c.next);
        v.u64({"slot_0", "the event its activity's end waits for, 0 when none"}, c.expected[0]);
        v.u64({"slot_1", "the event its first timer waits for, 0 when none"}, c.expected[1]);
        v.u64({"slot_2", "the event its second timer waits for, 0 when none"}, c.expected[2]);
        v.u64({"slot_3", "the event its third timer waits for, 0 when none"}, c.expected[3]);
    }
};

/// The slot of an owner's activity's end.
inline constexpr std::uint32_t kActivitySlot = 0;

}  // namespace kd::world
