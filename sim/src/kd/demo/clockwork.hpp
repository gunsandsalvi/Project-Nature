// A stand-in world for the foundations' demonstrations (MAT-16): until the world exists, it does a fixed amount of
// work for every game hour that passes, so the calendar on the phone runs as fast as the phone allows and the speed
// loop can be seen keeping up and falling behind.
#pragma once

#include <cstdint>

#include "kd/run/runner.hpp"

namespace kd::demo {

/// Implements PLT-01 and TIM-01, see A3.9: the work stands for a world's, so top speed shows what the phone can do.
class Clockwork final : public run::Steppable {
public:
    /// work_per_hour mixing steps for every game hour, at least 1.
    explicit Clockwork(std::uint64_t work_per_hour);

    /// Up to one game day a batch: the work of each game hour that begins within it.
    time::Seconds advance(time::Seconds frontier, time::Seconds goal) override;

    /// A digest of all the work done so far, which depends only on the hours passed, never on how the batches fell;
    /// read it only while the runner sleeps.
    [[nodiscard]] std::uint64_t state() const { return state_; }

private:
    std::uint64_t work_;
    std::uint64_t state_ = 0;
};

}  // namespace kd::demo
