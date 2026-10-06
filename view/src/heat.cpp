#include "heat.hpp"

#include <algorithm>

namespace kd::view {

double HeatGovernor::read(double forecast) {
    if (forecast >= rules_.near) {
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
