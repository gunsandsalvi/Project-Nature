// Frames by our own measure (A3.9, PLT-04): Godot gives no frame statistics, and its delta is smoothed and can hide a
// stall (research 18), so the extension times every frame itself, once a frame, from the steady clock. A frame is on
// time when it comes within a frame period plus half a refresh of the one before; a stall counts every period it
// skipped; and "more than 50 ms late" is a gap of more than 66.7 ms at 60 frames a second.
#pragma once

#include <cstdint>

namespace kd::view {

/// Implements PLT-04, see A3.9: the frames since the last reset, as the phone drew them.
class FrameMeter {
public:
    /// What the frames came to.
    struct Stats {
        std::int64_t frames = 0;
        std::int64_t on_time = 0;
        /// Frame periods skipped, in all.
        std::int64_t stalls = 0;
        /// Frames more than 50 ms late, and the slowest gap, in milliseconds.
        std::int64_t late = 0;
        double slowest_ms = 0.0;
    };

    /// Starts again, with the frame period asked of the screen and its refresh rate.
    void reset(double period_ms, double refresh_hz);
    /// A frame, at this moment of the steady clock, in milliseconds.
    void frame(double now_ms);
    [[nodiscard]] const Stats& stats() const { return stats_; }

private:
    double period_ms_ = 1000.0 / 60.0;
    double grace_ms_ = 1000.0 / 120.0;
    double last_ms_ = -1.0;
    Stats stats_;
};

/// The app's one meter, which the extension feeds once a frame.
[[nodiscard]] FrameMeter& frame_meter();

}  // namespace kd::view
