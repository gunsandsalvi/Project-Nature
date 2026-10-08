// A .kdtex file as Godot images (A5.4): its levels, each an RGBA8 image, and one image whose mipmaps are those levels,
// the texture's own rather than Godot's averaged ones (A5.3). The Look class, the model kit and the ground read
// textures through these, so a texture file is read one way (CLAUDE.md rule 4).
#pragma once

#include <vector>

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>

namespace kd::view {

/// A .kdtex file's levels, each an RGBA8 image, largest first, each half the one before and the last one texture
/// pixel across; or the problem, and none.
struct ReadLevels {
    std::vector<godot::Ref<godot::Image>> images;
    godot::String problem;
};

/// Implements PRE-22, see A5.4: the levels of the file at a path, checked.
[[nodiscard]] ReadLevels read_levels(const godot::String& path);

/// Implements PRE-22/PLT-04: decode owned bytes into fresh CPU Images, with no GPU or scene calls.
[[nodiscard]] ReadLevels read_levels_bytes(const godot::PackedByteArray& bytes);

/// One image of the levels, its mipmaps our own levels (A5.3).
[[nodiscard]] godot::Ref<godot::Image> with_levels(const std::vector<godot::Ref<godot::Image>>& images);

/// The textures of these files as one texture array, a layer each in the order given, all one size, each with its own
/// levels; its RID, which the caller frees, or the problem and none.
struct Layered {
    godot::RID texture;
    godot::String problem;
};

/// Implements PRE-22, see A5.4 and A4.6: a texture array of files, as the ground, the river and the water draw with.
[[nodiscard]] Layered make_layered(const godot::PackedStringArray& paths);

}  // namespace kd::view
