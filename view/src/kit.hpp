// The model kit's parts as the engine reads them (A6.1, A6.4): a family's parts, each with its triangles in a section
// for each material role, its texture coordinates in metres, its crease darkening and its named joints. The build
// exports each family's Blender file into one .kdkit file (tools/kit.py runs tools/kit/export.py in Blender) and
// Godot ships it untouched, as it ships .kdtex textures. This reads it, writes it for the tests, and checks it,
// touching no Godot, so its tests run alone.
//
// The file, all numbers little-endian: "KDKT", the format's version, the number of parts; each part: its name (a 16-bit
// length and its bytes), its bounds (six 32-bit floats), its sections, its joints. A section: its role, its number of
// vertices and of indices, the vertices' positions (3 floats each), normals (3), texture coordinates (2) and crease
// (one byte, 255 for open), and the indices (32 bits each, three to a triangle). A joint: its name, then a rotation
// (nine floats, by rows, its columns the joint's own axes) and its place (three floats). Everything is in the part's
// own metres in Godot's axes (x east, y up, z south), triangles wound as Godot's front faces are, clockwise seen from
// the front (A4.7); texture coordinates run u to the right and v down, in metres, so a texture pixel is 1/64 m at band
// 0.
#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace kd::view::kit {

/// The format's version this reads.
inline constexpr std::uint32_t kVersion = 1;

/// A part's joint: where another part plugs in, and which way it faces. rotation is by rows, so its columns are the
/// joint's own axes in the part's metres, Blender's x, y and z carried into Godot's axes; its third, Blender's z, is
/// its main one.
struct Joint {
    std::string name;
    std::array<float, 9> rotation{1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 1.0F};
    std::array<float, 3> at{};

    bool operator==(const Joint&) const = default;
};

/// The triangles of a part that wear one material role.
struct Section {
    std::string role;
    std::vector<float> positions;  // three a vertex
    std::vector<float> normals;    // three a vertex
    std::vector<float> uvs;        // two a vertex, in metres
    std::vector<std::uint8_t> crease;
    std::vector<std::uint32_t> indices;  // three a triangle

    [[nodiscard]] std::size_t vertices() const { return crease.size(); }
    [[nodiscard]] std::size_t triangles() const { return indices.size() / 3; }

    bool operator==(const Section&) const = default;
};

/// One shape of the kit.
struct Part {
    std::string name;
    std::array<float, 3> lowest{};
    std::array<float, 3> highest{};
    std::vector<Section> sections;
    std::vector<Joint> joints;

    [[nodiscard]] const Joint* joint(const std::string& joint_name) const;
    [[nodiscard]] std::size_t triangles() const;

    bool operator==(const Part&) const = default;
};

/// A family's parts, in the order of their names.
struct Family {
    std::vector<Part> parts;

    [[nodiscard]] const Part* part(const std::string& part_name) const;

    bool operator==(const Family&) const = default;
};

/// A family read from a file, or why the file is not one.
struct Read {
    Family family;
    std::string problem;
};

/// Implements PRE-46, see A6.1: a .kdkit file read into a family, every length and count checked against the file's
/// size, and every index against its section's vertices.
[[nodiscard]] Read read_family(std::span<const std::uint8_t> file);

/// A family as a .kdkit file's bytes, as tools/kit/export.py writes it; for the tests.
[[nodiscard]] std::vector<std::uint8_t> write_family(const Family& family);

/// A triangle whose texture pixels stretch past the line: where it is, and by how much.
struct Stretched {
    std::string part;
    std::string role;
    std::size_t triangle = 0;
    double stretch = 1.0;
};

/// How much each triangle of a part stretches its texture pixels, the ratio of the longest to the shortest way a
/// texture pixel is carried onto the surface, at least 1; one number for each triangle of each section in order, and
/// 0 for a triangle of no area, which carries nothing. Implements PRE-22 and PRE-46, see A6.4.
[[nodiscard]] std::vector<double> stretches(const Part& part);

/// Implements PRE-22 and PRE-46, see A6.4: every triangle of every part whose texture pixels stretch more than the
/// line, 1.5 to 1 by default, worst first; and the worst stretch of all.
struct StretchReport {
    std::vector<Stretched> over;
    double worst = 1.0;
    std::size_t triangles = 0;
};
[[nodiscard]] StretchReport check_stretch(const Family& family, double line = 1.5);

}  // namespace kd::view::kit
