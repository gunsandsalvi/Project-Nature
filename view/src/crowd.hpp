// The crowd class (A3.8): draws the crowd's newest snapshot into MultiMesh buffers through Godot's RenderingServer, one
// for each area of the crowd's square with its own bounding box, so what is off the screen is never drawn. Each walker
// is placed at the screen's game time along its way, in double precision relative to an origin that moves with the
// camera (A8.2). A few calls a frame, never one per walker; it converts, and never decides.
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/rid.hpp>

#include "world.hpp"

namespace kd::view {

/// Implements PLT-01 and WLD-13, see A3.8: the crowd drawn from the newest snapshot, at the screen's time.
class KdCrowd : public godot::RefCounted {
    GDCLASS(KdCrowd, godot::RefCounted)

public:
    /// Floats for each instance in a MultiMesh's buffer: a 3D transform's twelve, then a colour's four.
    static constexpr int kStride = 16;

    /// The world whose crowd it draws, which must have started one.
    void set_world(const godot::Ref<KdWorld>& world);
    /// The MultiMeshes it draws into, by their RIDs: one for each area, across by across in rows from the south-west
    /// corner of the crowd's square, and one for the camps.
    void set_areas(const godot::Array& areas, int64_t across);
    void set_camps(const godot::RID& camps);

    /// Draws the newest snapshot at a game time, relative to an origin in world centimetres, each walker this many
    /// metres across: east is x and north is -z, in metres. Returns how many walkers it drew.
    int64_t draw(double t, int64_t origin_east, int64_t origin_north, double size);
    /// How many walkers the last drawing put in each area.
    godot::PackedInt32Array area_counts() const;
    /// The drawing's own time on the main thread, for the benchmark (PLT-04): drawings since the last reset, and their
    /// mean and longest in milliseconds.
    godot::Dictionary draw_times() const;
    void draw_times_reset();

protected:
    static void _bind_methods();

private:
    struct Placed {
        float x = 0.0F;
        float z = 0.0F;
        std::uint32_t area = 0;
        float scale = 1.0F;
        std::array<float, 4> colour{};
    };

    void write(godot::PackedFloat32Array& buffer, int64_t at, float x, float y, float z, float scale,
               const std::array<float, 4>& colour) const;
    int64_t draw_now(double t, int64_t origin_east, int64_t origin_north, double size);

    godot::Ref<KdWorld> world_;
    std::vector<godot::RID> areas_;
    int64_t across_ = 1;
    godot::RID camps_;
    bool camps_ready_ = false;
    std::vector<godot::PackedFloat32Array> buffers_;
    std::vector<int32_t> capacities_;
    std::vector<int32_t> counts_;
    godot::PackedFloat32Array camp_buffer_;
    std::vector<std::array<float, 4>> kind_colours_;
    std::vector<Placed> placed_;
    int64_t draws_ = 0;
    double draw_ms_ = 0.0;
    double longest_draw_ms_ = 0.0;
};

}  // namespace kd::view
