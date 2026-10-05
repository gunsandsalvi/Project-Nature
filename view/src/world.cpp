#include "world.hpp"

#include <godot_cpp/core/class_db.hpp>

#include "kd/core/check.hpp"
#include "kd/num/convert.hpp"
#include "kd/time/calendar.hpp"

namespace kd::view {

KdWorld::KdWorld() = default;

KdWorld::~KdWorld() {
    // the runner's thread stops before the world it runs goes
    runner_.reset();
}

void KdWorld::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("start_clockwork", "work_per_hour"), &KdWorld::start_clockwork);
    ClassDB::bind_method(D_METHOD("set_speed", "game_per_real"), &KdWorld::set_speed);
    ClassDB::bind_method(D_METHOD("speed"), &KdWorld::speed);
    ClassDB::bind_method(D_METHOD("pause"), &KdWorld::pause);
    ClassDB::bind_method(D_METHOD("play"), &KdWorld::play);
    ClassDB::bind_method(D_METHOD("is_paused"), &KdWorld::is_paused);
    ClassDB::bind_method(D_METHOD("frame"), &KdWorld::frame);
    ClassDB::bind_method(D_METHOD("screen_time"), &KdWorld::screen_time);
    ClassDB::bind_method(D_METHOD("frontier"), &KdWorld::frontier);
    ClassDB::bind_method(D_METHOD("speed_shown"), &KdWorld::speed_shown);
    ClassDB::bind_method(D_METHOD("date_text"), &KdWorld::date_text);
    ClassDB::bind_method(D_METHOD("time_text"), &KdWorld::time_text);
}

void KdWorld::start_clockwork(int64_t work_per_hour) {
    KD_CHECK(!runner_, "view::KdWorld: the world has already started");
    KD_CHECK(work_per_hour >= 1, "view::KdWorld: the clockwork needs some work for each hour");
    clockwork_ = std::make_unique<demo::Clockwork>(static_cast<std::uint64_t>(work_per_hour));
    runner_ = std::make_unique<run::Runner>(*clockwork_, 0);
}

void KdWorld::set_speed(double game_per_real) {
    pace_.set_speed(game_per_real);
}

double KdWorld::speed() const {
    return pace_.speed();
}

void KdWorld::pause() {
    pace_.pause();
}

void KdWorld::play() {
    pace_.play();
}

bool KdWorld::is_paused() const {
    return pace_.paused();
}

void KdWorld::frame() {
    if (!runner_) {
        return;
    }
    // each frame's own real time, from the steady clock: Godot's delta is smoothed and can hide a stall (research 18)
    const auto now = std::chrono::steady_clock::now();
    const double real = framed_ ? std::chrono::duration<double>(now - last_frame_).count() : 0.0;
    last_frame_ = now;
    framed_ = true;
    runner_->set_goal(pace_.frame(real, runner_->frontier()));
}

double KdWorld::screen_time() const {
    return pace_.screen();
}

int64_t KdWorld::frontier() const {
    return runner_ ? runner_->frontier() : 0;
}

double KdWorld::speed_shown() const {
    return pace_.speed_shown();
}

godot::String KdWorld::date_text() const {
    const time::Date d = time::date_of(num::to_int(pace_.screen(), num::Round::down));
    return time::date_text(d).c_str();
}

godot::String KdWorld::time_text() const {
    const time::Date d = time::date_of(num::to_int(pace_.screen(), num::Round::down));
    return time::time_of_day_text(d).c_str();
}

}  // namespace kd::view
