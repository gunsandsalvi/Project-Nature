// BIO-09 RES-06: integer-centimetre motion must stay outside solid rock.
#pragma once
#include "kd/world/life.hpp"
namespace kd::world {
// Sufficient clearance for every integer-time interpolation, at any speed.
// Truncating each displacement toward its origin can shift the continuous
// point by less than one centimetre on either axis. Axis-aligned motion is exact.
inline bool camp_walk_line_clear(num::Offset a, num::Offset b, const Habitat& rock) {
    if (a.dx == b.dx || a.dy == b.dy) return camp_line_clear(a, b, rock);
    auto padded = rock;
    if (b.dx > a.dx)
        ++padded.rock_east;
    else
        --padded.rock_west;
    if (b.dy > a.dy)
        ++padded.rock_north;
    else
        --padded.rock_south;
    return camp_line_clear(a, b, padded);
}
}  // namespace kd::world
