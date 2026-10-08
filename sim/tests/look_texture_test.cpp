#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "catalogue_files.hpp"
#include "doctest.h"
#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/folder.hpp"
#include "kd/look/texture.hpp"

namespace data = kd::data;
namespace look = kd::look;

namespace {

constexpr const char* kArt = "id = \"art\"\nversion = 1\nabout = \"the art lane's textures\"\n";

// A near tile's record as the art lane writes it, with two levels.
constexpr const char* kMeadow =
    "about = \"meadow grass\"\n"
    "route = \"picture\"\n"
    "tile_texels = 256\n"
    "texels_a_metre = 64\n"
    "first_band = 0\n"
    "sources = [\"art/sources/meadow/meadow-00.webp\", \"art/sources/meadow/meadow-01.webp\"]\n"
    "original_sha256 = [\"aa\", \"bb\"]\n"
    "c2pa = [\"present\", \"present\"]\n"
    "requests = [\"art/requests/meadow-00.txt\", \"art/requests/meadow-01.txt\"]\n"
    "made = \"re-gridded\"\n"
    "regrid_loss = \"6.1%\"\n"
    "truth = \"art lane, 2026-10-06: grass only\"\n"
    "approved = \"waiting\"\n"
    "\n"
    "[[band]]\n"
    "level = 0\n"
    "file = \"art/textures/meadow/b0.png\"\n"
    "sha256 = \"0a\"\n"
    "way = \"re-gridded\"\n"
    "\n"
    "[[band]]\n"
    "level = 1\n"
    "file = \"art/textures/meadow/b1.png\"\n"
    "sha256 = \"1b\"\n"
    "made_from = \"0a\"\n"
    "way = \"GPT redraw\"\n"
    "regrid_loss = \"9.7%\"\n"
    "calibration = \"lightness -1.5%\"\n";

std::vector<data::SourceFile> with_art(std::vector<data::SourceFile> art) {
    std::vector<data::SourceFile> files = kd::test::good();
    files.push_back({"art/source.toml", kArt});
    for (data::SourceFile& f : art) {
        files.push_back(std::move(f));
    }
    return files;
}

std::vector<std::string> problems_of(std::vector<data::SourceFile> art) {
    data::Catalogue cat;
    std::vector<std::string> out;
    for (const data::Problem& p : cat.load(with_art(std::move(art)))) {
        out.push_back(data::problem_text(p));
    }
    return out;
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

}  // namespace

// checks: PRE-20 PRE-42 MAT-13
TEST_CASE("a texture's record is an entry named by its folder, each tile and version its own, with its levels") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_art({{"art/textures/meadow/record.toml", kMeadow},
                               {"art/textures/meadow/middle/record.toml", kMeadow},
                               {"art/textures/meadow/middle/v2/record.toml", kMeadow}}))
                .empty());
    const auto& textures = cat.kind<look::Texture>();
    REQUIRE(textures.size() == 3);
    CHECK(textures.name(0) == "art:meadow");
    CHECK(textures.name(1) == "art:meadow/middle");
    CHECK(textures.name(2) == "art:meadow/middle/v2");
    const look::Texture& meadow = textures[0];
    CHECK(meadow.tile_texels == 256);
    CHECK(meadow.texels_a_metre == 64);
    CHECK(meadow.c2pa == std::vector<std::string>{"present", "present"});
    REQUIRE(meadow.levels.size() == 2);
    CHECK(meadow.levels[1].level == 1);
    CHECK(meadow.levels[1].file == "art/textures/meadow/b1.png");
    CHECK(meadow.levels[1].made_from == "0a");
    CHECK(meadow.levels[0].made_from.empty());
    CHECK(meadow.levels[1].calibration == "lightness -1.5%");
    // the screen sees each level on one line
    const std::vector<data::FieldValue> values = cat.kind_in("textures")->values(0);
    CHECK(values.back().key == "band");
    REQUIRE(values.back().list.size() == 2);
    CHECK(values.back().list[0] ==
          "level = 0; file = \"art/textures/meadow/b0.png\"; sha256 = \"0a\"; made_from = \"\"; "
          "way = \"re-gridded\"; regrid_loss = \"\"; calibration = \"\"");
}

