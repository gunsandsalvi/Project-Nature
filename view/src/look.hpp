// Shared .kdtex decoding into Godot images for the self-check and 2D art preparation (A5.4, PRE-22).
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace kd::view {

class KdLook : public godot::RefCounted {
    GDCLASS(KdLook, godot::RefCounted)

public:
    [[nodiscard]] godot::Dictionary texture_image(const godot::String& path) const;
    [[nodiscard]] godot::Dictionary texture_levels(const godot::String& path) const;
    /// Implements PLT-04: owned encoded bytes become fresh private CPU Images through the shared decoder.
    [[nodiscard]] godot::Dictionary texture_levels_bytes(const godot::PackedByteArray& bytes) const;

protected:
    static void _bind_methods();
};

}  // namespace kd::view
