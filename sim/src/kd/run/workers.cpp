#include "kd/run/workers.hpp"

#include "kd/core/check.hpp"
#include "kd/num/fenv.hpp"

namespace kd::run {

Workers::Workers(int count, const std::string& name) {
    KD_CHECK(count >= 1 && count <= 16, "the simulation runs on 1 to 16 threads");
    threads_.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        threads_.push_back(std::make_unique<Thread>(name + "-" + std::to_string(i), [this] { loop(); }));
    }
}

Workers::~Workers() {
    {
        std::lock_guard lock(mutex_);
        stopping_ = true;
    }
    wake_.notify_all();
    threads_.clear();
}

void Workers::loop() {
    std::uint64_t seen = 0;
    for (;;) {
        Call call = nullptr;
        void* task = nullptr;
        std::size_t total = 0;
        {
            std::unique_lock lock(mutex_);
            wake_.wait(lock, [&] { return stopping_ || generation_ != seen; });
            if (stopping_) {
                return;
            }
            seen = generation_;
            call = call_;
            task = task_;
            total = total_;
        }
        num::fenv_assert_default();
        for (std::size_t i = next_.fetch_add(1); i < total; i = next_.fetch_add(1)) {
            call(task, i);
        }
        {
            std::lock_guard lock(mutex_);
            --busy_;
        }
        done_.notify_all();
    }
}

void Workers::run(std::size_t n, Call call, void* task) {
    if (n == 0) {
        return;
    }
    {
        std::lock_guard lock(mutex_);
        call_ = call;
        task_ = task;
        total_ = n;
        next_.store(0);
        busy_ = count();
        ++generation_;
    }
    wake_.notify_all();
    std::unique_lock lock(mutex_);
    done_.wait(lock, [&] { return busy_ == 0; });
}

std::vector<std::uint64_t> Workers::inherited_fenv() const {
    std::vector<std::uint64_t> out;
    out.reserve(threads_.size());
    for (const auto& t : threads_) {
        out.push_back(t->inherited_fenv());
    }
    return out;
}

std::vector<std::size_t> Workers::stack_sizes() const {
    std::vector<std::size_t> out;
    out.reserve(threads_.size());
    for (const auto& t : threads_) {
        out.push_back(t->stack());
    }
    return out;
}

}  // namespace kd::run
