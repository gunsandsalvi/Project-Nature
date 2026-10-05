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

void Runner::loop() {
    std::unique_lock lock(mutex_);
    for (;;) {
        wake_.wait(lock, [&] { return stopping_ || goal_ > frontier(); });
        if (stopping_) {
            return;
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
