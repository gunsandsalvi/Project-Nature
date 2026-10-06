#include "kd/world/parts.hpp"

#include <algorithm>

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

void Activity::cut(const num::Torus& torus, time::Seconds t) {
    if (t >= end) {
        return;
    }
    to = at(torus, t);
    end = std::max(t, start);
}

std::int64_t Activity::share(time::Seconds t) const {
    if (t >= end || end <= start) {
        return 1'000'000;
    }
    if (t <= start) {
        return 0;
    }
    return (t - start) * 1'000'000 / (end - start);
}

}  // namespace kd::world
