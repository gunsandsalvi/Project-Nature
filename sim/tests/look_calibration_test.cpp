#include <algorithm>
#include <string>
#include <vector>

#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/look/calibration.hpp"
#include "kd/num/letters.hpp"

namespace look = kd::look;

namespace {

// A scene that passes, for the tests to break one line at a time.
const std::string kGood = R"(about = "What a frame costs"
checks = ["PLT-04"]
draws = "nothing"
path = "still"
measure = "gpu"
decides = "msaa2"
line = 1100
expect_from = 900
expect_to = 1700

[[variant]]
name = "msaa2"

[[variant]]
name = "bare"
interface = false

[[decision]]
at_most = 2000
then = "keep"

[[decision]]
then = "look at the interface"
)";

std::string replaced(std::string text, const std::string& from, const std::string& to) {
    const std::size_t at = text.find(from);
    REQUIRE(at != std::string::npos);
    return text.replace(at, from.size(), to);
}

// The problems a scene's text has, in words.
std::vector<std::string> problems_of(const std::string& text) {
    std::vector<std::string> out;
    for (const kd::data::Problem& p : look::read_calibration(text, "data/scenes/look/c4.toml").problems) {
        out.push_back(p.what);
    }
    return out;
}

bool names(const std::vector<std::string>& problems, const std::string& words) {
    return std::any_of(problems.begin(), problems.end(),
                       [&](const std::string& p) { return p.find(words) != std::string::npos; });
}

look::CalibrationScene scene_of(const std::string& text, const std::string& path) {
    const look::CalibrationRead r = look::read_calibration(text, path);
    REQUIRE(r.problems.empty());
    return r.scene;
}

// A material scene taking C4's bare frame off, and a slope over rocks.
const std::string kMaterial = R"(about = "What the material costs"
checks = ["PLT-04", "PRE-01"]
draws = "field"
path = "still"
measure = "gpu"
decides = "msaa2"
minus = "c4/bare"
line = 2300
expect_from = 1700
expect_to = 4000

[[variant]]
name = "msaa2"

[[decision]]
at_most = 2500
then = "keep full resolution"

[[decision]]
at_most = 4000
then = "build the patch"

[[decision]]
then = "simplify"
)";

const std::string kRocks = R"(about = "What a triangle costs"
checks = ["PLT-04"]
draws = "rocks"
path = "still"
measure = "gpu_per_triangle"
line = 4000
expect_from = 2500
expect_to = 10000

[[variant]]
name = "rocks100"
triangles = 100
shadows = false

[[variant]]
name = "rocks200"
triangles = 200
shadows = false

[[variant]]
name = "rocks400"
triangles = 400
shadows = false

[[variant]]
name = "rocks800"
triangles = 800
shadows = false

[[variant]]
name = "rocks800-shadowed"
triangles = 800

[[decision]]
at_most = 4000
then = "0.6 million"

[[decision]]
at_most = 6000
then = "0.4 million"

[[decision]]
then = "0.3 million"
)";

// Leaves deciding by the least of three ways, less their bare ground; and figures by a slope over Godot's skeletons.
const std::string kLeaves = R"(about = "What leaves cost"
step = "α2.2b"
checks = ["PLT-04", "PRE-46"]
draws = "leaves"
path = "still"
measure = "gpu"
least_of = ["plain", "close", "cores"]
minus = "c2/none"
line = 800
expect_from = 600
expect_to = 6000

[[variant]]
name = "none"
way = "none"

[[variant]]
name = "plain"
way = "plain"

[[variant]]
name = "close"
way = "close"

[[variant]]
name = "cores"
way = "cores"

[[variant]]
name = "coverage"
way = "coverage"

[[decision]]
at_most = 1500
then = "no pre-pass"

[[decision]]
then = "the pre-pass"
)";

const std::string kFigures = R"(about = "What a figure costs the main thread"
checks = ["PLT-04", "PRE-27"]
draws = "figures"
path = "still"
measure = "cpu_per_figure"
line = 100000
expect_from = 25000
expect_to = 67000

[[variant]]
name = "godot30"
figures = 30
way = "godot"

[[variant]]
name = "godot100"
figures = 100
way = "godot"

