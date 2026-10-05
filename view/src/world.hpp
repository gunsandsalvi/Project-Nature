// The world class (A3.8): the world as GDScript sees it. For now, until the world itself exists, the clockwork
// stand-in on its runner, and the speed loop between it and the screen (A3.9); later make, open, save, commands,
// counters and events join it here.
#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>

#include "kd/demo/clockwork.hpp"
#include "kd/run/runner.hpp"
#include "pace.hpp"

namespace kd::view {

/// Implements TIM-01, TIM-10 and PLT-01, see A3.8 and A3.9: the world on its own thread, and the screen's time
/// following it at the speed asked.
class KdWorld : public godot::RefCounted {
    GDCLASS(KdWorld, godot::RefCounted)

public:
    KdWorld();
    ~KdWorld() override;

    /// Starts the stand-in world at Year 1, spring, day 1, doing this much work for each game hour (MAT-16).
    void start_clockwork(int64_t work_per_hour);

    /// The speed asked, in game seconds a real second, at least 1.
    void set_speed(double game_per_real);
    double speed() const;
    void pause();
    void play();
    bool is_paused() const;

    /// Once a frame: measures the real time since the last frame, moves the screen's time and sets the world's goal.
    void frame();

    /// The screen's game time, in game seconds.
    double screen_time() const;
    /// How far the world has got, in whole game seconds.
    int64_t frontier() const;
    /// The speed really drawn over the last real second, in game seconds a real second.
    double speed_shown() const;
    /// The screen's date, "Year 1, spring, day 1", and hour, "06:05" (TIM-14).
    godot::String date_text() const;
    godot::String time_text() const;

protected:
    static void _bind_methods();

private:
    std::unique_ptr<demo::Clockwork> clockwork_;
    std::unique_ptr<run::Runner> runner_;
    Pace pace_;
    std::chrono::steady_clock::time_point last_frame_;
    bool framed_ = false;
};

}  // namespace kd::view
