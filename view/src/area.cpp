#include "area.hpp"

#include <algorithm>
#include <cmath>

#include "kd/chance/chance.hpp"
#include "kd/num/maths.hpp"

namespace kd::view::area {

namespace {

// The wandering's three waves, as shares of the wobble length: their lengths are the wobble length over these, and
// each carries the share of the whole beside it, so the sum stays within one either way.
constexpr std::array<double, 3> kWaveLengths{1.0, 2.3, 5.1};
constexpr std::array<double, 3> kWaveShares{0.55, 0.3, 0.15};
// The least a bank or the depth may be, as a share of its mean, so the river never closes or dries.
constexpr double kLeastShare = 0.1;
// The strip's columns: as fine as four rows' width within this reach of the centre, where the camp is, and then
// spreading by this ratio a column up to this width; the strip's rows beyond the banks spread by their own ratio up
// to their own width (A8.1: fine where it is seen).
constexpr double kFineReach = 128.0;
constexpr double kColumnGrowth = 1.2;
constexpr double kColumnLeast = 16.0;
constexpr double kRowGrowth = 1.6;
constexpr double kRowLeast = 8.0;
// How far apart the two heights are that a vertex's facing is worked out from, in metres.
constexpr double kFacingStep = 0.1;

// The positions from the middle outward, 0 first, for a spacing that is fine up to a reach and then grows by a ratio
// to a most, the last exactly at the edge.
std::vector<double> outward(double fine_step, double fine_reach, double growth, double most, double edge) {
    std::vector<double> out{0.0};
    double at = 0.0;
    while (at + fine_step <= fine_reach && at + fine_step < edge) {
        at += fine_step;
        out.push_back(at);
    }
    double step = fine_step;
    while (at < edge) {
        step = std::min(step * growth, most);
        at = std::min(at + step, edge);
        // what is left past this would be a sliver: take it now
        if (edge - at < 0.5 * step) {
            at = edge;
        }
        out.push_back(at);
    }
    return out;
}

// Both sides: from the far negative edge through 0 to the far positive.
std::vector<double> both_ways(const std::vector<double>& outward_places) {
    std::vector<double> out;
    for (std::size_t i = outward_places.size(); i-- > 1;) {
        out.push_back(-outward_places[i]);
    }
    for (const double at : outward_places) {
        out.push_back(at);
    }
    return out;
}

// Adds a vertex to a mesh.
void push(Mesh& m, double x, double y, double z, const std::array<double, 3>& normal, double bed) {
    m.positions.push_back(static_cast<float>(x));
    m.positions.push_back(static_cast<float>(y));
    m.positions.push_back(static_cast<float>(z));
    m.normals.push_back(static_cast<float>(normal[0]));
    m.normals.push_back(static_cast<float>(normal[1]));
    m.normals.push_back(static_cast<float>(normal[2]));
    m.beds.push_back(static_cast<float>(bed));
}

// The two triangles of every cell of a grid of vertices, columns across and rows down from the first vertex, each
// cell's corners clockwise seen from above: Godot's front faces (A4.7).
void cells(Mesh& m, std::size_t first_vertex, std::size_t across, std::size_t down) {
    for (std::size_t j = 0; j + 1 < down; ++j) {
        for (std::size_t i = 0; i + 1 < across; ++i) {
            const auto top_left = static_cast<std::uint32_t>(first_vertex + j * across + i);
            const auto top_right = top_left + 1;
            const auto bottom_left = static_cast<std::uint32_t>(first_vertex + (j + 1) * across + i);
            const auto bottom_right = bottom_left + 1;
            for (const std::uint32_t corner :
                 {top_left, top_right, bottom_right, top_left, bottom_right, bottom_left}) {
                m.indices.push_back(corner);
            }
        }
    }
}

// A flat rectangle of meadow facing up: its corners clockwise from the north-west.
void flat(Mesh& m, double west, double east, double north_z, double south_z) {
    const std::size_t first = m.vertices();
    const std::array<double, 3> up{0.0, 1.0, 0.0};
    push(m, west, 0.0, north_z, up, 0.0);
    push(m, east, 0.0, north_z, up, 0.0);
    push(m, west, 0.0, south_z, up, 0.0);
    push(m, east, 0.0, south_z, up, 0.0);
    cells(m, first, 2, 2);
}

}  // namespace

River::River(const Shape& shape) : shape_(shape) {
    static const chance::Name kSystem = chance::name("area");
    static const chance::Name kPurpose = chance::name("wandering");
    const chance::Draws draws(shape.seed, kSystem, 0, 0, kPurpose);
    for (std::size_t i = 0; i < phases_.size(); ++i) {
        phases_[i] = draws.fraction(i);
    }
}

double River::wobble(double east, std::size_t which) const {
    double sum = 0.0;
    for (std::size_t i = 0; i < kWaveLengths.size(); ++i) {
        const double turns = east * kWaveLengths[i] / shape_.wobble_length + phases_[which * 3 + i];
        sum += kWaveShares[i] * num::sinpi(2.0 * turns);
    }
    return sum;
}

Banks River::banks(double east) const {
    const double half = 0.5 * shape_.width;
    return {std::max(half * (1.0 + shape_.width_wobble * wobble(east, 0)), kLeastShare * half),
            std::max(half * (1.0 + shape_.width_wobble * wobble(east, 1)), kLeastShare * half)};
}

double River::deepest(double east) const {
    return std::max(shape_.depth * (1.0 + shape_.depth_wobble * wobble(east, 2)), kLeastShare * shape_.depth);
}

double River::height(double east, double north) const {
    const Banks b = banks(east);
    const double across = std::abs(north);
    const double half = north >= 0.0 ? b.north : b.south;
    if (across <= half) {
        // the bed: deepest on the middle line, rising in a curve to the water's edge
        const double t = across / half;
        return level() - deepest(east) * (1.0 - t * t);
    }
    // the bank: from the water's edge up to the meadow, in a smooth step
    const double u = std::min((across - half) / shape_.run, 1.0);
    return level() + shape_.bank * u * u * (3.0 - 2.0 * u);
}

double River::fine() const {
    return 0.5 * shape_.width * (1.0 + shape_.width_wobble) + shape_.run + 1.0;
}

std::vector<double> River::columns() const {
    return both_ways(outward(4.0 * shape_.spacing, kFineReach, kColumnGrowth, kColumnLeast, shape_.reach));
}

std::vector<double> River::rows() const {
    return both_ways(outward(shape_.spacing, fine(), kRowGrowth, kRowLeast, shape_.strip));
}

Mesh River::strip() const {
    Mesh m;
    const std::vector<double> xs = columns();
    // rows run south, so each row's z is minus its north
    const std::vector<double> norths = rows();
    for (std::size_t j = norths.size(); j-- > 0;) {
        const double north = norths[j];
        for (const double east : xs) {
            const double h = height(east, north);
            const double hx =
                (height(east + kFacingStep, north) - height(east - kFacingStep, north)) / (2.0 * kFacingStep);
            const double hn =
                (height(east, north + kFacingStep) - height(east, north - kFacingStep)) / (2.0 * kFacingStep);
            const double length = std::sqrt(hx * hx + 1.0 + hn * hn);
            push(m, east, h, -north, {-hx / length, 1.0 / length, hn / length}, h);
        }
    }
    cells(m, 0, xs.size(), norths.size());
    return m;
}

Mesh River::surface() const {
    Mesh m;
    const std::vector<double> xs = columns();
    const std::vector<double> norths = rows();
    // the rows the river can reach, and the first beyond them
    const double reach = fine();
    std::size_t first = 0;
    std::size_t last = norths.size() - 1;
    while (first + 1 < norths.size() && norths[first + 1] < -reach) {
        ++first;
    }
    while (last > first + 1 && norths[last - 1] > reach) {
        --last;
    }
    const std::array<double, 3> up{0.0, 1.0, 0.0};
    for (std::size_t j = last + 1; j-- > first;) {
        const double north = norths[j];
        for (const double east : xs) {
            push(m, east, level(), -north, up, height(east, north));
        }
    }
    cells(m, 0, xs.size(), last - first + 1);
    return m;
}

Mesh River::carpet() const {
    Mesh m;
    // north of the strip, then south of it: z runs south, so north is the more negative
    flat(m, -shape_.reach, shape_.reach, -shape_.reach, -shape_.strip);
    flat(m, -shape_.reach, shape_.reach, shape_.strip, shape_.reach);
    return m;
}

}  // namespace kd::view::area