[[variant]]
name = "palette100"
figures = 100
way = "palette"

[[decision]]
at_most = 25000
then = "forty skeletons"

[[decision]]
then = "fewer skeletons"
)";

std::vector<look::CalibrationScene> three() {
    return {scene_of(kMaterial, "data/scenes/look/c1.toml"), scene_of(kRocks, "data/scenes/look/c3.toml"),
            scene_of(kGood, "data/scenes/look/c4.toml")};
}

look::CalibrationReading gpu(std::int64_t us) {
    look::CalibrationReading r;
    r.gpu_us = us;
    r.cpu_us = 400;
    r.on_time = 998;
    r.power_mw = 3210;
    r.heat = 45;
    return r;
}

}  // namespace

// checks: PLT-04, RES-09
TEST_CASE("a calibration scene's file states its items, variants, number, line, estimate and decisions") {
    const look::CalibrationRead r = look::read_calibration(kGood, "data/scenes/look/c4.toml");
    REQUIRE(r.problems.empty());
    const look::CalibrationScene& s = r.scene;
    CHECK(s.name == "c4");
    CHECK(s.checks == std::vector<std::string>{"PLT-04"});
    CHECK(s.draws == "nothing");
    CHECK(s.measure == "gpu");
    CHECK(s.decides == "msaa2");
    CHECK(s.line == 1100);
    CHECK(s.expect_from == 900);
    CHECK(s.expect_to == 1700);
    REQUIRE(s.variants.size() == 2);
    // a variant's switches are the game's own unless it says otherwise: MSAA 2×, full scale, the interface and shadows
    CHECK(s.variants[0].msaa == 2);
    CHECK(s.variants[0].scale == 100);
    CHECK(s.variants[0].interface);
    CHECK(s.variants[0].shadows);
    CHECK_FALSE(s.variants[1].interface);
    REQUIRE(s.decisions.size() == 2);
    CHECK(s.decisions[0].at_most == 2000);
    CHECK_FALSE(s.decisions[1].at_most.has_value());
}

// checks: PLT-04, RES-09
TEST_CASE("the runner refuses a scene without its items, a variant, its number, its line, its estimate or a decision") {
    CHECK(names(problems_of(replaced(kGood, "checks = [\"PLT-04\"]\n", "")), "\"checks\" is missing"));
    CHECK(names(problems_of(replaced(kGood, "checks = [\"PLT-04\"]", "checks = []")), "names the items it serves"));
    CHECK(names(problems_of(replaced(kGood, "\"PLT-04\"", "\"plt4\"")), "is not an item's ID"));
    CHECK(names(problems_of(replaced(kGood, "line = 1100\n", "")), "\"line\" is missing"));
    CHECK(names(problems_of(replaced(kGood, "expect_to = 1700\n", "")), "\"expect_to\" is missing"));
    CHECK(names(problems_of(replaced(kGood, "expect_to = 1700", "expect_to = 800")), "below its low end"));
    CHECK(names(problems_of(replaced(kGood, "measure = \"gpu\"\n", "")), "\"measure\" is missing"));
    CHECK(names(problems_of(replaced(kGood, "decides = \"msaa2\"", "decides = \"msaa4\"")), "names none"));
    // no variant, and no decision
    const std::string bare = kGood.substr(0, kGood.find("[[variant]]"));
    CHECK(names(problems_of(bare), "\"variant\" is missing"));
    CHECK(names(problems_of(bare), "\"decision\" is missing"));
    CHECK(names(problems_of(bare + "variant = []\ndecision = []\n"), "a variant at least"));
    CHECK(names(problems_of(bare + "variant = []\ndecision = []\n"), "a decision for every number"));
}

