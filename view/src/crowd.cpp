#include "crowd.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "kd/demo/crowd_world.hpp"
#include "kd/demo/marker.hpp"
#include "trace.hpp"

namespace kd::view {

namespace {

// How the screen shows what a walker is doing: a greeting flashes toward white, twice a real second, and larger;
// sleep is dimmed.
constexpr double kFlashes = 2.0;
constexpr float kGreetLight = 0.35F;
constexpr float kFlashLight = 0.5F;
constexpr float kGreetScale = 1.6F;
constexpr float kSleepDim = 0.4F;
// Heights above the ground, in metres, walkers over camps, and a camp's size in walkers'.
constexpr float kWalkerHeight = 1.0F;
constexpr float kCampHeight = 0.5F;
constexpr float kCampScale = 3.0F;

// Room for at least n instances: a power of two, at least 64, so a buffer is seldom made again.
int32_t room_for(int32_t n) {
    int32_t room = 64;
    while (room < n) {
        room *= 2;
    }
    return room;
}

godot::RenderingServer& server() {
    return *godot::RenderingServer::get_singleton();
}

}  // namespace

void KdCrowd::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("set_world", "world"), &KdCrowd::set_world);
    ClassDB::bind_method(D_METHOD("set_areas", "areas", "across"), &KdCrowd::set_areas);
    ClassDB::bind_method(D_METHOD("set_camps", "camps"), &KdCrowd::set_camps);
    ClassDB::bind_method(D_METHOD("draw", "t", "origin_east", "origin_north", "size"), &KdCrowd::draw);
    ClassDB::bind_method(D_METHOD("draw_times"), &KdCrowd::draw_times);
    ClassDB::bind_method(D_METHOD("draw_times_reset"), &KdCrowd::draw_times_reset);
    ClassDB::bind_method(D_METHOD("area_counts"), &KdCrowd::area_counts);
}

void KdCrowd::set_world(const godot::Ref<KdWorld>& world) {
    world_ = world;
    kind_colours_.clear();
    if (world_.is_null() || world_->catalogue() == nullptr) {
        return;
    }
    const data::Kind<demo::Marker>& kinds = world_->catalogue()->kind<demo::Marker>();
    for (std::size_t k = 0; k < kinds.size(); ++k) {
        const godot::Color c =
            godot::Color::html(godot::String::utf8(kinds[static_cast<std::uint32_t>(k)].colour.c_str()));
        kind_colours_.push_back({c.r, c.g, c.b, 1.0F});
    }
}

void KdCrowd::set_areas(const godot::Array& areas, int64_t across) {
    KD_CHECK(across >= 1 && areas.size() == across * across, "view::KdCrowd: the areas are across by across");
    areas_.clear();
    for (int64_t i = 0; i < areas.size(); ++i) {
        areas_.push_back(areas[i]);
    }
    across_ = across;
    buffers_.assign(areas_.size(), godot::PackedFloat32Array());
    capacities_.assign(areas_.size(), 0);
    counts_.assign(areas_.size(), 0);
}

void KdCrowd::set_camps(const godot::RID& camps) {
    camps_ = camps;
    camps_ready_ = false;
}

void KdCrowd::write(godot::PackedFloat32Array& buffer, int64_t at, float x, float y, float z, float scale,
                    const std::array<float, 4>& colour) const {
    float* f = buffer.ptrw() + at * kStride;
    // a transform's rows, each its basis then its origin, then the colour
    f[0] = scale;
    f[1] = 0.0F;
    f[2] = 0.0F;
    f[3] = x;
    f[4] = 0.0F;
    f[5] = scale;
    f[6] = 0.0F;
    f[7] = y;
    f[8] = 0.0F;
    f[9] = 0.0F;
    f[10] = scale;
    f[11] = z;
    f[12] = colour[0];
    f[13] = colour[1];
    f[14] = colour[2];
    f[15] = colour[3];
}

