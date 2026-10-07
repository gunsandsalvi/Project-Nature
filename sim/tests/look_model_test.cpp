#include <cstddef>
#include <string>
#include <vector>

#include "catalogue_files.hpp"
#include "doctest.h"
#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/folder.hpp"
#include "kd/look/model.hpp"
#include "kd/look/texture.hpp"

namespace data = kd::data;
namespace look = kd::look;

namespace {

constexpr const char* kArt = "id = \"art\"\nversion = 1\nabout = \"the art lane's textures and models\"\n";

// A texture's record, for a model's roles to wear.
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
    "truth = \"art lane, 2026-10-07: wood only\"\n"
    "approved = \"waiting\"\n"
    "\n"
    "[[band]]\n"
    "level = 0\n"
    "file = \"art/textures/x/b0.png\"\n"
    "sha256 = \"0a\"\n"
    "way = \"re-gridded\"\n";

// A cone tent's recipe as the art lane writes it: a cover, ten poles spanning from a ring to a meeting point, a ring of
// stones in two shapes and a binding plugged into the cover's apex.
constexpr const char* kTent =
    "about = \"a small round tent of hides on a cone of ten poles, ringed with stones\"\n"
    "family = \"camp\"\n"
    "truth = \"art lane, 2026-10-07: after the sheet\"\n"
    "approved = \"waiting\"\n"
    "\n"
    "[[material]]\n"
    "role = \"wood\"\n"
    "textures = [\"art:birch\", \"art:pine\"]\n"
    "\n"
    "[[material]]\n"
    "role = \"hide\"\n"
    "textures = [\"art:birch\"]\n"
    "\n"
    "[[place]]\n"
    "name = \"cover\"\n"
    "parts = [\"cover\"]\n"
    "rule = \"root\"\n"
    "\n"
    "[[place]]\n"
    "name = \"poles\"\n"
    "parts = [\"pole\"]\n"
    "rule = \"span\"\n"
    "count = 10\n"
    "joint = \"foot\"\n"
    "to_joint = \"bind\"\n"
    "radius = \"1.85 m\"\n"
    "height = \"2.7 m\"\n"
    "jitter_radius = \"2%\"\n"
    "jitter_size = \"10%\"\n"
    "\n"
    "[[place]]\n"
    "name = \"stones\"\n"
    "parts = [\"stone_a\", \"stone_b\"]\n"
    "rule = \"ring\"\n"
    "count = 24\n"
    "radius = \"2.1 m\"\n"
    "jitter_turn = 3\n"
    "\n"
    "[[place]]\n"
    "name = \"binding\"\n"
    "parts = [\"binding\"]\n"
    "rule = \"plug\"\n"
    "joint = \"centre\"\n"
    "onto = \"cover.apex\"\n";

std::vector<data::SourceFile> with_art(const std::string& model) {
    std::vector<data::SourceFile> files = kd::test::good();
    files.push_back({"art/source.toml", kArt});
    files.push_back({"art/textures/birch/record.toml", kTexture});
    files.push_back({"art/textures/pine/record.toml", kTexture});
    files.push_back({"art/models/hide_tent_cone/record.toml", model});
    return files;
}

std::vector<std::string> problems_of(const std::string& model) {
    data::Catalogue cat;
    std::vector<std::string> out;
    for (const data::Problem& p : cat.load(with_art(model))) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

// The checks' problems for a recipe that loads whole.
std::vector<std::string> checked_problems(const std::string& model) {
    data::Catalogue cat;
    REQUIRE(cat.load(with_art(model)).empty());
    std::vector<std::string> out;
    for (const data::Problem& p : data::run_checks(cat)) {
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

// checks: PRE-46 PRE-42 MAT-13
TEST_CASE(
    "a model's recipe is an entry named by its folder, with each placement's quantities read into whole units "
    "and each role's textures linked") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_art(kTent)).empty());
    const auto& models = cat.kind<look::Model>();
    REQUIRE(models.size() == 1);
    CHECK(models.name(0) == "art:hide_tent_cone");
    const look::Model& tent = models[0];
    CHECK(tent.family == "camp");
    REQUIRE(tent.places.size() == 4);
    const look::ModelPlace& poles = tent.places[1];
    CHECK(poles.rule == "span");
    CHECK(poles.count == 10);
    CHECK(poles.radius == 1850);
    CHECK(poles.height == 2700);
    CHECK(poles.jitter_radius == 20'000);
    CHECK(poles.jitter_size == 100'000);
    CHECK(tent.places[0].count == 1);
    CHECK(tent.places[2].parts == std::vector<std::string>{"stone_a", "stone_b"});
    CHECK(tent.places[2].jitter_turn == 3);
    CHECK(tent.places[3].onto == "cover.apex");
    // the links name the texture entries, resolved
    REQUIRE(tent.materials.size() == 2);
    CHECK(tent.materials[0].textures[0].name == "art:birch");
    CHECK(tent.materials[0].textures[1].name == "art:pine");
    const auto& textures = cat.kind<look::Texture>();
    CHECK(textures.name(tent.materials[0].textures[1].index) == "art:pine");
    // and the recipe passes the catalogue's checks
    CHECK(data::run_checks(cat).empty());
}

// checks: PRE-46 MAT-14
TEST_CASE("a model counts only in its source's look digest, and a placement's change changes it") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_art(kTent)).empty());
    const data::EntryDigests before = cat.kind<look::Model>().digests(0);
    CHECK(before.fields[0] == 0);  // rules
    CHECK(before.fields[1] == 0);  // world
    CHECK(before.fields[2] == 6);
    data::Catalogue changed;
    REQUIRE(changed.load(with_art(replaced(kTent, "count = 24", "count = 23"))).empty());
    const data::EntryDigests after = changed.kind<look::Model>().digests(0);
    CHECK(after.by_affects[2] != before.by_affects[2]);
    CHECK(after.by_affects[0] == before.by_affects[0]);
}

