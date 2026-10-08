#include "canvas.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace kd::view {
void KdCanvas::_bind_methods() {
    using namespace godot;
    ClassDB::bind_method(D_METHOD("set_world", "world", "east", "north"), &KdCanvas::set_world);
    ClassDB::bind_method(D_METHOD("frame", "width", "height"), &KdCanvas::frame);
    ClassDB::bind_method(D_METHOD("project", "east", "north", "height"), &KdCanvas::project);
    ClassDB::bind_method(D_METHOD("ground", "pixel", "height"), &KdCanvas::ground);
    ClassDB::bind_method(D_METHOD("from_screen", "pixel"), &KdCanvas::from_screen);
    ClassDB::bind_method(D_METHOD("focus", "east", "north"), &KdCanvas::focus);
    ClassDB::bind_method(D_METHOD("zoom", "ratio", "anchor", "snap"), &KdCanvas::zoom);
    ClassDB::bind_method(D_METHOD("touch", "action", "finger", "pixel", "seconds"), &KdCanvas::touch);
    ClassDB::bind_method(D_METHOD("records"), &KdCanvas::records);
}
void KdCanvas::set_world(const godot::Ref<KdWorld>& world, int64_t east, int64_t north) {
    world_ = world;
    origin_ = world::World::kTorus.wrap(east, north);
}
void KdCanvas::focus(double east, double north) {
    projection_.focus(east, north);
}
void KdCanvas::zoom(double ratio, godot::Vector2 anchor, bool snap) {
    projection_.zoom(ratio, {static_cast<double>(anchor.x), static_cast<double>(anchor.y)}, snap);
}
void KdCanvas::touch(int64_t action, int64_t finger, godot::Vector2 pixel, double seconds) {
    if (action == 0) {
        gestures_.press(static_cast<int>(finger), static_cast<double>(pixel.x), static_cast<double>(pixel.y), seconds);
    }
    if (action == 1) {
        gestures_.move(static_cast<int>(finger), static_cast<double>(pixel.x), static_cast<double>(pixel.y), seconds);
    }
    if (action == 2) {
        gestures_.lift(static_cast<int>(finger), static_cast<double>(pixel.x), static_cast<double>(pixel.y), seconds);
    }
}
godot::Dictionary KdCanvas::frame(int64_t width, int64_t height) {
    projection_.size(static_cast<int>(width), static_cast<int>(height));
    const Motion motion = gestures_.take();
    projection_.pan(motion.pan_x, motion.pan_y);
    // The local view cannot turn. Two fingers still share the existing pinch and drag recognition.
    projection_.zoom(motion.scale, {motion.at_x, motion.at_y}, motion.lifted);
    const Pixel residual = projection_.residual();
    godot::Dictionary result;
    result["size"] = godot::Vector2(static_cast<float>(projection_.width()), static_cast<float>(projection_.height()));
    result["scale"] = projection_.pixel_scale();
    result["density"] = projection_.resting_density();
    result["live_scale"] = projection_.presentation();
    result["residual"] = godot::Vector2(static_cast<float>(residual.x), static_cast<float>(residual.y));
    const Pixel offset = projection_.presentation_offset();
    result["offset"] = godot::Vector2(static_cast<float>(offset.x), static_cast<float>(offset.y));
    return result;
}
godot::Vector2 KdCanvas::from_screen(godot::Vector2 pixel) const {
    const Pixel result = projection_.from_screen({static_cast<double>(pixel.x), static_cast<double>(pixel.y)});
    return {static_cast<float>(result.x), static_cast<float>(result.y)};
}
godot::Vector2 KdCanvas::project(double east, double north, double height) const {
    const Pixel pixel = projection_.raster(east, north, height);
    return {static_cast<float>(pixel.x), static_cast<float>(pixel.y)};
}
godot::Vector2 KdCanvas::ground(godot::Vector2 pixel, double height) const {
    const Pixel point = projection_.ground({static_cast<double>(pixel.x), static_cast<double>(pixel.y)}, height);
    return {static_cast<float>(point.x), static_cast<float>(point.y)};
}
godot::Array KdCanvas::records() const {
    godot::Array result;
    if (world_.is_null()) {
        return result;
    }
    for (const auto& record : world_->display().sample(world::World::kTorus, origin_, world_->screen_time())) {
        godot::Dictionary row;
        row["epoch"] = static_cast<int64_t>(record.epoch);
        row["revision"] = static_cast<int64_t>(record.revision);
        row["second"] = record.second;
        row["id"] = static_cast<int64_t>(record.id);
        row["appearance"] = record.appearance;
        row["surface"] = record.surface;
        row["phase"] = record.phase;
        row["activity"] = record.activity;
        row["facing"] = record.facing;
        row["place"] = godot::Vector2(static_cast<float>(record.east), static_cast<float>(record.north));
        const Pixel pixel = projection_.raster(record.east, record.north);
        row["pixel"] = godot::Vector2(static_cast<float>(pixel.x), static_cast<float>(pixel.y));
        result.push_back(row);
    }
    return result;
}
}  // namespace kd::view
