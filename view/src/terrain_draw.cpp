#include "terrain_draw.hpp"
#include <chrono>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include "kd/num/maths.hpp"

namespace kd::view {
namespace {
godot::Vector3 vector(terrain::Point p) {
    return {static_cast<float>(p.east), static_cast<float>(p.north), static_cast<float>(p.up)};
}
terrain::Point point(godot::Vector3 p) {
    return {static_cast<double>(p.x), static_cast<double>(p.y), static_cast<double>(p.z)};
}
godot::Dictionary picked(terrain::Pick p) {
    godot::Dictionary d;
    d["found"] = p.found;
    d["surface"] = p.surface;
    d["point"] = vector(p.point);
    return d;
}
}  // namespace
void KdTerrain::_bind_methods() {
    using namespace godot;
    ClassDB::bind_method(D_METHOD("scene", "name"), &KdTerrain::scene);
    ClassDB::bind_method(D_METHOD("set_origin", "east_cm", "north_cm"), &KdTerrain::set_origin);
    ClassDB::bind_method(D_METHOD("set_light", "hour", "weather", "direction", "fire"), &KdTerrain::set_light);
    ClassDB::bind_method(D_METHOD("surfaces"), &KdTerrain::surfaces);
    ClassDB::bind_method(D_METHOD("proxies"), &KdTerrain::proxies);
    ClassDB::bind_method(D_METHOD("actors", "second"), &KdTerrain::actors);
    ClassDB::bind_method(D_METHOD("walk", "east", "north"), &KdTerrain::walk);
    ClassDB::bind_method(D_METHOD("bed", "east", "north"), &KdTerrain::bed);
    ClassDB::bind_method(D_METHOD("pick", "canvas", "raster", "cutaway"), &KdTerrain::pick);
    ClassDB::bind_method(D_METHOD("mask", "surface"), &KdTerrain::mask);
    ClassDB::bind_method(D_METHOD("hit_surface", "surface", "canvas", "raster"), &KdTerrain::hit_surface);
    ClassDB::bind_method(D_METHOD("visibility", "point", "ignore"), &KdTerrain::visibility);
    ClassDB::bind_method(D_METHOD("order", "pieces"), &KdTerrain::order);
    ClassDB::bind_method(D_METHOD("bodies", "records"), &KdTerrain::bodies);
    ClassDB::bind_method(D_METHOD("costs"), &KdTerrain::costs);
}
void KdTerrain::set_origin(int64_t east, int64_t north) {
    source_origin_ = world::World::kTorus.wrap(east, north);
    source_origin_set_ = true;
}
void KdTerrain::scene(const godot::String& name) {
    const auto next = scene_.revision + 1;
    scene_ = terrain::fixture(name.utf8().get_data());
    scene_.revision = next;
    masks_.clear();
    bodies_.clear();
}
godot::Dictionary KdTerrain::set_light(const godot::String& hour, const godot::String& weather, int64_t direction,
                                       bool fire) {
    const auto revision = light_.revision + 1;
    light_ = terrain::light(hour.utf8().get_data(), weather.utf8().get_data(), static_cast<int>(direction), fire);
    light_.revision = revision;
    // PRE-23/PRE-30: local fire sits above the same measured floor as feet and water.
    const auto floor = terrain::walk(scene_, light_.fire.east, light_.fire.north);
    if (floor.found) light_.fire.up += floor.point.up;
    godot::Dictionary d;
    d["sun"] = vector(light_.sun);
    d["sunlight"] = vector(light_.sunlight);
    d["sky"] = vector(light_.sky);
    d["fire"] = vector(light_.fire);
    d["fire_on"] = fire;
    d["revision"] = static_cast<int64_t>(revision);
    d["record"] = "Declared fixture hour/weather; not a generated physical sky";
    return d;
}
godot::Array KdTerrain::surfaces() const {
    godot::Array result;
    for (const auto& s : scene_.surfaces) {
        godot::Dictionary d;
        d["id"] = s.id;
        d["kind"] = static_cast<int>(s.kind);
        d["covers"] = s.covers;
        d["normal"] = vector(s.normal);
        d["material"] = s.material;
        godot::Array corners;
        for (const auto p : s.corners) corners.push_back(vector(p));
        d["corners"] = corners;
        result.push_back(d);
    }
    return result;
}
godot::Array KdTerrain::proxies() const {
    godot::Array result;
    for (const auto& caster : scene_.casters) {
        godot::Dictionary d;
        d["id"] = caster.id;
        d["low"] = vector(caster.bounds.low);
        d["high"] = vector(caster.bounds.high);
        d["round"] = caster.round;
        d["group"] = caster.group;
        result.push_back(d);
    }
    return result;
}
godot::Array KdTerrain::actors(double second) const {
    godot::Array result;
    for (const auto& a : scene_.actors) {
        godot::Dictionary d;
        d["id"] = a.id;
        d["art"] = a.art;
        auto foot = a.foot;
        if (a.id == -5) {
            foot.east += 0.4 * num::sinpi(second / 2.0);
            const auto sampled = terrain::walk(scene_, foot.east, foot.north);
            if (sampled.found) foot = sampled.point;
        }
        d["point"] = vector(foot);
        d["surface"] = a.surface;
        result.push_back(d);
    }
    return result;
}
godot::Dictionary KdTerrain::bed(double east, double north) const {
    terrain::Scene foundation;
    for (const auto& surface : scene_.surfaces)
        if (surface.id >= 100 && surface.kind == terrain::Kind::floor) foundation.surfaces.push_back(surface);
    return picked(terrain::walk(foundation, east, north));
}
godot::Dictionary KdTerrain::walk(double east, double north) const {
    return picked(terrain::walk(scene_, east, north));
}
godot::Dictionary KdTerrain::pick(const godot::Ref<KdCanvas>& canvas, godot::Vector2 raster, bool cutaway) const {
    if (canvas.is_null()) return {};
    return picked(
        terrain::pick(scene_, source_origin_set_ ? canvas->projection_at_origin(source_origin_) : canvas->projection(),
                      {static_cast<double>(raster.x), static_cast<double>(raster.y)}, cutaway));
}
godot::Dictionary KdTerrain::hit_surface(int64_t surface, const godot::Ref<KdCanvas>& canvas,
                                         godot::Vector2 raster) const {
    if (canvas.is_null()) return {};
    for (const auto& s : scene_.surfaces)
        if (s.id == surface) {
            terrain::Scene single;
            single.surfaces.push_back(s);
            return picked(terrain::pick(
                single, source_origin_set_ ? canvas->projection_at_origin(source_origin_) : canvas->projection(),
                {static_cast<double>(raster.x), static_cast<double>(raster.y)}, false));
        }
    return {};
}
godot::Dictionary KdTerrain::mask(int64_t surface) {
    for (const auto& s : scene_.surfaces)
        if (s.id == surface) {
            const auto start = std::chrono::steady_clock::now();
            const auto m = masks_.prepare(scene_, s, light_, bodies_);
            prepare_ms_ += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            godot::PackedByteArray pixels;
            pixels.resize(static_cast<int64_t>(m.rgba.size()));
            std::copy(m.rgba.begin(), m.rgba.end(), pixels.ptrw());
            godot::Dictionary d;
            d["width"] = m.width;
            d["height"] = m.height;
            d["rgba"] = pixels;
            return d;
        }
    return {};
}
godot::Dictionary KdTerrain::visibility(godot::Vector3 p, int64_t ignore) const {
    auto casters = scene_.casters;
    casters.insert(casters.end(), bodies_.begin(), bodies_.end());
    casters.erase(
        std::remove_if(casters.begin(), casters.end(),
                       [ignore](const auto& c) {
                           return ((ignore == -11 || ignore == -12) && (c.id == 1 || c.id == 2 || c.group == 1)) ||
                                  (ignore == -3 && c.id == 3) || (ignore == -2 && c.id == 4);
                       }),
        casters.end());
    godot::Dictionary d;
    d["sun"] = terrain::sunlight(point(p), light_, casters, ignore);
    auto from = point(p);
    from.up += 0.04;
    d["sky"] = terrain::sky_visibility(point(p), casters, ignore);
    d["fire"] = terrain::blocked(from, light_.fire, casters, ignore) ? 0.0 : 1.0;
    return d;
}
godot::Array KdTerrain::order(godot::Array pieces) const {
    std::vector<terrain::Piece> input;
    input.reserve(static_cast<std::size_t>(pieces.size()));
    for (int64_t i = 0; i < pieces.size(); ++i) {
        const godot::Dictionary d = pieces[i];
        const godot::Vector2 low = d["low"], high = d["high"];
        input.push_back({d["id"],
                         d["surface"],
                         d["covers"],
                         {static_cast<double>(low.x), static_cast<double>(low.y)},
                         {static_cast<double>(high.x), static_cast<double>(high.y)},
                         d["depth"],
                         d["receiver"],
                         d["roof"],
                         d.get("water", false),
                         d.get("minimum_height", 0.0),
                         d.get("maximum_height", 0.0)});
    }
    godot::Array result;
    for (auto id : terrain::order(input)) result.push_back(id);
    return result;
}
void KdTerrain::bodies(godot::Array records) {
    bodies_.clear();
    prepare_ms_ = 0;
    for (int64_t i = 0; i < std::min<int64_t>(records.size(), 32); ++i) {
        const godot::Dictionary r = records[i];
        const auto p = point(r["point"]);
        const int64_t id = r["id"];
        bodies_.push_back(
            {id, {{p.east - 0.22, p.north - 0.18, p.up}, {p.east + 0.22, p.north + 0.18, p.up + 1.65}}, true});
    }
}
godot::Dictionary KdTerrain::costs() const {
    godot::Dictionary d;
    d["prepare_ms"] = prepare_ms_;
    d["static_builds"] = static_cast<int64_t>(masks_.builds());
    d["mask_bytes"] = static_cast<int64_t>(masks_.bytes());
    d["moving_bodies"] = static_cast<int64_t>(bodies_.size());
    d["mask_cache_cap_bytes"] = 64 * 65 * 65 * 5;
    d["max_moving_bodies"] = 32;
    d["method"] = "steady-clock CPU preparation; GPU counters supplied separately";
    return d;
}
}  // namespace kd::view
