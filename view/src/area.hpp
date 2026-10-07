// The stand-in area's land (A4.6, A5.3, A4.5, T2.3b.2, T2.3b.4): a square of meadow with a river across it, made by
// code until the world's own ground comes (M3). The meadow is a carpet of flat quads, and the river runs in a strip
// of a heightfield, its bed below the water's level, its banks rising to the meadow, its width and depth wandering
// along it; the strip is a grid whose rows crowd round the banks and whose columns spread out with distance, so the
// shore is fine where it is seen and no seam opens where the strip meets the carpet. The water's surface is a mesh of
// the same grid at the level, each vertex carrying the bed's height under it, so the ground's shader and the surface's
// work out the same shore from the same heights. It touches no Godot, so its tests run alone; view/src/area_draw.hpp
// draws it. East is x, up is y and south is z, about the area's centre, in metres.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace kd::view::area {

/// The area's numbers in metres, from the tuning file (kd::look::AreaTuning).
struct Shape {
    double reach = 1024.0;        // half the side of the square the area covers
    double strip = 48.0;          // half the width of the strip the river runs in
    double bank = 0.4;            // how far the ground stands above the river's level
    double width = 12.0;          // the river's mean width, bank to bank
    double width_wobble = 0.3;    // how far each bank strays from it, as a share of half the width
    double depth = 0.9;           // the river's mean depth at its deepest line
    double depth_wobble = 0.6;    // how far its depth strays from that, as a share
    double wobble_length = 60.0;  // how far along the river its widths and depths take to repeat
    double run = 1.0;             // how far the bank runs from the water's edge up to the meadow
    double spacing = 0.25;        // the strip's rows round the banks
    std::uint64_t seed = 1;
};

/// How far the river's banks lie from its middle line at a place along it: the north bank and the south.
struct Banks {
    double north = 0.0;
    double south = 0.0;
};

/// A mesh the way Godot wants it: three numbers a vertex for its place and its facing, the height of the bed under it
/// (the surface's second texture coordinate), and three vertices a triangle, wound clockwise seen from above.
struct Mesh {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> beds;
    std::vector<std::uint32_t> indices;

    [[nodiscard]] std::size_t vertices() const { return positions.size() / 3; }
    [[nodiscard]] std::size_t triangles() const { return indices.size() / 3; }
};

/// Implements PRE-23 and PRE-26, see A4.6 and A4.5: the stand-in area's river, which every other part of the area is
/// worked out from. A river has its shape's numbers and the phases its wandering takes from the seed.
class River {
public:
    explicit River(const Shape& shape);

    [[nodiscard]] const Shape& shape() const { return shape_; }
    /// The river's level, in metres up from the meadow: below it by the bank's height.
    [[nodiscard]] double level() const { return -shape_.bank; }
    /// Where the banks lie, north and south of the middle line, at a place along the river in metres east.
    [[nodiscard]] Banks banks(double east) const;
    /// How deep the river is at its deepest line at that place.
    [[nodiscard]] double deepest(double east) const;
    /// The ground's height at a point, in metres east and north of the area's centre, up from the meadow: 0 on the
    /// meadow, the bank rising to it from the water's edge, and under the level the bed.
    [[nodiscard]] double height(double east, double north) const;

    /// The columns of the strip, from the area's west edge to its east, and the rows from its south edge to its north,
    /// in metres: the places its vertices lie.
    [[nodiscard]] std::vector<double> columns() const;
    [[nodiscard]] std::vector<double> rows() const;
    /// How far from the middle line the river and its banks can reach: the rows are fine within it.
    [[nodiscard]] double fine() const;

    /// The strip of heightfield the river runs in, the whole length of the area.
    [[nodiscard]] Mesh strip() const;
    /// The water's surface: the strip's grid where the river can be, at the level, each vertex carrying the bed's
    /// height under it.
    [[nodiscard]] Mesh surface() const;
    /// The flat carpet of meadow north and south of the strip, to the area's edge.
    [[nodiscard]] Mesh carpet() const;

private:
    [[nodiscard]] double wobble(double east, std::size_t which) const;

    Shape shape_;
    std::array<double, 9> phases_{};
};

}  // namespace kd::view::area
