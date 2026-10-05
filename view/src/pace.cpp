#include "pace.hpp"

#include <algorithm>

#include "kd/core/check.hpp"
#include "kd/num/convert.hpp"

namespace kd::view {

void Pace::set_speed(double game_per_real) {
    KD_CHECK(game_per_real >= 1.0, "view::Pace: the slowest speed is one game second a real second");
    speed_ = game_per_real;
}

void Pace::pause() {
    if (!paused_) {
        paused_ = true;
        since_pause_ = 0.0;
    }
}

void Pace::play() {
    paused_ = false;
}

std::int64_t Pace::frame(double real_seconds, std::int64_t frontier) {
    KD_CHECK(real_seconds >= 0.0, "view::Pace: a frame cannot take less than no time");
    const auto edge = static_cast<double>(frontier);
    double rate = speed_;
    if (paused_) {
        // glide at whatever rate reaches the frontier kLead after the pause, however far ahead the world got
        since_pause_ += real_seconds;
        const double left = std::max(kLead - since_pause_ + real_seconds, real_seconds);
        if (left > 0.0) {
            rate = std::max(rate, (edge - screen_) / left);
        }
    }
    const double next = std::max(screen_, std::min(screen_ + rate * real_seconds, edge));
    drawn_.push_back({real_seconds, next - screen_});
    drawn_real_ += real_seconds;
    while (drawn_.size() > 1 && drawn_real_ - drawn_.front().real >= kWindow) {
        drawn_real_ -= drawn_.front().real;
        drawn_.pop_front();
    }
    screen_ = next;
    if (paused_) {
        return frontier;
    }
    return num::to_int(screen_ + std::max(1.0, speed_ * kLead), num::Round::up);
}

double Pace::speed_shown() const {
    // summed afresh each time, so nothing is left over from frames that have left the window
    double real = 0.0;
    double game = 0.0;
    for (const Drawn& d : drawn_) {
        real += d.real;
        game += d.game;
    }
    return real > 0.0 ? game / real : 0.0;
}

}  // namespace kd::view