// checks: PLT-04, RES-09
TEST_CASE("the decisions rise, and only the last takes every number, so each number makes exactly one") {
    CHECK(names(problems_of(replaced(kGood, "then = \"look at the interface\"",
                                     "at_most = 3000\nthen = \"look at the interface\"")),
                "the last takes every number"));
    CHECK(names(problems_of(replaced(kGood, "at_most = 2000\n", "")), "every decision but the last"));
    const std::string three = replaced(kGood, "[[decision]]\nthen = \"look",
                                       "[[decision]]\nat_most = 1500\nthen = \"lower\"\n\n[[decision]]\nthen = \"look");
    CHECK(names(problems_of(three), "above the one before"));
    CHECK(names(problems_of(replaced(kGood, "msaa2\"\n\n[[variant]]\nname = \"bare\"",
                                     "msaa2\"\nmsaa = 3\n\n"
                                     "[[variant]]\nname = \"bare\"")),
                "MSAA takes 0, 2 or 4"));
    CHECK(names(problems_of(replaced(kGood, "name = \"bare\"", "name = \"msaa2\"")), "named twice"));
    // a slope needs two variants with triangles and no shadow pass, and takes nothing off
    CHECK_FALSE(names(problems_of(replaced(kRocks, "triangles = 200\nshadows = false", "triangles = 200")),
                      "needs two variants"));
    std::string one = kRocks;
    for (const char* t : {"200", "400", "800"}) {
        one = replaced(one, std::string("triangles = ") + t + "\nshadows = false", std::string("triangles = ") + t);
    }
    CHECK(names(problems_of(one), "needs two variants"));
    CHECK(names(problems_of(replaced(kRocks, "line = 4000", "minus = \"c4/bare\"\nline = 4000")), "takes nothing off"));
    // rocks give their triangles, copies their copies, and no other scene either
    CHECK(names(problems_of(replaced(kGood, "name = \"bare\"", "name = \"bare\"\ntriangles = 100")),
                "rocks give each variant its triangles"));
}

// checks: PLT-04
TEST_CASE("a set of scenes is read in the order of their names, and a minus naming nothing is caught") {
    const look::CalibrationSet set =
        look::read_calibrations({{"scenes/look/c4.toml", kGood}, {"scenes/look/c1.toml", kMaterial}});
    CHECK(set.problems.empty());
    REQUIRE(set.scenes.size() == 2);
    CHECK(set.scenes[0].name == "c1");
    CHECK(set.scenes[1].name == "c4");
    // without C4, C1's minus names nothing
    const look::CalibrationSet alone = look::read_calibrations({{"scenes/look/c1.toml", kMaterial}});
    REQUIRE(alone.problems.size() == 1);
    CHECK(alone.problems[0] == "c1: minus \"c4/bare\" names no scene and variant here");
    // a file with a problem is left out and named
    const look::CalibrationSet broken =
        look::read_calibrations({{"scenes/look/c4.toml", replaced(kGood, "line = 1100\n", "")}});
    CHECK(broken.scenes.empty());
    REQUIRE(broken.problems.size() == 1);
    CHECK(broken.problems[0].find("scenes/look/c4.toml:") == 0);
}

// checks: PLT-04
TEST_CASE("each scene's number is its deciding time less its minus's, or the slope over its triangles") {
    const std::vector<look::CalibrationScene> scenes = three();
    // C1's material at 3.5 ms less C4's bare frame of 0.3 ms; rocks at 2 µs a thousand triangles over 1.8 ms;
    // C4's frame at 1.2 ms
    const std::vector<std::vector<look::CalibrationReading>> readings{
        {gpu(3500)},
        {gpu(2000), gpu(2200), gpu(2600), gpu(3400), gpu(9000)},
        {gpu(1200), gpu(300)},
    };
    const std::vector<look::CalibrationVerdict> v = look::calibration_verdicts(scenes, readings);
    REQUIRE(v.size() == 3);
    CHECK(v[0].read);
    CHECK(v[0].number == 3200);
    CHECK_FALSE(v[0].in_line);
    CHECK(v[0].expected);
    CHECK(v[0].then == "build the patch");
    // the shadowed variant is left out of the slope: 2 µs a thousand triangles is 2000 ps a triangle
    CHECK(v[1].number == 2000);
    CHECK(v[1].in_line);
    CHECK_FALSE(v[1].expected);
    CHECK(v[1].then == "0.6 million");
    CHECK(v[2].number == 1200);
    CHECK(v[2].then == "keep");
    // past the last at_most, the last decision
    std::vector<std::vector<look::CalibrationReading>> slow = readings;
    slow[0][0] = gpu(6000);
    CHECK(look::calibration_verdicts(scenes, slow)[0].then == "simplify");
}

