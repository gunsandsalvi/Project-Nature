// The view's maps round things that stand on the ground (A4.4, A4.3, PRE-21, PRE-24, PRE-30): small pictures seen from
// above, made from the tops of what stands there, that the light function reads at every pixel, so no screen-space pass
// is needed. From the triangles of the things, each rasterised into a height for every texel (the top of whatever
// stands there, and nothing for bare ground), they make four channels and the heights themselves:
//   openness   how much of the sky a point of bare ground sees past what stands round it: full in the open, less
//              the nearer and taller the things are, from the horizon of each of a ring of directions;
//   contact    the ground's darkening just outside each thing's foot, soft at its outer edge, none under the thing;
//   sun angle  the steepest angle up to anything between a texel and the sun, as a share of a right angle;
//   distance   how far off that thing stands, as a share of the farthest caster's distance;
// so the light function can tell lit from shadowed, and soften a long shadow with its distance, since a shadow at a
// thing's foot stays sharp. The heights tell the light function which points lie under a roof. It touches no Godot, so
// its tests run alone; view/src/maps_draw.hpp makes the pictures. East is x, up is y and south is z, in metres; a
// texel row runs north from the picture's south edge.
#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace kd::view::maps {

/// A triangle in metres: three points of three numbers each, x east, y up, z south.
struct Triangle {
    std::array<double, 9> p{};
};

/// The numbers the maps are made with, in metres and shares (base/tuning/light.toml).
struct Params {
    double texel = 0.06;             // the side of a texel
    double open_reach = 2.0;         // how far round a point openness looks
    double open_strength = 0.8;      // how much of the sky the nearest, tallest things take at most
    double contact_width = 0.18;     // how far from a thing's foot the ground is darkened
    double contact_strength = 0.55;  // how much the ground is darkened at the foot
    double shadow_reach = 14.0;      // the farthest caster's distance the maps hold
};

/// Where the sun is: the horizontal way toward it, a unit vector east and south, and the tangent of its height above
/// the horizon.
struct Sun {
    double east = 0.0;
    double south = 1.0;
    double rise = 0.5;
};

/// The maps: a square of size by size texels, a texel row a row of the arrays from the square's south edge north.
struct Maps {
    int size = 0;        // texels along a side; 0 when there is nothing to map
    double west = 0.0;   // the west edge's place, in metres east
    double south = 0.0;  // the south edge's place, in metres north
    double texel = 0.06;
    double shadow_reach = 14.0;
    int footprint = 0;  // the texels something stands on
    std::vector<std::uint8_t>
        view;                 // four bytes a texel: openness, contact, sun angle, distance; 255, 255, 0, 0 for none
    std::vector<float> tops;  // each texel's top, in metres up
};

/// Implements PRE-21, PRE-24 and PRE-30, see A4.4: the maps round these triangles for a sun. The square covers them and
/// the farthest their effects reach, so its border is free of them; the same triangles, parameters and sun give the
/// same maps. No triangles give no maps.
[[nodiscard]] Maps make(const std::vector<Triangle>& triangles, const Sun& sun, const Params& params);

}  // namespace kd::view::maps
