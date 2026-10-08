// T2.8a: resource submission of the restricted CPU fixture. No world state is changed here.
#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include "canvas.hpp"
#include "terrain.hpp"
namespace kd::view {
/// Implements PRE-20, PRE-21, PRE-23, PRE-24, PRE-26, PRE-28, PRE-30, PRE-33, PLT-04, WLD-13.
class KdTerrain : public godot::RefCounted {
    GDCLASS(KdTerrain, godot::RefCounted)
public:
    void scene(const godot::String& name);
    godot::Dictionary set_light(const godot::String& hour, const godot::String& weather, int64_t direction, bool fire);
    godot::Array surfaces() const;
    godot::Array proxies() const;
    godot::Array actors(double second) const;
    godot::Dictionary walk(double east, double north) const;
    godot::Dictionary bed(double east, double north) const;
    godot::Dictionary pick(const godot::Ref<KdCanvas>& canvas, godot::Vector2 raster, bool cutaway) const;
    godot::Dictionary mask(int64_t surface);
    godot::Dictionary hit_surface(int64_t surface, const godot::Ref<KdCanvas>& canvas, godot::Vector2 raster) const;
    godot::Dictionary visibility(godot::Vector3 point, int64_t ignore) const;
    godot::Array order(godot::Array pieces) const;
    void bodies(godot::Array records);
    godot::Dictionary costs() const;

protected:
    static void _bind_methods();

private:
    terrain::Scene scene_ = terrain::fixture("flat");
    terrain::Light light_;
    terrain::Masks masks_;
    std::vector<terrain::Caster> bodies_;
    double prepare_ms_ = 0;
};
}  // namespace kd::view
