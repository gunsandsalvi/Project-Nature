#include <cstdint>

#include "doctest.h"
#include "gestures.hpp"
#include "kd/num/maths.hpp"
#include "rig.hpp"

using kd::view::Gestures;
using kd::view::Motion;
using kd::view::Rig;

namespace {

// The ground under a pixel in metres from the world's centre, through the origin wherever it has moved.
void world_at(const Rig& rig, double x, double y, double& east, double& north) {
    rig.ground_at(x, y, east, north);
    east += static_cast<double>(rig.origin_east()) / 100.0;
    north += static_cast<double>(rig.origin_north()) / 100.0;
}

}  // namespace

// checks: PRE-33
TEST_CASE("one finger drags, and a tap then a press that drags zooms with one thumb") {
    Gestures g;
    g.press(0, 500.0, 1000.0, 0.0);
    g.move(0, 520.0, 1050.0, 0.05);
    Motion m = g.take();
    CHECK(m.pan_x == 20.0);
    CHECK(m.pan_y == 50.0);
    CHECK(m.scale == 1.0);
    CHECK(m.twist == 0.0);
    CHECK(m.touching);
    g.lift(0, 520.0, 1050.0, 0.1);
    m = g.take();
    CHECK(m.lifted);
    CHECK_FALSE(m.touching);
    // a tap, then a press near it soon after: dragging down zooms in, and nothing pans
    g.press(0, 500.0, 1000.0, 1.0);
    g.lift(0, 502.0, 1001.0, 1.1);
    g.take();
    g.press(0, 505.0, 1003.0, 1.25);
    CHECK(g.thumb_zoom());
    g.move(0, 505.0, 1103.0, 1.3);
    m = g.take();
    CHECK(m.pan_x == 0.0);
    CHECK(m.pan_y == 0.0);
    CHECK(m.scale == doctest::Approx(kd::num::exp(100.0 / Gestures::kThumbZoom)));
    g.lift(0, 505.0, 1103.0, 1.4);
    // the same two presses too far apart in time are a tap and a drag
    g.press(0, 500.0, 1000.0, 2.0);
    g.lift(0, 500.0, 1000.0, 2.1);
    g.press(0, 500.0, 1000.0, 2.6);
    CHECK_FALSE(g.thumb_zoom());
    g.move(0, 500.0, 1100.0, 2.7);
    m = g.take();
    CHECK(m.pan_y == 100.0);
    CHECK(m.scale == 1.0);
}

// checks: PRE-33
TEST_CASE("two fingers pinch, twist and drag, and no one of them is read as another") {
    // a pinch: the fingers part about (540, 1200), wobbling 3 degrees, which turns nothing
    Gestures g;
    g.press(0, 440.0, 1200.0, 0.0);
    g.press(1, 640.0, 1200.0, 0.0);
    g.move(0, 340.0, 1210.0, 0.1);
    g.move(1, 740.0, 1190.0, 0.1);
    Motion m = g.take();
    CHECK(m.scale == doctest::Approx(kd::num::hypot(400.0, 20.0) / 200.0));
    CHECK(m.twist == 0.0);
    CHECK(m.pan_x == doctest::Approx(0.0));
    CHECK(m.pan_y == doctest::Approx(0.0));
    g.lift(0, 340.0, 1210.0, 0.2);
    g.lift(1, 740.0, 1190.0, 0.2);
    g.take();
    // a twist of 30 degrees clockwise about the same point: the first 10 begin it, the other 20 turn the view
    g.press(0, 440.0, 1200.0, 1.0);
    g.press(1, 640.0, 1200.0, 1.0);
    for (int i = 1; i <= 30; ++i) {
        const double a = static_cast<double>(i) / 180.0;  // half turns
        g.move(0, 540.0 - 100.0 * kd::num::cospi(a), 1200.0 - 100.0 * kd::num::sinpi(a), 1.0 + 0.01 * i);
        g.move(1, 540.0 + 100.0 * kd::num::cospi(a), 1200.0 + 100.0 * kd::num::sinpi(a), 1.0 + 0.01 * i);
    }
    m = g.take();
    CHECK(m.twist == doctest::Approx(20.0).epsilon(1e-9));
    CHECK(m.scale == doctest::Approx(1.0));
    CHECK(m.pan_x == doctest::Approx(0.0));
    g.lift(0, 0.0, 0.0, 1.5);
    g.lift(1, 0.0, 0.0, 1.5);
    g.take();
    // a two-finger drag: no pinch, no twist
    g.press(0, 440.0, 1200.0, 2.0);
    g.press(1, 640.0, 1200.0, 2.0);
    g.move(0, 440.0, 1300.0, 2.1);
    g.move(1, 640.0, 1300.0, 2.1);
    m = g.take();
    CHECK(m.pan_y == doctest::Approx(100.0));
    CHECK(m.scale == doctest::Approx(1.0));
    CHECK(m.twist == 0.0);
}

