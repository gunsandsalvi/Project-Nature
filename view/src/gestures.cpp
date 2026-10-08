#include "gestures.hpp"

#include <iterator>

#include "kd/num/maths.hpp"

namespace kd::view {

namespace {

// An angle's change in degrees, taken the short way round, from -180 to 180.
double short_way(double degrees) {
    while (degrees > 180.0) {
        degrees -= 360.0;
    }
    while (degrees < -180.0) {
        degrees += 360.0;
    }
    return degrees;
}

}  // namespace

void Gestures::press(int finger, double x, double y, double seconds) {
    fold();
    fingers_[finger] = {x, y};
    before_[finger] = {x, y};
    motion_.touching = true;
    if (fingers_.size() == 1) {
        // a press soon after a tap and near it starts the one-thumb zoom
        thumb_ = tap_ready_ && seconds - tap_at_ <= kTapSeconds && num::hypot(x - tap_x_, y - tap_y_) <= 4.0 * kTapSlop;
        tap_ready_ = false;
        down_x_ = x;
        down_y_ = y;
        down_at_ = seconds;
        still_ = true;
        motion_.at_x = x;
        motion_.at_y = y;
        return;
    }
    // a second finger: the two-finger rules take over, and nothing that began with two fingers taps
    thumb_ = false;
    still_ = false;
    turning_ = false;
    pending_twist_ = 0.0;
}

void Gestures::move(int finger, double x, double y, double /*seconds*/) {
    const auto it = fingers_.find(finger);
    if (it == fingers_.end()) {
        return;
    }
    it->second = {x, y};
    if (still_ && num::hypot(x - down_x_, y - down_y_) > kTapSlop) {
        still_ = false;
    }
}

void Gestures::lift(int finger, double x, double y, double seconds) {
    const auto it = fingers_.find(finger);
    if (it == fingers_.end()) {
        return;
    }
    it->second = {x, y};
    fold();
    fingers_.erase(finger);
    before_.erase(finger);
    if (!fingers_.empty()) {
        still_ = false;
        return;
    }
    motion_.touching = false;
    motion_.lifted = true;
    // a quick press that stayed put is a tap, which may begin a one-thumb zoom; the end of a zoom is no tap
    tap_ready_ = still_ && !thumb_ && seconds - down_at_ <= kTapSeconds;
    tap_x_ = x;
    tap_y_ = y;
    tap_at_ = seconds;
    thumb_ = false;
    still_ = false;
}

void Gestures::fold() {
    if (fingers_.size() == 1) {
        const auto& [id, now] = *fingers_.begin();
        const Finger was = before_[id];
        if (thumb_) {
            // dragging down zooms in, about where the thumb went down
            motion_.scale *= num::exp((now.y - was.y) / kThumbZoom);
            motion_.at_x = down_x_;
            motion_.at_y = down_y_;
        } else {
            motion_.pan_x += now.x - was.x;
            motion_.pan_y += now.y - was.y;
            motion_.at_x = now.x;
            motion_.at_y = now.y;
        }
    } else if (fingers_.size() >= 2) {
        // the first two fingers, by their numbers, are read as a pair
        const auto first = fingers_.begin();
        const auto second = std::next(first);
        const Finger a1 = first->second;
        const Finger b1 = second->second;
        const Finger a0 = before_[first->first];
        const Finger b0 = before_[second->first];
        motion_.pan_x += (a1.x + b1.x - a0.x - b0.x) / 2.0;
        motion_.pan_y += (a1.y + b1.y - a0.y - b0.y) / 2.0;
        motion_.at_x = (a1.x + b1.x) / 2.0;
        motion_.at_y = (a1.y + b1.y) / 2.0;
        const double spread0 = num::hypot(b0.x - a0.x, b0.y - a0.y);
        const double spread1 = num::hypot(b1.x - a1.x, b1.y - a1.y);
        // fingers on top of each other have no spread or direction
        if (spread0 >= 1.0 && spread1 >= 1.0) {
            motion_.scale *= spread1 / spread0;
            // y grows downward, so a growing angle turns clockwise on the screen
            const double turned =
                short_way((num::atan2pi(b1.y - a1.y, b1.x - a1.x) - num::atan2pi(b0.y - a0.y, b0.x - a0.x)) * 180.0);
            if (!can_turn_) {
                // T2.7a.2, PRE-02: fixed local cameras have no turning route.
                pending_twist_ = 0.0;
            } else if (turning_) {
                motion_.twist += turned;
            } else {
                // a twist begins only once the fingers have turned kTwistStart together, so a pinch never turns the
                // view; from then on the view turns with them, less those first degrees
                pending_twist_ += turned;
                if (pending_twist_ >= kTwistStart) {
                    turning_ = true;
                    motion_.twist += pending_twist_ - kTwistStart;
                } else if (pending_twist_ <= -kTwistStart) {
                    turning_ = true;
                    motion_.twist += pending_twist_ + kTwistStart;
                }
            }
        }
    }
    before_ = fingers_;
}

Motion Gestures::take() {
    fold();
    const Motion out = motion_;
    motion_.pan_x = 0.0;
    motion_.pan_y = 0.0;
    motion_.scale = 1.0;
    motion_.twist = 0.0;
    motion_.lifted = false;
    return out;
}

}  // namespace kd::view
