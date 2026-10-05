#include "kd/world/parts.hpp"

namespace kd::world {

num::Point Activity::at(const num::Torus& torus, time::Seconds t) const {
    if (t <= start || end <= start) {
        return from;
    }
    if (t >= end) {
        return to;
    }
    const num::Offset way = torus.offset(from, to);
    const time::Seconds gone = t - start;
    const time::Seconds all = end - start;
    return torus.moved(from, {way.dx * gone / all, way.dy * gone / all});
}

}  // namespace kd::world
