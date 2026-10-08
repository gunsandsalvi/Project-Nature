#include "texture_images.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <cstdint>
#include <cstring>
#include <span>

#include "textures.hpp"

namespace kd::view {

ReadLevels read_levels_bytes(const godot::PackedByteArray& bytes) {
    ReadLevels out;
    const Levels levels = split_levels(std::span<const std::uint8_t>(bytes.ptr(), bytes.size()));
    if (!levels.problem.empty()) {
        out.problem = godot::String(levels.problem.c_str());
        return out;
    }
    std::int64_t width = 0;
    for (std::size_t i = 0; i < levels.pictures.size(); ++i) {
        godot::PackedByteArray png;
        png.resize(static_cast<int64_t>(levels.pictures[i].size()));
        std::memcpy(png.ptrw(), levels.pictures[i].data(), levels.pictures[i].size());
        godot::Ref<godot::Image> image;
        image.instantiate();
        if (image->load_png_from_buffer(png) != godot::OK) {
            out.problem = godot::String("level ") + godot::String::num_int64(static_cast<int64_t>(i)) +
                          godot::String(" is not a PNG");
            out.images.clear();
            return out;
        }
        image->convert(godot::Image::FORMAT_RGBA8);
        if (i == 0) {
            width = image->get_width();
        }
        if (image->get_width() != (width >> i) || image->get_height() != (width >> i)) {
            out.problem = godot::String("level ") + godot::String::num_int64(static_cast<int64_t>(i)) +
                          godot::String(" is not half the size of the one before");
            out.images.clear();
            return out;
        }
        out.images.push_back(image);
    }
    if (width == 0 || (width >> (levels.pictures.size() - 1)) != 1) {
        out.problem = godot::String("its levels do not run down to one texture pixel");
        out.images.clear();
    }
    return out;
}

ReadLevels read_levels(const godot::String& path) {
    const godot::PackedByteArray bytes = godot::FileAccess::get_file_as_bytes(path);
    if (bytes.is_empty()) {
        return {{}, godot::String("no texture at ") + path};
    }
    ReadLevels result = read_levels_bytes(bytes);
    if (!result.problem.is_empty()) {
        result.problem = path + godot::String(": ") + result.problem;
    }
    return result;
}

// One image of the levels, its mipmaps our own levels rather than Godot's averaged ones (A5.3).
godot::Ref<godot::Image> with_levels(const std::vector<godot::Ref<godot::Image>>& images) {
    godot::PackedByteArray all;
    for (const godot::Ref<godot::Image>& image : images) {
        all.append_array(image->get_data());
    }
    const auto width = static_cast<int32_t>(images[0]->get_width());
    return godot::Image::create_from_data(width, width, true, godot::Image::FORMAT_RGBA8, all);
}

}  // namespace kd::view