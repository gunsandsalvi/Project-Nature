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

// The trees standing in a square `side` metres across from (east, north), three numbers each: metres east and north
// of its corner, and the ground's height there. As many as the cover holds, each drawn by keyed chance from its place
// on a grid of `spacing` metres, so a place always grows the same trees and squares side by side share none (A9).
// The grid's spacing divides the world's.
std::vector<float> ground_trees(const World& w, double east, double north, double side, double spacing);

}  // namespace worldgen
