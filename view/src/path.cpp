#include "path.hpp"

#include <algorithm>

#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"

namespace kd::view {

namespace {

// An ease in and out over 0 to 1.
double smooth(double f) {
    return f * f * (3.0 - 2.0 * f);
}

}  // namespace

Path::Kind Path::named(std::string_view name) {
    if (name == "pan") {
        return Kind::pan;
    }
    if (name == "turn") {
        return Kind::turn;
    }
    if (name == "pinch") {
        return Kind::pinch;
    }
    return Kind::none;
}

std::string_view Path::name(Kind kind) {
    switch (kind) {
        case Kind::pan:
            return "pan";
        case Kind::turn:
            return "turn";
        case Kind::pinch:
            return "pinch";
        case Kind::none:
            break;
    }
    return "";
}

void Path::start(Kind kind, const Rig& rig) {
    kind_ = kind;
    heading_ = rig.heading();
    mpp_ = rig.metres_per_pixel();
    width_metres_ = rig.short_side() * mpp_;
    panned_ = 0.0;
}

void Path::apply(Rig& rig, double since_start) {
    const double t = std::max(0.0, since_start);
    switch (kind_) {
        case Kind::pan: {
            // to the screen's right, steadily, four screen widths, in metres so nothing steps
            const double f = std::min(1.0, t / kPanSeconds);
            const double target = 4.0 * width_metres_ * f;
            const double step = target - panned_;
            panned_ = target;
            rig.move_by(step * num::cospi(heading_ / 180.0), -step * num::sinpi(heading_ / 180.0));
            if (t >= kPanSeconds) {
                kind_ = Kind::none;
            }
            break;
        }
        case Kind::turn: {
            // eight steps of 45 degrees clockwise, each eased in and out over an eighth of the time
            const double each = kTurnSeconds / 8.0;
            const double at = std::min(t, kTurnSeconds);
            const auto k = std::min<std::int64_t>(7, num::to_int(at / each, num::Round::down));
            const double f = smooth(std::min(1.0, (at - static_cast<double>(k) * each) / each));
            rig.set_heading(heading_ + 45.0 * (static_cast<double>(k) + f));
            if (t >= kTurnSeconds) {
                kind_ = Kind::none;
            }
            break;
        }
        case Kind::pinch: {
            // out to four times the metres a screen pixel, at a steady rate of change
            const double f = std::min(1.0, t / kPinchSeconds);
            rig.set_metres_per_pixel(mpp_ * num::exp2(2.0 * f));
            if (t >= kPinchSeconds) {
                kind_ = Kind::none;
            }
            break;
        }
        case Kind::none:
            break;
    }
}

}  // namespace kd::view