int64_t KdCrowd::draw(double t, int64_t origin_east, int64_t origin_north, double size) {
    const TraceSection section("kd draw");
    const auto started = std::chrono::steady_clock::now();
    const int64_t drawn = draw_now(t, origin_east, origin_north, size);
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    ++draws_;
    draw_ms_ += ms;
    longest_draw_ms_ = std::max(longest_draw_ms_, ms);
    return drawn;
}

godot::Dictionary KdCrowd::draw_times() const {
    godot::Dictionary out;
    out["draws"] = draws_;
    out["mean_ms"] = draws_ > 0 ? draw_ms_ / static_cast<double>(draws_) : 0.0;
    out["most_ms"] = longest_draw_ms_;
    return out;
}

void KdCrowd::draw_times_reset() {
    draws_ = 0;
    draw_ms_ = 0.0;
    longest_draw_ms_ = 0.0;
}

int64_t KdCrowd::draw_now(double t, int64_t origin_east, int64_t origin_north, double size) {
    if (world_.is_null() || world_->stepper() == nullptr || areas_.empty()) {
        return 0;
    }
    CrowdStepper& stepper = *world_->stepper();
    stepper.snapshots().take();
    const Snapshot& s = stepper.snapshots().front();
    const num::Torus& torus = world::World::kTorus;
    const demo::Square square = world_->crowd()->square();
    const num::Point origin = torus.wrap(origin_east, origin_north);
    // from the origin to the square's corner, once; each walker is placed from the corner, so its area is plain
    const num::Offset corner = torus.offset(origin, square.south_west);
    const double area_side = static_cast<double>(square.side) / static_cast<double>(across_);
    const auto scale = static_cast<float>(size);
    const double real = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
    // a triangle wave from 0 to 1 and back, with no platform maths (A3.4)
    const double phase = real * kFlashes - std::floor(real * kFlashes);
    const auto flash = static_cast<float>(1.0 - std::abs(2.0 * phase - 1.0));
    const float greet_light = kGreetLight + kFlashLight * flash;

    // where each walker is, in which area, in what colour
    placed_.resize(s.walkers.size());
    std::fill(counts_.begin(), counts_.end(), 0);
    for (std::size_t i = 0; i < s.walkers.size(); ++i) {
        const world::Activity& way = s.way_at(i, t);
        double east = 0.0;
        double north = 0.0;
        place(torus, way, t, square.south_west, east, north);
        const auto ax = static_cast<int64_t>(std::clamp(std::floor(east / area_side), 0.0, double(across_ - 1)));
        const auto ay = static_cast<int64_t>(std::clamp(std::floor(north / area_side), 0.0, double(across_ - 1)));
        Placed& p = placed_[i];
        p.x = static_cast<float>((static_cast<double>(corner.dx) + east) / 100.0);
        p.z = static_cast<float>(-(static_cast<double>(corner.dy) + north) / 100.0);
        p.area = static_cast<std::uint32_t>(ay * across_ + ax);
        const std::uint32_t kind = s.walkers[i].kind;
        p.colour = kind < kind_colours_.size() ? kind_colours_[kind] : std::array<float, 4>{1.0F, 1.0F, 1.0F, 1.0F};
        p.scale = scale;
        if (way.what == static_cast<std::uint8_t>(demo::Doing::greet)) {
            for (int c = 0; c < 3; ++c) {
                p.colour[c] += (1.0F - p.colour[c]) * greet_light;
            }
            p.scale = scale * kGreetScale;
        } else if (way.what == static_cast<std::uint8_t>(demo::Doing::sleep)) {
            for (int c = 0; c < 3; ++c) {
                p.colour[c] *= kSleepDim;
            }
        }
        ++counts_[p.area];
    }

    // each area's buffer, made larger when it must be, and its bounding box from what is in it
    std::vector<int32_t> next(areas_.size(), 0);
    for (std::size_t a = 0; a < areas_.size(); ++a) {
        if (counts_[a] > capacities_[a]) {
            capacities_[a] = room_for(counts_[a]);
            server().multimesh_allocate_data(areas_[a], capacities_[a], godot::RenderingServer::MULTIMESH_TRANSFORM_3D,
                                             true);
            buffers_[a].resize(static_cast<int64_t>(capacities_[a]) * kStride);
            buffers_[a].fill(0.0);
        }
    }
    std::vector<godot::Vector3> low(areas_.size(), godot::Vector3(1.0e30F, 0.0F, 1.0e30F));
    std::vector<godot::Vector3> high(areas_.size(), godot::Vector3(-1.0e30F, 0.0F, -1.0e30F));
    for (const Placed& p : placed_) {
        write(buffers_[p.area], next[p.area]++, p.x, kWalkerHeight, p.z, p.scale, p.colour);
        low[p.area].x = std::min(low[p.area].x, p.x);
        low[p.area].z = std::min(low[p.area].z, p.z);
        high[p.area].x = std::max(high[p.area].x, p.x);
        high[p.area].z = std::max(high[p.area].z, p.z);
    }
    for (std::size_t a = 0; a < areas_.size(); ++a) {
        if (capacities_[a] == 0) {
            continue;
        }
        server().multimesh_set_buffer(areas_[a], buffers_[a]);
        server().multimesh_set_visible_instances(areas_[a], counts_[a]);
        if (counts_[a] > 0) {
            const float r = scale * kGreetScale;
            const godot::Vector3 from(low[a].x - r, 0.0F, low[a].z - r);
            const godot::Vector3 to(high[a].x + r, kWalkerHeight + r, high[a].z + r);
            server().multimesh_set_custom_aabb(areas_[a], godot::AABB(from, to - from));
        }
    }

    // the camps, each in its own colour, which never move but follow the origin
    if (camps_.is_valid()) {
        const std::vector<num::Point>& camps = stepper.camps();
        const auto n = static_cast<int32_t>(camps.size());
        if (!camps_ready_) {
            server().multimesh_allocate_data(camps_, n, godot::RenderingServer::MULTIMESH_TRANSFORM_3D, true);
            camp_buffer_.resize(static_cast<int64_t>(n) * kStride);
            camps_ready_ = true;
        }
        float x0 = 1.0e30F;
        float z0 = 1.0e30F;
        float x1 = -1.0e30F;
        float z1 = -1.0e30F;
        for (int32_t c = 0; c < n; ++c) {
            const num::Offset o = torus.offset(origin, camps[static_cast<std::size_t>(c)]);
            const auto x = static_cast<float>(static_cast<double>(o.dx) / 100.0);
            const auto z = static_cast<float>(-static_cast<double>(o.dy) / 100.0);
            // hues a golden angle apart, so neighbouring camps differ
            const double hue = std::fmod(static_cast<double>(c) * 0.6180339887, 1.0);
            const godot::Color colour = godot::Color::from_hsv(static_cast<float>(hue), 0.55F, 0.9F);
            write(camp_buffer_, c, x, kCampHeight, z, scale * kCampScale, {colour.r, colour.g, colour.b, 1.0F});
            x0 = std::min(x0, x);
            z0 = std::min(z0, z);
            x1 = std::max(x1, x);
            z1 = std::max(z1, z);
        }
        server().multimesh_set_buffer(camps_, camp_buffer_);
        if (n > 0) {
            const float r = scale * kCampScale;
            const godot::Vector3 from(x0 - r, 0.0F, z0 - r);
            const godot::Vector3 to(x1 + r, kCampHeight + r, z1 + r);
            server().multimesh_set_custom_aabb(camps_, godot::AABB(from, to - from));
        }
    }
    return static_cast<int64_t>(s.walkers.size());
}

godot::PackedInt32Array KdCrowd::area_counts() const {
    godot::PackedInt32Array out;
    for (const int32_t n : counts_) {
        out.push_back(n);
    }
    return out;
}

}  // namespace kd::view
