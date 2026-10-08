#include "maps_draw.hpp"

#include <cstring>

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "kd/num/maths.hpp"
#include "maps.hpp"
#include "patches.hpp"

namespace kd::view {

namespace {

// A number the page gave in its parameters, or the maps' own.
double given(const godot::Dictionary& params, const char* key, double otherwise) {
    const godot::String name(key);
    return params.has(name) ? static_cast<double>(params[name]) : otherwise;
}

godot::PackedByteArray bytes_of(const void* data, std::size_t length) {
    godot::PackedByteArray out;
    out.resize(static_cast<int64_t>(length));
    if (length > 0) {
        std::memcpy(out.ptrw(), data, length);
    }
    return out;
}

}  // namespace

godot::Dictionary KdMaps::make(const godot::PackedVector3Array& triangles, godot::Vector3 sun_toward,
                               const godot::Dictionary& params) const {
    godot::Dictionary out;
    out["problem"] = "";
    if (triangles.size() % 3 != 0) {
        out["problem"] = godot::String("the triangles are not a multiple of three points");
        return out;
    }
    std::vector<maps::Triangle> shapes;
    shapes.reserve(static_cast<std::size_t>(triangles.size() / 3));
    for (int64_t i = 0; i + 2 < triangles.size(); i += 3) {
        maps::Triangle t;
        for (int64_t k = 0; k < 3; ++k) {
            const godot::Vector3 p = triangles[i + k];
            t.p[static_cast<std::size_t>(3 * k)] = static_cast<double>(p.x);
            t.p[static_cast<std::size_t>(3 * k + 1)] = static_cast<double>(p.y);
            t.p[static_cast<std::size_t>(3 * k + 2)] = static_cast<double>(p.z);
        }
        shapes.push_back(t);
    }
    maps::Params numbers;
    numbers.texel = given(params, "texel", numbers.texel);
    numbers.open_reach = given(params, "open_reach", numbers.open_reach);
    numbers.open_strength = given(params, "open_strength", numbers.open_strength);
    numbers.contact_width = given(params, "contact_width", numbers.contact_width);
    numbers.contact_strength = given(params, "contact_strength", numbers.contact_strength);
    numbers.shadow_reach = given(params, "shadow_reach", numbers.shadow_reach);
    const double across = num::hypot(static_cast<double>(sun_toward.x), static_cast<double>(sun_toward.z));
    maps::Sun sun;
    if (across > 1e-9) {
        sun.east = static_cast<double>(sun_toward.x) / across;
        sun.south = static_cast<double>(sun_toward.z) / across;
        sun.rise = static_cast<double>(sun_toward.y) / across;
    }
    const maps::Maps made = maps::make(shapes, sun, numbers);
    out["west"] = made.west;
    out["south"] = made.south;
    out["width"] = made.size * made.texel;
    out["shadow_reach"] = made.shadow_reach;
    out["footprint"] = static_cast<int64_t>(made.footprint);
    out["size"] = static_cast<int64_t>(made.size);
    if (made.size == 0) {
        return out;
    }
    out["view"] = godot::Image::create_from_data(made.size, made.size, false, godot::Image::FORMAT_RGBA8,
                                                 bytes_of(made.view.data(), made.view.size()));
    out["tops"] = godot::Image::create_from_data(made.size, made.size, false, godot::Image::FORMAT_RF,
                                                 bytes_of(made.tops.data(), made.tops.size() * sizeof(float)));
    return out;
}

godot::Dictionary KdMaps::make_patches(double camp_east, double camp_north, const godot::Dictionary& params) const {
    godot::Dictionary out;
    out["problem"] = "";
    patches::Params numbers;
    numbers.patch = given(params, "patch", numbers.patch);
    numbers.size = static_cast<int>(given(params, "size", static_cast<double>(numbers.size)));
    numbers.growth_scale = given(params, "growth_scale", numbers.growth_scale);
    numbers.bare_below = given(params, "bare_below", numbers.bare_below);
    numbers.clearing = given(params, "clearing", numbers.clearing);
    numbers.clearing_fade = given(params, "clearing_fade", numbers.clearing_fade);
    numbers.seed = static_cast<std::uint64_t>(given(params, "seed", static_cast<double>(numbers.seed)));
    if (numbers.size < 1 || numbers.size > 1024 || numbers.patch <= 0.0) {
        out["problem"] = godot::String("the patch picture's size or patch is out of range");
        return out;
    }
    const patches::Patches made = patches::make(camp_east, camp_north, numbers);
    out["west"] = made.west;
    out["south"] = made.south;
    out["width"] = made.size * made.patch;
    out["picture"] = godot::Image::create_from_data(made.size, made.size, false, godot::Image::FORMAT_RGBA8,
                                                    bytes_of(made.data.data(), made.data.size()));
    return out;
}

void KdMaps::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("make", "triangles", "sun_toward", "params"), &KdMaps::make);
    godot::ClassDB::bind_method(godot::D_METHOD("make_patches", "camp_east", "camp_north", "params"),
                                &KdMaps::make_patches);
}

}  // namespace kd::view
