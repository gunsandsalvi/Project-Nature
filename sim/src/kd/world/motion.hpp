// Exact integer proximity intervals. Conservative affine bounds skip constant
// spans; uncertain rounded centimetres use the original point-at-second test.
#pragma once
#include <algorithm>
#include <array>
#include <vector>
#include "kd/world/parts.hpp"

namespace kd::world {
struct MotionSpan {
    time::Seconds begin = 0, end = 0;  // [begin, end)
};
namespace motion_detail {
inline std::array<std::int64_t, 2> unwrapped(const Activity& a, const num::Torus& torus, time::Seconds at) {
    const auto d = torus.offset(a.from, a.to);
    if (a.end <= a.start || at <= a.start) return {a.from.x, a.from.y};
    const auto gone = std::min(at, a.end) - a.start;
    const auto duration = a.end - a.start;
    return {a.from.x + static_cast<std::int64_t>(static_cast<__int128>(d.dx) * gone / duration),
            a.from.y + static_cast<std::int64_t>(static_cast<__int128>(d.dy) * gone / duration)};
}
inline std::array<std::int64_t, 2> axis_bounds(std::int64_t low, std::int64_t high, std::int64_t period) {
    const auto distance = [&](std::int64_t value) {
        const auto folded = num::floor_mod(value, period);
        return std::min(folded, period - folded);
    };
    const auto next_zero = low + num::floor_mod(-low, period);
    const auto next_half = low + num::floor_mod(period / 2 - low, period);
    return {next_zero <= high ? 0 : std::min(distance(low), distance(high)),
            next_half <= high ? period / 2 : std::max(distance(low), distance(high))};
}
}  // namespace motion_detail

inline std::vector<MotionSpan> proximity_spans(const Activity& a, const Activity& b, const num::Torus& torus,
                                               time::Seconds begin, time::Seconds end, std::int64_t radius) {
    KD_CHECK(begin <= end && radius >= 0 && radius <= 1'000'000'000, "bounded motion interval");
    std::vector<MotionSpan> out;
    const auto append = [&](time::Seconds low, time::Seconds high) {
        if (!out.empty() && out.back().end == low)
            out.back().end = high;
        else
            out.push_back({low, high});
    };
    const auto same_translation =
        (a.from == a.to && b.from == b.to) ||
        (a.start == b.start && a.end == b.end && torus.offset(a.from, a.to) == torus.offset(b.from, b.to));
    const auto squared = radius * radius;
    const auto visit = [&](auto&& self, time::Seconds low, time::Seconds high) -> void {
        if (low >= high) return;
        if (high - low == 1) {
            if (torus.squared_distance(a.at(torus, low), b.at(torus, low)) <= squared) append(low, high);
            return;
        }
        const auto af = motion_detail::unwrapped(a, torus, low);
        const auto bf = motion_detail::unwrapped(b, torus, low);
        const auto al = motion_detail::unwrapped(a, torus, high - 1);
        const auto bl = motion_detail::unwrapped(b, torus, high - 1);
        // Each relative rounded endpoint differs from the affine position by <2
        // cm. Comparing their interpolation with an interior point needs <4 cm.
        const std::int64_t slack = same_translation ? 0 : 4;
        std::int64_t minimum = 0, maximum = 0;
        for (std::size_t axis = 0; axis < 2; ++axis) {
            const auto first = af[axis] - bf[axis], last = al[axis] - bl[axis];
            const auto bounds = motion_detail::axis_bounds(std::min(first, last) - slack, std::max(first, last) + slack,
                                                           axis == 0 ? torus.width() : torus.height());
            minimum += bounds[0] * bounds[0];
            maximum += bounds[1] * bounds[1];
        }
        if (minimum > squared) return;
        if (maximum <= squared) {
            append(low, high);
            return;
        }
        const auto middle = low + (high - low) / 2;
        self(self, low, middle);
        self(self, middle, high);
    };
    std::vector<time::Seconds> cuts{begin, end};
    for (const auto cut : {a.start, a.end, b.start, b.end})
        if (cut > begin && cut < end) cuts.push_back(cut);
    std::stable_sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
    for (std::size_t n = 1; n < cuts.size(); ++n) visit(visit, cuts[n - 1], cuts[n]);
    return out;
}
}  // namespace kd::world
