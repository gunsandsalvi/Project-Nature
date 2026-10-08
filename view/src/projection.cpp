#include "projection.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>

#include "kd/num/maths.hpp"

namespace kd::view {
void Projection::size(int width, int height, int pixel_scale) {
    width_ = std::max(1, width);
    height_ = std::max(1, height);
    // Reference windows use exactly 2x. Smaller windows use 1x; larger ones reveal more of the same world.
    scale_ = pixel_scale == 1 || pixel_scale == 2 ? pixel_scale : std::min(width_, height_) >= 1080 ? 2 : 1;
}
num::Offset Projection::raster_origin() const {
    return {std::llround(raster_origin_east_ * 100), std::llround(raster_origin_north_ * 100)};
}
bool Projection::restore_raster_origin(num::Offset origin) {
    if (origin.dx < -100000000000000LL || origin.dx > 100000000000000LL || origin.dy < -100000000000000LL ||
        origin.dy > 100000000000000LL || origin.dx % 409600 != 0 || origin.dy % 409600 != 0)
        return false;
    raster_origin_east_ = static_cast<double>(origin.dx) / 100.;
    raster_origin_north_ = static_cast<double>(origin.dy) / 100.;
    return true;
}
Projection Projection::at_origin(Pixel delta) const {
    Projection translated = *this;
    translated.east_ += delta.x;
    translated.north_ += delta.y;
    translated.raster_origin_east_ -= delta.x;
    translated.raster_origin_north_ -= delta.y;
    return translated;
}
num::Offset Projection::rebase() {
    const auto axis = [](double metres) -> std::int64_t {
        if (!std::isfinite(metres) || std::abs(metres) < 16384.0 || std::abs(metres) > 1e12) return 0;
        return static_cast<std::int64_t>(std::floor(metres / 4096.0)) * 409600;
    };
    const num::Offset shift{axis(east_), axis(north_)};
    raster_origin_east_ += static_cast<double>(shift.dx) / 100.0;
    raster_origin_north_ += static_cast<double>(shift.dy) / 100.0;
    east_ -= static_cast<double>(shift.dx) / 100.0;
    north_ -= static_cast<double>(shift.dy) / 100.0;
    return shift;
}
void Projection::focus(double east, double north) {
    cancel_settle();
    east_ = east;
    north_ = north;
}
double Projection::resting_density() const {
    return raster_density_;
}
SourceStep Projection::source() const {
    const double density = resting_density();
    if (density >= 2.0) {
        const int family = density >= 32.0 ? 64 : density >= 8.0 ? 16 : 4;
        return {family, density == family ? 0 : 1, "individual"};
    }
    return {0, -1,
            density >= std::ldexp(1.0, tiny_)    ? "tiny"
            : density >= std::ldexp(1.0, group_) ? "group"
                                                 : "overview-fixture"};
}
Footprint Projection::footprint(double height, double overscan, double shadow_east, double shadow_north) const {
    height = std::max(0.0, height);
    overscan = std::max(0.0, overscan);
    const Pixel top = ground({-overscan, -overscan});
    const Pixel bottom = ground({width_ + overscan, height_ + overscan}, height);
    // A caster lies opposite the direction its shadow reaches from it. Include elevated occluders too.
    return {top.x - std::max(0.0, shadow_east), bottom.y - std::max(0.0, shadow_north),
            bottom.x - std::min(0.0, shadow_east), top.y - std::min(0.0, shadow_north)};
}
namespace {
double nearest_density(double density, int minimum, int maximum) {
    double best = std::ldexp(1.0, minimum);
    for (int power = minimum; power <= maximum; ++power) {
        const double step = std::ldexp(1.0, power);
        if (density >= step / 1.4142135623730951) {
            best = step;
        }
    }
    return best;
}
}  // namespace
// T2.9a.1: gesture release settles without changing the decided resting grids.
void Projection::release(Pixel anchor, double seconds) {
    cancel_settle();
    if (!std::isfinite(seconds) || seconds <= 0.0) return;
    settle_target_ = nearest_density(density_, minimum_, maximum_);
    if (density_ == settle_target_) {
        raster_density_ = density_;
        return;
    }
    settle_anchor_ = anchor;
    settle_start_ = density_;
    settle_total_ = seconds;
    settle_left_ = seconds;
}
void Projection::advance(double seconds) {
    if (!settling() || !std::isfinite(seconds) || seconds <= 0.0) return;
    const Pixel before = ground(settle_anchor_);
    settle_left_ = std::max(0.0, settle_left_ - seconds);
    if (settle_left_ <= settle_total_ * 1e-12) settle_left_ = 0.0;
    const double t = 1.0 - settle_left_ / settle_total_;
    const double smooth = t * t * (3.0 - 2.0 * t);
    density_ = num::exp(num::log(settle_start_) * (1.0 - smooth) + num::log(settle_target_) * smooth);
    if (!settling()) {
        density_ = settle_target_;
        raster_density_ = density_;
    }
    const Pixel after = ground(settle_anchor_);
    east_ += before.x - after.x;
    north_ += before.y - after.y;
}
void Projection::cancel_settle() {
    settle_left_ = 0.0;
}
bool Projection::configure(int minimum, int maximum, int tiny, int group) {
    if (minimum < -20 || minimum > -8 || maximum != 6 || group < minimum || tiny <= group || tiny > 0) return false;
    minimum_ = minimum;
    maximum_ = maximum;
    tiny_ = tiny;
    group_ = group;
    zoom(1.0, {width_ / 2.0, height_ / 2.0}, true);
    return true;
}
Pixel Projection::project(double east, double north, double height) const {
    const double s = density_ * scale_;
    return {width_ / 2.0 + s * (east - east_), height_ / 2.0 + s * (-kA * (north - north_) - kB * height)};
}
Pixel Projection::ground(Pixel pixel, double height) const {
    const double s = density_ * scale_;
    return {east_ + (pixel.x - width_ / 2.0) / s, north_ - ((pixel.y - height_ / 2.0) / s + kB * height) / kA};
}
void Projection::pan(double x, double y) {
    if (x != 0.0 || y != 0.0) cancel_settle();
    east_ -= x / (density_ * scale_);
    north_ += y / (density_ * scale_ * kA);
}
void Projection::zoom(double ratio, Pixel anchor, bool snap) {
    if (!std::isfinite(ratio) || ratio <= 0.0) {
        return;
    }
    cancel_settle();
    const Pixel before = ground(anchor);
    // T2.9a.1, PRE-03/PRE-28: include every intermediate stop through the whole torus.
    density_ = std::clamp(density_ * ratio, std::ldexp(1.0, minimum_), std::ldexp(1.0, maximum_));
    if (!snap) {
        // A live gesture covers the adjacent steps, bounding its temporary target to four times the resting area.
        // Releasing commits that step; another gesture continues through the complete navigation range.
        density_ = std::clamp(density_, raster_density_ / 2.0, raster_density_ * 2.0);
    }
    if (snap) {
        density_ = nearest_density(density_, minimum_, maximum_);
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
    const double x = -s * (east_ + raster_origin_east_);
    const double y = s * kA * (north_ + raster_origin_north_);
    return {std::round((x - std::round(x)) * scale_), std::round((y - std::round(y)) * scale_)};
}
Pixel Projection::raster(double east, double north, double height) const {
    const double s = resting_density();
    // Camera and world are rounded separately: a fractional pan moves the complete image, never its pieces.
    return {std::round(s * (east + raster_origin_east_)) - std::round(s * (east_ + raster_origin_east_)) +
                std::floor(static_cast<double>(width()) / 2.0),
            std::round(s * (-kA * (north + raster_origin_north_) - kB * height)) -
                std::round(-s * kA * (north_ + raster_origin_north_)) +
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
        camps_.clear();
        for (std::size_t i = 0; i < stepper.camps().size(); ++i)
            camps_.push_back({stepper.camp_ids()[i].value, stepper.camps()[i]});
    }
}
RevisionManifest DisplaySnapshot::manifest(double second) const {
    RevisionManifest result{epoch_, revision_, second, {}};
    // The only current physical surface is the flat fixture ground; its identity survives packet skips.
    result.records["surface"]["0"] = 1;
    auto& casters = result.records["caster"];
    auto& appearances = result.records["appearance"];
    for (const auto& walker : snapshot_.walkers) {
        casters[std::to_string(walker.id)] = revision_;
        appearances[std::to_string(walker.kind)] = 1;
    }
    return result;
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
                           elapsed - std::floor(elapsed), way.what, facing, walker.camp});
    }
    return records;
}
}  // namespace kd::view
