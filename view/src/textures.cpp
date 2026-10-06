#include "textures.hpp"

namespace kd::view {

namespace {

// A 32-bit little-endian number at a place in the file.
std::uint32_t read32(std::span<const std::uint8_t> file, std::size_t at) {
    return static_cast<std::uint32_t>(file[at]) | static_cast<std::uint32_t>(file[at + 1]) << 8U |
           static_cast<std::uint32_t>(file[at + 2]) << 16U | static_cast<std::uint32_t>(file[at + 3]) << 24U;
}

}  // namespace

Levels split_levels(std::span<const std::uint8_t> file) {
    Levels out;
    if (file.size() < 12 || file[0] != 'K' || file[1] != 'D' || file[2] != 'T' || file[3] != 'X') {
        out.problem = "not a texture file";
        return out;
    }
    if (read32(file, 4) != kTextureVersion) {
        out.problem = "a texture file of another version";
        return out;
    }
    const std::uint32_t count = read32(file, 8);
    if (count == 0 || count > 16) {
        out.problem = "a texture file with no levels, or too many";
        return out;
    }
    std::size_t at = 12;
    for (std::uint32_t i = 0; i < count; ++i) {
        if (file.size() - at < 4) {
            out.problem = "a texture file cut short";
            out.pictures.clear();
            return out;
        }
        const std::uint32_t length = read32(file, at);
        at += 4;
        if (length == 0 || file.size() - at < length) {
            out.problem = "a texture file cut short";
            out.pictures.clear();
            return out;
        }
        out.pictures.push_back(file.subspan(at, length));
        at += length;
    }
    if (at != file.size()) {
        out.problem = "a texture file with bytes after its last level";
        out.pictures.clear();
    }
    return out;
}

}  // namespace kd::view
