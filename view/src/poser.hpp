// The poser's stand-in (A6.3, α2.2b): a figure on a skeleton of 24 bones, posed by joint angles 10 times a second,
// each figure's seed offsetting its walk and its steps. Its skinning matrices go to Godot's own skeletons or into a
// bone palette from here, so both ways of drawing a figure show the same pose (CLAUDE.md rule 4); calibration scene C6
// times both. No Godot here, so its tests run alone.
#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace kd::view {

/// A bone's skinning matrix as a palette holds it, three texels a bone: three rows of a rotation and a translation,
/// the translation in the last column, in metres.
struct BoneRows {
    std::array<std::array<float, 4>, 3> m{};
};

/// A bone of the stand-in skeleton: its parent, -1 for the root; where it sits in its parent at rest, in metres; and
/// the body part on it, a tube of this radius out to its tail, none where the radius is 0.
struct Bone {
    int parent = -1;
    std::array<float, 3> at{};
    std::array<float, 3> tail{};
    float radius = 0.0F;
};

/// The stand-in skeleton: 24 bones, a standing adult about 1.7 m tall, its feet on the ground at the origin.
[[nodiscard]] const std::vector<Bone>& figure_bones();

/// The stand-in figure at rest: positions and normals, three numbers a vertex; four bones and four weights a vertex,
/// the weights adding to 1; and its triangles, three indices each, about 1,500 of them.
struct FigureMesh {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<std::int32_t> bones;
    std::vector<float> weights;
    std::vector<std::int32_t> indices;
};

/// Implements PRE-27, see A6.3: the stand-in figure's mesh, each body part a tube round its bone, near a joint
/// following both bones half each, so it bends smoothly.
[[nodiscard]] FigureMesh figure_mesh();

/// How many times a second a figure takes a new pose (A6.3).
inline constexpr double kPosesASecond = 10.0;

/// The pose a figure is in at a time, a whole number that moves on 10 times a second, its steps offset by its seed so
/// figures do not all take their next pose in the same frame.
[[nodiscard]] std::int64_t pose_step(std::uint64_t seed, double seconds);

/// Implements PRE-27, see A6.3: a figure's skinning matrices for a pose step: for each bone, its posed place times the
/// inverse of its place at rest, from the joint angles of a walk whose stride its seed offsets.
[[nodiscard]] std::vector<BoneRows> figure_pose(std::uint64_t seed, std::int64_t step);

/// The skinning matrices of the figure at rest: each one changes nothing.
[[nodiscard]] std::vector<BoneRows> rest_pose();

/// The way a stand-in figure faces, from its seed: radians turned about its up axis from facing +z, from 0 to 2π.
[[nodiscard]] float facing(std::uint64_t seed);

/// Where each bone sits in a pose, in the figure's own metres: its posed place's translation.
[[nodiscard]] std::vector<std::array<float, 3>> bone_places(std::uint64_t seed, std::int64_t step);

/// A number as a half float's bits, rounded to the nearest, ties to even: how a palette holds it (A6.3).
[[nodiscard]] std::uint16_t half_bits(float value);

/// Texels a bone takes in a palette's row: its skinning matrix's three rows.
inline constexpr int kTexelsABone = 3;

/// Implements PRE-27, see A6.3: a figure's row of a bone palette, three texels a bone and four half floats a texel,
/// each texel a row of the bone's skinning matrix, as the figures' shader reads it.
[[nodiscard]] std::vector<std::uint16_t> palette_row(const std::vector<BoneRows>& pose);

}  // namespace kd::view
