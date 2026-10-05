// Places on the world (A3.4): whole centimetres on a torus that wraps east to west and north to south (WLD-01), with
// every difference and squared distance exact in 64-bit arithmetic, so no place ever depends on rounding.
#pragma once

#include <bit>
#include <cstdint>

#include "kd/core/check.hpp"
#include "kd/num/whole.hpp"

namespace kd::num {

/// A place: centimetres east of the western edge and north of the southern, each from 0 up to the torus's size.
struct Point {
    std::int32_t x = 0;
    std::int32_t y = 0;
    friend constexpr bool operator==(Point, Point) = default;
};

/// The way from one place to another, the short way round, in centimetres east and north.
struct Offset {
    std::int64_t dx = 0;
    std::int64_t dy = 0;
    friend constexpr bool operator==(Offset, Offset) = default;
};

/// The largest whole number whose square is at most n, exactly and with no floating point: Newton's steps down from
/// a power of two above the root. Implements RES-05, see A3.4.
constexpr std::uint64_t isqrt(std::uint64_t n) {
    if (n < 2) {
        return n;
    }
    std::uint64_t x = std::uint64_t{1} << ((std::bit_width(n) + 1) / 2);
    while (true) {
        const std::uint64_t next = (x + n / x) / 2;
        if (next >= x) {
            return x;
        }
        x = next;
    }
}

/// The world's surface: its width and height in centimetres, and the arithmetic of places on it. Implements WLD-01
/// and RES-05, see A3.4: the map wraps both ways, and every answer is a whole number.
class Torus {
public:
    /// Each side from 2 cm to 2^30 cm (about 10,700 km), so that differences and squared distances stay exact.
    constexpr Torus(std::int32_t width, std::int32_t height) : width_(width), height_(height) {
        KD_CHECK(width >= 2 && width <= (1 << 30) && height >= 2 && height <= (1 << 30),
                 "a torus's sides must be from 2 cm to 2^30 cm");
    }

    [[nodiscard]] constexpr std::int32_t width() const { return width_; }
    [[nodiscard]] constexpr std::int32_t height() const { return height_; }

    /// Any place, however far outside, brought onto the torus.
    [[nodiscard]] constexpr Point wrap(std::int64_t x, std::int64_t y) const {
        return {static_cast<std::int32_t>(floor_mod(x, width_)), static_cast<std::int32_t>(floor_mod(y, height_))};
    }

    /// A place moved by an offset, wrapping as it goes.
    [[nodiscard]] constexpr Point moved(Point p, Offset d) const {
        return wrap(std::int64_t{p.x} + floor_mod(d.dx, width_), std::int64_t{p.y} + floor_mod(d.dy, height_));
    }

    /// The short way from one place to another: each part at most half the torus. Exactly half way round, both
    /// ways are as short, and the one taken does not cross the edge where the map wraps, so the way back is always
    /// the exact reverse.
    [[nodiscard]] constexpr Offset offset(Point from, Point to) const {
        return {shortest(from.x, to.x, width_), shortest(from.y, to.y, height_)};
    }

    /// The square of the distance between two places, the short way, exactly.
    [[nodiscard]] constexpr std::int64_t squared_distance(Point a, Point b) const {
        const Offset d = offset(a, b);
        return d.dx * d.dx + d.dy * d.dy;
    }

    /// The distance between two places in whole centimetres, rounded down.
    [[nodiscard]] constexpr std::int64_t distance(Point a, Point b) const {
        return static_cast<std::int64_t>(isqrt(static_cast<std::uint64_t>(squared_distance(a, b))));
    }

private:
    static constexpr std::int64_t shortest(std::int32_t from, std::int32_t to, std::int64_t size) {
        std::int64_t d = floor_mod(std::int64_t{to} - from, size);
        if (2 * d > size || (2 * d == size && from > to)) {
            d -= size;
        }
        return d;
    }

    std::int32_t width_;
    std::int32_t height_;
};

}  // namespace kd::num
