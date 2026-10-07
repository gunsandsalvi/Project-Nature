// The view's maps as pictures for the light function (A4.4, A4.3, PRE-21, PRE-24, PRE-30): view/src/maps.hpp's maps
// made from the triangles of what stands on the ground, as one RGBA picture (openness, contact, sun angle and distance)
// and one picture of the tops' heights, with where the square lies. This class converts and never decides for the world
// (WLD-13); game/look/view_maps.gd puts the pictures where the shaders read them.
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace kd::view {

/// Implements PRE-21, PRE-24 and PRE-30, see A4.4: the maps round the things, made.
class KdMaps : public godot::RefCounted {
    GDCLASS(KdMaps, godot::RefCounted)

public:
    /// The maps round triangles (every three points one triangle, in metres: x east, y up, z south) for a sun
    /// (sun_toward: the way to it, in the same axes), with params: texel, open_reach, open_strength, contact_width,
    /// contact_strength and shadow_reach, in metres and shares. Gives view (the Image of four channels), tops (the
    /// Image of heights, one float a texel), west and south (the square's edges, metres east and north), width (its
    /// side), shadow_reach, footprint (texels under things) and problem ("" when all is well).
    [[nodiscard]] godot::Dictionary make(const godot::PackedVector3Array& triangles, godot::Vector3 sun_toward,
                                         const godot::Dictionary& params) const;

protected:
    static void _bind_methods();
};

}  // namespace kd::view
