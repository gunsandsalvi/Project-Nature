// The world's runner (A3.9): the world on its own simulation thread, working toward a goal in game time and sleeping
// once it gets there. The screen sets the goal a little ahead of what it shows and reads how far the world has got,
// its frontier, so neither ever waits for the other (A3.8).
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "kd/run/thread.hpp"
#include "kd/time/calendar.hpp"

namespace kd::run {

/// What a runner advances: the world, or a stand-in for it.
class Steppable {
public:
    virtual ~Steppable() = default;
    /// Works from the frontier toward the goal for one batch, and returns the new frontier: past the old one and
    /// never past the goal. The batch is the world's to choose; a cut between batches never changes the result.
    virtual time::Seconds advance(time::Seconds frontier, time::Seconds goal) = 0;
};

/// Implements PLT-01 and TIM-01, see A3.9: the world on its own thread, working toward a goal and sleeping there.
class Runner {
public:
    /// Starts the runner's thread with the world at the moment start, where it sleeps until a goal is set.
    Runner(Steppable& world, time::Seconds start, std::string name = "kd-world");
    ~Runner();
    Runner(const Runner&) = delete;
    Runner& operator=(const Runner&) = delete;

    /// Asks the world to reach this moment. A goal at or behind the frontier lets it sleep where it is, after the
    /// batch it is in.
    void set_goal(time::Seconds goal);

    /// How far the world has got: every moment before it is done.
    [[nodiscard]] time::Seconds frontier() const { return frontier_.load(std::memory_order_acquire); }

    /// Waits until the frontier reaches the moment, which the goal must have reached.
    void wait_for(time::Seconds moment);

    /// A job for the runner's thread between two batches, such as a command or a save, run even while the world
    /// sleeps at its goal, after every job given before it.
    void call(std::function<void()> job);
    /// The same, waiting until the job has run.
    void call_and_wait(std::function<void()> job);

    /// The runner's own thread, for the self-check.
    [[nodiscard]] const Thread& thread() const { return *thread_; }

private:
    void loop();

    Steppable& world_;
    std::atomic<time::Seconds> frontier_;
    std::mutex mutex_;
    std::condition_variable wake_;
    std::condition_variable reached_;
    time::Seconds goal_;
    std::deque<std::function<void()>> jobs_;
    std::uint64_t jobs_given_ = 0;
    std::uint64_t jobs_done_ = 0;
    bool stopping_ = false;
    // last, so the thread stops before what it uses is gone
    std::unique_ptr<Thread> thread_;
};

}  // namespace kd::run
