#include <string>
#include <vector>

#include "catalogue_files.hpp"
#include "doctest.h"
#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/folder.hpp"
#include "kd/look/light_tuning.hpp"

namespace data = kd::data;
namespace look = kd::look;

namespace {

// The light as base/tuning/light.toml writes it.
constexpr const char* kLight =
    "sun_height = 28\n"
    "sun_turn = 215\n"
    "sun_colour = \"#ffd9a0\"\n"
    "sun_energy = \"125%\"\n"
    "sky_colour = \"#b8c8d4\"\n"
    "ambient_colour = \"#b5b8bd\"\n"
    "ambient_energy = \"45%\"\n"
    "haze_colour = \"#a8bccb\"\n"
    "haze_sun_colour = \"#f2cf9e\"\n"
    "haze_density = \"0.25%\"\n"
    "bounce_colour = \"#3a3318\"\n"
    "maps_texel = \"6 cm\"\n"
    "open_reach = \"2 m\"\n"
    "open_strength = \"80%\"\n"
    "contact_width = \"18 cm\"\n"
    "contact_strength = \"55%\"\n"
    "shadow_reach = \"14 m\"\n"
    "rim = \"45%\"\n";

std::vector<data::SourceFile> with_light(const std::string& light) {
    std::vector<data::SourceFile> files = kd::test::good();
    files.push_back({"base/tuning/light.toml", light});
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

std::vector<std::string> load_problems(const std::string& light) {
    data::Catalogue cat;
    std::vector<std::string> out;
    for (const data::Problem& p : cat.load(with_light(light))) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

std::vector<std::string> checked_problems(const std::string& light) {
    data::Catalogue cat;
    REQUIRE(cat.load(with_light(light)).empty());
    std::vector<std::string> out;
    for (const data::Problem& p : data::run_checks(cat)) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

}  // namespace

// checks: PRE-30 MAT-13
TEST_CASE("the light of the moment is one tuning entry, its quantities read into whole units") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_light(kLight)).empty());
    const auto& lights = cat.kind<look::LightTuning>();
    REQUIRE(lights.size() == 1);
    const look::LightTuning& l = lights[0];
    CHECK(l.sun_height == 28);
    CHECK(l.sun_turn == 215);
    CHECK(l.sun_colour == "#ffd9a0");
    CHECK(l.sun_energy == 1'250'000);
    CHECK(l.ambient_energy == 450'000);
    CHECK(l.haze_density == 2'500);
    CHECK(l.bounce_colour == "#3a3318");
    // the view's maps round things and the lit edge, in millimetres and parts of a million
    CHECK(l.maps_texel == 60);
    CHECK(l.open_reach == 2'000);
    CHECK(l.open_strength == 800'000);
    CHECK(l.contact_width == 180);
    CHECK(l.contact_strength == 550'000);
    CHECK(l.shadow_reach == 14'000);
    CHECK(l.rim == 450'000);
    CHECK(data::run_checks(cat).empty());
}

// checks: PRE-30 MAT-14
TEST_CASE("the light counts only in the look's digest, never in the world's rules, and a colour's change changes it") {
    data::Catalogue cat;
    REQUIRE(cat.load(with_light(kLight)).empty());
    const data::EntryDigests before = cat.kind<look::LightTuning>().digests(0);
    CHECK(before.fields[0] == 0);  // rules
    CHECK(before.fields[1] == 0);  // world
    CHECK(before.fields[2] == 18);
    data::Catalogue changed;
    REQUIRE(changed.load(with_light(replaced(kLight, "#ffd9a0", "#ffd9a1"))).empty());
    const data::EntryDigests after = changed.kind<look::LightTuning>().digests(0);
    CHECK(after.by_affects[2] != before.by_affects[2]);
    CHECK(after.by_affects[0] == before.by_affects[0]);
    CHECK(after.by_affects[1] == before.by_affects[1]);
}

// checks: PRE-30 MAT-13
TEST_CASE("a fault in the light is refused where it is written: a field missing, unknown or out of range") {
    CHECK(names(load_problems(replaced(kLight, "sun_turn = 215\n", "")), "\"sun_turn\" is missing"));
    CHECK(names(load_problems(replaced(kLight, "sun_height = 28", "sun_height = 0")), "out of its range"));
    CHECK(names(load_problems(replaced(kLight, "sun_energy = \"125%\"", "sun_energy = \"5%\"")), "out of its range"));
    CHECK(names(load_problems(replaced(kLight, "sun_height = 28\n", "sun_height = 28\nmoon = 1\n")), "moon"));
}

// checks: PRE-30 MAT-17
TEST_CASE("every colour of the light is written #rrggbb in lower case, and the check names the one that is not") {
    CHECK(checked_problems(kLight).empty());
    CHECK(names(checked_problems(replaced(kLight, "\"#ffd9a0\"", "\"#FFD9A0\"")), "sun_colour"));
    CHECK(names(checked_problems(replaced(kLight, "\"#b8c8d4\"", "\"b8c8d4\"")), "sky_colour"));
    CHECK(names(checked_problems(replaced(kLight, "\"#3a3318\"", "\"#3a33\"")), "bounce_colour"));
    CHECK(names(checked_problems(replaced(kLight, "\"#a8bccb\"", "\"#a8bccg\"")), "haze_colour"));
}

// checks: PRE-30
TEST_CASE("the game's own light passes the catalogue's checks") {
    data::Catalogue cat;
    std::string problems;
    for (const data::Problem& p : cat.load(data::read_catalogue(KD_REPO "/data"))) {
        problems += data::problem_text(p) + "\n";
    }
    INFO(problems);
    REQUIRE(problems.empty());
    CHECK(cat.kind<look::LightTuning>().size() == 1);
    for (const data::Problem& p : data::run_checks(cat)) {
        problems += data::problem_text(p) + "\n";
    }
    CHECK(problems.empty());
}