// checks: PRE-20 PRE-46
TEST_CASE("a texture is tiled unless its record lays it once from a trunk's foot") {
    data::Catalogue cat;
    REQUIRE(
        cat.load(with_art({{"art/textures/meadow/record.toml", kMeadow},
                           {"art/textures/birch_bark_old/record.toml",
                            replaced(kMeadow, "first_band = 0\n", "first_band = 0\nlaid = \"once from the foot\"\n")}}))
            .empty());
    const auto& textures = cat.kind<look::Texture>();
    CHECK(textures[0].laid == "once from the foot");
    CHECK(textures[1].laid.empty());
}

// checks: PRE-20 MAT-14
TEST_CASE("a texture counts only in its source's look digest, and a level's change changes it") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_art({{"art/textures/meadow/record.toml", kMeadow}})).empty());
    const data::EntryDigests before = cat.kind<look::Texture>().digests(0);
    CHECK(before.fields[0] == 0);  // rules
    CHECK(before.fields[1] == 0);  // world
    CHECK(before.fields[2] == 15);
    data::Catalogue changed;
    REQUIRE(
        changed.load(with_art({{"art/textures/meadow/record.toml", replaced(kMeadow, "\"1b\"", "\"1c\"")}})).empty());
    const data::EntryDigests after = changed.kind<look::Texture>().digests(0);
    CHECK(after.by_affects[2] != before.by_affects[2]);
    CHECK(after.by_affects[0] == before.by_affects[0]);
    CHECK(after.by_affects[1] == before.by_affects[1]);
}

// checks: PRE-42 MAT-13
TEST_CASE("a fault in a texture's record or one of its levels is refused where it is written") {
    const std::string path = "art/textures/meadow/record.toml";
    // a level's field missing, out of range, unknown or of the wrong type
    CHECK(names(problems_of({{path, replaced(kMeadow, "sha256 = \"1b\"\n", "")}}), "\"sha256\" is missing"));
    CHECK(names(problems_of({{path, replaced(kMeadow, "level = 1\n", "level = 13\n")}}), "out of its range"));
    CHECK(names(problems_of({{path, replaced(kMeadow, "way = \"GPT redraw\"\n", "way = \"GPT redraw\"\nwhy = 1\n")}}),
                "why"));
    CHECK(
        names(problems_of({{path, replaced(kMeadow, "c2pa = [\"present\", \"present\"]", "c2pa = [\"present\", 2]")}}),
              "each is a text"));
    CHECK(names(problems_of({{path, replaced(kMeadow, "route = \"picture\"", "route = \"painting\"")}}), "route"));
    CHECK(names(problems_of({{path, replaced(kMeadow, "first_band = 0\n", "first_band = 0\nlaid = \"twice\"\n")}}),
                "laid"));
    // its levels missing altogether
    const std::string levels = kMeadow;
    CHECK(names(problems_of({{path, levels.substr(0, levels.find("[[band]]"))}}), "\"band\" is missing"));
    // a folder that is no name, and a record with no folder of its own
    CHECK(names(problems_of({{"art/textures/Meadow/record.toml", kMeadow}}), "a record's name is its folders"));
    CHECK(names(problems_of({{"art/textures/record.toml", kMeadow}}), "no kind of entry lives here"));
}