// checks: PRE-33
TEST_CASE("the ground under the fingers stays under them as they drag, pinch and twist") {
    Rig rig;
    rig.set_screen(1080.0, 2404.0);
    rig.set_metres_per_pixel(0.02);
    double e0 = 0.0;
    double n0 = 0.0;
    double e1 = 0.0;
    double n1 = 0.0;
    world_at(rig, 300.0, 600.0, e0, n0);
    rig.drag(150.0, 400.0, 450.0, 1000.0);
    world_at(rig, 450.0, 1000.0, e1, n1);
    CHECK(e1 == doctest::Approx(e0).epsilon(1e-9));
    CHECK(n1 == doctest::Approx(n0).epsilon(1e-9));
    world_at(rig, 700.0, 500.0, e0, n0);
    rig.pinch(1.6, 700.0, 500.0);
    CHECK(rig.metres_per_pixel() == doctest::Approx(0.02 / 1.6));
    world_at(rig, 700.0, 500.0, e1, n1);
    CHECK(e1 == doctest::Approx(e0).epsilon(1e-9));
    CHECK(n1 == doctest::Approx(n0).epsilon(1e-9));
    world_at(rig, 200.0, 1800.0, e0, n0);
    rig.twist(25.0, 200.0, 1800.0);
    CHECK(rig.heading() == doctest::Approx(335.0));
    world_at(rig, 200.0, 1800.0, e1, n1);
    CHECK(e1 == doctest::Approx(e0).epsilon(1e-9));
    CHECK(n1 == doctest::Approx(n0).epsilon(1e-9));
    // the zoom stops at the closest, and the point under the fingers still holds
    rig.pinch(100.0, 540.0, 1202.0);
    CHECK(rig.metres_per_pixel() == Rig().metres_per_pixel());
}

// checks: PRE-33
TEST_CASE("when the fingers lift, the rig eases to rest on 5-degree turns and 1.25-times zooms") {
    Rig rig;
    rig.hold(true);
    rig.twist(-12.0, 540.0, 1202.0);
    rig.pinch(1.0 / 2.1, 540.0, 1202.0);
    rig.step(1.0);
    CHECK_FALSE(rig.resting());
    CHECK(rig.heading() == doctest::Approx(12.0));
    rig.hold(false);
    for (int i = 0; i < 120 && !rig.resting(); ++i) {
        rig.step(1.0 / 60.0);
    }
    CHECK(rig.resting());
    CHECK(rig.heading() == 10.0);
    // 2.1 times the closest rests on 1.25 to the third, 1.953125 times
    CHECK(rig.metres_per_pixel() == doctest::Approx(Rig().metres_per_pixel() * 1.953125).epsilon(1e-12));
}

// checks: PLT-02 PRE-22
TEST_CASE("turning the phone keeps the focus, the heading and the texture pixel's size") {
    Rig rig;
    rig.set_screen(1080.0, 2404.0);
    rig.set_focus(123'456, -7'890);
    rig.set_heading(40.0);
    rig.set_metres_per_pixel(0.03);
    const int band = rig.band();
    const double texel = rig.texel_pixels(band);
    rig.set_screen(2404.0, 1080.0);
    CHECK(rig.focus_east() == 123'456);
    CHECK(rig.focus_north() == -7'890);
    CHECK(rig.heading() == 40.0);
    CHECK(rig.metres_per_pixel() == 0.03);
    CHECK(rig.band() == band);
    CHECK(rig.texel_pixels(rig.band()) == texel);
    CHECK(rig.pose().wide);
}

// The closest zoom is the person's stop: a texture pixel 2 screen pixels wide, so a person 100 texture pixels tall
// stands about 200 screen pixels.
// checks: PRE-03 PRE-22
TEST_CASE("at every zoom the band's texture pixel is 1.4 to 2.8 screen pixels wide at the focus") {
    Rig rig;
    int bands_seen = 0;
    int last = -1;
    // from the closest zoom to the farthest, 1% at a time
    for (int step = 0; step <= 418; ++step) {
        rig.set_metres_per_pixel(kd::num::pow(1.01, static_cast<double>(step)) / 128.0);
        const double px = rig.texel_pixels(rig.band());
        CHECK(px >= Rig::kTexelLeast * (1.0 - 1e-12));
        CHECK(px < 2.0 * Rig::kTexelLeast * (1.0 + 1e-12));
        if (rig.band() != last) {
            ++bands_seen;
            last = rig.band();
        }
    }
    CHECK(bands_seen == 7);  // bands 0 to 6, 64 texture pixels a metre to 1
    rig.set_metres_per_pixel(1.0 / 128.0);
    CHECK(rig.band() == 0);
    CHECK(rig.texel_pixels(0) == 2.0);
}

// checks: PRE-22
TEST_CASE("far from the world's centre the origin follows the focus, which stays exact to the centimetre") {
    Rig rig;
    rig.set_metres_per_pixel(0.5);
    rig.set_focus(4'000'000'000, -3'000'000'000);  // 40,000 km east, 30,000 km south
    double e = 0.0;
    double n = 0.0;
    for (int i = 0; i < 100; ++i) {
        rig.drag(0.0, -100.0, 540.0, 1102.0);  // the fingers pull the ground up: the view goes south
    }
    rig.ground_at(540.0, 1202.0, e, n);
    CHECK(rig.origin_north() != -3'000'000'000);
    CHECK(e < Rig::kOriginReach);
    CHECK(n > -Rig::kOriginReach);
    CHECK(rig.focus_east() == 4'000'000'000);
    CHECK(rig.focus_north() < -3'000'000'000);
}
