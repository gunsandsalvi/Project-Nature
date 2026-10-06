// Texture files (A5.4): a .kdtex file holds one texture's levels, each drawn for its zoom band (A5.3), as PNG
// pictures, largest first: "KDTX", then the format's version, the number of levels and each level's length in bytes
// as 32-bit little-endian numbers, each followed by its PNG. Godot ships such a file untouched, since it imports no
// file of that name, and the engine makes the levels the texture's own mipmaps, never Godot's averaged ones.
// The cloud writes them (tools/standins.py, and the build from the art lane's levels); this reads them, touching no
// Godot, so its tests run alone.
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace kd::view {

/// The format's version this reads.
inline constexpr std::uint32_t kTextureVersion = 1;

/// A .kdtex file's levels, each a PNG's bytes within the file, largest first, or why the file is not one.
struct Levels {
    std::vector<std::span<const std::uint8_t>> pictures;
    std::string problem;
};

/// Implements PRE-22, see A5.4: a .kdtex file split into its levels.
Levels split_levels(std::span<const std::uint8_t> file);

}  // namespace kd::view
