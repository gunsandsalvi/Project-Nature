// The triple buffer (A3.8): after each batch the simulation fills one slot and publishes it, and the screen takes the
// newest, so neither ever waits for the other and the screen never sees a slot being filled.
#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace kd::view {

/// Implements WLD-13, see A3.8: three slots, one the producer fills, one the consumer reads, and one between them.
template <typename T>
class TripleBuffer {
public:
    /// The producer's slot, to fill before publishing it.
    [[nodiscard]] T& back() { return slots_[back_]; }

    /// Makes the back slot the newest, and takes the slot between for the next fill.
    void publish() {
        const std::uint8_t between =
            middle_.exchange(static_cast<std::uint8_t>(back_ | kFresh), std::memory_order_acq_rel);
        back_ = static_cast<std::uint8_t>(between & kIndex);
    }

    /// Takes the newest slot if one was published since the last take: true if front() changed.
    bool take() {
        if ((middle_.load(std::memory_order_acquire) & kFresh) == 0) {
            return false;
        }
        const std::uint8_t between = middle_.exchange(front_, std::memory_order_acq_rel);
        front_ = static_cast<std::uint8_t>(between & kIndex);
        return true;
    }

    /// The consumer's slot, the newest it has taken.
    [[nodiscard]] const T& front() const { return slots_[front_]; }

private:
    static constexpr std::uint8_t kFresh = 4;
    static constexpr std::uint8_t kIndex = 3;

    std::array<T, 3> slots_{};
    std::uint8_t back_ = 0;
    std::atomic<std::uint8_t> middle_{1};
    std::uint8_t front_ = 2;
};

}  // namespace kd::view
