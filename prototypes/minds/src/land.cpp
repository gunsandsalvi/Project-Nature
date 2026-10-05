// P6's land (A7, A11): see land.hpp. Pre-production code (research 00).
#include "land.hpp"

#include <algorithm>
#include <cmath>

#include "chance.hpp"
#include "draws.hpp"
#include "maths.hpp"

namespace minds {

namespace {

// A number from 0 up to 1 at a lattice point of the land's noise.
double lattice(std::uint64_t seed, int ix, int iy) {
    return samebits::chance(seed, static_cast<std::uint32_t>(ix), static_cast<std::uint32_t>(iy), Draw::kLand);
}

double smooth(double t) {
    return t * t * (3.0 - (2.0 * t));
}

// Value noise: the lattice's numbers blended smoothly between its points.
double noise(std::uint64_t seed, double x, double y) {
    const double fx = std::floor(x);
    const double fy = std::floor(y);
    const int ix = static_cast<int>(fx);
    const int iy = static_cast<int>(fy);
    const double tx = smooth(x - fx);
    const double ty = smooth(y - fy);
    const double a = lattice(seed, ix, iy);
    const double b = lattice(seed, ix + 1, iy);
    const double c = lattice(seed, ix, iy + 1);
    const double d = lattice(seed, ix + 1, iy + 1);
    const double top = a + ((b - a) * tx);
    const double bottom = c + ((d - c) * tx);
    return top + ((bottom - top) * ty);
}

// The river's middle, in cells from the north, at a column.
int river_y(int x) {
    const double c = 560.0 + (90.0 * samebits::sine(x / 110.0)) + (35.0 * samebits::sine((x / 37.0) + 1.3));
    return static_cast<int>(std::floor(c));
}

struct Stock {
    Kind kind;
    int count;
    float most;
};

// How many spots of each kind the land holds, and the most each holds: meals of berries, nuts or roots, animals,
// loads of firewood, nodules of flint.
constexpr std::array<Stock, 6> kStocks = {{
    {Kind::kBerries, 500, 30.0F},
    {Kind::kNuts, 300, 40.0F},
    {Kind::kRoots, 400, 20.0F},
    {Kind::kGame, 120, 4.0F},
    {Kind::kWood, 500, 60.0F},
    {Kind::kFlint, 40, 30.0F},
}};

constexpr int kCamps = 40;
constexpr float kWaterMost = 1.0e6F;

}  // namespace

double metres(int a, int b) {
    const double dx = cell_x(a) - cell_x(b);
    const double dy = cell_y(a) - cell_y(b);
    return std::sqrt((dx * dx) + (dy * dy)) * kCellMetres;
}

Land::Land(std::uint64_t seed)
    : ground_(static_cast<std::size_t>(kCells), Ground::kOpen),
      in_block_(static_cast<std::size_t>(kBlocks) * static_cast<std::size_t>(kBlocks)) {
    for (int y = 0; y < kSide; ++y) {
        for (int x = 0; x < kSide; ++x) {
            const double n = (0.65 * noise(seed, x / 48.0, y / 48.0)) + (0.35 * noise(seed + 1, x / 14.0, y / 14.0));
            Ground g = Ground::kOpen;
            if (n > 0.70) {
                g = Ground::kBlocked;  // rock and thicket
            } else if (n > 0.61) {
                g = Ground::kRough;
            }
            ground_[static_cast<std::size_t>(cell_at(x, y))] = g;
        }
    }
    // The river, five cells wide, crossed at fords four cells wide every 96 cells.
    for (int x = 0; x < kSide; ++x) {
        const bool ford = (x % 96) < 4;
        for (int dy = -2; dy <= 2; ++dy) {
            const int y = river_y(x) + dy;
            if (y >= 0 && y < kSide) {
                ground_[static_cast<std::size_t>(cell_at(x, y))] = ford ? Ground::kOpen : Ground::kWater;
            }
        }
    }
    // Water: the river's banks every 24 cells, and 40 springs.
    for (int x = 0; x < kSide; x += 24) {
        for (const int y : {river_y(x) - 3, river_y(x) + 3}) {
            if (y >= 0 && y < kSide && passable(cell_at(x, y))) {
                spots.push_back({cell_at(x, y), Kind::kWater, kWaterMost, kWaterMost});
            }
        }
    }
    // Anything else placed by keyed chance on ground people can stand on, tries counted so each is the same each time.
    std::uint64_t tries = 0;
    auto place = [this, seed, &tries]() {
        for (;;) {
            const auto x = static_cast<int>(samebits::chance(seed, tries, 0, Draw::kPlace) * kSide);
            const auto y = static_cast<int>(samebits::chance(seed, tries, 1, Draw::kPlace) * kSide);
            ++tries;
            if (passable(cell_at(x, y))) {
                return cell_at(x, y);
            }
        }
    };
    for (int i = 0; i < 40; ++i) {
        spots.push_back({place(), Kind::kWater, kWaterMost, kWaterMost});
    }
    for (const Stock& s : kStocks) {
        for (int i = 0; i < s.count; ++i) {
            spots.push_back({place(), s.kind, s.most, s.most});
        }
    }
    for (std::size_t i = 0; i < spots.size(); ++i) {
        in_block_[static_cast<std::size_t>(block_of(spots[i].cell))].push_back(static_cast<std::int32_t>(i));
    }
    // The bands' camps: on open ground, near water, at least 400 m apart.
    std::vector<std::int32_t> near;
    while (static_cast<int>(camps.size()) < kCamps) {
        const int c = place();
        if (ground(c) != Ground::kOpen) {
            continue;
        }
        const bool apart =
            std::all_of(camps.begin(), camps.end(), [c](const Camp& o) { return metres(c, o.cell) >= 400.0; });
        if (!apart) {
            continue;
        }
        spots_near(c, 200.0, &near);
        const bool water = std::any_of(near.begin(), near.end(), [this](std::int32_t s) {
            return spots[static_cast<std::size_t>(s)].kind == Kind::kWater;
        });
        if (water) {
            camps.push_back({c, 20.0F, 6.0F, 4.0F});
        }
    }
}

void Land::spots_near(int cell, double within, std::vector<std::int32_t>* out) const {
    out->clear();
    const int reach = static_cast<int>(std::ceil(within / (kBlock * kCellMetres)));
    const int bx = cell_x(cell) / kBlock;
    const int by = cell_y(cell) / kBlock;
    for (int y = std::max(0, by - reach); y <= std::min(kBlocks - 1, by + reach); ++y) {
        for (int x = std::max(0, bx - reach); x <= std::min(kBlocks - 1, bx + reach); ++x) {
            const int block = (y * kBlocks) + x;
            for (const std::int32_t s : in_block_[static_cast<std::size_t>(block)]) {
                if (metres(cell, spots[static_cast<std::size_t>(s)].cell) <= within) {
                    out->push_back(s);
                }
            }
        }
    }
}

void Land::regrow(int season) {
    for (Spot& s : spots) {
        float add = 0.0F;
        switch (s.kind) {
            case Kind::kBerries:
                add = season == 3 ? 0.0F : 5.0F;
                break;
            case Kind::kNuts:
                add = season == 2 ? 4.0F : 0.0F;
                break;
            case Kind::kRoots:
                add = season == 3 ? 1.0F : 2.0F;
                break;
            case Kind::kGame:
                add = 0.2F;
                break;
            case Kind::kWood:
                add = 0.4F;
                break;
            case Kind::kWater:
                add = s.most;
                break;
            case Kind::kFlint:
                break;
        }
        s.amount = std::min(s.most, s.amount + add);
    }
}

}  // namespace minds
