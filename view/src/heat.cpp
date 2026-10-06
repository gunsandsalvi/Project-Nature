#include "heat.hpp"

#include <algorithm>
#include <cmath>

namespace kd::view {

void HeatGovernor::set_light(double threshold) {
    light_ = threshold > 0.0 && threshold <= 1.0 ? threshold : std::numeric_limits<double>::quiet_NaN();
}

double HeatGovernor::near() const {
    return std::isnan(light_) ? rules_.near : light_ - rules_.margin;
}

double HeatGovernor::read(double forecast) {
    if (std::isnan(forecast) || forecast < 0.0) {
        return share_;  // no reading: neither hot nor calm
    }
    if (forecast >= near()) {
        share_ = std::max(rules_.floor, share_ * rules_.cut);
        calm_ = 0;
        return share_;
    }
    ++calm_;
    if (calm_ > rules_.calm_readings) {
        share_ = std::min(1.0, share_ + rules_.give_back);
    }
    return share_;
}

}  // namespace kd::view
