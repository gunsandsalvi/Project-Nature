#include "canvas.hpp"
#include <godot_cpp/core/class_db.hpp>
#include "kd/look/navigation.hpp"
#include "wide.hpp"

namespace kd::view {
namespace {
godot::Dictionary tile_dictionary(const GroundTile& tile) {
    godot::Dictionary row;
    row["key"] = godot::String::utf8(tile.key.c_str());
    row["power"] = tile.power;
    row["x"] = tile.x;
    row["y"] = tile.y;
    row["west_cm"] = tile.west_south.x;
    row["south_cm"] = tile.west_south.y;
    row["east_cm"] = tile.east_north.x;
    row["north_cm"] = tile.east_north.y;
    row["local_west"] = tile.local.west;
    row["local_south"] = tile.local.south;
    row["local_east"] = tile.local.east;
    row["local_north"] = tile.local.north;
    row["variant"] = tile.variant;
    row["accent"] = tile.accent;
    row["stamp_key"] = godot::String::utf8(tile.stamp_key.c_str());
    godot::Array neighbours, borders, details;
    for (const auto& key : tile.neighbours) neighbours.push_back(godot::String::utf8(key.c_str()));
    for (const auto key : tile.border_keys) borders.push_back(godot::String::utf8(std::to_string(key).c_str()));
    for (std::size_t i = 0; i < tile.details.size(); ++i) {
        const auto point = tile.details[i];
        godot::Dictionary detail;
        detail["key"] = godot::String::utf8(tile.detail_keys[i].c_str());
        detail["east_cm"] = point.x;
        detail["north_cm"] = point.y;
        details.push_back(detail);
    }
    row["neighbours"] = neighbours;
    row["border_keys"] = borders;
    row["details"] = details;
    return row;
}
}  // namespace
void KdCanvas::_bind_methods() {
    using namespace godot;
    ClassDB::bind_method(D_METHOD("set_world", "world", "east", "north"), &KdCanvas::set_world);
    ClassDB::bind_method(D_METHOD("frame", "width", "height", "seconds"), &KdCanvas::frame, DEFVAL(-1.0));
    ClassDB::bind_method(D_METHOD("project", "east", "north", "height"), &KdCanvas::project);
    ClassDB::bind_method(D_METHOD("ground", "pixel", "height"), &KdCanvas::ground);
    ClassDB::bind_method(D_METHOD("from_screen", "pixel"), &KdCanvas::from_screen);
    ClassDB::bind_method(D_METHOD("focus", "east", "north"), &KdCanvas::focus);
    ClassDB::bind_method(D_METHOD("zoom", "ratio", "anchor", "snap"), &KdCanvas::zoom);
    ClassDB::bind_method(D_METHOD("touch", "action", "finger", "pixel", "seconds"), &KdCanvas::touch);
    ClassDB::bind_method(D_METHOD("records"), &KdCanvas::records);
    ClassDB::bind_method(D_METHOD("set_detail_seed", "seed"), &KdCanvas::set_detail_seed);
    ClassDB::bind_method(D_METHOD("restore_origin", "east_cm", "north_cm", "raster_east_cm", "raster_north_cm"),
                         &KdCanvas::restore_origin);
    ClassDB::bind_method(D_METHOD("tile_at", "east_cm", "north_cm", "power", "look_seed"), &KdCanvas::tile_at);
    ClassDB::bind_method(D_METHOD("world_local", "east_cm", "north_cm"), &KdCanvas::world_local);
    ClassDB::bind_method(D_METHOD("project_world", "east_cm", "north_cm", "height"), &KdCanvas::project_world);
}
void KdCanvas::set_world(const godot::Ref<KdWorld>& world, int64_t east, int64_t north) {
    world_ = world;
    origin_ = world::World::kTorus.wrap(east, north);
    initial_origin_ = origin_;
    projection_.restore_raster_origin({0, 0});
    projection_.cancel_settle();
    framed_ = false;
    if (world_.is_valid() && world_->catalogue()) {
        const auto& catalogue = *world_->catalogue();
        const auto index = catalogue.find("tuning/navigation", "base:navigation");
        if (index) {
            const auto& n = catalogue.kind<look::NavigationTuning>()[*index];
            projection_.configure(static_cast<int>(n.minimum_power), static_cast<int>(n.maximum_power),
                                  static_cast<int>(n.tiny_power), static_cast<int>(n.group_power));
            settle_seconds_ = static_cast<double>(n.settle_ms) / 1000.0;
            maximum_height_ = static_cast<double>(n.maximum_height) / 1000.0;
            overscan_ = static_cast<double>(n.overscan_pixels);
            shadow_reach_ = static_cast<double>(n.shadow_reach) / 1000.0;
        }
    }
}
void KdCanvas::focus(double east, double north) {
    projection_.focus(east, north);
}
void KdCanvas::zoom(double ratio, godot::Vector2 anchor, bool snap) {
    projection_.zoom(ratio, {static_cast<double>(anchor.x), static_cast<double>(anchor.y)}, snap);
}
void KdCanvas::touch(int64_t action, int64_t finger, godot::Vector2 pixel, double seconds) {
    if (action == 0) {
        projection_.cancel_settle();
        gestures_.press(static_cast<int>(finger), static_cast<double>(pixel.x), static_cast<double>(pixel.y), seconds);
    }
    if (action == 1) {
        gestures_.move(static_cast<int>(finger), static_cast<double>(pixel.x), static_cast<double>(pixel.y), seconds);
    }
    if (action == 2) {
        gestures_.lift(static_cast<int>(finger), static_cast<double>(pixel.x), static_cast<double>(pixel.y), seconds);
    }
}
godot::Dictionary KdCanvas::frame(int64_t width, int64_t height, double seconds) {
    const auto now = std::chrono::steady_clock::now();
    const double elapsed = seconds >= 0.0 ? seconds
                           : framed_      ? std::chrono::duration<double>(now - last_frame_).count()
                                          : 0.0;
    last_frame_ = now;
    framed_ = true;
    projection_.size(static_cast<int>(width), static_cast<int>(height));
    const Motion motion = gestures_.take();
    if (motion.touching) projection_.cancel_settle();
    projection_.pan(motion.pan_x, motion.pan_y);
    // The local view cannot turn. Two fingers still share the existing pinch and drag recognition.
    if (motion.scale != 1.0) projection_.zoom(motion.scale, {motion.at_x, motion.at_y}, false);
    if (motion.lifted) projection_.release({motion.at_x, motion.at_y}, settle_seconds_);
    if (!motion.touching) projection_.advance(elapsed);
    const auto shift = projection_.rebase();
    origin_ = world::World::kTorus.moved(origin_, shift);
    const Pixel residual = projection_.residual();
    godot::Dictionary result;
    result["size"] = godot::Vector2(static_cast<float>(projection_.width()), static_cast<float>(projection_.height()));
    result["scale"] = projection_.pixel_scale();
    result["height_basis"] = Projection::kB;
    result["ground_basis"] = Projection::kA;
    result["density"] = projection_.resting_density();
    result["live_scale"] = projection_.presentation();
    result["physical_density"] = projection_.density() * projection_.pixel_scale();
    result["settling"] = projection_.settling();
    result["target_density"] = projection_.target_density();
    const auto source = projection_.source();
    result["world_width_cm"] = world::World::kTorus.width();
    result["world_height_cm"] = world::World::kTorus.height();
    result["source_density"] = source.density;
    result["source_level"] = source.level;
    result["form"] = source.form;
    godot::Dictionary rebase;
    rebase["east_cm"] = shift.dx;
    rebase["north_cm"] = shift.dy;
    result["rebase"] = rebase;
    const auto raster_origin = projection_.raster_origin();
    result["raster_origin_east_cm"] = raster_origin.dx;
    result["raster_origin_north_cm"] = raster_origin.dy;
    const auto footprint = projection_.footprint(maximum_height_, overscan_, 0.0, 0.0);
    godot::Dictionary bounds;
    // Local centimetres stay unwrapped; the exact origin is separately wrapped for seam-crossing queries.
    bounds["west_cm"] = footprint.west * 100.0 - shadow_reach_ * 100.0;
    bounds["south_cm"] = footprint.south * 100.0 - shadow_reach_ * 100.0;
    bounds["east_cm"] = footprint.east * 100.0 + shadow_reach_ * 100.0;
    bounds["north_cm"] = footprint.north * 100.0 + shadow_reach_ * 100.0;
    bounds["origin_east_cm"] = origin_.x;
    bounds["origin_north_cm"] = origin_.y;
    result["footprint"] = bounds;
    const Footprint demand{footprint.west - shadow_reach_, footprint.south - shadow_reach_,
                           footprint.east + shadow_reach_, footprint.north + shadow_reach_};
    godot::Array tiles;
    const auto visible_ground = projection_.footprint(0, 0, 0, 0);
    const int ground_power = source.density == 64 ? 2 : source.density == 16 ? 4 : 6;
    for (const auto& tile : ground_tiles(world::World::kTorus, origin_, visible_ground, ground_power, detail_seed_))
        tiles.push_back(tile_dictionary(tile));
    result["ground_tiles"] = tiles;
    if (world_.is_valid()) {
        result["epoch"] = static_cast<int64_t>(world_->display().epoch());
        result["revision"] = static_cast<int64_t>(world_->display().revision());
        result["second"] = world_->screen_time();
        const auto owned = world_->display().manifest(world_->screen_time());
        godot::Dictionary manifest;
        manifest["epoch"] = static_cast<int64_t>(owned.epoch);
        manifest["revision"] = static_cast<int64_t>(owned.revision);
        manifest["second"] = owned.second;
        for (const auto& [singular, plural] : std::vector<std::pair<std::string, std::string>>{
                 {"surface", "surfaces"}, {"caster", "casters"}, {"appearance", "appearances"}}) {
            godot::Dictionary records;
            for (const auto& [id, revision] : owned.records.at(singular))
                records[godot::String::utf8(id.c_str())] = static_cast<int64_t>(revision);
            manifest[godot::String::utf8(plural.c_str())] = records;
        }
        result["manifest"] = manifest;
        const auto sampled = world_->display().sample(world::World::kTorus, origin_, world_->screen_time());
        const auto wide =
            wide_records(sampled, world_->display().camps(), world::World::kTorus, origin_, source.form, &demand);
        godot::Dictionary overview;
        overview["form"] = source.form;
        overview["epoch"] = static_cast<int64_t>(owned.epoch);
        overview["revision"] = static_cast<int64_t>(owned.revision);
        overview["second"] = owned.second;
        overview["record_count"] = static_cast<int64_t>(wide.record_count);
        overview["member_count"] = static_cast<int64_t>(wide.member_count);
        overview["population_count"] = static_cast<int64_t>(wide.population_count);
        overview["truncated"] = wide.truncated;
        godot::Array rows;
        for (const auto& record : wide.records) {
            godot::Dictionary row;
            row["key"] = godot::String::utf8(record.key.c_str());
            row["id"] = static_cast<int64_t>(record.id);
            row["camp_id"] = static_cast<int64_t>(record.camp_id);
            row["camp_index"] = record.camp_index;
            row["place"] = godot::Vector2(static_cast<float>(record.east), static_cast<float>(record.north));
            const auto pixel = projection_.raster(record.east, record.north);
            row["pixel"] = godot::Vector2(static_cast<float>(pixel.x), static_cast<float>(pixel.y));
            row["appearance"] = record.appearance;
            row["activity"] = record.activity;
            row["facing"] = record.facing;
            row["phase"] = record.phase;
            row["count"] = static_cast<int64_t>(record.count);
            godot::PackedInt64Array members;
            for (const auto id : record.members) members.push_back(static_cast<int64_t>(id));
            row["members"] = members;
            rows.push_back(row);
        }
        overview["records"] = rows;
        result["wide"] = overview;
    }
    result["residual"] = godot::Vector2(static_cast<float>(residual.x), static_cast<float>(residual.y));
    const Pixel offset = projection_.presentation_offset();
    result["offset"] = godot::Vector2(static_cast<float>(offset.x), static_cast<float>(offset.y));
    return result;
}
bool KdCanvas::restore_origin(int64_t east, int64_t north, int64_t raster_east, int64_t raster_north) {
    const auto torus = world::World::kTorus;
    if (east < 0 || east >= torus.width() || north < 0 || north >= torus.height()) return false;
    const num::Point requested{static_cast<std::int32_t>(east), static_cast<std::int32_t>(north)};
    if (torus.moved(initial_origin_, {raster_east, raster_north}) != requested ||
        !projection_.restore_raster_origin({raster_east, raster_north}))
        return false;
    origin_ = requested;
    projection_.cancel_settle();
    gestures_ = Gestures{false};
    return true;
}
Projection KdCanvas::projection_at_origin(num::Point source_origin) const {
    const auto offset = world::World::kTorus.offset(source_origin, origin_);
    return projection_.at_origin({static_cast<double>(offset.dx) / 100., static_cast<double>(offset.dy) / 100.});
}
void KdCanvas::set_detail_seed(int64_t seed) {
    detail_seed_ = static_cast<std::uint64_t>(seed);
}
godot::Dictionary KdCanvas::tile_at(int64_t east, int64_t north, int64_t power, int64_t seed) const {
    if (power < 0 || power > 22) return {};
    return tile_dictionary(
        canonical_tile(world::World::kTorus, east, north, static_cast<int>(power), static_cast<std::uint64_t>(seed)));
}
godot::Vector2 KdCanvas::world_local(int64_t east, int64_t north) const {
    const auto offset = world::World::kTorus.offset(origin_, world::World::kTorus.wrap(east, north));
    return {static_cast<float>(static_cast<double>(offset.dx) / 100.),
            static_cast<float>(static_cast<double>(offset.dy) / 100.)};
}
godot::Vector2 KdCanvas::project_world(int64_t east, int64_t north, double height) const {
    const auto offset = world::World::kTorus.offset(origin_, world::World::kTorus.wrap(east, north));
    const auto pixel =
        projection_.raster(static_cast<double>(offset.dx) / 100., static_cast<double>(offset.dy) / 100., height);
    return {static_cast<float>(pixel.x), static_cast<float>(pixel.y)};
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
        row["camp_index"] = static_cast<int64_t>(record.camp);
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
