#include <string>
#include <vector>

#include "catalogue_files.hpp"
#include "doctest.h"
#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/folder.hpp"
#include "kd/look/area_tuning.hpp"
#include "kd/look/water_tuning.hpp"

namespace data = kd::data;
namespace look = kd::look;

namespace {

constexpr const char* kArt = "id = \"art\"\nversion = 1\nabout = \"the art lane's textures\"\n";

// A texture's record, for the area's surfaces to name.
constexpr const char* kTexture =
    "about = \"a texture\"\n"
    "route = \"picture\"\n"
    "tile_texels = 128\n"
    "texels_a_metre = 64\n"
    "first_band = 0\n"
    "sources = [\"art/sources/x.webp\"]\n"
    "original_sha256 = [\"aa\"]\n"
    "c2pa = [\"present\"]\n"
    "requests = [\"art/requests/x.txt\"]\n"
    "made = \"re-gridded\"\n"
    "regrid_loss = \"6.1%\"\n"
    "truth = \"art lane, 2026-10-07: ground only\"\n"
    "approved = \"waiting\"\n"
    "\n"
    "[[band]]\n"
    "level = 0\n"
    "file = \"art/textures/x/b0.png\"\n"
    "sha256 = \"0a\"\n"
    "way = \"re-gridded\"\n";

// A recipe for the camp's tent and club to name: one part of one role, wearing a texture the catalogue has.
constexpr const char* kThing =
    "about = \"a thing\"\n"
    "family = \"camp\"\n"
    "truth = \"art lane, 2026-10-07: after the sheet\"\n"
    "approved = \"waiting\"\n"
    "\n"
    "[[material]]\n"
    "role = \"wood\"\n"
    "textures = [\"art:meadow\"]\n"
    "\n"
    "[[place]]\n"
    "name = \"cover\"\n"
    "parts = [\"cover\"]\n"
    "rule = \"root\"\n";

// The area as base/tuning/area.toml writes it.
constexpr const char* kArea =
    "ground = \"art:meadow\"\n"
    "bed = \"art:river_bed\"\n"
    "marks = \"art:river_marks\"\n"
    "reach = \"1024 m\"\n"
    "strip = \"48 m\"\n"
    "bank = \"0.4 m\"\n"
    "width = \"12 m\"\n"
    "width_wobble = \"30%\"\n"
    "depth = \"0.9 m\"\n"
    "depth_wobble = \"60%\"\n"
    "wobble_length = \"60 m\"\n"
    "run = \"1 m\"\n"
    "spacing = \"0.25 m\"\n"
    "seed = 7\n"
    "tent = \"art:hide_tent_cone\"\n"
    "club = \"art:club\"\n"
    "camp_back = \"8 m\"\n"
    "club_away = \"3.6 m\"\n"
    "club_bearing = 125\n"
    "tent_turn = 0\n"
    "club_turn = 70\n"
    "camp_seed = 3\n"
    "patch_seed = 11\n"
    "growth_scale = \"48 m\"\n"
    "growth_swing = \"60%\"\n"
    "bare_below = \"20%\"\n"
    "clearing = \"5 m\"\n"
    "clearing_fade = \"5 m\"\n"
    "earth_colour = \"#b39161\"\n";

// The water as base/tuning/water.toml writes it.
constexpr const char* kWater =
    "fade_red = \"0.43 m\"\n"
    "fade_green = \"0.9 m\"\n"
    "fade_blue = \"1.65 m\"\n"
    "murk = \"0.28 m\"\n"
    "deep_colour = \"#173e43\"\n"
    "wet_margin = \"0.12 m\"\n"
    "wet_darkening = \"80%\"\n"
    "ragged = \"100%\"\n"
    "sky_share = \"3%\"\n"
    "flow = \"0.5 m/s\"\n"
    "step_rate = 10\n"
    "glint_colour = \"#f4ead0\"\n"
    "glint_share = \"1.2%\"\n"
    "glint_life = 3\n"
    "shore_colour = \"#aac4b6\"\n"
    "shore_width = 1\n";

std::vector<data::SourceFile> with_files(const std::string& area, const std::string& water) {
    std::vector<data::SourceFile> files = kd::test::good();
    files.push_back({"art/source.toml", kArt});
    for (const char* name : {"meadow", "river_bed", "river_marks"}) {
        files.push_back({std::string("art/textures/") + name + "/record.toml", kTexture});
    }
    for (const char* name : {"hide_tent_cone", "club"}) {
        files.push_back({std::string("art/models/") + name + "/record.toml", kThing});
    }
    files.push_back({"base/tuning/area.toml", area});
    files.push_back({"base/tuning/water.toml", water});
    return files;
}

std::string replaced(std::string text, const std::string& from, const std::string& to) {
    const std::size_t at = text.find(from);
    REQUIRE(at != std::string::npos);
    return text.replace(at, from.size(), to);
}

bool names(const std::vector<std::string>& problems, const std::string& part) {
    for (const std::string& p : problems) {
        if (p.find(part) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> load_problems(const std::string& area, const std::string& water) {
    data::Catalogue cat;
    std::vector<std::string> out;
    for (const data::Problem& p : cat.load(with_files(area, water))) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

std::vector<std::string> checked_problems(const std::string& area, const std::string& water) {
    data::Catalogue cat;
    REQUIRE(cat.load(with_files(area, water)).empty());
    std::vector<std::string> out;
    for (const data::Problem& p : data::run_checks(cat)) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

}  // namespace

// checks: PRE-23 PRE-26 MAT-13
TEST_CASE("the stand-in area is one tuning entry: three surfaces named by their near tiles, and the river's shape") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_files(kArea, kWater)).empty());
    const auto& areas = cat.kind<look::AreaTuning>();
    REQUIRE(areas.size() == 1);
    const look::AreaTuning& a = areas[0];
    // each surface is the name of its texture's near tile
    CHECK(a.ground == "art:meadow");
    CHECK(a.bed == "art:river_bed");
    CHECK(a.marks == "art:river_marks");
    CHECK(a.reach == 1'024'000);
    CHECK(a.strip == 48'000);
    CHECK(a.bank == 400);
    CHECK(a.width == 12'000);
    CHECK(a.width_wobble == 300'000);
    CHECK(a.depth == 900);
    CHECK(a.depth_wobble == 600'000);
    CHECK(a.wobble_length == 60'000);
    CHECK(a.run == 1'000);
    CHECK(a.spacing == 250);
    CHECK(a.seed == 7);
    // the camp on the bank: two things named by their recipes' entries, and where they stand
    CHECK(a.tent == "art:hide_tent_cone");
    CHECK(a.club == "art:club");
    CHECK(a.camp_back == 8'000);
    CHECK(a.club_away == 3'600);
    CHECK(a.club_bearing == 125);
    CHECK(a.tent_turn == 0);
    CHECK(a.club_turn == 70);
    CHECK(a.camp_seed == 3);
    // the patch picture's numbers
    CHECK(a.patch_seed == 11);
    CHECK(a.growth_scale == 48'000);
    CHECK(a.growth_swing == 600'000);
    CHECK(a.bare_below == 200'000);
    CHECK(a.clearing == 5'000);
    CHECK(a.clearing_fade == 5'000);
    CHECK(a.earth_colour == "#b39161");
    CHECK(data::run_checks(cat).empty());
}

// checks: PRE-26 MAT-13
TEST_CASE("the river's water is one tuning entry, its lengths in millimetres and its shares in parts of a million") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_files(kArea, kWater)).empty());
    const auto& waters = cat.kind<look::WaterTuning>();
    REQUIRE(waters.size() == 1);
    const look::WaterTuning& w = waters[0];
    CHECK(w.fade_red == 430);
    CHECK(w.fade_green == 900);
    CHECK(w.fade_blue == 1'650);
    CHECK(w.murk == 280);
    CHECK(w.deep_colour == "#173e43");
    CHECK(w.wet_margin == 120);
    CHECK(w.wet_darkening == 800'000);
    CHECK(w.ragged == 1'000'000);
    CHECK(w.sky_share == 30'000);
    CHECK(w.flow == 500);
    CHECK(w.step_rate == 10);
    CHECK(w.glint_share == 12'000);
    CHECK(w.glint_life == 3);
    CHECK(w.shore_width == 1);
}

// checks: PRE-23 PRE-26 MAT-14
TEST_CASE("the area and the water count only in the look's digest, and a change of either changes it") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_files(kArea, kWater)).empty());
    const data::EntryDigests area_before = cat.kind<look::AreaTuning>().digests(0);
    const data::EntryDigests water_before = cat.kind<look::WaterTuning>().digests(0);
    CHECK(area_before.fields[0] == 0);
    CHECK(area_before.fields[1] == 0);
    CHECK(area_before.fields[2] == 29);
    CHECK(water_before.fields[0] == 0);
    CHECK(water_before.fields[1] == 0);
    CHECK(water_before.fields[2] == 16);
    data::Catalogue changed;
    REQUIRE(
        changed.load(with_files(replaced(kArea, "seed = 7", "seed = 8"), replaced(kWater, "0.9 m", "0.8 m"))).empty());
    const data::EntryDigests area_after = changed.kind<look::AreaTuning>().digests(0);
    const data::EntryDigests water_after = changed.kind<look::WaterTuning>().digests(0);
    CHECK(area_after.by_affects[2] != area_before.by_affects[2]);
    CHECK(area_after.by_affects[0] == area_before.by_affects[0]);
    CHECK(area_after.by_affects[1] == area_before.by_affects[1]);
    CHECK(water_after.by_affects[2] != water_before.by_affects[2]);
    CHECK(water_after.by_affects[0] == water_before.by_affects[0]);
}

// checks: PRE-23 PRE-26 MAT-13
TEST_CASE("a fault in the area or the water is refused where it is written") {
    // a field missing, out of range, or the wrong measure
    CHECK(names(load_problems(replaced(kArea, "seed = 7\n", ""), kWater), "\"seed\" is missing"));
    CHECK(names(load_problems(replaced(kArea, "bank = \"0.4 m\"", "bank = \"0 m\""), kWater), "out of its range"));
    CHECK(names(load_problems(replaced(kArea, "width = \"12 m\"", "width = \"12 kg\""), kWater), "width"));
    CHECK(names(load_problems(kArea, replaced(kWater, "step_rate = 10", "step_rate = 100")), "out of its range"));
    CHECK(
        names(load_problems(kArea, replaced(kWater, "shore_width = 1\n", "shore_width = 1\nrapids = 1\n")), "rapids"));
}

// checks: PRE-26 MAT-17
TEST_CASE("the strip must hold the river and the square be wider than the strip; each colour of the water is #rrggbb") {
    CHECK(checked_problems(kArea, kWater).empty());
    // a strip too narrow to hold a river 12 m wide with its banks
    CHECK(names(checked_problems(replaced(kArea, "strip = \"48 m\"", "strip = \"14 m\""), kWater), "strip"));
    // a square no wider than its strip has no meadow north or south of it
    CHECK(names(checked_problems(replaced(replaced(kArea, "reach = \"1024 m\"", "reach = \"64 m\""), "strip = \"48 m\"",
                                          "strip = \"64 m\""),
                                 kWater),
                "reach"));
    // the earth's colour is #rrggbb
    CHECK(names(checked_problems(replaced(kArea, "\"#b39161\"", "\"#B39161\""), kWater), "earth_colour"));
    // a camp too far from the river for the strip to hold it
    CHECK(names(checked_problems(replaced(kArea, "camp_back = \"8 m\"", "camp_back = \"40 m\""), kWater), "camp_back"));
    // a thing no recipe has, where there are recipes
    CHECK(
        names(checked_problems(replaced(kArea, "art:club", "art:no_club"), kWater), "names no recipe \"art:no_club\""));
    CHECK(names(checked_problems(replaced(kArea, "art:hide_tent_cone", "art:no_tent"), kWater),
                "names no recipe \"art:no_tent\""));
    // a bearing is a whole number of degrees round the compass
    CHECK(
        names(load_problems(replaced(kArea, "club_bearing = 125", "club_bearing = 400"), kWater), "out of its range"));
    // a surface no texture entry has, where there are textures
    CHECK(names(checked_problems(replaced(kArea, "art:river_bed", "art:no_bed"), kWater),
                "names no texture \"art:no_bed\""));
    CHECK(names(checked_problems(kArea, replaced(kWater, "\"#173e43\"", "\"#173E43\"")), "deep_colour"));
    CHECK(names(checked_problems(kArea, replaced(kWater, "\"#aac4b6\"", "\"aac4b6\"")), "shore_colour"));
    CHECK(names(checked_problems(kArea, replaced(kWater, "\"#f4ead0\"", "\"#f4ead\"")), "glint_colour"));
}

// checks: PRE-20 PRE-23 PRE-26
TEST_CASE(
    "the area and the water load and pass the checks with no art in the catalogue, the engine running with none") {
    std::vector<data::SourceFile> files = kd::test::good();
    files.push_back({"base/tuning/area.toml", kArea});
    files.push_back({"base/tuning/water.toml", kWater});
    data::Catalogue cat;
    REQUIRE(cat.load(files).empty());
    CHECK(cat.kind<look::AreaTuning>().size() == 1);
    CHECK(data::run_checks(cat).empty());
}

// checks: PRE-23 PRE-26
TEST_CASE("the game's own area and water pass the catalogue's checks and name textures the art source has") {
    data::Catalogue cat;
    std::string problems;
    for (const data::Problem& p : cat.load(data::read_catalogue(KD_REPO "/data"))) {
        problems += data::problem_text(p) + "\n";
    }
    INFO(problems);
    REQUIRE(problems.empty());
    CHECK(cat.kind<look::AreaTuning>().size() == 1);
    CHECK(cat.kind<look::WaterTuning>().size() == 1);
    for (const data::Problem& p : data::run_checks(cat)) {
        problems += data::problem_text(p) + "\n";
    }
    CHECK(problems.empty());
}
