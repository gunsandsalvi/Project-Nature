// A simulation thread (A3.9): an explicit stack (bionic's default is about 1 MiB, glibc's usually 8), a name, a
// slightly lower priority than Godot's main thread, and the default floating-point environment, whatever its creator
// had. The workers and the world's runner are both made of these.
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include <pthread.h>

namespace kd::run {

/// Implements PLT-01 and RES-05, see A3.4 and A3.9: a thread the simulation may run on.
class Thread {
public:
    /// Starts body on a new thread with this name (cut to 15 characters), and returns once the thread has set itself
    /// up; the destructor waits for body to return.
    Thread(std::string name, std::function<void()> body);
    ~Thread();
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    /// The floating-point control register the thread found as it started, before it set the default.
    [[nodiscard]] std::uint64_t inherited_fenv() const { return inherited_fenv_; }
    /// The thread's stack size, as the system reports it.
    [[nodiscard]] std::size_t stack() const { return stack_; }

private:
    static void* start(void* self);

    std::string name_;
    std::function<void()> body_;
    pthread_t handle_{};
    std::uint64_t inherited_fenv_ = 0;
    std::size_t stack_ = 0;
    std::atomic<bool> ready_{false};
};

}  // namespace kd::run
