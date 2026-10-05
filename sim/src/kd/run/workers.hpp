// The simulation's own threads (A3.9): each made with an explicit stack (bionic's default is about 1 MiB, glibc's
// usually 8), a name, a slightly lower priority than Godot's main thread, and the default floating-point environment.
// Work is cut into pieces whose number depends only on the data, never on how many threads run them (A3.4).
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include <pthread.h>

namespace kd::run {

/// Implements PLT-01, see A3.9: the threads the simulation runs on, up to the phone's four middle cores.
class Workers {
public:
    explicit Workers(int count, std::string name = "kd-sim");
    ~Workers();
    Workers(const Workers&) = delete;
    Workers& operator=(const Workers&) = delete;

    [[nodiscard]] int count() const { return static_cast<int>(threads_.size()); }

    /// Runs task(i) once for every i in [0, n), spread over the workers, and returns when all are done.
    /// Implements RES-05, see A3.4: each piece writes only what belongs to it, and the caller gathers the results
    /// in the pieces' order, so the outcome never depends on the number of threads or which one ran a piece.
    template <typename Task>
    void for_each(std::size_t n, Task&& task) {
        run(n, &call<Task>, &task);
    }

    /// The floating-point control register each worker found as it started, before it set the default.
    [[nodiscard]] std::vector<std::uint64_t> inherited_fenv() const;

    /// Each worker's stack size, as the system reports it.
    [[nodiscard]] std::vector<std::size_t> stack_sizes() const;

private:
    using Call = void (*)(void*, std::size_t);
    template <typename Task>
    static void call(void* task, std::size_t i) {
        (*static_cast<Task*>(task))(i);
    }

    void run(std::size_t n, Call call, void* task);
    void loop(int index);
    static void* start(void* self);

    struct Thread {
        Workers* owner = nullptr;
        int index = 0;
        pthread_t handle{};
        std::uint64_t inherited_fenv = 0;
        std::size_t stack = 0;
    };

    std::string name_;
    std::vector<Thread> threads_;
    std::mutex mutex_;
    std::condition_variable wake_;
    std::condition_variable done_;
    std::uint64_t generation_ = 0;
    int busy_ = 0;
    int started_ = 0;
    bool stopping_ = false;
    Call call_ = nullptr;
    void* task_ = nullptr;
    std::size_t total_ = 0;
    std::atomic<std::size_t> next_{0};
};

}  // namespace kd::run
