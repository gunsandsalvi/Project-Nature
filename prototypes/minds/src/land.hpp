// P6 A thousand minds (IMPLEMENTATION α0.4b): the land its people walk, 4 km a side in cells of 4 m. Ground is open
// or rough, rock and thicket block it, and a river is crossed only at its fords. On it lie the spots people use:
// water, berries, nuts, roots, game, firewood and flint, each with an amount that use takes and the seasons bring
// back, and the bands' camps. Made from the seed alone. Pre-production code (research 00).
#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace minds {

constexpr int kSide = 1024;          // cells a side
constexpr double kCellMetres = 4.0;  // a cell's side
constexpr int kCells = kSide * kSide;
constexpr int kBlock = 32;  // cells a side of a block, the unit of paths' clusters and of finding spots near a place
constexpr int kBlocks = kSide / kBlock;

enum class Ground : std::uint8_t { kOpen = 0, kRough = 1, kBlocked = 2, kWater = 3 };

enum class Kind : std::uint8_t { kWater = 0, kBerries, kNuts, kRoots, kGame, kWood, kFlint };
constexpr int kKinds = 7;

struct Spot {
    std::int32_t cell = 0;
    Kind kind = Kind::kWater;
    float amount = 0.0F;
    float most = 0.0F;
};

struct Camp {
    std::int32_t cell = 0;
    float food = 0.0F;  // stored meals
    float wood = 0.0F;  // stored loads of firewood
    float fire = 0.0F;  // hours of fuel left in the hearth; burning while above 0
};

inline int cell_x(int cell) {
    return cell % kSide;
}
inline int cell_y(int cell) {
    return cell / kSide;
}
inline int cell_at(int x, int y) {
    return (y * kSide) + x;
}
inline int block_of(int cell) {
    return ((cell_y(cell) / kBlock) * kBlocks) + (cell_x(cell) / kBlock);
}
// The straight-line distance between two cells, in metres.
double metres(int a, int b);

class Land {
public:
    explicit Land(std::uint64_t seed);

    [[nodiscard]] Ground ground(int cell) const { return ground_[static_cast<std::size_t>(cell)]; }
    [[nodiscard]] bool passable(int cell) const { return ground(cell) <= Ground::kRough; }
    // The cost of crossing a cell, as a multiple of open ground's.
    [[nodiscard]] float effort(int cell) const { return ground(cell) == Ground::kRough ? 1.6F : 1.0F; }
    // The spots within the given metres of a cell, from the blocks round it.
    void spots_near(int cell, double within, std::vector<std::int32_t>* out) const;
    // A day's regrowth of every spot, by the season (0 spring to 3 winter).
    void regrow(int season);

    std::vector<Spot> spots;
    std::vector<Camp> camps;

private:
    std::vector<Ground> ground_;
    std::vector<std::vector<std::int32_t>> in_block_;  // each block's spots
};

}  // namespace minds
