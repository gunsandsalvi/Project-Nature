// T2.8a: restricted, disposable look surfaces. Metres here; never simulation state or a save schema.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "projection.hpp"

namespace kd::view::terrain {
struct Point {
    double east = 0, north = 0, up = 0;
};
struct Bounds {
    Point low, high;
};
enum class Kind : std::uint8_t { floor, cliff, roof, wall, water, calibration };
struct Surface {
    std::int64_t id = 0;
    Kind kind = Kind::floor;
    std::array<Point, 4> corners{};
    Point normal{0, 0, 1};
    std::int64_t covers = 0;
    int material = 4;
};
struct Caster {
    std::int64_t id = 0;
    Bounds bounds;
    bool round = false;
    enum class Shape : std::uint8_t { legacy, cylinder, cone, upper_ellipsoid };
    Shape shape = Shape::legacy;
    // Optional grounded contact footprint; e.g. a root collar wider than the actual trunk cylinder.
    double contact_east = 0.0, contact_north = 0.0;
    std::int64_t group = 0;  // one logical body may have several shadow volumes
};
struct Actor {
    std::int64_t id = 0;
    int art = 4;
    Point foot;
    std::int64_t surface = 0;
};
struct Light {
    Point sun{0.5, -0.4, 0.75};
    Point sunlight{0.78, 0.75, 0.66};
    Point sky{0.3, 0.35, 0.43};
    Point fire{3, 2, 0.6};
    bool fire_on = true;
    std::uint64_t revision = 1;
    std::string hour = "noon", weather = "dry";
};
struct Scene {
    std::string name = "flat";
    std::uint64_t revision = 1;
    std::vector<Surface> surfaces;
    std::vector<Caster> casters;
    std::vector<Actor> actors;
};
struct Piece {
    std::int64_t id = 0, surface = 0, covers = 0;
    Pixel low, high;
    double depth = 0;
    bool receiver = false, roof = false, water = false;
    double minimum_height = 0, maximum_height = 0;
};
struct Pick {
    std::int64_t surface = 0;
    Point point;
    bool found = false;
};
struct Mask {
    int width = 0, height = 0;
    std::vector<std::uint8_t> sun_bits;
    std::vector<std::uint8_t> rgba;  // direct sun, sky openness, contact, local-fire visibility
};

/// Implements PRE-23, PRE-24, PRE-26, WLD-13: labelled surfaces, shared floor/bed/water heights and cave scope.
[[nodiscard]] Scene fixture(const std::string& name);
[[nodiscard]] Light light(const std::string& hour, const std::string& weather, int direction, bool fire);
[[nodiscard]] Point on(const Surface& surface, double u, double v);
[[nodiscard]] Point normal(const Surface& surface);
[[nodiscard]] Pick walk(const Scene& scene, double east, double north);
[[nodiscard]] Pick pick(const Scene& scene, const Projection& projection, Pixel raster, bool cutaway);
/// Implements PRE-24, PRE-28, PRE-33: local overlap graph, shared cross-chunk order and persistent-ID ties.
[[nodiscard]] std::vector<std::int64_t> order(const std::vector<Piece>& pieces);
[[nodiscard]] double sky_visibility(Point point, const std::vector<Caster>& casters, std::int64_t ignore = 0,
                                    const std::vector<Caster>& bodies = {});
[[nodiscard]] bool overlaps(const Piece& a, const Piece& b);
/// Implements PRE-21, PRE-30: rays begin at actual receivers; union of logical proxies, no black overlay.
[[nodiscard]] bool blocked(Point from, Point to, const std::vector<Caster>& casters, std::int64_t ignore = 0);
[[nodiscard]] double sunlight(Point receiver, const Light& sun, const std::vector<Caster>& casters,
                              std::int64_t ignore = 0);
[[nodiscard]] double reach(double height, const Light& light);

/// Implements PRE-20, PRE-21, PRE-30, PLT-04: bounded CPU preparation, static revisions, separate moving bodies.
class Masks {
public:
    [[nodiscard]] Mask prepare(const Scene& scene, const Surface& surface, const Light& sun,
                               const std::vector<Caster>& bodies);
    void clear();
    [[nodiscard]] std::size_t bytes() const;
    [[nodiscard]] std::uint64_t builds() const { return builds_; }

private:
    struct Entry {
        std::uint64_t terrain = 0, sun = 0;
        std::int64_t surface = 0;
        Mask mask;
    };
    std::vector<Entry> cache_;
    std::uint64_t builds_ = 0;
};
}  // namespace kd::view::terrain