// checks: PLT-04
TEST_CASE("a number whose readings the phone did not give decides nothing") {
    const std::vector<look::CalibrationScene> scenes = three();
    std::vector<std::vector<look::CalibrationReading>> readings{
        {gpu(3500)},
        {gpu(2000), gpu(-1), gpu(-1), gpu(-1), gpu(9000)},
        {gpu(1200), gpu(-1)},
    };
    const std::vector<look::CalibrationVerdict> v = look::calibration_verdicts(scenes, readings);
    // C1's minus was not measured, and the slope has one point left
    CHECK_FALSE(v[0].read);
    CHECK(v[0].then.empty());
    CHECK_FALSE(v[1].read);
    CHECK(v[2].read);
    CHECK(look::verdict_words(scenes[0], v[0]) == "c1: not measured, so it decides nothing yet");
    // two points still make a slope
    readings[1][3] = gpu(3400);
    CHECK(look::calibration_verdicts(scenes, readings)[1].number == 2000);
}

// checks: PLT-04
TEST_CASE("the readings' code reads back as written, a missing reading included") {
    const std::vector<look::CalibrationScene> scenes = three();
    look::CalibrationReading missing = gpu(65'000);
    missing.power_mw = -1;
    missing.heat = -1;
    look::CalibrationReading high = gpu(1'000'000);  // past its field: held at the most it takes
    high.on_time = 1000;
    const std::vector<std::vector<look::CalibrationReading>> readings{
        {gpu(3500)},
        {gpu(0), gpu(2200), high, missing, gpu(9000)},
        {gpu(1200), gpu(300)},
    };
    const std::string code = look::calibration_code(30201, scenes, readings);
    const look::CalibrationCodeRead r = look::read_calibration_code(code, scenes);
    REQUIRE(r.why.empty());
    CHECK(r.build == 30201);
    REQUIRE(r.readings.size() == 3);
    CHECK(r.readings[0][0].gpu_us == 3500);
    CHECK(r.readings[1][0].gpu_us == 0);
    CHECK(r.readings[1][2].gpu_us == 65'534);
    CHECK(r.readings[1][2].on_time == 1000);
    CHECK(r.readings[1][3].gpu_us == 65'000);
    CHECK(r.readings[1][3].power_mw == -1);
    CHECK(r.readings[1][3].heat == -1);
    CHECK(r.readings[2][1].cpu_us == 400);
    CHECK(r.readings[2][1].on_time == 998);
    CHECK(r.readings[2][1].power_mw == 3210);
    CHECK(r.readings[2][1].heat == 45);
    // its letters in lower case and broken across lines read the same
    std::string lower = code;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c); });
    CHECK(look::read_calibration_code(lower.substr(0, 20) + "\n" + lower.substr(20), scenes).why.empty());
}

// checks: PLT-04
TEST_CASE("a code written for other scenes, of another layout, or with a wrong letter is refused") {
    const std::vector<look::CalibrationScene> scenes = three();
    const std::vector<std::vector<look::CalibrationReading>> readings{
        {gpu(3500)},
        {gpu(2000), gpu(2200), gpu(2600), gpu(3400), gpu(9000)},
        {gpu(1200), gpu(300)},
    };
    const std::string code = look::calibration_code(30201, scenes, readings);
    // C4 without its bare frame: the code is too long for these scenes
    std::vector<look::CalibrationScene> fewer = scenes;
    fewer[2].variants.pop_back();
    CHECK(look::read_calibration_code(code, fewer).why.find("the scenes it was written for differ") !=
          std::string::npos);
    // one wrong letter
    std::string wrong = code;
    wrong[3] = wrong[3] == 'A' ? 'B' : 'A';
    CHECK_FALSE(look::read_calibration_code(wrong, scenes).why.empty());
    CHECK(look::read_calibration_code("", scenes).why.size() > 0);
}

