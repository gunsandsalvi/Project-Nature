// The scripted camera paths (A8.4, A4.8): a pan, a full turn in eased steps and a pinch through a band, the same
// every time, so the cloud's frames and the phone's readings of a change can be set side by side. Each moves the rig
// to where the path is at a time since it began. It touches no Godot, so its tests run alone.
#pragma once

#include <cstdint>
#include <string_view>

#include "rig.hpp"

namespace kd::view {

/// Implements PRE-22, see A4.8: the camera's scripted paths.
class Path {
public:
    enum class Kind : std::uint8_t { none, pan, turn, pinch };

    /// The seconds each path lasts.
    static constexpr double kPanSeconds = 8.0;    // east across four screen widths, steadily
    static constexpr double kTurnSeconds = 12.0;  // a full turn in eight eased steps of 45 degrees
    static constexpr double kPinchSeconds = 8.0;  // out to four times the metres a screen pixel, steadily

    /// The path by its name, "pan", "turn" or "pinch"; none for any other.
    static Kind named(std::string_view name);
    static std::string_view name(Kind kind);

    /// Starts a path from where the rig is.
    void start(Kind kind, const Rig& rig);
    /// Moves the rig to where the path is this many seconds after it began; once it is over it stops and leaves
    /// the rig at its end.
    void apply(Rig& rig, double since_start);
    [[nodiscard]] Kind kind() const { return kind_; }

private:
    Kind kind_ = Kind::none;
    double heading_ = 0.0;
    double mpp_ = 0.0;
    double width_metres_ = 0.0;
    double panned_ = 0.0;  // metres panned so far
};

}  // namespace kd::view
