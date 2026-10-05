#include <chrono>
#include <thread>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/demo/clockwork.hpp"
#include "kd/num/fenv.hpp"
#include "kd/num/whole.hpp"
#include "kd/run/runner.hpp"

namespace kt = kd::time;

namespace {

// The clockwork advanced without a runner, batch after batch, to a goal.
std::uint64_t worked_alone(kt::Seconds start, kt::Seconds goal) {
    kd::demo::Clockwork cw(50);
    for (kt::Seconds at = start; at < goal;) {
        at = cw.advance(at, goal);
    }
    return cw.state();
}

}  // namespace

// checks: PLT-01 TIM-01
TEST_CASE("the runner reaches its goal and sleeps there, never past it") {
    kd::demo::Clockwork cw(50);
    kd::run::Runner runner(cw, 0);
    const kt::Seconds goal = 10 * kt::kDay + 5;
    runner.set_goal(goal);
    runner.wait_for(goal);
    CHECK(runner.frontier() == goal);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    CHECK(runner.frontier() == goal);
    CHECK(cw.state() == worked_alone(0, goal));
}

// checks: PLT-01 TIM-01
TEST_CASE("however the goals fall, the work done is the same") {
    const kt::Seconds goal = 3 * kt::kDay + 7 * kt::kHour;
    kd::demo::Clockwork steps(50);
    {
        kd::run::Runner runner(steps, -kt::kHour);
        for (kt::Seconds g = 0; g <= goal; g += 1'234) {
            runner.set_goal(g);
            runner.wait_for(g);
        }
        runner.set_goal(goal);
        runner.wait_for(goal);
    }
    CHECK(steps.state() == worked_alone(-kt::kHour, goal));
    CHECK(worked_alone(0, goal) != worked_alone(0, goal + kt::kHour));
    CHECK(kd::num::floor_div(-1, kt::kHour) == -1);
    CHECK(kd::num::floor_mod(-1, kt::kHour) == kt::kHour - 1);
}

// checks: PLT-01 TIM-01
TEST_CASE("a goal behind the frontier lets the runner sleep where it is") {
    kd::demo::Clockwork cw(2'000);
    kd::run::Runner runner(cw, 0);
    const kt::Seconds far = 1'000'000 * kt::kDay;
    runner.set_goal(far);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    runner.set_goal(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    const kt::Seconds stopped = runner.frontier();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    CHECK(runner.frontier() == stopped);
    CHECK(stopped < far);
}

// checks: PLT-01
TEST_CASE("the runner's thread starts in the default environment and with its own stack") {
    kd::test::set_flush_to_zero();
    {
        kd::demo::Clockwork cw(10);
        kd::run::Runner runner(cw, 0);
        runner.set_goal(kt::kDay);
        runner.wait_for(kt::kDay);
        CHECK_FALSE(kd::num::fenv_is_default(runner.thread().inherited_fenv()));
        CHECK(runner.thread().stack() >= (std::size_t{8} << 20));
    }
    kd::num::fenv_reset();
}
