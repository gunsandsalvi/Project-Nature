#include "kd/event/queue.hpp"

#include <utility>

#include "kd/num/sort.hpp"

namespace kd::event {

// The heap is our own, since the library's leave ties to it (A3.4); keys are unique, so no two events ever tie.

void Queue::push(const Event& e) {
    heap_.push_back(e);
    sift_up(heap_.size() - 1);
}

Event Queue::pop() {
    KD_CHECK(!heap_.empty(), "event::Queue: nothing to pop");
    const Event top = heap_.front();
    heap_.front() = heap_.back();
    heap_.pop_back();
    if (!heap_.empty()) {
        sift_down(0);
    }
    return top;
}

void Queue::sift_up(std::size_t i) {
    while (i > 0) {
        const std::size_t parent = (i - 1) / 2;
        if (!(heap_[i].key < heap_[parent].key)) {
            return;
        }
        std::swap(heap_[i], heap_[parent]);
        i = parent;
    }
}

void Queue::sift_down(std::size_t i) {
    const std::size_t n = heap_.size();
    while (true) {
        const std::size_t left = 2 * i + 1;
        const std::size_t right = left + 1;
        std::size_t least = i;
        if (left < n && heap_[left].key < heap_[least].key) {
            least = left;
        }
        if (right < n && heap_[right].key < heap_[least].key) {
            least = right;
        }
        if (least == i) {
            return;
        }
        std::swap(heap_[i], heap_[least]);
        i = least;
    }
}

void Queue::sort_by_key(std::vector<Event>& events) {
    num::sort_strict(events.begin(), events.end(), [](const Event& a, const Event& b) { return a.key < b.key; });
}

std::optional<Queue> Queue::read(ByteReader& r) {
    std::uint64_t n = 0;
    if (!r.u64(n)) {
        return std::nullopt;
    }
    Queue q;
    Key last{};
    for (std::uint64_t i = 0; i < n; ++i) {
        Event e;
        if (!r.i64(e.key.second) || !r.u64(e.key.owner) || !r.u64(e.key.sequence) || !r.u32(e.slot)) {
            return std::nullopt;
        }
        if (i > 0 && !(last < e.key)) {
            return std::nullopt;
        }
        last = e.key;
        q.push(e);
    }
    return q;
}

}  // namespace kd::event
