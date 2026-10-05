// P7's smooth noise on the torus (A7.2): value noise on lattices that wrap with the world, summed over octaves, read at
// a place given as fractions of the world's width and height, so a field is the same at the coarse size and at full
// size. Each lattice point is keyed chance (A3.5). Pre-production code (research 00).
#pragma once

#include <cstdint>
#include <vector>

namespace worldgen {

class Noise {
public:
    // The first octave has `base` lattice cells from pole to pole and twice as many around; each octave after has
    // twice as many, with `keep` times the amplitude of the one before.
    Noise(std::uint64_t seed, std::uint64_t field, int base, int octaves, double keep);

    // About -1 to 1.
    [[nodiscard]] double at(double u, double v) const;

private:
    struct Octave {
        int nx = 0;
        int ny = 0;
        double amplitude = 0.0;
        std::vector<double> values;
    };
    std::vector<Octave> octaves_;
};

}  // namespace worldgen
