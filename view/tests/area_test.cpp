#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "area.hpp"
#include "doctest.h"

namespace area = kd::view::area;

namespace {

// A small area, so the tests are quick: the whole square 400 m across, the strip 40 m.
area::Shape small_shape() {
    area::Shape s;
    s.reach = 200.0;
    s.strip = 40.0;
    return s;
}

// The twice-signed area of a mesh's triangles seen from above, in x and z, for each triangle: positive when it winds
// clockwise seen from above, the way Godot's front faces do (A4.7).
std::vector<double> signed_areas(const area::Mesh& m) {
    std::vector<double> out;
    for (std::size_t t = 0; t < m.triangles(); ++t) {
        std::array<std::array<double, 2>, 3> p{};
        for (std::size_t k = 0; k < 3; ++k) {
            const std::size_t v = m.indices[3 * t + k];
            p[k] = {static_cast<double>(m.positions[3 * v]), static_cast<double>(m.positions[3 * v + 2])};
        }
        out.push_back(((p[1][0] - p[0][0]) * (p[2][1] - p[0][1]) - (p[1][1] - p[0][1]) * (p[2][0] - p[0][0])) / 2.0);
    }
    return out;
}

double total(const std::vector<double>& areas) {
    double sum = 0.0;
    for (const double a : areas) {
        sum += a;
    }
    return sum;
}

}  // namespace

// checks: PRE-26
TEST_CASE("the river's bed lies below its level and its banks rise to the meadow, deepest on the middle line") {
    const area::River river(small_shape());
    const double level = river.level();
    CHECK(level == doctest::Approx(-0.4));
    for (int i = 0; i <= 41; ++i) {
        const double east = -150.0 + 7.3 * i;
        const area::Banks b = river.banks(east);
        CHECK(b.north > 0.5);
        CHECK(b.south > 0.5);
        // the middle line is the deepest, and under the level
        const double middle = river.height(east, 0.0);
        CHECK(middle == doctest::Approx(level - river.deepest(east)));
        // at each bank's edge the ground is at the level
        CHECK(river.height(east, b.north) == doctest::Approx(level));
        CHECK(river.height(east, -b.south) == doctest::Approx(level));
        // across the bed the ground rises toward each bank, and across the bank up to the meadow, never past it
        double before = middle;
        for (int k = 0; 0.05 * k <= b.north + 3.0; ++k) {
            const double n = 0.05 * k;
            const double h = river.height(east, n);
            CHECK(h >= before - 1e-12);
            CHECK(h <= 1e-12);
            before = h;
        }
        // beyond the bank it is meadow
        CHECK(river.height(east, b.north + 1.5) == doctest::Approx(0.0));
        CHECK(river.height(east, -b.south - 1.5) == doctest::Approx(0.0));
    }
}

// checks: PRE-26
TEST_CASE("the river wanders with its seed: the same seed the same river, another seed another") {
    area::Shape a = small_shape();
    area::Shape b = a;
    b.seed = 2;
    const area::River first(a);
    const area::River again(a);
    const area::River other(b);
    bool differs = false;
    for (int i = 0; i <= 60; ++i) {
        const double east = -150.0 + 5.0 * i;
        CHECK(first.banks(east).north == again.banks(east).north);
        CHECK(first.deepest(east) == again.deepest(east));
        differs = differs || first.banks(east).north != other.banks(east).north;
    }
    CHECK(differs);
    // the banks stay within what the strip's rows are fine for, and the river never closes or dries
    for (int i = 0; i <= 223; ++i) {
        const double east = -190.0 + 1.7 * i;
        const area::Banks banks = first.banks(east);
        CHECK(std::max(banks.north, banks.south) < first.fine());
        CHECK(first.deepest(east) > 0.0);
    }
}

// checks: PRE-26
TEST_CASE("the strip and the carpet tile the whole square with no gap and no overlap, each triangle facing up") {
    const area::River river(small_shape());
    const area::Mesh strip = river.strip();
    const area::Mesh carpet = river.carpet();
    const std::vector<double> strip_areas = signed_areas(strip);
    const std::vector<double> carpet_areas = signed_areas(carpet);
    for (const double a : strip_areas) {
        CHECK(a > 0.0);
    }
    for (const double a : carpet_areas) {
        CHECK(a > 0.0);
    }
    // the strip is the whole length by twice its half width, the carpet the rest of the square
    CHECK(total(strip_areas) == doctest::Approx(2.0 * 200.0 * 2.0 * 40.0).epsilon(1e-9));
    CHECK(total(carpet_areas) == doctest::Approx(2.0 * 200.0 * 2.0 * 160.0).epsilon(1e-9));
    // every index is a vertex, and every normal points up
    for (const std::uint32_t i : strip.indices) {
        CHECK(i < strip.vertices());
    }
    for (std::size_t v = 0; v < strip.vertices(); ++v) {
        const double nx = static_cast<double>(strip.normals[3 * v]);
        const double ny = static_cast<double>(strip.normals[3 * v + 1]);
        const double nz = static_cast<double>(strip.normals[3 * v + 2]);
        CHECK(ny > 0.5);
        CHECK(std::sqrt(nx * nx + ny * ny + nz * nz) == doctest::Approx(1.0).epsilon(1e-5));
    }
}