// checks: PRE-20 PRE-42
TEST_CASE("the art lane's textures load whole from art/, beside the data folder, however many there are") {
    const std::vector<data::SourceFile> art = data::read_art(KD_REPO);
    // the records in a folder of art/: no folder yet is none
    const auto records_in = [](const char* folder) {
        std::size_t found = 0;
        std::error_code error;
        for (std::filesystem::recursive_directory_iterator it(folder, error);
             !error && it != std::filesystem::recursive_directory_iterator(); it.increment(error)) {
            found += it->path().filename() == "record.toml" ? 1 : 0;
        }
        return found;
    };
    const std::size_t records = records_in(KD_REPO "/art/textures");
    // the source's own file, the textures' records and the models' recipes
    CHECK(art.size() == records + records_in(KD_REPO "/art/models") + 1);
    data::Catalogue cat;
    std::string problems;
    for (const data::Problem& p : cat.load(data::read_catalogue(KD_REPO "/data"))) {
        problems += data::problem_text(p) + "\n";
    }
    INFO(problems);
    CHECK(problems.empty());
    const auto& textures = cat.kind<look::Texture>();
    CHECK(textures.size() == records);
    for (std::size_t i = 0; i < textures.size(); ++i) {
        // each level's file lies in its own record's folder
        const std::string folder = "art/textures/" + textures.name(i).substr(4) + "/";
        for (const look::TextureLevel& l : textures[i].levels) {
            CHECK_MESSAGE(l.file.starts_with(folder), l.file);
        }
    }
}

namespace {
std::vector<data::SourceFile> sprite_files() {
    std::string channel = kMeadow;
    channel.erase(channel.find("[[band]]"));
    for (int level = 0; level <= 8; ++level) {
        channel += "[[band]]\nlevel = " + std::to_string(level) + "\nfile = \"art/levels/" + std::to_string(level) +
                   ".png\"\nsha256 = \"ab\"\nway = \"authored\"\n";
    }
    constexpr const char* sprite =
        "about = \"fixture tent\"\nasset = \"tent\"\naction = \"static\"\nseason = \"all\"\n"
        "part = \"whole\"\nfamily = \"near\"\ndensity = 64\ncolour = \"art:colour\"\n"
        "normal = \"art:normal\"\nmaterial = \"art:material\"\npage_width = 256\npage_height = 256\n"
        "frames = 1\nfacings = 1\nnormal_basis = \"world-east-south-up\"\nmaterial_map = \"fixture27-v1\"\n"
        "sheet = \"art/catalogue/sheets/tent.webp\"\napproved = \"waiting\"\n"
        "[[cells]]\nframe = 0\nfacing = 0\nx = 0\ny = 0\nwidth = 256\nheight = 256\n"
        "pivot_x_256 = 32768\npivot_y_256 = 65024\ncanvas_width = 256\ncanvas_height = 256\n"
        "trim_x = 0\ntrim_y = 0\ngutter = 0\n";
    return with_art({{"art/textures/colour/record.toml", channel},
                     {"art/textures/normal/record.toml", channel},
                     {"art/textures/material/record.toml", channel},
                     {"art/sprites/tent/near/record.toml", sprite}});
}
}  // namespace

// checks: PRE-22 PRE-42 PRE-46 PLT-09 (T2.9a.1): aligned art families are catalogue data.
TEST_CASE("an aligned sprite family loads through the shared catalogue with exact fixed pivots") {
    data::Catalogue cat;
    const auto problems = cat.load(sprite_files());
    CHECK(problems.empty());
    CHECK(data::run_checks(cat).empty());
    const auto* kind = cat.kind_in("sprites");
    REQUIRE(kind != nullptr);
    if (kind == nullptr) {
        return;
    }
    REQUIRE(kind->size() == 1);
    CHECK(kind->name(0) == "art:tent/near");
}

