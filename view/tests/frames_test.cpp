#include "frames.hpp"
#include "doctest.h"

// checks: PLT-04
TEST_CASE("frames are on time within a period and half a refresh, and a stall counts each period it skipped") {
    kd::view::FrameMeter meter;
    // 60 frames a second on a 120 Hz screen: on time within 16.7 + 4.2 ms
    meter.reset(1000.0 / 60.0, 120.0);
    double t = 1000.0;
    meter.frame(t);
    for (int i = 0; i < 100; ++i) {
        t += 1000.0 / 60.0;
        meter.frame(t);
    }
    // one frame 20 ms after the last, still on time; one 24 ms after, late but no period skipped
    t += 20.0;
    meter.frame(t);
    t += 24.0;
    meter.frame(t);
    // a stall of 70 ms skips three periods and is more than 50 ms late
    t += 70.0;
    meter.frame(t);
    const kd::view::FrameMeter::Stats& s = meter.stats();
    CHECK(s.frames == 103);
    CHECK(s.on_time == 101);
    CHECK(s.stalls == 3);
    CHECK(s.late == 1);
    CHECK(s.slowest_ms == doctest::Approx(70.0));
    // a reset starts again, the first frame after it timing nothing
    meter.reset(1000.0 / 60.0, 60.0);
    meter.frame(t + 500.0);
    CHECK(meter.stats().frames == 0);
    meter.frame(t + 500.0 + 24.0);
    CHECK(meter.stats().on_time == 1);
}
