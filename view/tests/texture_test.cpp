#include <cstdint>
#include <vector>

#include "doctest.h"
#include "textures.hpp"

namespace {

// A .kdtex file of levels given as stand-in bytes.
std::vector<std::uint8_t> kdtex(const std::vector<std::vector<std::uint8_t>>& levels, std::uint32_t version = 1) {
    std::vector<std::uint8_t> out{'K', 'D', 'T', 'X'};
    auto put = [&out](std::uint32_t v) {
        for (int i = 0; i < 4; ++i) {
            out.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
        }
    };
    put(version);
    put(static_cast<std::uint32_t>(levels.size()));
    for (const auto& level : levels) {
        put(static_cast<std::uint32_t>(level.size()));
        out.insert(out.end(), level.begin(), level.end());
    }
    return out;
}

}  // namespace

// checks: PRE-22
TEST_CASE("a texture file splits into its levels, and a damaged one is refused with words") {
    const std::vector<std::uint8_t> file = kdtex({{1, 2, 3}, {4, 5}, {6}});
    const kd::view::Levels levels = kd::view::split_levels(file);
    CHECK(levels.problem.empty());
    REQUIRE(levels.pictures.size() == 3);
    CHECK(levels.pictures[0].size() == 3);
    CHECK(levels.pictures[1][1] == 5);
    CHECK(levels.pictures[2][0] == 6);
    std::vector<std::uint8_t> cut = file;
    cut.pop_back();
    CHECK(kd::view::split_levels(cut).problem == "a texture file cut short");
    CHECK(kd::view::split_levels(cut).pictures.empty());
    std::vector<std::uint8_t> longer = file;
    longer.push_back(0);
    CHECK(kd::view::split_levels(longer).problem == "a texture file with bytes after its last level");
    CHECK(kd::view::split_levels(kdtex({{1}}, 2)).problem == "a texture file of another version");
    CHECK(kd::view::split_levels(std::vector<std::uint8_t>{'P', 'N', 'G'}).problem == "not a texture file");
}
