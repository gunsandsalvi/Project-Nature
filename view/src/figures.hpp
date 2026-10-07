// Calibration scene C6's figures (A6.3, A18.1, α2.2b): stand-in figures posed by the poser 10 times a second and drawn
// one of two ways. By Godot's own skeletons: a skeleton and an instance for each figure, bent by Godot's compute
// program when its pose changes. By our bone palettes: every figure's 24 bones a row of a half-float texture written at
// its pose step, read by one MultiMesh's shader, which bends each vertex by the row its copy names. Both take the same
// skinning matrices from the poser (CLAUDE.md rule 4); this class converts and draws, and never decides for the world
// (WLD-13).
#pragma once

#include <cstdint>
#include <vector>

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>

namespace kd::view {

/// Implements PRE-27, see A6.3 and A18.1: calibration scene C6's figures, posed by the poser and drawn by Godot's own
/// skeletons or by bone palettes read in a MultiMesh's shader.
class KdFigures : public godot::RefCounted {
    GDCLASS(KdFigures, godot::RefCounted)

public:
    ~KdFigures() override;

    /// Puts a figure at each place in a scenario, facing a way its seed gives, drawn with a shader one way: "godot",
    /// each figure on a Godot skeleton of its own; or "palette", every figure a copy of one MultiMesh whose custom data
    /// names its row of the palette, which the shader takes as kd_palette. A figure's seed is its place's index.
    /// points: each figure as points, one a vertex, for the cloud's check of the palettes (A18.1).
    void build(const godot::RID& scenario, const godot::String& way, const godot::RID& shader,
               const godot::PackedVector3Array& places, bool points);
    /// One frame: the figures whose pose step has come take their new pose. The time it took, in microseconds, which
    /// the Calibrate page adds to the main thread's.
    double frame(double seconds);
    /// A figure's triangles, and its vertices.
    [[nodiscard]] int64_t triangles() const;
    [[nodiscard]] int64_t vertices() const;
    /// Frees everything it made in the RenderingServer.
    void clear();

protected:
    static void _bind_methods();

private:
    void pose(std::size_t figure, std::int64_t step);

    double clock_ = 0.0;
    bool palette_way_ = false;
    std::vector<std::int64_t> steps_;  // each figure's pose step, as last posed
    godot::RID mesh_;
    godot::RID material_;
    std::vector<godot::RID> skeletons_;  // by figure, for Godot's skeletons
    std::vector<godot::RID> instances_;  // by figure, or the MultiMesh's one
    godot::RID multimesh_;
    godot::RID palette_;
    godot::PackedByteArray rows_;  // the palette's texels, a row a figure
    godot::Ref<godot::Image> image_;
    bool rows_changed_ = false;
};

}  // namespace kd::view
