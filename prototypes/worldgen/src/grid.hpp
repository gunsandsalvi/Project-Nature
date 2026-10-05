// P7 World generation (IMPLEMENTATION α0.5a, A7): the world's cells on its torus (WLD-01): the map wraps both
// ways, the equator across its middle and the poles along the line where it wraps north to south. A grid of
// width by height cells, about 1 km each at full size (WLD-03), and the same world at a quarter that size each way
// for the coarse candidates (WLD-10). Pre-production code (research 00).
#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace worldgen {

struct Grid {
    int width = 0;
    int height = 0;
    double km = 1.0;  // a cell's side

    [[nodiscard]] int cells() const { return width * height; }
    [[nodiscard]] int x(int cell) const { return cell % width; }
    [[nodiscard]] int y(int cell) const { return cell / width; }
    [[nodiscard]] int at(int cx, int cy) const {
        const int wx = ((cx % width) + width) % width;
        const int wy = ((cy % height) + height) % height;
        return (wy * width) + wx;
    }
    // The latitude in degrees, +90 at the north pole along the wrap line, 0 at the equator in the middle, -90 at the
    // south pole, back along the wrap line.
    [[nodiscard]] double latitude(int cell) const {
        return 90.0 - (180.0 * (static_cast<double>(y(cell)) + 0.5) / static_cast<double>(height));
    }
    // The shortest step between two coordinates on the wrapping axis of the given length.
    static int wrap_delta(int d, int length) {
        if (d > length / 2) {
            return d - length;
        }
        if (d < -length / 2) {
            return d + length;
        }
        return d;
    }
    // The distance between two cells across the torus, in cells.
    [[nodiscard]] double distance(int a, int b) const;
};

// The eight neighbours' offsets: four straight, then four diagonal.
constexpr std::array<int, 8> kDx = {1, -1, 0, 0, 1, 1, -1, -1};
constexpr std::array<int, 8> kDy = {0, 0, 1, -1, 1, -1, 1, -1};

// Value noise that wraps with the torus: the lattice's numbers keyed by the seed, blended smoothly, over octaves.
// period_x and period_y are the lattice's points a side at the first octave; each octave doubles them.
double torus_noise(std::uint64_t seed, double fx, double fy, int period_x, int period_y, int octaves);

}  // namespace worldgen
