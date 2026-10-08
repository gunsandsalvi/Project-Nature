// T2.7a.1–2: Godot submits pixels; this shared helper owns the local camera and samples the owned snapshot.
#pragma once
#include <chrono>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include "gestures.hpp"
#include "projection.hpp"
#include "world.hpp"

namespace kd::view {
/// Implements PRE-01, PRE-02, PRE-03, PRE-22, PRE-33, WLD-13, TIM-17.
class KdCanvas : public godot::RefCounted {
    GDCLASS(KdCanvas, godot::RefCounted)
public:
    void set_world(const godot::Ref<KdWorld>& world, int64_t east, int64_t north);
    godot::Dictionary frame(int64_t width, int64_t height, double seconds = -1.0);
    godot::Vector2 project(double east, double north, double height) const;
    godot::Vector2 ground(godot::Vector2 pixel, double height) const;
    godot::Vector2 from_screen(godot::Vector2 pixel) const;
    void focus(double east, double north);
    void set_pixel_scale(int64_t scale);
    void set_detail_seed(int64_t seed);
    bool restore_origin(int64_t east, int64_t north, int64_t raster_east, int64_t raster_north);
    [[nodiscard]] Projection projection_at_origin(num::Point source_origin) const;
    godot::Dictionary tile_at(int64_t east, int64_t north, int64_t power, int64_t seed) const;
    godot::Vector2 world_local(int64_t east, int64_t north) const;
    godot::Vector2 project_world(int64_t east, int64_t north, double height) const;
    void zoom(double ratio, godot::Vector2 anchor, bool snap);
    void touch(int64_t action, int64_t finger, godot::Vector2 pixel, double seconds);
    godot::Array records() const;
    [[nodiscard]] const Projection& projection() const { return projection_; }

protected:
    static void _bind_methods();

private:
    godot::Ref<KdWorld> world_;
    num::Point origin_;
    num::Point initial_origin_;
    std::uint64_t detail_seed_ = 1;
    Projection projection_;
    Gestures gestures_{false};
    double settle_seconds_ = 0.320;
    int pixel_scale_ = 0;
    double maximum_height_ = 32.0;
    double overscan_ = 32.0;
    double shadow_reach_ = 128.0;
    std::chrono::steady_clock::time_point last_frame_;
    bool framed_ = false;
};
}  // namespace kd::view
