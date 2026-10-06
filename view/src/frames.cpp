#include "frames.hpp"

#include <algorithm>

#include "kd/num/convert.hpp"

namespace kd::view {

namespace {

// More than 50 ms late at 60 frames a second (PLT-04, research 18).
constexpr double kLateMs = 1000.0 / 60.0 + 50.0;

}  // namespace

void FrameMeter::reset(double period_ms, double refresh_hz) {
    period_ms_ = std::max(1.0, period_ms);
    grace_ms_ = refresh_hz > 0.0 ? 500.0 / refresh_hz : period_ms_ / 2.0;
    last_ms_ = -1.0;
    stats_ = {};
}

void FrameMeter::frame(double now_ms) {
    if (last_ms_ >= 0.0) {
        const double gap = now_ms - last_ms_;
        ++stats_.frames;
        if (gap <= period_ms_ + grace_ms_) {
            ++stats_.on_time;
        }
        stats_.stalls += std::max<std::int64_t>(0, num::to_int(gap / period_ms_, num::Round::nearest) - 1);
        if (gap > kLateMs) {
            ++stats_.late;
        }
        stats_.slowest_ms = std::max(stats_.slowest_ms, gap);
    }
    last_ms_ = now_ms;
}

FrameMeter& frame_meter() {
    static FrameMeter meter;
    return meter;
}

}  // namespace kd::view
