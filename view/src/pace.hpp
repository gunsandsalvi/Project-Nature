// The speed loop (A3.9): each frame the screen's game time moves by the speed asked times the frame's real time, but
// never past the world's frontier, so when the world can't keep up time slows instead of the screen waiting
// (PRN-11); the world is asked to stay about a quarter of a real second ahead; pausing glides to the frontier within
// that quarter second, so what is shown is exactly the world's state; and the speed shown is measured from what was
// drawn, so it is always the real one (TIM-01). It touches no Godot, so its tests run alone.
#pragma once

#include <cstdint>
#include <deque>

namespace kd::view {

/// Implements TIM-01 and TIM-10, see A3.9: the screen's game time and the goal it sets the world.
class Pace {
public:
    /// How far ahead of the screen the world is asked to stay, in real seconds (A3.9).
    static constexpr double kLead = 0.25;
    /// The real time the speed shown is measured over.
    static constexpr double kWindow = 1.0;

    /// The loop with the screen at a moment, running at one game second a real second.
    explicit Pace(double start = 0.0) : screen_(start) {}

    /// The speed asked, in game seconds a real second, at least 1.
    void set_speed(double game_per_real);
    [[nodiscard]] double speed() const { return speed_; }
    /// The most the speed may be, whatever is asked, such as while the phone is hot (A3.9); at least 1.
    void set_limit(double game_per_real);
    [[nodiscard]] double limit() const { return limit_; }

    /// Pausing: the world is asked to go no further, and the screen glides to it within kLead real seconds.
    void pause();
    void play();
    [[nodiscard]] bool paused() const { return paused_; }

    /// One frame: the real seconds since the last and the world's frontier now. Moves the screen's time and returns
    /// the goal to set the world.
    std::int64_t frame(double real_seconds, std::int64_t frontier);

    /// The screen's game time, in game seconds with their fractions.
    [[nodiscard]] double screen() const { return screen_; }

    /// Game seconds a real second over the last kWindow of real time drawn: 0 once paused and still.
    [[nodiscard]] double speed_shown() const;

private:
    struct Drawn {
        double real;
        double game;
    };

    [[nodiscard]] double rate() const { return speed_ < limit_ ? speed_ : limit_; }

    double screen_;
    double speed_ = 1.0;
    double limit_ = 1.0e300;
    bool paused_ = false;
    double since_pause_ = 0.0;
    std::deque<Drawn> drawn_;
    double drawn_real_ = 0.0;  // the real time in drawn_, to know when the oldest frame leaves the window
};

}  // namespace kd::view
