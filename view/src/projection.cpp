#include "projection.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>

#include "kd/num/maths.hpp"

namespace kd::view {
void Projection::size(int width, int height) {
    width_ = std::max(1, width);
    height_ = std::max(1, height);
    // Reference windows use exactly 2x. Smaller windows use 1x; larger ones reveal more of the same world.
    scale_ = std::min(width_, height_) >= 1080 ? 2 : 1;
}
void Projection::focus(double east, double north) {
    east_ = east;
    north_ = north;
}
double Projection::resting_density() const {
    return raster_density_;
}
namespace {
double nearest_density(double density) {
    constexpr double steps[] = {2.0, 4.0, 8.0, 16.0, 32.0, 64.0};
    double best = steps[0];
    for (double step : steps) {
        if (density >= step / 1.4142135623730951) {
            best = step;
        }
    }
    return best;
}
}  // namespace
Pixel Projection::project(double east, double north, double height) const {
    const double s = density_ * scale_;
    return {width_ / 2.0 + s * (east - east_), height_ / 2.0 + s * (-kA * (north - north_) - kB * height)};
}
Pixel Projection::ground(Pixel pixel, double height) const {
    const double s = density_ * scale_;
    return {east_ + (pixel.x - width_ / 2.0) / s, north_ - ((pixel.y - height_ / 2.0) / s + kB * height) / kA};
}
void Projection::pan(double x, double y) {
    east_ -= x / (density_ * scale_);
    north_ += y / (density_ * scale_ * kA);
}
void Projection::zoom(double ratio, Pixel anchor, bool snap) {
    const Pixel before = ground(anchor);
    density_ = std::clamp(density_ * ratio, 2.0, 64.0);
    if (!snap) {
        // A live gesture covers the adjacent steps, bounding its temporary target to four times the resting area.
        // Releasing commits that step; another gesture continues through all six densities.
        density_ = std::clamp(density_, raster_density_ / 2.0, raster_density_ * 2.0);
    }
    if (snap) {
        density_ = nearest_density(density_);
        raster_density_ = density_;
    }
    const Pixel after = ground(anchor);
    east_ += before.x - after.x;
    north_ += before.y - after.y;
}
int Projection::width() const {
    return static_cast<int>(std::ceil(width_ / (scale_ * std::min(1.0, presentation())))) + 2;
}
int Projection::height() const {
    return static_cast<int>(std::ceil(height_ / (scale_ * std::min(1.0, presentation())))) + 2;
}
Pixel Projection::residual() const {
    const double s = resting_density();
    const double x = -s * east_;
    const double y = s * kA * north_;
    return {std::round((x - std::round(x)) * scale_), std::round((y - std::round(y)) * scale_)};
}
Pixel Projection::raster(double east, double north, double height) const {
    const double s = resting_density();
    // Camera and world are rounded separately: a fractional pan moves the complete image, never its pieces.
    return {std::round(s * east) - std::round(s * east_) + std::floor(static_cast<double>(width()) / 2.0),
            std::round(s * (-kA * north - kB * height)) - std::round(-s * kA * north_) +
                std::floor(static_cast<double>(this->height()) / 2.0)};
}
Pixel Projection::presentation_offset() const {
    const double ratio = presentation();
    const Pixel offset = residual();
    Pixel result{(width_ - width() * scale_ * ratio) / 2.0 + offset.x * ratio,
                 (height_ - height() * scale_ * ratio) / 2.0 + offset.y * ratio};
    if (ratio == 1.0) {
        result.x = std::round(result.x);
        result.y = std::round(result.y);
    }
    return result;
}
Pixel Projection::from_screen(Pixel pixel) const {
    const Pixel offset = presentation_offset();
    const double scale = scale_ * presentation();
    return {(pixel.x - offset.x) / scale, (pixel.y - offset.y) / scale};
}
DisplaySnapshot::DisplaySnapshot() {
    static std::atomic<std::uint64_t> next{0};
    epoch_ = next.fetch_add(1, std::memory_order_relaxed) + 1;
}
void DisplaySnapshot::acquire(CrowdStepper& stepper) {
    if (stepper.snapshots().take()) {
        snapshot_ = stepper.snapshots().front();
        ++revision_;
    }
}
std::vector<DrawRecord> DisplaySnapshot::sample(const num::Torus& torus, num::Point origin, double second) const {
    std::vector<DrawRecord> records;
    records.reserve(snapshot_.walkers.size());
    for (std::size_t i = 0; i < snapshot_.walkers.size(); ++i) {
        const auto& walker = snapshot_.walkers[i];
        const auto& way = snapshot_.way_at(i, second);
        double east = 0.0;
        double north = 0.0;
        place(torus, way, second, origin, east, north);
        const num::Offset direction = torus.offset(way.from, way.to);
        const double angle = direction.dx == 0 && direction.dy == 0
                                 ? 0.0
                                 : num::atan2pi(static_cast<double>(direction.dx), static_cast<double>(direction.dy));
        const auto facing = static_cast<std::uint8_t>((static_cast<int>(std::round(angle * 4.0)) + 8) % 8);
        const double elapsed = second - static_cast<double>(way.start);
        records.push_back({epoch_, revision_, second, walker.id, walker.kind, 0, east / 100.0, north / 100.0,
                           elapsed - std::floor(elapsed), way.what, facing});
    }
    return records;
}
}  // namespace kd::view
