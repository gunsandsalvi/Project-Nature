// The area's patch picture (A4.6, A5.3, PRE-20, WLD-31): a small picture of the ground's patches, each 4 m across, that
// the ground's shader reads so the one ground tile does not spread an even carpet over the whole area. For each patch
// it holds how tall and lush the growth is (a mass of taller and shorter growth, from smooth noise at the scale the
// tuning gives) and how worn the ground is (the clearing round the camp, and bare earth showing where growth is thin).
// The world makes the real picture from its seed and what people did (M3); this is the stand-in area's, made by code
// from the area's own numbers, the same every time. It touches no Godot, so its tests run alone; view/src/maps_draw.hpp
// makes the picture. East is x and north is y, in metres.
#pragma once

#include <cstdint>
#include <vector>

namespace kd::view::patches {

/// The numbers the picture is made with, in metres and shares (base/tuning/area.toml).
struct Params {
    double patch = 4.0;          // a patch's side
    int size = 64;               // patches along a side
    double growth_scale = 48.0;  // the width of the biggest masses of taller and shorter growth
    double bare_below = 0.2;     // the growth under which bare earth shows between the blades
    double clearing = 5.0;       // the radius round the camp where the ground is worn bare
    double clearing_fade = 5.0;  // how far past it the wear takes to leave
    std::uint64_t seed = 1;
};

/// The picture: size by size patches, four bytes a patch, a row of the array from the picture's south edge north, a
/// patch's four bytes: growth (0 short and dry to 255 tall and lush), wear (0 none to 255 bare earth), damp (0, kept
/// for the shore) and 255.
struct Patches {
    int size = 0;
    double west = 0.0;   // the west edge's place, in metres east
    double south = 0.0;  // the south edge's place, in metres north
    double patch = 4.0;
    std::vector<std::uint8_t> data;
};

/// Implements PRE-20 and WLD-31, see A4.6 and A5.3: the patch picture round a camp at (east, north), in metres; the
/// camp is in the middle of the picture's middle patch, so a half patch from the middle of the square. The same
/// camp, parameters and seed give the same picture.
[[nodiscard]] Patches make(double camp_east, double camp_north, const Params& params);

/// The growth at a place from 0 to 1, the smooth noise the picture's red is made from: the seed's masses over a lattice
/// of the scale, three octaves, so a test can ask what the picture holds without reading it.
[[nodiscard]] double growth_at(double east, double north, const Params& params);

}  // namespace kd::view::patches