// checks: PRE-22 PRE-42 PRE-46 (T2.9a.1): reject incoherent bundles before any resource upload.
TEST_CASE("sprite families reject invalid density facings alignment and cropped cells") {
    for (const auto& change : std::vector<std::pair<std::string, std::string>>{
             {"density = 64", "density = 32"},
             {"facings = 1", "facings = 3"},
             {"page_width = 256", "page_width = 128"},
             {"x = 0", "x = 32"},
             {"frame = 0", "frame = 1"},
             {"material_map = \"fixture27-v1\"", "material_map = \"unknown\""}}) {
        auto files = sprite_files();
        files.back().text = replaced(files.back().text, change.first, change.second);
        data::Catalogue cat;
        REQUIRE(cat.load(files).empty());
        CHECK_FALSE(data::run_checks(cat).empty());
    }
}

// checks: PLT-04 PRE-03 PLT-09 (T2.9a.2): cache limits come from validated look-only catalogue tuning.
TEST_CASE("stream budget tuning loads through the shared look catalogue") {
    auto files = kd::test::good();
    files.push_back({"base/tuning/stream.toml",
                     "queued_jobs=32\npreparing_jobs=1\ninput_bytes=8388608\nprepared_bytes=67108864\n"
                     "staging_bytes=4194304\nresident_bytes=134217728\ntarget_bytes=33554432\n"
                     "maps_bytes=50331648\nsprites_bytes=50331648\nground_bytes=16777216\nmasks_bytes=16777216\n"});
    data::Catalogue cat;
    CHECK(cat.load(files).empty());
    CHECK(cat.kind_in("tuning/stream") != nullptr);
}

// checks: PLT-04 (T2.9a.2): partition limits never promise more memory than the aggregate permits.
TEST_CASE("stream budget partitions cannot exceed their aggregate") {
    auto files = kd::test::good();
    files.push_back({"base/tuning/stream.toml",
                     "queued_jobs=1\npreparing_jobs=2\ninput_bytes=10\nprepared_bytes=30\n"
                     "staging_bytes=20\nresident_bytes=60\ntarget_bytes=32\n"
                     "maps_bytes=30\nsprites_bytes=30\nground_bytes=30\nmasks_bytes=30\n"});
    data::Catalogue cat;
    REQUIRE(cat.load(files).empty());
    CHECK_FALSE(data::run_checks(cat).empty());
}

// checks: PRE-03 PRE-28 PRE-33 (T2.9a.1): navigation and release motion are validated look data.
TEST_CASE("navigation tuning loads as look-only data") {
    auto files = kd::test::good();
    files.push_back({"base/tuning/navigation.toml",
                     "minimum_power=-12\nmaximum_power=6\nperson_power=6\nclose_camp_power=4\ncamp_power=1\n"
                     "valley_power=-4\nregion_power=-7\ntiny_power=-2\ngroup_power=-6\n"
                     "maximum_height=\"32 m\"\noverscan_pixels=32\nshadow_reach=\"128 m\"\nsettle_ms=160\n"});
    data::Catalogue cat;
    CHECK(cat.load(files).empty());
    CHECK(cat.kind_in("tuning/navigation") != nullptr);
}

// checks: PRE-03 PRE-28 (T2.9a.1): reordered time anchors and out-of-range forms cannot silently alter navigation.
TEST_CASE("navigation anchors reject reordered time and form thresholds") {
    for (const auto& extra :
         {"close_camp_power=6\ncamp_power=1\nvalley_power=-4\nregion_power=-7\ntiny_power=-2\ngroup_power=-6\n",
          "close_camp_power=4\ncamp_power=1\nvalley_power=-4\nregion_power=-7\ntiny_power=-8\ngroup_power=-6\n"}) {
        auto files = kd::test::good();
        files.push_back({"base/tuning/navigation.toml",
                         std::string("minimum_power=-12\nmaximum_power=6\nperson_power=6\n") + extra +
                             "maximum_height=\"32 m\"\noverscan_pixels=32\nshadow_reach=\"128 m\"\nsettle_ms=160\n"});
        data::Catalogue cat;
        REQUIRE(cat.load(files).empty());
        CHECK_FALSE(data::run_checks(cat).empty());
    }
}