// checks: PRE-46 MAT-13
TEST_CASE(
    "a fault in a model's recipe is refused where it is written: a field missing, unknown, out of range or of "
    "the wrong kind, and a texture no entry has") {
    CHECK(names(problems_of(replaced(kTent, "family = \"camp\"\n", "")), "\"family\" is missing"));
    CHECK(names(problems_of(replaced(kTent, "rule = \"root\"", "rule = \"hang\"")), "rule"));
    CHECK(names(problems_of(replaced(kTent, "count = 24", "count = 0")), "out of its range"));
    CHECK(names(problems_of(replaced(kTent, "radius = \"2.1 m\"\n", "radius = \"2.1 m\"\nwhy = 1\n")), "why"));
    CHECK(names(problems_of(replaced(kTent, "radius = \"2.1 m\"", "radius = \"2.1 kg\"")), "radius"));
    CHECK(names(problems_of(replaced(kTent, "\"art:pine\"", "\"art:oak\"")), "names no entry of kind textures"));
    CHECK(names(problems_of(replaced(kTent, "textures = [\"art:birch\", \"art:pine\"]",
                                     "textures = [\"art:birch\", \"art:birch\"]")),
                "listed twice"));
}

// checks: PRE-46 MAT-17
TEST_CASE(
    "the catalogue's check refuses a recipe whose placements are named twice, plug into what comes later, or lack "
    "what their rule needs") {
    CHECK(checked_problems(kTent).empty());
    CHECK(names(checked_problems(replaced(kTent, "name = \"stones\"", "name = \"poles\"")), "is named twice"));
    CHECK(names(checked_problems(replaced(kTent, "name = \"stones\"", "name = \"stone.s\"")), "holds no \".\""));
    CHECK(names(checked_problems(replaced(kTent, "onto = \"cover.apex\"", "onto = \"roof.apex\"")),
                "not placed before it"));
    CHECK(names(checked_problems(replaced(kTent, "onto = \"cover.apex\"", "onto = \"cover\"")), "\"placement.joint\""));
    CHECK(names(checked_problems(replaced(kTent, "joint = \"centre\"\n", "")), "needs the joint of the part"));
    CHECK(names(checked_problems(replaced(kTent, "to_joint = \"bind\"\n", "")), "needs both joints"));
    CHECK(names(checked_problems(replaced(kTent, "to_joint = \"bind\"", "to_joint = \"foot\"")), "at both ends"));
    CHECK(names(checked_problems(replaced(kTent, "height = \"2.7 m\"\n", "")), "needs a height above 0"));
    CHECK(names(checked_problems(replaced(kTent, "radius = \"2.1 m\"\n", "")), "needs a radius above 0"));
    CHECK(names(checked_problems(replaced(kTent, "rule = \"root\"", "rule = \"root\"\ncount = 2")), "is one copy"));
    CHECK(names(checked_problems(replaced(kTent, "parts = [\"cover\"]", "parts = []")), "names no part"));
    CHECK(names(checked_problems(replaced(kTent, "role = \"hide\"", "role = \"wood\"")), "is given twice"));
    CHECK(names(checked_problems(replaced(kTent, "textures = [\"art:birch\"]\n", "textures = []\n")),
                "lists no texture"));
}

// checks: PRE-46
TEST_CASE("the art lane's recipes load beside its textures from art/, in the order of their folders") {
    const std::vector<data::SourceFile> art = data::read_art(KD_REPO);
    for (const data::SourceFile& f : art) {
        CHECK(
            (f.path == "art/source.toml" || f.path.starts_with("art/textures/") || f.path.starts_with("art/models/")));
        if (f.path.starts_with("art/models/")) {
            CHECK(f.path.ends_with("/record.toml"));
        }
    }
}
