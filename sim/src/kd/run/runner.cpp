#include "kd/run/runner.hpp"

#include <utility>

#include "kd/core/check.hpp"
#include "kd/num/fenv.hpp"

namespace kd::run {

Runner::Runner(Steppable& world, time::Seconds start, std::string name)
    : world_(world), frontier_(start), goal_(start) {
    thread_ = std::make_unique<Thread>(std::move(name), [this] { loop(); });
}

Runner::~Runner() {
    {
        std::lock_guard lock(mutex_);
        stopping_ = true;
    }
    wake_.notify_all();
    thread_.reset();
}

void Runner::set_goal(time::Seconds goal) {
    {
        std::lock_guard lock(mutex_);
        goal_ = goal;
    }
    wake_.notify_all();
}

void Runner::wait_for(time::Seconds moment) {
    std::unique_lock lock(mutex_);
    KD_CHECK(goal_ >= moment, "run::Runner::wait_for: the goal is behind the moment waited for");
    reached_.wait(lock, [&] { return frontier() >= moment; });
}

void Runner::call(std::function<void()> job) {
    {
        std::lock_guard lock(mutex_);
        jobs_.push_back(std::move(job));
        ++jobs_given_;
    }
    wake_.notify_all();
}

void Runner::call_and_wait(std::function<void()> job) {
    std::unique_lock lock(mutex_);
    jobs_.push_back(std::move(job));
    const std::uint64_t mine = ++jobs_given_;
    wake_.notify_all();
    reached_.wait(lock, [&] { return jobs_done_ >= mine; });
}

void Runner::loop() {
    std::unique_lock lock(mutex_);
    for (;;) {
        wake_.wait(lock, [&] { return stopping_ || !jobs_.empty() || goal_ > frontier(); });
        // jobs first, between batches, so a command or a save meets the world at an event boundary
        while (!jobs_.empty()) {
            std::function<void()> job = std::move(jobs_.front());
            jobs_.pop_front();
            lock.unlock();
            num::fenv_assert_default();
            job();
            lock.lock();
            ++jobs_done_;
            reached_.notify_all();
        }
        if (stopping_) {
            return;
        }
        if (goal_ <= frontier()) {
            continue;
        }
        const time::Seconds goal = goal_;
        const time::Seconds from = frontier();
        lock.unlock();
        num::fenv_assert_default();
        const time::Seconds reached = world_.advance(from, goal);
        KD_CHECK(reached > from && reached <= goal, "a world must advance toward its goal and never past it");
        lock.lock();
        frontier_.store(reached, std::memory_order_release);
        reached_.notify_all();
    }
}

}  // namespace kd::run
