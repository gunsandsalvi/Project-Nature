#include <algorithm>
#include <string>
#include <vector>

#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/look/calibration.hpp"

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

// checks: PLT-04, PRE-01, RES-09
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
    CHECK(named == std::vector<std::string>{"c1", "c3", "c3-draws", "c4"});
    const auto at_most = [](const look::CalibrationScene& s) {
        std::vector<std::int64_t> out;
        out.reserve(s.decisions.size());
        for (const look::CalibrationDecision& d : s.decisions) {
            out.push_back(d.at_most.value_or(-1));
        }
        return out;
    };
    // IMPLEMENTATION.md's α2.2a: C1 at most 2.5 ms, 2.5 to 4.0 and over; C3 at most 4 ns, 4 to 6 and over; C4 over 2.0
    CHECK(set.scenes[0].line == 2300);
    CHECK(at_most(set.scenes[0]) == std::vector<std::int64_t>{2500, 4000, -1});
    CHECK(set.scenes[0].minus == "c4/bare-msaa2");
    CHECK(set.scenes[1].measure == "gpu_per_triangle");
    CHECK(at_most(set.scenes[1]) == std::vector<std::int64_t>{4000, 6000, -1});
    CHECK(set.scenes[2].measure == "cpu");
    CHECK(set.scenes[3].line == 1100);
    CHECK(at_most(set.scenes[3]) == std::vector<std::int64_t>{2000, -1});
    // the estimates: C4 0.9 to 1.7 ms, C1 1.7 to 4.0, C3 2.5 to 10 ns a triangle
    CHECK(set.scenes[3].expect_from == 900);
    CHECK(set.scenes[3].expect_to == 1700);
    CHECK(set.scenes[0].expect_from == 1700);
    CHECK(set.scenes[0].expect_to == 4000);
    CHECK(set.scenes[1].expect_from == 2500);
    CHECK(set.scenes[1].expect_to == 10000);
    // the rocks' triangles a pass: 100, 200, 400 and 800 thousand, each with the shadow pass and without
    std::vector<std::int64_t> triangles;
    for (const look::CalibrationVariant& v : set.scenes[1].variants) {
        triangles.push_back(v.triangles);
    }
    CHECK(triangles == std::vector<std::int64_t>{100, 200, 400, 800, 100, 200, 400, 800});
}