// checks: PLT-04
TEST_CASE("a variant's readings and a scene's verdict in words, the same on the phone and in the cloud") {
    const std::vector<look::CalibrationScene> scenes = three();
    CHECK(look::reading_words(scenes[2].variants[0], gpu(1235)) ==
          "msaa2: the graphics chip 1.24 ms, the main thread 0.40 ms, 99.8% of frames on time, 3.21 W, heat 0.45");
    look::CalibrationReading none;
    CHECK(look::reading_words(scenes[2].variants[1], none) ==
          "bare: the graphics chip not measured, the main thread not measured, frames on time not measured, power "
          "not measured, heat not measured");
    const std::vector<std::vector<look::CalibrationReading>> readings{
        {gpu(3500)},
        {gpu(2000), gpu(2200), gpu(2600), gpu(3400), gpu(9000)},
        {gpu(1200), gpu(300)},
    };
    const std::vector<look::CalibrationVerdict> v = look::calibration_verdicts(scenes, readings);
    CHECK(look::verdict_words(scenes[0], v[0]) ==
          "c1: the graphics chip 3.20 ms for msaa2 less c4/bare, over its line of 2.30 ms, within its estimate of "
          "1.70 to 4.00 ms; so: build the patch");
    CHECK(look::verdict_words(scenes[1], v[1]) ==
          "c3: the graphics chip 2.00 ns a triangle in the main pass, within its line of 4.00 ns a triangle, outside "
          "its estimate of 2.50 to 10.00 ns a triangle; so: 0.6 million");
    // a number below nothing, as noise may leave a difference, keeps its sign
    std::vector<std::vector<look::CalibrationReading>> noisy = readings;
    noisy[0][0] = gpu(250);
    CHECK(look::verdict_words(scenes[0], look::calibration_verdicts(scenes, noisy)[0]).find("-0.05 ms") !=
          std::string::npos);
}

// checks: PLT-04, PRE-01, PRE-46, RES-09
TEST_CASE("the calibration scenes in data/scenes/look read whole, with the lines and decisions the plan states") {
    const look::CalibrationSet set = look::read_calibrations(kd::data::read_files_in(KD_REPO "/data", "scenes/look"));
    std::string problems;
    for (const std::string& p : set.problems) {
        problems += p + "\n";
    }
    INFO(problems);
    REQUIRE(set.problems.empty());
    std::vector<std::string> named;
    named.reserve(set.scenes.size());
    for (const look::CalibrationScene& s : set.scenes) {
        named.push_back(s.name);
    }
    CHECK(named == std::vector<std::string>{"c1", "c2", "c3", "c3-draws", "c4"});
    const auto scene = [&](const std::string& name) -> const look::CalibrationScene& {
        const auto it = std::find_if(set.scenes.begin(), set.scenes.end(),
                                     [&](const look::CalibrationScene& s) { return s.name == name; });
        REQUIRE(it != set.scenes.end());
        return *it;
    };
    const auto at_most = [](const look::CalibrationScene& s) {
        std::vector<std::int64_t> out;
        out.reserve(s.decisions.size());
        for (const look::CalibrationDecision& d : s.decisions) {
            out.push_back(d.at_most.value_or(-1));
        }
        return out;
    };
    // IMPLEMENTATION.md's α2.2a: C1 at most 2.5 ms, 2.5 to 4.0 and over; C3 at most 4 ns, 4 to 6 and over; C4 over 2.0
    const look::CalibrationScene& c1 = scene("c1");
    const look::CalibrationScene& c3 = scene("c3");
    const look::CalibrationScene& c4 = scene("c4");
    CHECK(c1.step == "α2.2a");
    CHECK(c1.line == 2300);
    CHECK(at_most(c1) == std::vector<std::int64_t>{2500, 4000, -1});
    CHECK(c1.minus == "c4/bare-msaa2");
    CHECK(c3.measure == "gpu_per_triangle");
    CHECK(at_most(c3) == std::vector<std::int64_t>{4000, 6000, -1});
    CHECK(scene("c3-draws").measure == "cpu");
    CHECK(c4.line == 1100);
    CHECK(at_most(c4) == std::vector<std::int64_t>{2000, -1});
    // the estimates: C4 0.9 to 1.7 ms, C1 1.7 to 4.0, C3 2.5 to 10 ns a triangle
    CHECK(c4.expect_from == 900);
    CHECK(c4.expect_to == 1700);
    CHECK(c1.expect_from == 1700);
    CHECK(c1.expect_to == 4000);
    CHECK(c3.expect_from == 2500);
    CHECK(c3.expect_to == 10000);
    // the rocks' triangles a pass: 100, 200, 400 and 800 thousand, each with the shadow pass and without
    std::vector<std::int64_t> triangles;
    triangles.reserve(c3.variants.size());
    for (const look::CalibrationVariant& v : c3.variants) {
        triangles.push_back(v.triangles);
    }
    CHECK(triangles == std::vector<std::int64_t>{100, 200, 400, 800, 100, 200, 400, 800});
    // α2.2b: C2's leaves drawn every way, the cheapest of those that look like plain cards less the bare ground,
    // expected 0.6 to 6.0 ms against A18.1's 0.8, and the leaf pre-pass built if it is over 1.5 ms
    const look::CalibrationScene& c2 = scene("c2");
    CHECK(c2.step == "α2.2b");
    std::vector<std::string> ways;
    ways.reserve(c2.variants.size());
    for (const look::CalibrationVariant& v : c2.variants) {
        ways.push_back(v.way);
    }
    CHECK(ways == std::vector<std::string>{"none", "plain", "close", "cores", "coverage"});
    CHECK(c2.least_of == std::vector<std::string>{"plain", "close", "cores"});
    CHECK(c2.minus == "c2/none");
    CHECK(c2.line == 800);
    CHECK(c2.expect_from == 600);
    CHECK(c2.expect_to == 6000);
    CHECK(at_most(c2) == std::vector<std::int64_t>{1500, -1});
    CHECK(c2.decisions[1].then.find("leaf pre-pass is built") != std::string::npos);
}

