// The look class (A4.1, A4.6, A8.4): the engine's spine on the Look page. It reads the page's touches through the
// gesture reader into the camera rig, publishes the rig's globals to every shader, and draws the stage's ground
// through Godot's RenderingServer with textures whose levels are our own. The page keeps Godot's camera, light and
// switches; this class converts and draws, and never decides for the world (WLD-13).
#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "gestures.hpp"
#include "path.hpp"
#include "rig.hpp"
#include "stage.hpp"

namespace kd::view {

/// Implements PRE-01, PRE-22 and PRE-33, see A4.1, A4.6 and A8.4: the Look page's camera, globals and ground.
class KdLook : public godot::RefCounted {
    GDCLASS(KdLook, godot::RefCounted)

public:
    ~KdLook() override;

    // touches, from the page's input events, in the window's pixels
    void press(int64_t finger, godot::Vector2 at, double seconds);
    void move(int64_t finger, godot::Vector2 at, double seconds);
    void lift(int64_t finger, godot::Vector2 at, double seconds);
    /// A mouse's wheel and buttons, for the cloud's runs: zoom by a ratio, and turn by degrees, about a point.
    void zoom_by(double ratio, godot::Vector2 at);
    void turn_by(double degrees, godot::Vector2 at);

    /// The window's size in pixels, as Godot gives it.
    void set_screen(godot::Vector2 size);
    /// The lens across the short side: 5 or 10 degrees.
    void set_lens(double degrees);
    /// One frame: the gestures to the rig, its ease, a scripted path's step, the globals and the ground's place.
    void frame(double seconds);
    /// The camera: transform, fov (degrees), keep_width (the lens spans the width), near and far.
    godot::Dictionary pose() const;
    /// The rig's state: focus_east and focus_north (cm), heading (degrees), metres_per_pixel, metres_across (the
    /// screen's short side), band, texel_pixels (the band's texture pixel at the focus, in screen pixels) and resting.
    godot::Dictionary state() const;

    /// Builds the ground's texture array from .kdtex files, one layer each, all one size; "" or the problem.
    godot::String load_layers(const godot::PackedStringArray& paths);
    /// Puts the stand-in ground on the stage and draws it into the scenario with the ground shader.
    void build(const godot::RID& scenario, const godot::RID& shader);
    /// Shows or hides a part of the drawing: "ground" or "pattern". Never touches a world.
    void set_part(const godot::String& part, bool on);
    /// Draws the ground with another shader from now on, such as calibration scene C5's for each way of drawing
    /// fire shadows (A18.1).
    void set_shader(const godot::RID& shader);
    /// Puts the ground on these visual layers, so only the cameras that see them draw it: the Compare page's fire
    /// shadows draw one ground each way, each seen by one of its pictures (A5.5).
    void set_layers(int64_t layers);
    /// Frees everything it made in the RenderingServer.
    void clear();

    /// Puts the view at a place, for the cloud's fixed views (A4.8): the focus in centimetres east and north, the
    /// heading in degrees clockwise from north and the zoom in metres a screen pixel; any path stops.
    void set_view(int64_t east, int64_t north, double heading, double metres_per_pixel);
    /// For the cloud's many-sample pictures (A4.8): the picture is drawn this many times larger, so each texture
    /// level is still read as at the frame's own size; 1, the game's, by default. It takes effect at the next frame.
    void set_many(int64_t times);
    /// The blind test's comparisons (A5.5), in their order on the Compare page, and one's words: a dictionary of
    /// name, compares and asks.
    [[nodiscard]] godot::PackedInt32Array blind_comparisons() const;
    [[nodiscard]] godot::Dictionary blind_words(int64_t comparison) const;
    /// A comparison's pairs from a seed of 16 bits: each a dictionary of better_first, heading (degrees), east and
    /// north (centimetres).
    [[nodiscard]] godot::Array blind_pairs(int64_t comparison, int64_t seed) const;
    /// A taken blind test's code, and its answers that chose the better way: chose_first holds a bool for each pair.
    [[nodiscard]] godot::String blind_code(int64_t comparison, int64_t seed, const godot::Array& chose_first) const;
    [[nodiscard]] int64_t blind_right(int64_t comparison, int64_t seed, const godot::Array& chose_first) const;
    /// Starts a scripted camera path: "pan", "turn" or "pinch" (A8.4); "" stops it.
    void play(const godot::String& path);
    /// The scripted path under way, or "".
    godot::String playing() const;

protected:
    static void _bind_methods();

private:
    void place(std::uint64_t id, const Copy& copy);
    void place_all();
    void publish() const;
    void clear_drawing();

    Gestures gestures_;
    Rig rig_;
    Stage stage_;
    Path path_;
    double clock_ = 0.0;
    double path_started_ = 0.0;
    double many_ = 1.0;
    std::int64_t origin_east_ = 0;
    std::int64_t origin_north_ = 0;
    godot::RID scenario_;
    godot::RID layers_;
    godot::RID material_;
    std::vector<godot::RID> meshes_;                 // by form
    std::map<std::uint64_t, godot::RID> instances_;  // by copy
    std::map<std::uint32_t, bool> hidden_;           // forms hidden by the page's switches
    std::uint32_t layer_mask_ = 1;                   // the visual layers the ground is drawn on
};

}  // namespace kd::view
