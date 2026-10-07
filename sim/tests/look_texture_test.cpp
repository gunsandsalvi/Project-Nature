#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "catalogue_files.hpp"
#include "doctest.h"
#include "kd/data/catalogue.hpp"
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

// checks: PRE-20 MAT-14
TEST_CASE("a texture counts only in its source's look digest, and a level's change changes it") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_art({{"art/textures/meadow/record.toml", kMeadow}})).empty());
    const data::EntryDigests before = cat.kind<look::Texture>().digests(0);
    CHECK(before.fields[0] == 0);  // rules
    CHECK(before.fields[1] == 0);  // world
    CHECK(before.fields[2] == 14);
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
    // its levels missing altogether
    const std::string levels = kMeadow;
    CHECK(names(problems_of({{path, levels.substr(0, levels.find("[[band]]"))}}), "\"band\" is missing"));
    // a folder that is no name, and a record with no folder of its own
    CHECK(names(problems_of({{"art/textures/Meadow/record.toml", kMeadow}}), "a record's name is its folders"));
    CHECK(names(problems_of({{"art/textures/record.toml", kMeadow}}), "no kind of entry lives here"));
}

// checks: PRE-20 PRE-42
TEST_CASE("the art lane's textures load whole from art/, beside the data folder") {
    const std::vector<data::SourceFile> art = data::read_art(KD_REPO);
    std::size_t records = 0;
    for (const auto& item : std::filesystem::recursive_directory_iterator(KD_REPO "/art/textures")) {
        records += item.path().filename() == "record.toml" ? 1 : 0;
    }
    REQUIRE(records > 0);
    CHECK(art.size() == records + 1);
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
