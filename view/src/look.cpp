// Shared texture-file decoding for the self-check and the 2D renderer (A5.4, PRE-22).
#include "look.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

#include "texture_images.hpp"

namespace kd::view {

godot::Dictionary KdLook::texture_image(const godot::String& path) const {
    const ReadLevels read = read_levels(path);
    godot::Dictionary out;
    out["problem"] = read.problem;
    if (read.problem.is_empty()) {
        out["image"] = with_levels(read.images);
    }
    return out;
}

namespace {
godot::Dictionary images_of(const ReadLevels& read) {
    godot::Dictionary out;
    out["problem"] = read.problem;
    godot::Array levels;
    for (const godot::Ref<godot::Image>& image : read.images) {
        levels.append(image);
    }
    out["levels"] = levels;
    return out;
}

}  // namespace

godot::Dictionary KdLook::texture_levels(const godot::String& path) const {
    return images_of(read_levels(path));
}
godot::Dictionary KdLook::texture_levels_bytes(const godot::PackedByteArray& bytes) const {
    return images_of(read_levels_bytes(bytes));
}

void KdLook::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("texture_image", "path"), &KdLook::texture_image);
    ClassDB::bind_method(D_METHOD("texture_levels", "path"), &KdLook::texture_levels);
    ClassDB::bind_method(D_METHOD("texture_levels_bytes", "bytes"), &KdLook::texture_levels_bytes);
}

}  // namespace kd::view