// checks: PLT-04, RES-09
TEST_CASE("a scene's variants are drawn in its own ways and give its own counts") {
    const look::CalibrationScene leaves = scene_of(kLeaves, "data/scenes/look/c2.toml");
    CHECK(leaves.step == "α2.2b");
    CHECK(leaves.least_of == std::vector<std::string>{"plain", "close", "cores"});
    CHECK(leaves.variants[4].way == "coverage");
    CHECK(scene_of(kGood, "data/scenes/look/c4.toml").step.empty());
    CHECK(look::calibration_ways("fires") == std::vector<std::string_view>{"none", "walk", "walk-half", "map"});
    CHECK(look::calibration_ways("rocks").empty());
    // a way that is not the scene's, and a way where none is drawn
    CHECK(names(problems_of(replaced(kLeaves, "way = \"close\"", "way = \"map\"")),
                "leaves are drawn none, plain, close, cores or coverage, not \"map\""));
    CHECK(names(problems_of(replaced(kGood, "name = \"bare\"", "name = \"bare\"\nway = \"plain\"")),
                "only leaves, fires and figures are drawn a way"));
    // fires give their fires, and reads their vertices
    CHECK(names(problems_of(replaced(kLeaves, "draws = \"leaves\"", "draws = \"fires\"")), "fires their fires"));
    CHECK(names(problems_of(replaced(kGood, "name = \"bare\"", "name = \"bare\"\nreads = 3")), "reads their vertices"));
    // the least of several names them, and never with decides as well
    CHECK(names(problems_of(replaced(kLeaves, "\"cores\"]", "\"core\"]")), "\"core\" names none"));
    CHECK(names(problems_of(replaced(kLeaves, "minus = ", "decides = \"plain\"\nminus = ")), "not both"));
    // a figure's slope needs two variants on Godot's skeletons
    CHECK(names(problems_of(replaced(kFigures, "figures = 30\nway = \"godot\"", "figures = 30\nway = \"palette\"")),
                "two variants of figures on Godot's skeletons"));
    CHECK(names(problems_of(replaced(kFigures, "line = 100000", "minus = \"c4/bare\"\nline = 100000")),
                "takes nothing off"));
}

