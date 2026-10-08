#include "time_requests.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
#include "kd/num/maths.hpp"
namespace kd::view {
namespace {
bool rate_ok(double rate) {
    return std::isfinite(rate) && rate >= 1.0;
}
double between(double density, TimeAnchor near, TimeAnchor far) {
    const double fraction =
        std::clamp(num::log2(near.density / density) / num::log2(near.density / far.density), 0.0, 1.0);
    return num::exp(num::log(near.rate) * (1.0 - fraction) + num::log(far.rate) * fraction);
}
}  // namespace
bool TimeRequests::configure(std::vector<TimeAnchor> anchors, double top_density) {
    if (anchors.empty() || !std::isfinite(top_density) || top_density <= 0.0) return false;
    for (std::size_t i = 0; i < anchors.size(); ++i) {
        if (!std::isfinite(anchors[i].density) || anchors[i].density <= 0.0 || !rate_ok(anchors[i].rate)) return false;
        if (i != 0 && (anchors[i].density >= anchors[i - 1].density || anchors[i].rate < anchors[i - 1].rate))
            return false;
    }
    if (top_density >= anchors.back().density) return false;
    anchors_ = std::move(anchors);
    top_density_ = top_density;
    return true;
}
void TimeRequests::zoom(double density) {
    if (std::isfinite(density) && density > 0.0) density_ = density;
}
void TimeRequests::manual(double rate) {
    if (rate_ok(rate)) {
        manual_ = rate;
        if (locked_) locked_rate_ = rate;
    }
}
void TimeRequests::clear_manual() {
    manual_.reset();
    locked_ = false;
}
void TimeRequests::lock(bool on) {
    if (on && !locked_) locked_rate_ = manual_ ? *manual_ : zoom_rate();
    locked_ = on;
}
void TimeRequests::pause(bool on) {
    paused_ = on;
}
void TimeRequests::skip(std::optional<double> rate) {
    if (!rate || rate_ok(*rate)) skip_ = rate;
}
void TimeRequests::director(std::optional<double> rate) {
    if (!rate || rate_ok(*rate)) director_ = rate;
}
void TimeRequests::capacity(double rate) {
    if (rate_ok(rate)) capacity_ = rate;
}
double TimeRequests::zoom_rate() const {
    if (anchors_.empty()) return 1.0;
    if (density_ >= anchors_.front().density) return anchors_.front().rate;
    for (std::size_t i = 1; i < anchors_.size(); ++i) {
        if (density_ == anchors_[i].density) return anchors_[i].rate;
        if (density_ > anchors_[i].density) return between(density_, anchors_[i - 1], anchors_[i]);
    }
    // A slower phone still asks monotonically; the accepted downstream governor clamps the actual rate.
    const double top = std::max(anchors_.back().rate, capacity_);
    if (density_ <= top_density_) return top;
    return between(density_, anchors_.back(), {top_density_, top});
}
TimeRequest TimeRequests::resolve() const {
    if (paused_) return {0.0, "pause"};
    if (skip_) return {*skip_, "skip"};
    if (locked_) return {locked_rate_, "lock"};
    if (manual_) return {*manual_, "manual"};
    if (director_) return {*director_, "director"};
    return {zoom_rate(), "zoom"};
}
}  // namespace kd::view
