#include <cstdint>
#include <vector>

#include "doctest.h"
#include "path.hpp"
#include "rig.hpp"
#include "stage.hpp"
#include "textures.hpp"

using kd::view::Change;
using kd::view::Copy;
using kd::view::Path;
using kd::view::Rig;
using kd::view::Stage;

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
TEST_CASE("the stage reports each copy put, changed or dropped once, and nothing for a copy put again unchanged") {
    Stage stage;
    Copy a;
    a.east = 100;
    stage.put(7, a);
    stage.put(3, a);
    std::vector<Change> changes = stage.drain();
    REQUIRE(changes.size() == 2);
    CHECK(changes[0].id == 3);
    CHECK(changes[1].id == 7);
    CHECK_FALSE(changes[0].gone);
    // still copies upload once: put again unchanged, nothing changes
    stage.put(7, a);
    CHECK(stage.drain().empty());
    a.north = 50;
    stage.put(7, a);
    stage.drop(3);
    stage.drop(99);  // never there: no change
    changes = stage.drain();
    REQUIRE(changes.size() == 2);
    CHECK(changes[0].id == 3);
    CHECK(changes[0].gone);
    CHECK(changes[1].id == 7);
    CHECK_FALSE(changes[1].gone);
    CHECK(stage.size() == 1);
    REQUIRE(stage.find(7) != nullptr);
    CHECK(stage.find(7)->north == 50);
    CHECK(stage.find(3) == nullptr);
}

// checks: PRE-22
TEST_CASE("each scripted camera path moves the rig the same way every time and ends where it should") {
    Rig rig;
    rig.set_screen(1080.0, 2404.0);
    rig.set_metres_per_pixel(0.01);
    Path path;
    path.start(Path::named("pan"), rig);
    for (int frame = 1; frame <= 8 * 60; ++frame) {
        path.apply(rig, frame / 60.0);
    }
    CHECK(path.kind() == Path::Kind::none);
    // four screen widths east: 4 x 1080 pixels x 1 cm
    CHECK(rig.focus_east() == 4 * 1080);
    CHECK(rig.focus_north() == 0);
    path.start(Path::named("turn"), rig);
    path.apply(rig, 1.5 / 2.0);
    CHECK(rig.heading() == doctest::Approx(22.5));  // halfway through the first eased step of 45 degrees
    path.apply(rig, Path::kTurnSeconds);
    CHECK(rig.heading() == doctest::Approx(0.0).epsilon(1e-9));
    path.start(Path::named("pinch"), rig);
    path.apply(rig, Path::kPinchSeconds / 2.0);
    CHECK(rig.metres_per_pixel() == doctest::Approx(0.02));
    path.apply(rig, Path::kPinchSeconds);
    CHECK(rig.metres_per_pixel() == doctest::Approx(0.04));
    CHECK(path.kind() == Path::Kind::none);
    CHECK(Path::named("spin") == Path::Kind::none);
}

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
