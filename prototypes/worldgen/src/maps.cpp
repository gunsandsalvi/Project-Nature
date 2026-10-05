// P7's world map, for the app's offer and the note: a pixel for every few cells, coloured by biome and depth, shaded
// by the relief with light from the north-west, with the big rivers drawn. Pre-production code (research 00).
#include <algorithm>
#include <cmath>

#include "world.hpp"

namespace worldgen {

namespace {

struct Rgb {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
};

// Each biome's colour, taken from the art book's world map (art/book/plates/zoom/map-noon.png): sea, ice, tundra,
// conifer forest, broadleaf forest, grassland, dry scrub, desert, savanna, tropical forest, marsh, mountain heights and
// shore.
constexpr std::array<Rgb, kBiomes> kColours = {
    Rgb{60, 104, 121}, Rgb{236, 240, 227}, Rgb{176, 182, 132}, Rgb{65, 93, 77},   Rgb{77, 124, 75},
    Rgb{139, 153, 70}, Rgb{190, 180, 100}, Rgb{236, 211, 136}, Rgb{176, 182, 84}, Rgb{53, 94, 66},
    Rgb{93, 132, 72},  Rgb{150, 135, 146}, Rgb{231, 207, 158},
};

}  // namespace

std::vector<std::uint8_t> map_rgb(const World& w, int scale) {
    const Grid& g = w.grid;
    const int mw = g.width / scale;
    const int mh = g.height / scale;
    std::vector<std::uint8_t> out(static_cast<std::size_t>(mw) * static_cast<std::size_t>(mh) * 3);
    const double river = 4000.0;  // km² drained before a river shows at this scale
    for (int my = 0; my < mh; ++my) {
        for (int mx = 0; mx < mw; ++mx) {
            const int x = (mx * scale) + (scale / 2);
            const int y = (my * scale) + (scale / 2);
            const int c = g.at(x, y);
            const auto cs = static_cast<std::size_t>(c);
            const auto b = static_cast<Biome>(w.biome[cs]);
            Rgb col = kColours[static_cast<std::size_t>(b)];
            if (w.sea(c)) {
                // shelves light, the deep sea one colour, as the art book's map
                const double deep = std::clamp(-w.height[cs] / 800.0, 0.0, 1.0);
                col = {85.0 - (25.0 * deep), 134.0 - (30.0 * deep), 139.0 - (18.0 * deep)};
            } else {
                // light from the north-west over the relief, a few cells across
                const int k = std::max(1, scale);
                const double west = w.height[static_cast<std::size_t>(g.at(x - k, y))];
                const double east = w.height[static_cast<std::size_t>(g.at(x + k, y))];
                const double north = w.height[static_cast<std::size_t>(g.at(x, y + k))];
                const double south = w.height[static_cast<std::size_t>(g.at(x, y - k))];
                const double rise = ((west - east) + (north - south)) / (2.0 * k * g.metres);
                const double shade = std::clamp(1.0 + (3.5 * rise), 0.7, 1.2);
                col = {col.r * shade, col.g * shade, col.b * shade};
                bool wet = w.lake[cs] > 0.0F;
                for (int dy = 0; dy < scale && !wet; ++dy) {
                    for (int dx = 0; dx < scale && !wet; ++dx) {
                        wet = w.area[static_cast<std::size_t>(g.at((mx * scale) + dx, (my * scale) + dy))] > river;
                    }
                }
                if (wet) {
                    col = {98, 133, 212};
                }
            }
            // the map's north at its top
            const std::size_t at = ((static_cast<std::size_t>(mh - 1 - my) * static_cast<std::size_t>(mw)) +
                                    static_cast<std::size_t>(mx)) *
                                   3;
            out[at] = static_cast<std::uint8_t>(std::clamp(col.r, 0.0, 255.0));
            out[at + 1] = static_cast<std::uint8_t>(std::clamp(col.g, 0.0, 255.0));
            out[at + 2] = static_cast<std::uint8_t>(std::clamp(col.b, 0.0, 255.0));
        }
    }
    return out;
}

}  // namespace worldgen
