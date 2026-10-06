// The gesture reader (A15, PRE-33): every gesture read from the raw touches by one set of rules, so a scripted test
// can tell each from the others. One finger drags; two fingers drag, pinch and twist at once; a tap and then a second
// press that drags zooms with one thumb. The fingers are compared once a frame, where they were then and where they
// are now, so the order in which their moves arrive never reads as a twist. It touches no Godot, so its tests run
// alone; the page hands it Godot's touches and gives what it reads to the camera rig.
#pragma once

#include <map>

namespace kd::view {

/// What the fingers did since it was last asked, in screen pixels from the window's top left.
struct Motion {
    double pan_x = 0.0;  // the fingers' centre moved, in pixels
    double pan_y = 0.0;
    double scale = 1.0;  // the fingers' spread grew by this ratio: more than 1 zooms in
    double twist = 0.0;  // the fingers turned by this many degrees, clockwise on the screen
    double at_x = 0.0;   // where the fingers are now, about which pinches and twists turn
    double at_y = 0.0;
    bool touching = false;  // a finger is down now
    bool lifted = false;    // the last finger came up since it was last asked
};

/// Implements PRE-33, see A15: drag, pinch, twist and the one-thumb zoom, from raw touches.
class Gestures {
public:
    /// How far a finger may move, in pixels, and still tap; and the most seconds a tap lasts and may wait for the
    /// press that makes it a one-thumb zoom.
    static constexpr double kTapSlop = 24.0;
    static constexpr double kTapSeconds = 0.3;
    /// Pixels of one-thumb drag that zoom in by e, dragging down.
    static constexpr double kThumbZoom = 300.0;
    /// Degrees two fingers must turn together before the view turns, so a pinch never turns it.
    static constexpr double kTwistStart = 10.0;

    void press(int finger, double x, double y, double seconds);
    void move(int finger, double x, double y, double seconds);
    void lift(int finger, double x, double y, double seconds);

    /// What the fingers did since the last call.
    Motion take();

    /// Whether the one-thumb zoom is under way.
    [[nodiscard]] bool thumb_zoom() const { return thumb_; }

private:
    struct Finger {
        double x = 0.0;
        double y = 0.0;
    };

    // adds what the fingers did since they were last compared, and compares from here on
    void fold();

    std::map<int, Finger> fingers_;  // where each finger is now
    std::map<int, Finger> before_;   // where each was when last compared
    Motion motion_;
    bool thumb_ = false;
    // the last tap, for the one-thumb zoom: where and when it lifted
    bool tap_ready_ = false;
    double tap_x_ = 0.0;
    double tap_y_ = 0.0;
    double tap_at_ = -1.0;
    // the press under way, to tell a tap
    double down_x_ = 0.0;
    double down_y_ = 0.0;
    double down_at_ = 0.0;
    bool still_ = false;
    // the two-finger twist: begun, or still gathering toward kTwistStart
    bool turning_ = false;
    double pending_twist_ = 0.0;
};

}  // namespace kd::view