// checks: PRE-26
TEST_CASE("the strip's vertices lie on the ground and its edges meet the carpet's at the meadow's height") {
    const area::River river(small_shape());
    const area::Mesh strip = river.strip();
    double most = 0.0;
    for (std::size_t v = 0; v < strip.vertices(); ++v) {
        const double east = static_cast<double>(strip.positions[3 * v]);
        const double up = static_cast<double>(strip.positions[3 * v + 1]);
        const double north = -static_cast<double>(strip.positions[3 * v + 2]);
        most = std::max(most, std::abs(up - river.height(east, north)));
        // the bed under a vertex is the vertex's own height
        CHECK(static_cast<double>(strip.beds[v]) == doctest::Approx(up).epsilon(1e-6));
        // on the strip's edge rows the ground is the meadow, so the carpet beside it meets it with no step
        if (std::abs(std::abs(north) - 40.0) < 1e-6) {
            CHECK(up == doctest::Approx(0.0).epsilon(1e-9));
        }
    }
    CHECK(most < 1e-5);
}

// checks: PRE-26 PRE-22
TEST_CASE("the strip's rows are as fine as asked round the banks and its columns spread out with distance") {
    const area::River river(small_shape());
    const std::vector<double> rows = river.rows();
    const std::vector<double> columns = river.columns();
    // from edge to edge, in order, the ends at the strip's and the area's
    CHECK(rows.front() == doctest::Approx(-40.0));
    CHECK(rows.back() == doctest::Approx(40.0));
    CHECK(columns.front() == doctest::Approx(-200.0));
    CHECK(columns.back() == doctest::Approx(200.0));
    CHECK(std::is_sorted(rows.begin(), rows.end()));
    CHECK(std::is_sorted(columns.begin(), columns.end()));
    // no gap in the fine region is more than the spacing, and none beyond it more than the most
    for (std::size_t i = 0; i + 1 < rows.size(); ++i) {
        const double gap = rows[i + 1] - rows[i];
        CHECK(gap > 0.0);
        if (std::abs(rows[i]) < river.fine() - 0.5 && std::abs(rows[i + 1]) < river.fine() - 0.5) {
            CHECK(gap <= 0.25 + 1e-9);
        }
        CHECK(gap <= 8.0 + 1e-9);
    }
    for (std::size_t i = 0; i + 1 < columns.size(); ++i) {
        const double gap = columns[i + 1] - columns[i];
        CHECK(gap > 0.0);
        CHECK(gap <= 16.0 + 1e-9);
        if (std::abs(columns[i]) < 100.0) {
            CHECK(gap <= 1.0 + 1e-9);
        }
    }
}

// checks: PRE-26
TEST_CASE("the water's surface lies at the level over the river, each vertex carrying the bed's height under it") {
    const area::River river(small_shape());
    const area::Mesh water = river.surface();
    const area::Mesh strip = river.strip();
    REQUIRE(water.vertices() > 0);
    CHECK(water.vertices() < strip.vertices());
    for (const double a : signed_areas(water)) {
        CHECK(a > 0.0);
    }
    double north_most = -1e9;
    double south_most = 1e9;
    for (std::size_t v = 0; v < water.vertices(); ++v) {
        const double east = static_cast<double>(water.positions[3 * v]);
        const double up = static_cast<double>(water.positions[3 * v + 1]);
        const double north = -static_cast<double>(water.positions[3 * v + 2]);
        CHECK(up == doctest::Approx(river.level()));
        CHECK(static_cast<double>(water.beds[v]) == doctest::Approx(river.height(east, north)).epsilon(1e-5));
        north_most = std::max(north_most, north);
        south_most = std::min(south_most, north);
    }
    // it reaches past the widest the river can be, so no water lies outside it
    CHECK(north_most > river.fine() - 0.3);
    CHECK(south_most < -river.fine() + 0.3);
    for (int i = 0; i <= 122; ++i) {
        const double east = -190.0 + 3.1 * i;
        CHECK(river.banks(east).north < north_most);
        CHECK(river.banks(east).south < -south_most);
    }
}

// checks: PRE-26
TEST_CASE("along any column the depth over the bed changes sign exactly at each bank: one shore a side") {
    const area::River river(small_shape());
    for (int i = 0; i <= 27; ++i) {
        const double east = -150.0 + 11.0 * i;
        int crossings = 0;
        double before = river.level() - river.height(east, -river.fine());
        for (int k = 0; 0.01 * k <= 2.0 * river.fine(); ++k) {
            const double north = -river.fine() + 0.01 * k;
            const double depth = river.level() - river.height(east, north);
            if ((before > 0.0) != (depth > 0.0)) {
                ++crossings;
            }
            before = depth;
        }
        CHECK(crossings == 2);
    }
}
