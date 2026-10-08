// T2.7a.1–2: Godot submits pixels; this shared helper owns the local camera and samples the owned snapshot.
#pragma once
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
    godot::Dictionary frame(int64_t width, int64_t height);
    godot::Vector2 project(double east, double north, double height) const;
    godot::Vector2 ground(godot::Vector2 pixel, double height) const;
    godot::Vector2 from_screen(godot::Vector2 pixel) const;
    void focus(double east, double north);
    void zoom(double ratio, godot::Vector2 anchor, bool snap);
    void touch(int64_t action, int64_t finger, godot::Vector2 pixel, double seconds);
    godot::Array records() const;
    [[nodiscard]] const Projection& projection() const { return projection_; }

protected:
    static void _bind_methods();

private:
    godot::Ref<KdWorld> world_;
    num::Point origin_;
    Projection projection_;
    Gestures gestures_{false};
};
}  // namespace kd::view
