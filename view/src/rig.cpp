#include "rig.hpp"

#include <algorithm>

#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"

namespace kd::view {

namespace {

// Sine and cosine of an angle in degrees, through kd::num's half turns.
double sin_deg(double degrees) {
    return num::sinpi(degrees / 180.0);
}
double cos_deg(double degrees) {
    return num::cospi(degrees / 180.0);
}

// An angle in degrees brought into 0 to 360.
double wrap360(double degrees) {
    while (degrees >= 360.0) {
        degrees -= 360.0;
    }
    while (degrees < 0.0) {
        degrees += 360.0;
    }
    return degrees;
}

// An angle's change in degrees, the short way round.
double short_way(double degrees) {
    const double d = wrap360(degrees);
    return d > 180.0 ? d - 360.0 : d;
}

}  // namespace

Rig::Rig(RigTuning tuning) : tuning_(tuning), mpp_(tuning.closest) {}

void Rig::set_screen(double width, double height) {
    if (width >= 1.0 && height >= 1.0) {
        width_ = width;
        height_ = height;
    }
}

void Rig::set_lens(double degrees) {
    tuning_.lens = std::clamp(degrees, 1.0, 60.0);
}

double Rig::distance() const {
    return mpp_ * short_side() / 2.0 / num::tanpi(tuning_.lens / 360.0);
}

void Rig::ground_at(double x, double y, double& east, double& north) const {
    offset_at(x, y, east, north);
    east += east_;
    north += north_;
}

void Rig::offset_at(double x, double y, double& east, double& north) const {
    // the ray from the camera through the pixel, met with the flat ground through the focus
    const double h = heading_;
    const double fwd_e = sin_deg(h);
    const double fwd_n = cos_deg(h);
    const double right_e = cos_deg(h);
    const double right_n = -sin_deg(h);
    const double st = sin_deg(tuning_.tilt);
    const double ct = cos_deg(tuning_.tilt);
    const double per_pixel = num::tanpi(tuning_.lens / 360.0) / (short_side() / 2.0);
    const double sx = (x - width_ / 2.0) * per_pixel;
    const double sy = (y - height_ / 2.0) * per_pixel;
    const double d = distance();
    // the ray's direction: forward, then right by sx and down by sy on the picture's plane at distance 1
    const double dir_e = ct * fwd_e + sx * right_e - sy * st * fwd_e;
    const double dir_n = ct * fwd_n + sx * right_n - sy * st * fwd_n;
    const double dir_up = -st - sy * ct;
    // the camera stands back from the focus by d along the view and is d sin(tilt) up; the ray falls to the ground
    const double along = dir_up < -1e-9 ? d * st / -dir_up : 0.0;
    east = -d * ct * fwd_e + along * dir_e;
    north = -d * ct * fwd_n + along * dir_n;
}

void Rig::move_focus(double east, double north) {
    east_ += east;
    north_ += north;
    rebase();
}

void Rig::rebase() {
    if (east_ > kOriginReach || east_ < -kOriginReach || north_ > kOriginReach || north_ < -kOriginReach) {
        const std::int64_t e = num::to_int(east_ * 100.0, num::Round::nearest);
        const std::int64_t n = num::to_int(north_ * 100.0, num::Round::nearest);
        origin_east_ += e;
        origin_north_ += n;
        east_ -= static_cast<double>(e) / 100.0;
        north_ -= static_cast<double>(n) / 100.0;
    }
}

void Rig::drag(double dx, double dy, double at_x, double at_y) {
    // the ground that was under the fingers comes to be under them again
    double e0 = 0.0;
    double n0 = 0.0;
    double e1 = 0.0;
    double n1 = 0.0;
    offset_at(at_x - dx, at_y - dy, e0, n0);
    offset_at(at_x, at_y, e1, n1);
    move_focus(e0 - e1, n0 - n1);
}

void Rig::pinch(double scale, double at_x, double at_y) {
    if (!(scale > 0.0)) {
        return;
    }
    const double target = std::clamp(mpp_ / scale, tuning_.closest, tuning_.farthest);
    const double s = mpp_ / target;
    // every ground offset from the focus shrinks by s, so the focus moves to keep the point under the fingers
    double e = 0.0;
    double n = 0.0;
    offset_at(at_x, at_y, e, n);
    mpp_ = target;
    move_focus(e * (1.0 - 1.0 / s), n * (1.0 - 1.0 / s));
}

void Rig::twist(double degrees, double at_x, double at_y) {
    // the ground turns with the fingers, so the view turns the other way about the point under them
    double e0 = 0.0;
    double n0 = 0.0;
    offset_at(at_x, at_y, e0, n0);
    heading_ = wrap360(heading_ - degrees);
    double e1 = 0.0;
    double n1 = 0.0;
    offset_at(at_x, at_y, e1, n1);
    move_focus(e0 - e1, n0 - n1);
}

void Rig::hold(bool touching) {
    if (touching_ && !touching) {
        easing_ = true;
    }
    if (touching) {
        easing_ = false;
    }
    touching_ = touching;
}

void Rig::step(double seconds) {
    if (!easing_ || seconds <= 0.0) {
        return;
    }
    const double turn_target = wrap360(
        static_cast<double>(num::to_int(heading_ / tuning_.turn_step, num::Round::nearest)) * tuning_.turn_step);
    const double steps = num::log(mpp_ / tuning_.closest) / num::log(tuning_.zoom_step);
    const double zoom_target = std::clamp(
        tuning_.closest * num::pow(tuning_.zoom_step, static_cast<double>(num::to_int(steps, num::Round::nearest))),
        tuning_.closest, tuning_.farthest);
    const double f = 1.0 - num::exp(-seconds / tuning_.ease);
    const double turn_left = short_way(turn_target - heading_);
    const double zoom_left = num::log(zoom_target / mpp_);
    if ((turn_left < 0.01 && turn_left > -0.01) && (zoom_left < 1e-4 && zoom_left > -1e-4)) {
        heading_ = turn_target;
        mpp_ = zoom_target;
        easing_ = false;
        return;
    }
    heading_ = wrap360(heading_ + turn_left * f);
    mpp_ = mpp_ * num::exp(zoom_left * f);
}

std::int64_t Rig::focus_east() const {
    return origin_east_ + num::to_int(east_ * 100.0, num::Round::nearest);
}

std::int64_t Rig::focus_north() const {
    return origin_north_ + num::to_int(north_ * 100.0, num::Round::nearest);
}

void Rig::set_focus(std::int64_t east, std::int64_t north) {
    origin_east_ = east;
    origin_north_ = north;
    east_ = 0.0;
    north_ = 0.0;
}

void Rig::move_by(double east, double north) {
    move_focus(east, north);
}

void Rig::set_heading(double degrees) {
    heading_ = wrap360(degrees);
}

void Rig::set_metres_per_pixel(double mpp) {
    mpp_ = std::clamp(mpp, tuning_.closest, tuning_.farthest);
}

double Rig::texel_pixels(int band) const {
    return num::exp2(static_cast<double>(band)) / (kBand0Texels * mpp_);
}

int Rig::band() const {
    // the first band whose texture pixel is at least kTexelLeast wide at the focus
    const double b = num::log2(kTexelLeast * kBand0Texels * mpp_);
    return static_cast<int>(std::clamp<std::int64_t>(num::to_int(b, num::Round::up), 0, 8));
}

Pose Rig::pose() const {
    const double d = distance();
    const double st = sin_deg(tuning_.tilt);
    const double ct = cos_deg(tuning_.tilt);
    const double fwd_e = sin_deg(heading_);
    const double fwd_n = cos_deg(heading_);
    Pose p;
    p.x = east_ - d * ct * fwd_e;
    p.y = d * st;
    p.z = -(north_ - d * ct * fwd_n);
    p.look_x = east_;
    p.look_y = 0.0;
    p.look_z = -north_;
    p.lens = tuning_.lens;
    p.near = d / 4.0;
    p.far = d * 4.0;
    p.wide = width_ > height_;
    return p;
}

}  // namespace kd::view