// checks: PLT-04
TEST_CASE("the least of several ways decides, and a figure's cost is the slope over Godot's skeletons") {
    const std::vector<look::CalibrationScene> scenes{scene_of(kLeaves, "data/scenes/look/c2.toml"),
                                                     scene_of(kFigures, "data/scenes/look/c6.toml")};
    look::CalibrationReading slow = gpu(1800);
    std::vector<std::vector<look::CalibrationReading>> readings{
        {gpu(1000), gpu(2600), gpu(2200), gpu(2000), slow},
        {gpu(0), gpu(0), gpu(0)},
    };
    // the main thread: 2.0 ms for 30 figures and 4.8 for 100, 40 µs a figure; the palettes' are left out
    readings[1][0].cpu_us = 2000;
    readings[1][1].cpu_us = 4800;
    readings[1][2].cpu_us = 100;
    std::vector<look::CalibrationVerdict> v = look::calibration_verdicts(scenes, readings);
    // cores, the least of the three, less the bare ground; alpha to coverage is not among them
    CHECK(v[0].number == 1000);
    CHECK(v[0].then == "no pre-pass");
    CHECK(v[1].number == 40000);
    CHECK(v[1].then == "fewer skeletons");
    CHECK(look::verdict_words(scenes[0], v[0]) ==
          "c2: the graphics chip 1.00 ms for the least of plain, close and cores less c2/none, over its line of 0.80 "
          "ms, within its estimate of 0.60 to 6.00 ms; so: no pre-pass");
    CHECK(look::verdict_words(scenes[1], v[1]) ==
          "c6: the main thread 40.00 µs a figure on Godot's skeletons, within its line of 100.00 µs a figure, within "
          "its estimate of 25.00 to 67.00 µs a figure; so: fewer skeletons");
    // a way the phone did not time is left out of the least
    readings[0][3] = gpu(-1);
    CHECK(look::calibration_verdicts(scenes, readings)[0].number == 1200);
}

// checks: PLT-04
TEST_CASE("a code holds the scenes run and only them, and α2.2a's codes, which held every scene, still read") {
    std::vector<look::CalibrationScene> scenes = three();
    const std::vector<std::vector<look::CalibrationReading>> readings{
        {},
        {gpu(2000), gpu(2200), gpu(2600), gpu(3400), gpu(9000)},
        {gpu(1200), gpu(300)},
    };
    const std::string code = look::calibration_code(30202, scenes, readings);
    const look::CalibrationCodeRead r = look::read_calibration_code(code, scenes);
    REQUIRE(r.why.empty());
    CHECK(r.build == 30202);
    REQUIRE(r.readings.size() == 3);
    CHECK(r.readings[0].empty());
    CHECK(r.readings[1][3].gpu_us == 3400);
    CHECK(r.readings[2][1].gpu_us == 300);
    const std::vector<look::CalibrationVerdict> v = look::calibration_verdicts(scenes, r.readings);
    CHECK_FALSE(v[0].ran);
    CHECK(look::verdict_words(scenes[0], v[0]) == "c1: not run");
    CHECK(v[1].ran);
    CHECK(v[1].number == 2000);
    // α2.2a's layout 1: the version, the build, then every scene's readings
    std::vector<bool> bits;
    kd::num::put_bits(bits, 1, 8);
    kd::num::put_bits(bits, 30201, 17);
    for (const std::int64_t us : {3500, 2000, 2200, 2600, 3400, 9000, 1200, 300}) {
        for (const auto& [value, width] :
             std::vector<std::pair<std::int64_t, unsigned>>{{us + 1, 16}, {401, 16}, {999, 11}, {3211, 15}, {46, 9}}) {
            kd::num::put_bits(bits, static_cast<std::uint64_t>(value), width);
        }
    }
    const look::CalibrationCodeRead old = look::read_calibration_code(kd::num::write_letters(bits), scenes);
    REQUIRE(old.why.empty());
    CHECK(old.build == 30201);
    CHECK(old.readings[0][0].gpu_us == 3500);
    CHECK(old.readings[2][1].heat == 45);
    // a code that ran a scene these files lack
    scenes.pop_back();
    const std::string ran_c4 = look::calibration_code(1, three(), {{}, {}, {gpu(1200), gpu(300)}});
    CHECK(look::read_calibration_code(ran_c4, scenes).why.find("a scene these files do not have") != std::string::npos);
}
