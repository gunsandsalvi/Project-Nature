// The heat governor (A3.9): the phone forecasts its heat headroom 10 s ahead, read every 2 s, where 1 is the first
// level at which it throttles. As the forecast nears it, the simulation's working share is cut at once, and it is
// given back slowly, only after the forecast has stayed below for a while, so time slows before the phone does.
// It touches no Godot, so its tests run alone; its numbers come from the tuning file base/tuning/heat.toml.
#pragma once

#include <limits>

namespace kd::view {

/// The governor's numbers, always from base/tuning/heat.toml; until it gives them, the share is never cut.
struct HeatRules {
    double near = std::numeric_limits<double>::infinity();  // the forecast at which the share is cut
    double cut = 1.0;                                       // what is kept of the share at each such reading
    double floor = 1.0;                                     // the least share
    int calm_readings = 0;                                  // readings below near before any share comes back
    double give_back = 0.0;                                 // the share given back each calm reading after that
};

/// Implements PLT-01, see A3.9: the simulation's working share, from 1 down to the floor.
class HeatGovernor {
public:
    explicit HeatGovernor(HeatRules rules = {}) : rules_(rules) {}

    /// One reading of the forecast; returns the share.
    double read(double forecast);
    [[nodiscard]] double share() const { return share_; }

private:
    HeatRules rules_;
    double share_ = 1.0;
    int calm_ = 0;
};

}  // namespace kd::view
