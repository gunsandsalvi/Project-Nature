// P6's threads: see pool.hpp. Pre-production code (research 00).
#include "pool.hpp"

namespace minds {

Pool::Pool(int threads) : threads_(threads < 1 ? 1 : threads) {
    workers_.reserve(static_cast<std::size_t>(threads_ - 1));
    for (int t = 1; t < threads_; ++t) {
        workers_.emplace_back([this, t] { loop(t); });
    }
}

Pool::~Pool() {
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    wake_.notify_all();
    for (auto& w : workers_) {
        w.join();
    }
}

void Pool::drain(int thread) {
    for (int i = next_.fetch_add(1); i < count_; i = next_.fetch_add(1)) {
        (*work_)(i, thread);
    }
}

void Pool::run(int count, const std::function<void(int, int)>& work) {
    if (threads_ <= 1 || count <= 1) {
        for (int i = 0; i < count; ++i) {
            work(i, 0);
        }
        return;
    }
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        work_ = &work;
        count_ = count;
        next_.store(0);
        busy_ = threads_ - 1;
        ++generation_;
    }
    wake_.notify_all();
    drain(0);
    std::unique_lock<std::mutex> lock(mutex_);
    done_.wait(lock, [this] { return busy_ == 0; });
    work_ = nullptr;
}

void Pool::loop(int thread) {
    std::uint64_t seen = 0;
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            wake_.wait(lock, [this, seen] { return stop_ || generation_ != seen; });
            if (stop_) {
                return;
            }
            seen = generation_;
        }
        drain(thread);
        {
            const std::lock_guard<std::mutex> lock(mutex_);
            if (--busy_ == 0) {
                done_.notify_one();
            }
        }
    }
}

}  // namespace minds
