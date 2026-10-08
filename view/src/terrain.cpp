#include "terrain.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include "kd/num/maths.hpp"

namespace kd::view::terrain {
namespace {
Point minus(Point a, Point b) {
    return {a.east - b.east, a.north - b.north, a.up - b.up};
}
double dot(Point a, Point b) {
    return a.east * b.east + a.north * b.north + a.up * b.up;
}
Point unit(Point p) {
    const double length = num::sqrt(dot(p, p));
    return {p.east / length, p.north / length, p.up / length};
}
Point blend(Point a, Point b, double t) {
    return {a.east + (b.east - a.east) * t, a.north + (b.north - a.north) * t, a.up + (b.up - a.up) * t};
}
Surface floor(std::int64_t id, double west, double south, double width, double height, double up, double slope = 0,
              Kind kind = Kind::floor) {
    Surface s;
    s.id = id;
    s.kind = kind;
    s.corners = {Point{west, south, up}, Point{west + width, south, up},
                 Point{west + width, south + height, up + slope * height},
                 Point{west, south + height, up + slope * height}};
    s.normal = normal(s);
    return s;
}
void face(Scene& s, std::int64_t id, double west, double north, double width, double bottom, double top,
          Kind kind = Kind::cliff) {
    Surface f;
    f.id = id;
    f.kind = kind;
    f.material = 3;
    f.corners = {Point{west, north, bottom}, Point{west + width, north, bottom}, Point{west + width, north, top},
                 Point{west, north, top}};
    f.normal = {0, -1, 0};
    s.surfaces.push_back(f);
}
bool hit(Point from, Point to, const Caster& c) {
    const Point direction = minus(to, from);
    if (c.round) {
        const Point centre = blend(c.bounds.low, c.bounds.high, 0.5);
        const Point half = minus(c.bounds.high, centre);
        const Point f{(from.east - centre.east) / half.east, (from.north - centre.north) / half.north,
                      (from.up - centre.up) / half.up};
        const Point d{direction.east / half.east, direction.north / half.north, direction.up / half.up};
        const double a = dot(d, d), b = dot(f, d), disc = b * b - a * (dot(f, f) - 1);
        if (disc < 0 || a == 0) return false;
        const double root = num::sqrt(disc), first = (-b - root) / a, last = (-b + root) / a;
        return last > 0.001 && first < 0.999;
    }
    double low = 0.001, high = 0.999;
    const std::array<double, 3> f{from.east, from.north, from.up}, d{direction.east, direction.north, direction.up};
    const std::array<double, 3> lo{c.bounds.low.east, c.bounds.low.north, c.bounds.low.up},
        hi{c.bounds.high.east, c.bounds.high.north, c.bounds.high.up};
    for (std::size_t axis = 0; axis < 3; ++axis) {
        if (std::abs(d[axis]) < 1e-12) {
            if (f[axis] < lo[axis] || f[axis] > hi[axis]) return false;
        } else {
            const double a = (lo[axis] - f[axis]) / d[axis], b = (hi[axis] - f[axis]) / d[axis];
            low = std::max(low, std::min(a, b));
            high = std::min(high, std::max(a, b));
            if (low > high) return false;
        }
    }
    return true;
}
std::uint8_t byte(double value) {
    return static_cast<std::uint8_t>(std::lround(std::clamp(value, 0.0, 1.0) * 255));
}
double contact(Point p, const std::vector<Caster>& casters, std::int64_t ignore = 0) {
    double strongest = 0;
    for (const auto& c : casters) {
        if (c.id == ignore || c.bounds.low.up > p.up + 0.05 || c.bounds.high.up < p.up) continue;
        const double dx = std::max({c.bounds.low.east - p.east, 0.0, p.east - c.bounds.high.east});
        const double dy = std::max({c.bounds.low.north - p.north, 0.0, p.north - c.bounds.high.north});
        if (dx >= 0.4 || dy >= 0.4) continue;
        const double distance = num::hypot(dx, dy);
        strongest = std::max(strongest, std::max(0.0, 1.0 - distance / 0.4));
    }
    return 1.0 - 0.18 * strongest;
}
bool inside(double value, double low, double high) {
    return value >= std::min(low, high) - 1e-8 && value <= std::max(low, high) + 1e-8;
}
}  // namespace
Point on(const Surface& s, double u, double v) {
    return blend(blend(s.corners[0], s.corners[1], u), blend(s.corners[3], s.corners[2], u), v);
}
Point normal(const Surface& s) {
    const Point a = minus(s.corners[1], s.corners[0]), b = minus(s.corners[3], s.corners[0]);
    return unit({a.north * b.up - a.up * b.north, a.up * b.east - a.east * b.up, a.east * b.north - a.north * b.east});
}
Scene fixture(const std::string& name) {
    Scene s;
    s.name = name;
    std::int64_t id = 100;
    for (int n = -40; n < 40; n += 8)
        for (int e = -24; e < 24; e += 16) {
            double h = 0, slope = 0;
            if (name == "slope") {
                h = (n + 24) * 0.10;
                slope = 0.10;
            }
            if (name == "cliff" && n >= 0) h = 3;
            if (name == "water") {
                h = -0.2 - 0.012 * (n + 24);
                slope = -0.012;
            }
            s.surfaces.push_back(floor(id++, e, n, 16, 8, h, slope));
        }
    s.casters = {{1, {{-5.2, -7.2, 0}, {-4.8, -6.8, 5}}, false},
                 {2, {{-6.8, -8.7, 3}, {-3.2, -5.3, 9}}, true},
                 {3, {{-4, -5, 0}, {-2, -3, 1.6}}, true},
                 {4, {{1, 3, 0}, {5, 5, 2.2}}, false}};
    s.actors = {{-5, 4, {0, 0, 0}, 0}, {-6, 5, {4, -5, 0}, 0}, {-7, 4, {-5, -6.5, 0}, 0}};
    if (name == "flat") {
        const std::array<Point, 3> axes{Point{1, 0, 0}, Point{0, 1, 0}, Point{0, 0, 1}};
        for (std::size_t axis = 0; axis < axes.size(); ++axis) {
            auto panel = floor(900 + static_cast<std::int64_t>(axis), 3.0 + 2.0 * static_cast<double>(axis), -14, 1.5,
                               2, 0.01, 0, Kind::calibration);
            panel.material = 0;
            panel.normal = axes[axis];
            s.surfaces.push_back(panel);
        }
    }
    if (name == "cliff") {
        for (int e = -24; e < 24; e += 16) {
            face(s, id++, e, 0, 16, 0, 3);
            s.casters.push_back(
                {id - 1, {{static_cast<double>(e), 0, 0}, {static_cast<double>(e + 16), 40, 3}}, false});
        }
        // A low foreground crown overlaps the rear shelf; chunk borders never partition the order graph.
        s.actors[0].foot = {-5, 1, 3};
        s.actors[2].foot = {-5.6, 1.1, 3};
        s.casters[0].bounds = {{-5.2, -2.2, 0}, {-4.8, -1.8, 5}};
        s.casters[1].bounds = {{-6.8, -3.7, 3}, {-3.2, -0.3, 9}};
        auto front = floor(200, -1, -6, 5, 3, 1.5);
        s.surfaces.push_back(front);
        face(s, 201, -1, -6, 5, 0, 1.5);
    }
    if (name == "shelter" || name == "cave") {
        s.casters.erase(
            std::remove_if(s.casters.begin(), s.casters.end(), [](const auto& caster) { return caster.id == 4; }),
            s.casters.end());
        s.surfaces.push_back(floor(20, 1, 1, 4, 4, 0));
        auto roof = floor(21, 1, 1, 4, 4, 3, 0, Kind::roof);
        roof.covers = 20;
        roof.material = 2;
        s.surfaces.push_back(roof);
        face(s, 22, 1, 5, 4, 0, 3, Kind::wall);
        face(s, 23, 1, 1, 1, 0, 2.8, Kind::wall);
        face(s, 24, 4, 1, 1, 0, 2.8, Kind::wall);
        Surface side;
        side.id = 25;
        side.kind = Kind::wall;
        side.material = 2;
        side.corners = {Point{1, 1, 0}, Point{1, 5, 0}, Point{1, 5, 3}, Point{1, 1, 3}};
        side.normal = {-1, 0, 0};
        s.surfaces.push_back(side);
        s.casters.push_back({21, {{1, 1, 3}, {5, 5, 3.2}}, false});
        s.casters.push_back({22, {{1, 4.9, 0}, {5, 5.1, 3}}, false});
        s.casters.push_back({25, {{0.9, 1, 0}, {1.1, 5, 3}}, false});
        s.casters.push_back({26, {{4.9, 1, 0}, {5.1, 5, 3}}, false});
        s.casters.push_back({23, {{1, 0.9, 0}, {2, 1.1, 2.8}}, false});
        s.casters.push_back({24, {{4, 0.9, 0}, {5, 1.1, 2.8}}, false});
        s.actors[0].foot = {3, 3, 0};
        s.actors[2].foot = {2, 3.5, 0};
        if (name == "cave") {  // Explicit interior scene, not stacked overhangs or PRE-25's geological slice.
            s.surfaces.erase(std::remove_if(s.surfaces.begin(), s.surfaces.end(),
                                            [](const Surface& v) { return v.id >= 100 || v.kind == Kind::roof; }),
                             s.surfaces.end());
            s.actors[1].foot = {4, 3, 0};
        }
    }
    if (name == "water") {
        auto water = floor(30, -8, -12, 16, 24, 0, 0, Kind::water);
        water.material = 8;
        s.surfaces.push_back(water);
        s.actors[0].foot = {0, -6, 0};
        auto bank = floor(31, 8, -12, 8, 24, 0.2);
        s.surfaces.push_back(bank);
    }
    for (auto& a : s.actors) {
        const auto p = walk(s, a.foot.east, a.foot.north);
        if (p.found) {
            a.foot = p.point;
            a.surface = p.surface;
        }
    }
    if (name == "slope" || name == "water") {
        for (auto& c : s.casters) {
            const auto centre = blend(c.bounds.low, c.bounds.high, 0.5);
            const auto base = walk(s, centre.east, centre.north);
            if (base.found) {
                c.bounds.low.up += base.point.up;
                c.bounds.high.up += base.point.up;
            }
        }
    }
    return s;
}
Light light(const std::string& hour, const std::string& weather, int direction, bool fire) {
    Light l;
    l.hour = hour;
    l.weather = weather;
    l.fire_on = fire;
    const double elevation = hour == "dusk" ? 1.0 / 12.0 : hour == "night" ? 1.0 / 6.0 : 0.36;
    const double angle = static_cast<double>((direction % 4 + 4) % 4) / 2.0;
    const double horizontal = num::cospi(elevation), up = num::sinpi(elevation);
    l.sun = {horizontal * num::cospi(angle), horizontal * num::sinpi(angle), up};
    if (hour == "dusk") {
        l.sunlight = {0.8, 0.40, 0.18};
        l.sky = {0.25, 0.28, 0.38};
    }
    if (hour == "night") {
        l.sunlight = {0.025, 0.03, 0.05};
        l.sky = {0.07, 0.10, 0.19};
    }
    if (weather == "rain") {
        l.sunlight = {0.15, 0.16, 0.18};
        l.sky = {0.30, 0.33, 0.36};
    }
    if (weather == "winter") {
        l.sunlight = {0.78, 0.73, 0.62};
        l.sky = {0.34, 0.42, 0.52};
    }
    return l;
}
Pick walk(const Scene& s, double east, double north) {
    Pick result;
    double top = -std::numeric_limits<double>::max();
    for (const auto& f : s.surfaces) {
        if (f.kind != Kind::floor) continue;
        const Point a = f.corners[0], b = f.corners[2];
        if (!inside(east, a.east, b.east) || !inside(north, a.north, b.north)) continue;
        const Point p = on(f, (east - a.east) / (b.east - a.east), (north - a.north) / (b.north - a.north));
        if (p.up > top || (p.up == top && f.id < result.surface)) {
            result = {f.id, p, true};
            top = p.up;
        }
    }
    return result;
}
bool overlaps(const Piece& a, const Piece& b) {
    return a.low.x < b.high.x && b.low.x < a.high.x && a.low.y < b.high.y && b.low.y < a.high.y;
}
std::vector<std::int64_t> order(const std::vector<Piece>& pieces) {
    const std::size_t n = pieces.size();
    std::vector<std::vector<std::size_t>> next(n);
    std::vector<int> degree(n, 0);
    for (std::size_t a = 0; a < n; ++a)
        for (std::size_t b = a + 1; b < n; ++b) {
            if (!overlaps(pieces[a], pieces[b])) continue;
            const auto& p = pieces[a];
            const auto& q = pieces[b];
            bool before;
            const bool p_water_cover = p.water && q.receiver && p.minimum_height >= q.maximum_height;
            const bool q_water_cover = q.water && p.receiver && q.minimum_height >= p.maximum_height;
            const bool p_roof_cover = p.covers != 0 && p.covers == q.surface && !q.receiver;
            const bool q_roof_cover = q.covers != 0 && q.covers == p.surface && !p.receiver;
            if (p_water_cover || q_water_cover)
                before = !p_water_cover;
            else if ((p.water && !q.receiver) || (q.water && !p.receiver))
                before = p.water;
            else if (p_roof_cover || q_roof_cover)
                before = !p_roof_cover;
            else if ((p.receiver && p.id == q.surface) || (q.receiver && q.id == p.surface))
                before = p.receiver && p.id == q.surface;
            else
                before = p.depth < q.depth || (p.depth == q.depth && p.id < q.id);
            const std::size_t first = before ? a : b, last = before ? b : a;
            next[first].push_back(last);
            ++degree[last];
        }
    std::vector<std::int64_t> result;
    std::vector<bool> used(n, false);
    while (result.size() < n) {
        std::size_t best = n;
        for (std::size_t i = 0; i < n; ++i)
            if (!used[i] && degree[i] == 0 &&
                (best == n || pieces[i].depth < pieces[best].depth ||
                 (pieces[i].depth == pieces[best].depth && pieces[i].id < pieces[best].id)))
                best = i;
        if (best == n) return {};  // The restricted piece split failed. Caller must report it, never hide a cycle.
        result.push_back(pieces[best].id);
        used[best] = true;
        for (auto after : next[best]) --degree[after];
    }
    return result;
}
Pick pick(const Scene& s, const Projection& projection, Pixel raster, bool cutaway) {
    Pick result;
    double closest = -std::numeric_limits<double>::max();
    // Solve the projected quad basis rather than applying a flat inverse to a raised surface.
    for (const auto& f : s.surfaces) {
        if (cutaway && f.kind == Kind::roof) continue;
        const auto a = projection.raster(f.corners[0].east, f.corners[0].north, f.corners[0].up);
        const auto b = projection.raster(f.corners[1].east, f.corners[1].north, f.corners[1].up);
        const auto c = projection.raster(f.corners[3].east, f.corners[3].north, f.corners[3].up);
        const double bx = b.x - a.x, by = b.y - a.y, cx = c.x - a.x, cy = c.y - a.y, det = bx * cy - by * cx;
        if (std::abs(det) < 1e-8) continue;
        const double u = ((raster.x - a.x) * cy - (raster.y - a.y) * cx) / det,
                     v = (bx * (raster.y - a.y) - by * (raster.x - a.x)) / det;
        if (!inside(u, 0, 1) || !inside(v, 0, 1)) continue;
        const Point p = on(f, u, v);
        const double depth = -Projection::kB * p.north + Projection::kA * p.up;
        if (depth > closest || (depth == closest && f.id < result.surface)) {
            result = {f.id, p, true};
            closest = depth;
        }
    }
    return result;
}
bool blocked(Point from, Point to, const std::vector<Caster>& casters, std::int64_t ignore) {
    for (const auto& c : casters)
        if (c.id != ignore && hit(from, to, c)) return true;
    return false;
}
double reach(double height, const Light& l) {
    return height * num::hypot(l.sun.east, l.sun.north) / std::max(0.05, l.sun.up);
}
namespace {
std::uint8_t sun_bits(Point p, const Light& l, const std::vector<Caster>& casters, std::int64_t ignore) {
    // Five bounded angular rays give increasing penumbra with distance. The union is sampled once per ray.
    std::uint8_t visible = 0;
    int bit = 0;
    p.up += 0.035;
    constexpr std::array<Pixel, 5> offsets{Pixel{0, 0}, Pixel{0.012, 0}, Pixel{-0.012, 0}, Pixel{0, 0.012},
                                           Pixel{0, -0.012}};
    for (const auto o : offsets) {
        const Point end{p.east + (l.sun.east + o.x) * 160, p.north + (l.sun.north + o.y) * 160, p.up + l.sun.up * 160};
        if (!blocked(p, end, casters, ignore)) visible |= static_cast<std::uint8_t>(1U << bit);
        ++bit;
    }
    return visible;
}
}  // namespace
double sunlight(Point p, const Light& l, const std::vector<Caster>& casters, std::int64_t ignore) {
    return std::popcount(sun_bits(p, l, casters, ignore)) / 5.0;
}
Mask Masks::prepare(const Scene& scene, const Surface& s, const Light& sun, const std::vector<Caster>& bodies) {
    auto it = std::find_if(cache_.begin(), cache_.end(), [&](const Entry& e) {
        return e.terrain == scene.revision && e.sun == sun.revision && e.surface == s.id;
    });
    if (it == cache_.end()) {
        Mask m;
        const auto a = minus(s.corners[1], s.corners[0]), b = minus(s.corners[3], s.corners[0]);
        m.width = std::clamp(static_cast<int>(std::ceil(num::sqrt(dot(a, a)) / 0.5)) + 1, 2, 65);
        m.height = std::clamp(static_cast<int>(std::ceil(num::sqrt(dot(b, b)) / 0.5)) + 1, 2, 65);
        m.rgba.resize(static_cast<std::size_t>(m.width) * m.height * 4);
        m.sun_bits.resize(static_cast<std::size_t>(m.width) * m.height);
        for (int y = 0; y < m.height; ++y)
            for (int x = 0; x < m.width; ++x) {
                Point p = on(s, x / static_cast<double>(m.width - 1), y / static_cast<double>(m.height - 1));
                const std::size_t at = (static_cast<std::size_t>(y) * m.width + x) * 4;
                m.sun_bits[at / 4] = sun_bits(p, sun, scene.casters, s.id);
                m.rgba[at] = byte(std::popcount(m.sun_bits[at / 4]) / 5.0);
                Point above = p;
                above.up += 0.04;
                Point end = p;
                end.up += 100;
                m.rgba[at + 1] = blocked(above, end, scene.casters, s.id) ? 100 : 255;
                m.rgba[at + 2] = byte(contact(p, scene.casters, s.id));
                const Point fire_receiver{p.east + s.normal.east * 0.12, p.north + s.normal.north * 0.12,
                                          p.up + s.normal.up * 0.12};
                m.rgba[at + 3] = blocked(fire_receiver, sun.fire, scene.casters) ? 0 : 255;
            }
        if (cache_.size() == 64) cache_.erase(cache_.begin());
        cache_.push_back({scene.revision, sun.revision, s.id, std::move(m)});
        it = cache_.end() - 1;
        ++builds_;
    }
    Mask result = it->mask;

    if (!bodies.empty())
        for (int y = 0; y < result.height; ++y)
            for (int x = 0; x < result.width; ++x) {
                const auto p =
                    on(s, x / static_cast<double>(result.width - 1), y / static_cast<double>(result.height - 1));
                const std::size_t at = (static_cast<std::size_t>(y) * result.width + x) * 4;
                bool candidate = false;
                for (const auto& body : bodies) {
                    const auto& bounds = body.bounds;
                    if (p.up >= bounds.high.up) continue;
                    const double span = (bounds.high.up - p.up) / sun.sun.up;
                    const double shift_e = -sun.sun.east * span, shift_n = -sun.sun.north * span;
                    const double soft = 0.03 * span + 0.4;
                    candidate = candidate || (p.east >= bounds.low.east + std::min(0.0, shift_e) - soft &&
                                              p.east <= bounds.high.east + std::max(0.0, shift_e) + soft &&
                                              p.north >= bounds.low.north + std::min(0.0, shift_n) - soft &&
                                              p.north <= bounds.high.north + std::max(0.0, shift_n) + soft);
                }
                if (candidate) {
                    const auto visible =
                        static_cast<std::uint8_t>(result.sun_bits[at / 4] & sun_bits(p, sun, bodies, s.id));
                    result.rgba[at] = byte(std::popcount(visible) / 5.0);
                }
                result.rgba[at + 2] = std::min(result.rgba[at + 2], byte(contact(p, bodies)));
            }
    return result;
}
void Masks::clear() {
    cache_.clear();
}
std::size_t Masks::bytes() const {
    std::size_t bytes = 0;
    for (const auto& e : cache_) bytes += e.mask.rgba.size() + e.mask.sun_bits.size();
    return bytes;
}
}  // namespace kd::view::terrain
