// Detail on demand (A7.5, WLD-13), for P8 The zoom (IMPLEMENTATION α0.5b): the ground anywhere in a world, at any
// spacing down to a metre, made from the world's cells and its seed alone, so it is the same every time it is made and
// making it changes nothing. Heights are the cells' own, smoothly joined, with relief added at scales the cells are too
// coarse to hold, rougher where the land is steep; colours are the cell's cover, varied in patches. Pre-production code
// (research 00).
#pragma once

#include <cstdint>
#include <vector>

#include "world.hpp"

namespace worldgen {

// The ground's height, metres above sea level, at a place given in metres east and north of the world's corner; the
// torus wraps both ways.
double ground_height(const World& w, double east, double north);

// A square of n × n heights, `spacing` metres apart, its first at (east, north), row by row from the south.
std::vector<float> ground_heights(const World& w, double east, double north, int n, double spacing);

// The ground's colour at each of those places as RGB, given their heights from ground_heights: its cell's cover, in
// the art book's colours, varied in patches a few tens of metres across; sea and lakes in their water colour. Cover
// meets cover along natural lines, the cells' edges warped by noise.
std::vector<std::uint8_t> ground_colours(const World& w, double east, double north, int n, double spacing,
                                         const std::vector<float>& heights);

// The world's cell heights averaged over 2, 4, 8 and more cells each way, for ground seen from far off (P8): level 0
// is the cells' own, each level after half as many each way, down to a few cells around.
struct HeightMips {
    std::vector<std::vector<float>> level;
    std::vector<int> width;
    std::vector<int> height;
    double metres = 0.0;  // a level-0 cell's side
};
HeightMips height_mips(const World& w);

// The ground's height at a place as seen with its points `spacing` metres apart: the cells' heights averaged over about
// that much, and only the relief at least twice that size, so ground seen from far off holds no detail it cannot show
// and does not shimmer as it is seen closer. At a spacing of about 1.5 m or less it is ground_height's.
double ground_height_at(const World& w, const HeightMips& mips, double east, double north, double spacing);

// A chunk of ground for P8's descent (A8.1), as continuous distance-dependent levels of detail draw it (CDLOD, Strugar
// 2010): n × n points `spacing` metres apart from (east, north), rows from the south, each from the west; n is odd.
// Toward its far edge a chunk morphs into the grid half as fine, each point sliding onto the even-numbered point
// before it each way, so where it meets a chunk of the next level the two are the same ground. For each point: its
// surface (the sea's where the ground lies under it), its true height, its normal, and the same three for the grid
// half as fine where it slides to.
struct GroundChunk {
    std::vector<float> surface;
    std::vector<float> truth;
    std::vector<float> normal;  // three a point: east, up and south, as the picture's axes
    std::vector<float> to_surface;
    std::vector<float> to_truth;
    std::vector<float> to_normal;
};
GroundChunk ground_chunk(const World& w, const HeightMips& mips, double east, double north, int n, double spacing);

// The world's cells as textures for P8's picture, rows from the south: the climate, four numbers a cell (the year's
// mean temperature, the coldest and warmest months', °C, and the moisture index); the cover, four bytes a cell (its
// colour on the art book's map, unshaded, and the share under trees); and the water, four bytes a cell (where its
// river goes, 0 to 7 as kDx and kDy, or 255 at sea; the land it drains, as 16 times the base-2 logarithm of its km²;
// the depth of a lake, metres; and where its main donor lies, the neighbour draining into it that drains the most,
// or 255 for none), so the picture can draw a river's course as a smooth curve through the cells.
std::vector<float> climate_texture(const World& w);
std::vector<std::uint8_t> cover_texture(const World& w);
std::vector<std::uint8_t> water_texture(const World& w);

// Noise for P8's clouds (A8.6), size × size × size points that wrap every way, four bytes a point: the clouds' shape,
// Perlin noise eroded by Worley's cells (Schneider 2015), then Worley noise at 2, 4 and 8 times its frequency, for
// their edges; then its smaller copies for its mipmaps, each half the last each way, the average of the eight points
// under each, down to one point. Made from the seed alone; size a power of two.
std::vector<std::uint8_t> cloud_noise(std::uint64_t seed, int size);

// The trees standing in a square `side` metres across from (east, north), three numbers each: metres east and north
// of its corner, and the ground's height there. As many as the cover holds, each drawn by keyed chance from its place
// on a grid of `spacing` metres, so a place always grows the same trees and squares side by side share none (A9).
// The grid's spacing divides the world's.
std::vector<float> ground_trees(const World& w, double east, double north, double side, double spacing);

}  // namespace worldgen
