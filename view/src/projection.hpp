// T2.7a.1–2: the local 37-degree view. All lengths here are metres, all pixels are physical window pixels.
#pragma once

#include <cstdint>
#include <vector>

#include "crowd_core.hpp"
#include "stream.hpp"

namespace kd::view {
struct Pixel {
    double x = 0.0;
    double y = 0.0;
};

struct SourceStep {
    int density = 0;
    int level = -1;
    const char* form = "overview-fixture";
};

/// Unwrapped local metres about the exact wrapped drawing origin (PRE-33, A8.2).
struct Footprint {
    double west = 0.0;
    double south = 0.0;
    double east = 0.0;
    double north = 0.0;
};

/// Implements PRE-02, PRE-03, PRE-33: one projection for endpoints, bounds, camera and inverse picks.
class Projection {
public:
    static constexpr double kA = 0.6018150231520483;
    static constexpr double kB = 0.7986355100472928;
    void size(int width, int height, int pixel_scale = 0);
    void focus(double east, double north);
    num::Offset rebase();
    [[nodiscard]] num::Offset raster_origin() const;
    bool restore_raster_origin(num::Offset origin);
    [[nodiscard]] Projection at_origin(Pixel delta) const;
    void pan(double x, double y);
    void zoom(double ratio, Pixel anchor, bool snap);
    void release(Pixel anchor, double seconds);
    void advance(double seconds);
    void cancel_settle();
    bool configure(int minimum, int maximum, int tiny, int group);
    [[nodiscard]] bool settling() const { return settle_left_ > 0.0; }
    [[nodiscard]] double target_density() const { return settling() ? settle_target_ : raster_density_; }
    [[nodiscard]] Pixel project(double east, double north, double height = 0.0) const;
    [[nodiscard]] Pixel ground(Pixel pixel, double height = 0.0) const;
    [[nodiscard]] Pixel raster(double east, double north, double height = 0.0) const;
    [[nodiscard]] Pixel residual() const;
    [[nodiscard]] Pixel presentation_offset() const;
    [[nodiscard]] Pixel from_screen(Pixel pixel) const;
    [[nodiscard]] SourceStep source() const;
    [[nodiscard]] Footprint footprint(double height, double overscan, double shadow_east, double shadow_north) const;
    [[nodiscard]] Pixel centre() const { return {east_, north_}; }
    [[nodiscard]] double density() const { return density_; }
    [[nodiscard]] double resting_density() const;
    [[nodiscard]] double presentation() const { return density_ / resting_density(); }
    [[nodiscard]] int pixel_scale() const { return scale_; }
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;

private:
    double raster_origin_east_ = 0.0;
    double raster_origin_north_ = 0.0;
    int minimum_ = -12;
    int maximum_ = 6;
    int tiny_ = -2;
    int group_ = -6;
    double settle_left_ = 0.0;
    double settle_total_ = 0.0;
    double settle_start_ = 32.0;
    double settle_target_ = 32.0;
    Pixel settle_anchor_;
    int width_ = 1080;
    int height_ = 2400;
    int scale_ = 2;
    double east_ = 0.0;
    double north_ = 0.0;
    double density_ = 32.0;
    double raster_density_ = 32.0;
};

struct DrawRecord {
    std::uint64_t epoch = 0;
    std::uint64_t revision = 0;
    double second = 0.0;
    std::uint64_t id = 0;
    std::uint32_t appearance = 0;
    std::uint32_t surface = 0;  // the local flat fixture surface, until T2.8
    double east = 0.0;
    double north = 0.0;
    double phase = 0.0;
    std::uint8_t activity = 0;
    std::uint8_t facing = 0;
    std::uint32_t camp = 0;
};

struct CampRecord {
    std::uint64_t id = 0;
    num::Point place;
};

/// Implements WLD-13, TIM-17: the sole consumer owns a copy before any renderer reads it. Never retains a slot.
class DisplaySnapshot {
public:
    DisplaySnapshot();
    void acquire(CrowdStepper& stepper);
    [[nodiscard]] std::uint64_t epoch() const { return epoch_; }
    [[nodiscard]] std::uint64_t revision() const { return revision_; }
    [[nodiscard]] const Snapshot& snapshot() const { return snapshot_; }
    [[nodiscard]] RevisionManifest manifest(double second) const;
    [[nodiscard]] const std::vector<CampRecord>& camps() const { return camps_; }
    [[nodiscard]] std::vector<DrawRecord> sample(const num::Torus& torus, num::Point origin, double second) const;

private:
    Snapshot snapshot_;
    std::vector<CampRecord> camps_;
    std::uint64_t revision_ = 0;
    std::uint64_t epoch_ = 0;
};
}  // namespace kd::view
