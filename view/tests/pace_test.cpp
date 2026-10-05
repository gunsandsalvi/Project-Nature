#include <algorithm>
#include <cmath>
#include <cstdint>

#include "doctest.h"
#include "pace.hpp"

using kd::view::Pace;

namespace {

// A stand-in world: it moves toward its goal at most `capacity` game seconds a real second, and reports the whole
// seconds it has done, as the runner does.
struct StandIn {
    double capacity;
    double at = 0.0;
    std::int64_t goal = 0;
    void work(double real) { at = std::min(static_cast<double>(goal), at + capacity * real); }
    [[nodiscard]] std::int64_t frontier() const { return static_cast<std::int64_t>(std::floor(at)); }
};

// Frames of dt real seconds for a real duration: the world works through each frame toward the goal the last one set,
// then the screen draws it and sets the next; check runs after every frame.
template <typename Check>
void frames(Pace& pace, StandIn& world, double seconds, double dt, Check check) {
    const auto count = static_cast<int>(std::ceil(seconds / dt - 1e-9));
    for (int i = 0; i < count; ++i) {
        world.work(dt);
        world.goal = pace.frame(dt, world.frontier());
        check();
    }
}

constexpr double k60 = 1.0 / 60.0;

}  // namespace

// checks: TIM-01
TEST_CASE("the screen keeps the speed asked when the world keeps up, never passing its frontier") {
    for (double speed : {1.0, 60.0, 480.0, 21'600.0, 259'200.0}) {
        CAPTURE(speed);
        Pace pace;
        pace.set_speed(speed);
        StandIn world{1e9};
        frames(pace, world, 1.5, k60, [&] { REQUIRE(pace.screen() <= static_cast<double>(world.frontier())); });
        double before = pace.screen();
        frames(pace, world, 3.0, k60, [&] {
            REQUIRE(pace.screen() <= static_cast<double>(world.frontier()));
            // never waits: every frame moves by the speed asked
            REQUIRE(std::fabs(pace.screen() - before - speed * k60) <= speed * k60 * 1e-6);
            before = pace.screen();
            REQUIRE(std::fabs(pace.speed_shown() - speed) <= speed * 0.01);
        });
        // the world is asked to stay about a quarter of a real second ahead, and no further
        CHECK(world.goal <= static_cast<std::int64_t>(std::ceil(pace.screen() + std::max(1.0, speed * Pace::kLead))));
    }
}

// checks: TIM-01
TEST_CASE("when the world can't keep up, time slows to the world's pace and the speed shown says so") {
    Pace pace;
    pace.set_speed(21'600.0);
    StandIn world{1'000.0};
    frames(pace, world, 3.0, k60, [&] { REQUIRE(pace.screen() <= static_cast<double>(world.frontier())); });
    CHECK(pace.speed_shown() == doctest::Approx(1'000.0).epsilon(0.02));
}

// checks: TIM-01
TEST_CASE("pausing stops the screen within a quarter of a second, on the world's frontier") {
    for (double speed : {1.0, 480.0, 259'200.0}) {
        CAPTURE(speed);
        Pace pace;
        pace.set_speed(speed);
        StandIn world{1e9};
        frames(pace, world, 2.0, k60, [] {});
        // and after a drop in speed, when the world may still be far ahead of the screen
        pace.set_speed(1.0);
        frames(pace, world, 0.1, k60, [] {});
        pace.pause();
        double waited = 0.0;
        frames(pace, world, Pace::kLead + k60, k60, [&] { waited += k60; });
        const double shown = pace.screen();
        CHECK(shown == static_cast<double>(world.frontier()));
        frames(pace, world, 2.0, k60, [&] { REQUIRE(pace.screen() == shown); });
        CHECK(pace.speed_shown() == 0.0);
        // and play carries on from there
        pace.play();
        frames(pace, world, 1.5, k60, [] {});
        CHECK(pace.screen() > shown);
    }
}

// checks: TIM-01
TEST_CASE("the speed shown is within 1% of the rate drawn over each second, however the frames fall") {
    Pace pace;
    pace.set_speed(480.0);
    StandIn world{1e9};
    const double steps[] = {1.0 / 60.0, 1.0 / 30.0, 1.0 / 120.0, 1.0 / 45.0};
    double real = 0.0;
    double mark_real = 0.0;
    double mark_screen = 0.0;
    for (int i = 0; i < 2'000; ++i) {
        const double dt = steps[i % 4];
        world.work(dt);
        world.goal = pace.frame(dt, world.frontier());
        real += dt;
        if (real - mark_real >= 1.0) {
            const double drawn = (pace.screen() - mark_screen) / (real - mark_real);
            if (mark_real > 0.0) {
                REQUIRE(std::fabs(pace.speed_shown() - drawn) <= drawn * 0.01);
            }
            mark_real = real;
            mark_screen = pace.screen();
        }
    }
}

// checks: TIM-10
TEST_CASE("at one game second a real second, a game minute takes a real minute") {
    Pace pace;
    StandIn world{1e9};
    frames(pace, world, 1.0, k60, [] {});
    const double start = pace.screen();
    frames(pace, world, 60.0, k60, [] {});
    CHECK(pace.screen() - start == doctest::Approx(60.0).epsilon(1.0 / 60.0));
}
