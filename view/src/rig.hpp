// The camera rig (A8.4, A4.1): one perspective camera at every stop, with no pixel lock. Up close it looks through a
// narrow lens, tilted down toward the focus as in the pictures you liked; drags, pinches and twists move it so the
// ground under the fingers stays under them; when the fingers lift it eases to rest on whole steps of turn and zoom.
// The zoom is kept as metres a screen pixel across the screen's short side, so turning the phone keeps the texture
// pixel's size (PLT-02). Its focus is kept in whole centimetres about an origin that moves with it, so far from the
// world's centre the floats Godot draws with stay exact (A8.2). It touches no Godot, so its tests run alone.
#pragma once

#include <cstdint>

namespace kd::view {

/// The rig's numbers, from the camera's tuning file (A8.4).
struct RigTuning {
    double tilt = 37.0;            // degrees below the horizon that the view looks down, up close
    double lens = 10.0;            // degrees the view spans across the screen's short side, up close
    double turn_step = 5.0;        // degrees: the turns the ease rests on
    double zoom_step = 1.25;       // the ratio between the zooms the ease rests on
    double ease = 0.12;            // seconds for the ease to close all but a third of the way
    double closest = 1.0 / 128.0;  // metres a screen pixel at the closest zoom: band 0's texture pixel 2 wide
    double farthest = 0.5;         // metres a screen pixel at the farthest zoom it allows
};

/// Where the camera is and how it looks, about the rig's origin, in metres: east is x, up is y, north is -z.
struct Pose {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double look_x = 0.0;  // the point it looks at: the focus
    double look_y = 0.0;
    double look_z = 0.0;
    double lens = 0.0;  // degrees across the screen's short side
    double near = 0.0;  // the near and far planes, hugging the ground in view
    double far = 0.0;
    bool wide = false;  // the screen is wider than tall, so the lens spans its height
};

/// Implements PRE-03, PRE-33 and PLT-02, see A8.4: the camera's state, moved by gestures and eased to rest.
class Rig {
public:
    /// The texture pixels a metre at band 0, the closest zoom's level (A5.3).
    static constexpr double kBand0Texels = 64.0;
    /// The least a texture pixel may be on screen, in screen pixels, before the next band's level takes over: its
    /// width runs from this to twice this (A4.2). The sampling function reads the same number from the rig's globals.
    static constexpr double kTexelLeast = 1.4;
    /// How far the focus may stray from the origin, in metres, before the origin moves to it.
    static constexpr double kOriginReach = 256.0;

    explicit Rig(RigTuning tuning = {});

    /// The window's size in pixels; turning the phone swaps them and keeps everything else.
    void set_screen(double width, double height);
    /// The lens across the short side, 5 or 10 degrees by the Look page's switch; the zoom is kept.
    void set_lens(double degrees);
    /// The closest the rig may zoom, in metres a screen pixel: the pages that show a texture pixel enlarged go closer
    /// than band 0's. The zoom is kept, or brought to it if it was closer than this.
    void set_closest(double metres_per_pixel);

    /// The fingers' centre moved by this many pixels, to this point of the window.
    void drag(double dx, double dy, double at_x, double at_y);
    /// The fingers spread by this ratio about a point of the window, in pixels: more than 1 zooms in.
    void pinch(double scale, double at_x, double at_y);
    /// The fingers turned this many degrees clockwise about a point of the window.
    void twist(double degrees, double at_x, double at_y);
    /// Fingers on the screen hold the rig; their lifting lets it ease to rest.
    void hold(bool touching);
    /// Moves the ease on by this many seconds.
    void step(double seconds);

    /// The focus, in centimetres east and north of the world's centre.
    [[nodiscard]] std::int64_t focus_east() const;
    [[nodiscard]] std::int64_t focus_north() const;
    /// Puts the focus at a point, in centimetres east and north.
    void set_focus(std::int64_t east, std::int64_t north);
    /// Moves the focus by metres east and north.
    void move_by(double east, double north);
    /// The origin the pose and everything drawn are placed about, in whole centimetres east and north.
    [[nodiscard]] std::int64_t origin_east() const { return origin_east_; }
    [[nodiscard]] std::int64_t origin_north() const { return origin_north_; }

    /// The view's heading, in degrees clockwise from north, from 0 to 360.
    [[nodiscard]] double heading() const { return heading_; }
    void set_heading(double degrees);
    /// Metres a screen pixel at the focus, across the screen's short side.
    [[nodiscard]] double metres_per_pixel() const { return mpp_; }
    void set_metres_per_pixel(double mpp);
    /// The degrees the view looks down.
    [[nodiscard]] double tilt() const { return tuning_.tilt; }
    /// The window's short side, in pixels.
    [[nodiscard]] double short_side() const { return width_ < height_ ? width_ : height_; }

    /// The texture level band 0 is read at, at the focus: its texture pixels from kTexelLeast to twice that wide.
    [[nodiscard]] int band() const;
    /// A texture pixel's width at the focus, in screen pixels, at a band's level.
    [[nodiscard]] double texel_pixels(int band) const;

    /// The camera's pose about the origin.
    [[nodiscard]] Pose pose() const;
    /// The ground point under a pixel of the window, on flat ground, in metres east and north of the origin.
    void ground_at(double x, double y, double& east, double& north) const;

    /// Whether the rig is at rest: no fingers, and the ease done.
    [[nodiscard]] bool resting() const { return !touching_ && !easing_; }

private:
    // the ground point under a pixel, in metres east and north of the focus
    void offset_at(double x, double y, double& east, double& north) const;
    void move_focus(double east, double north);
    void rebase();
    [[nodiscard]] double distance() const;

    RigTuning tuning_;
    double width_ = 1080.0;
    double height_ = 2404.0;
    std::int64_t origin_east_ = 0;
    std::int64_t origin_north_ = 0;
    double east_ = 0.0;  // the focus about the origin, in metres
    double north_ = 0.0;
    double heading_ = 0.0;
    double mpp_ = 1.0 / 128.0;
    bool touching_ = false;
    bool easing_ = false;
};

}  // namespace kd::view
