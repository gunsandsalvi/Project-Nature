// The event queue (A3.3): all work happens at events on one queue, each with a unique key, (game second, owner's id,
// owner's sequence number), and the key alone decides the order, never the structure: events at the same second
// settle by owner, then by sequence, the same everywhere (TIM-17).
//
// Cancelling is lazy: an owner keeps the sequence it expects for each of its slots, and an event whose sequence no
// longer matches is skipped when it comes up. The queue counts such dead events, and once they pass a quarter of the
// live ones it is rebuilt without them, which no outcome can see.
#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "kd/core/bytes.hpp"
#include "kd/time/calendar.hpp"

namespace kd::event {

/// An event's key, unique in its world: its game second, its owner's id and the owner's sequence number.
struct Key {
    time::Seconds second = 0;
    std::uint64_t owner = 0;
    std::uint64_t sequence = 0;

    friend constexpr auto operator<=>(const Key&, const Key&) = default;
};

/// An event: its key, and the owner's slot it wakes, such as the end of its activity or one of its timers.
struct Event {
    Key key;
    std::uint32_t slot = 0;
};

/// Implements TIM-17, see A3.3: a binary heap of events by key, smallest first.
class Queue {
public:
    void push(const Event& e);
    [[nodiscard]] bool empty() const { return heap_.empty(); }
    [[nodiscard]] std::size_t size() const { return heap_.size(); }
    [[nodiscard]] const Event& top() const { return heap_.front(); }
    Event pop();

    /// An event in the queue has died, its owner no longer expecting it: past a quarter of the live ones, the queue
    /// is rebuilt with only the live (A3.3).
    template <typename Live>
    void died(Live live) {
        ++dead_;
        if (dead_ * 4 > heap_.size() - dead_) {
            rebuild(live);
        }
    }
    /// A dead event came up and was skipped.
    void skipped() { --dead_; }
    [[nodiscard]] std::size_t dead() const { return dead_; }

    /// Rebuilds the heap with only the live events.
    template <typename Live>
    void rebuild(Live live) {
        std::vector<Event> kept;
        kept.reserve(heap_.size() - dead_);
        for (const Event& e : heap_) {
            if (live(e)) {
                kept.push_back(e);
            }
        }
        heap_.clear();
        dead_ = 0;
        for (const Event& e : kept) {
            push(e);
        }
    }

    /// The live events in key order, as a save writes them (A3.3).
    template <typename Live>
    [[nodiscard]] std::vector<Event> live_in_order(Live live) const {
        std::vector<Event> out;
        for (const Event& e : heap_) {
            if (live(e)) {
                out.push_back(e);
            }
        }
        sort_by_key(out);
        return out;
    }

    /// The live events written in key order, each field little-endian.
    template <typename Live>
    void write(ByteWriter& w, Live live) const {
        const std::vector<Event> events = live_in_order(live);
        w.u64(events.size());
        for (const Event& e : events) {
            w.i64(e.key.second);
            w.u64(e.key.owner);
            w.u64(e.key.sequence);
            w.u32(e.slot);
        }
    }

    /// A queue as write() wrote it, or nothing if the bytes are short or out of order.
    static std::optional<Queue> read(ByteReader& r);

private:
    static void sort_by_key(std::vector<Event>& events);
    void sift_up(std::size_t i);
    void sift_down(std::size_t i);

    std::vector<Event> heap_;
    std::size_t dead_ = 0;
};

}  // namespace kd::event
