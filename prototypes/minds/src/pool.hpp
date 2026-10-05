// P6's threads (A3.4, A3.9): a fixed pool that runs a batch of items and waits for all of them. An item's work writes
// only what the item owns, so which thread does it never changes the result: one thread and four end the same.
// Pre-production code (research 00).
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace minds {

class Pool {
public:
    explicit Pool(int threads);
    ~Pool();
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool(Pool&&) = delete;
    Pool& operator=(Pool&&) = delete;

    [[nodiscard]] int threads() const { return threads_; }
    // Runs work(item, thread) for every item from 0 up to count, the calling thread taking a share, and returns when
    // all are done.
    void run(int count, const std::function<void(int, int)>& work);

private:
    void loop(int thread);
    void drain(int thread);

    int threads_;
    std::vector<std::thread> workers_;
    std::mutex mutex_;
    std::condition_variable wake_;
    std::condition_variable done_;
    std::uint64_t generation_ = 0;
    const std::function<void(int, int)>* work_ = nullptr;
    int count_ = 0;
    std::atomic<int> next_{0};
    int busy_ = 0;
    bool stop_ = false;
};

}  // namespace minds
