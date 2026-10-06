#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/demo/crowd_scene.hpp"
#include "kd/save/files.hpp"
#include "kd/scene/report.hpp"
#include "kd/scene/scene.hpp"

namespace {

using kd::test::fixture;

const std::array<kd::scene::WorldKind, 1> kKinds{kd::demo::crowd_kind()};

// A small scene of the crowd, as a file would state it, with what the test changes.
std::string scene_text(const std::string& extra = "", const std::string& until = "6 d",
                       const std::string& runs = "20") {
    return "about = \"the crowd greets every day and sleeps at night\"\n"
           "checks = [\"RES-21\", \"RES-12\"]\n"
           "world = \"crowd\"\n"
           "camps = 2\n"
           "seed = 40\n"
           "runs = " +
           runs + "\nuntil = \"" + until +
           "\"\n"
           "limit = \"1 min\"\n"
           "budget = \"0.1 h\"\n" +
           extra +
           "[pass]\n"
           "measure = \"greetings_per_camp_day\"\n"
           "at_least = 1\n"
           "in = 16\n"
           "[[expect]]\n"
           "measure = \"greetings_per_camp_day\"\n"
           "from = 5\n"
           "to = 25\n"
           "[[expect]]\n"
           "measure = \"awake_at_midnight\"\n"
           "from = 0\n"
           "to = 3\n"
           "[[never]]\n"
           "rule = \"farther_from_home\"\n"
           "limit = 5000\n"
           "[[never]]\n"
           "rule = \"awake_at_midnight\"\n"
           "limit = 2\n";
}

kd::scene::Scene scene_of(const std::string& text) {
    const kd::scene::Read r = kd::scene::read_scene(text, "data/scenes/greeting.toml", kKinds);
    for (const kd::data::Problem& p : r.problems) {
        INFO(kd::data::problem_text(p));
    }
    REQUIRE(r.problems.empty());
    return r.scene;
}

bool mentions(const std::vector<std::string>& oddities, const std::string& words) {
    return std::any_of(oddities.begin(), oddities.end(),
                       [&](const std::string& o) { return o.find(words) != std::string::npos; });
}

}  // namespace

// checks: RES-21 RES-09
TEST_CASE("a scene is read as written, and each mistake in its file is named at its line") {
    const kd::scene::Scene s = scene_of(scene_text("switches = [\"no_greetings\"]\n"));
    CHECK(s.name == "greeting");
    CHECK(s.checks == std::vector<std::string>{"RES-21", "RES-12"});
    CHECK(s.world == "crowd");
    CHECK(s.camps == 2);
    CHECK(s.runs == 20);
    CHECK(s.until == 6 * kd::time::kDay);
    CHECK(s.limit == 60);
    CHECK(s.budget == 360);
    CHECK(s.switches == std::vector<kd::world::Switch>{kd::world::Switch::no_greetings});
    CHECK(s.pass.measure == "greetings_per_camp_day");
    CHECK(s.pass.at_least == 1);
    CHECK_FALSE(s.pass.at_most.has_value());
    CHECK(s.pass.in == 16);
    CHECK(s.expects.size() == 2);
    CHECK(s.nevers.size() == 2);
    // each mistake named where it is
    const auto problems = [](const std::string& text) {
        std::vector<std::string> out;
        for (const kd::data::Problem& p : kd::scene::read_scene(text, "bad.toml", kKinds).problems) {
            out.push_back(kd::data::problem_text(p));
        }
        return out;
    };
    std::string text = scene_text();
    text.replace(text.find("\"RES-12\""), 8, "\"res12\"");
    CHECK(mentions(problems(text), "bad.toml:2:"));
    CHECK(mentions(problems(scene_text("switches = [\"no_such\"]\n")), "is not a test switch"));
    CHECK(mentions(problems(scene_text("speed = 3\n")), "\"speed\" is not a field"));
    text = scene_text();
    text.replace(text.find("greetings_per_camp_day"), 22, "smiles");
    CHECK(mentions(problems(text), "\"smiles\" is not one of"));
    text = scene_text();
    text.replace(text.find("at_least = 1\n"), 13, "");
    CHECK(mentions(problems(text), "a rule needs at_least, at_most or both"));
    text = scene_text();
    text.replace(text.find("from = 5\n"), 9, "from = 90\n");
    CHECK(mentions(problems(text), "back to 25"));
    text = scene_text();
    text.replace(text.find("until = \"6 d\""), 13, "until = \"6.5 kg\"");
    CHECK(!problems(text).empty());
}

// checks: RES-13 RES-09
TEST_CASE("a scene's runs are judged by its rule, a failed rule on 20 more, and fewer runs only provisionally") {
    const kd::scene::Scene s = scene_of(scene_text());
    std::vector<std::optional<std::int64_t>> m(20, 5);
    // 16 of 20 meet the rule: it passes; 15 do not, nor does a run that gave nothing, as one that crashed
    for (std::size_t i = 0; i < 4; ++i) {
        m[i] = 0;
    }
    kd::scene::Verdict v = kd::scene::judge(s, m);
    CHECK(v.passed);
    CHECK(v.passes == 16);
    CHECK(v.needed == 16);
    CHECK_FALSE(v.provisional);
    CHECK_FALSE(v.reran);
    m[4] = std::nullopt;
    v = kd::scene::judge(s, m);
    CHECK_FALSE(v.passed);
    CHECK(kd::scene::rerun_due(s, v));
    // judged again on 40, it needs 32: 15 and 17 of the fresh 20 pass, 18 fail
    std::vector<std::optional<std::int64_t>> forty = m;
    for (int i = 0; i < 20; ++i) {
        forty.emplace_back(i < 17 ? 5 : 0);
    }
    v = kd::scene::judge(s, forty);
    CHECK(v.passed);
    CHECK(v.passes == 32);
    CHECK(v.needed == 32);
    CHECK(v.reran);
    CHECK_FALSE(kd::scene::rerun_due(s, v));
    forty.back() = 0;
    forty[forty.size() - 4] = 0;
    CHECK_FALSE(kd::scene::judge(s, forty).passed);
    // twelve runs judged, as when its budget ran out, scale 16 of 20 to 10 of 12, rounded against passing, and count
    // only provisionally; and a scene of fewer than 20 runs is provisional, with no rerun
    std::vector<std::optional<std::int64_t>> few(12, 5);
    few[0] = 0;
    few[1] = 0;
    v = kd::scene::judge(s, few);
    CHECK(v.needed == 10);
    CHECK(v.passed);
    CHECK(v.provisional);
    few[2] = 0;
    CHECK_FALSE(kd::scene::judge(s, few).passed);
    std::string small = scene_text("", "6 d", "12");
    small.replace(small.find("in = 16"), 7, "in = 10");
    const kd::scene::Scene twelve = scene_of(small);
    const kd::scene::Verdict failed = kd::scene::judge(twelve, std::vector<std::optional<std::int64_t>>(12));
    CHECK(failed.provisional);
    CHECK_FALSE(kd::scene::rerun_due(twelve, failed));
}

// checks: PLT-05 RES-21
TEST_CASE("a scene's run stopped at a checkpoint and resumed ends exactly as one never stopped") {
    const kd::scene::Scene whole = scene_of(scene_text());
    kd::save::FakeFiles unbroken;
    const kd::scene::RunResult want = kd::demo::run_crowd(whole, 3, unbroken, fixture(), "test");
    CHECK(want.seed == 43);
    CHECK(want.days == 6);
    CHECK(want.oddities.empty());
    CHECK(want.measure("greetings").value_or(0) > 0);
    CHECK(want.measure("awake_at_midnight") == 0);
    // stopped after three days, then resumed, and once more with the power cut in the middle of a day
    kd::save::FakeFiles resumed;
    const kd::scene::Scene half = scene_of(scene_text("", "3 d"));
    (void)kd::demo::run_crowd(half, 3, resumed, fixture(), "test");
    kd::save::FakeFiles cut = resumed;
    const kd::scene::RunResult got = kd::demo::run_crowd(whole, 3, resumed, fixture(), "test");
    CHECK(got.digest == want.digest);
    CHECK(got.measures == want.measures);
    CHECK(got.oddities.empty());
    const kd::scene::Scene more = scene_of(scene_text("", "4 d"));
    (void)kd::demo::run_crowd(more, 3, cut, fixture(), "test");
    cut.power_cut();
    const kd::scene::RunResult after_cut = kd::demo::run_crowd(whole, 3, cut, fixture(), "test");
    CHECK(after_cut.digest == want.digest);
    CHECK(after_cut.measures == want.measures);
}

// checks: RES-12 RES-10
TEST_CASE("each planted oddity in a crowd's run is flagged, and a run with none flags none") {
    const auto run_with = [](const std::string& sw) {
        const kd::scene::Scene s = scene_of(scene_text(sw.empty() ? "" : "switches = [\"" + sw + "\"]\n"));
        kd::save::FakeFiles folder;
        return kd::demo::run_crowd(s, 0, folder, fixture(), "test");
    };
    const kd::scene::RunResult clean = run_with("");
    CHECK(clean.oddities.empty());
    const kd::scene::RunResult wander = run_with("plant_wander");
    CHECK(mentions(wander.oddities, "20000 m from its camp, farther than 5000"));
    const kd::scene::RunResult insomnia = run_with("plant_insomnia");
    CHECK(mentions(insomnia.oddities, "awake at midnight 2 nights running"));
    const kd::scene::RunResult chatter = run_with("plant_chatter");
    CHECK(chatter.measure("greetings").value_or(0) > clean.measure("greetings").value_or(0));
    CHECK(mentions(chatter.oddities, "greetings_per_camp_day"));
    const kd::scene::RunResult bad_save = run_with("plant_bad_save");
    CHECK(mentions(bad_save.oddities, "did not open again"));
    // a switch off: no greetings at all, and the run says it ran with it
    const kd::scene::RunResult quiet = run_with("no_greetings");
    CHECK(quiet.measure("greetings") == 0);
    CHECK(quiet.switches == std::vector<kd::world::Switch>{kd::world::Switch::no_greetings});
}

// checks: RES-06 RES-13
TEST_CASE("a scene's report holds its rule, its verdict, each measure's range over its runs, and every run") {
    const kd::scene::Scene s = scene_of(scene_text());
    std::vector<kd::scene::RunResult> runs(3);
    for (std::size_t i = 0; i < runs.size(); ++i) {
        runs[i].index = static_cast<std::int64_t>(i);
        runs[i].seed = 40 + static_cast<std::int64_t>(i);
        runs[i].days = 6;
        runs[i].digest = 0xabcdef;
    }
    runs[0].measures = {{"greetings_per_camp_day", 12}, {"awake_at_midnight", 0}};
    runs[1].measures = {{"greetings_per_camp_day", 4}, {"awake_at_midnight", 0}};
    runs[1].oddities = {"greetings_per_camp_day 4, outside its expected 5 to 25"};
    // the third crashed, and gave no measures
    runs[2].oddities = {"it crashed, stopped by signal 6"};
    kd::scene::Outcome outcome;
    outcome.verdict = kd::scene::judge(s, std::vector<std::optional<std::int64_t>>{12, 4, std::nullopt});
    outcome.seconds = 3;
    outcome.build = "α1.5a \"test\"";
    const std::string json = kd::scene::report_json(s, kd::demo::crowd_kind(), runs, outcome);
    const auto has = [&](const std::string& part) { return json.find(part) != std::string::npos; };
    CHECK(has("\"rule\": \"greetings_per_camp_day at least 1, in at least 16 of 20 runs\""));
    CHECK(has("\"in\": 16,"));
    // two of the three met it, three were needed of three, and so few runs count only provisionally
    CHECK(has("\"passed\": false,"));
    CHECK(has("\"passes\": 2,"));
    CHECK(has("\"judged\": 3,"));
    CHECK(has("\"needed\": 3,"));
    CHECK(has("\"provisional\": true,"));
    CHECK(has("\"oddities\": 2,"));
    CHECK(has("\"seconds\": 3,"));
    CHECK(has("\"build\": \"α1.5a \\\"test\\\"\","));
    // each measure's range over the runs that gave one, and how many runs its expected range held
    CHECK(
        has("{\"measure\": \"greetings_per_camp_day\", \"about\": \"the greetings a camp a game day, rounded down\", "
            "\"lowest\": 4, \"median\": 12, \"highest\": 12, \"runs\": 2, \"expected\": [5, 25], \"inside\": 1}"));
    CHECK(
        has("\"measure\": \"awake_at_midnight\", \"about\": \"the most markers awake at any midnight\", \"lowest\": 0, "
            "\"median\": 0, \"highest\": 0, \"runs\": 2, \"expected\": [0, 3], \"inside\": 2}"));
    CHECK_FALSE(has("{\"measure\": \"greetings\","));
    // every run, its digest in hex, its measures and its oddities
    CHECK(
        has("{\"index\": 1, \"seed\": 41, \"days\": 6, \"digest\": \"0000000000abcdef\", \"switches\": [], "
            "\"measures\": {\"greetings_per_camp_day\": 4, \"awake_at_midnight\": 0}, "
            "\"oddities\": [\"greetings_per_camp_day 4, outside its expected 5 to 25\"]}"));
    CHECK(
        has("{\"index\": 2, \"seed\": 42, \"days\": 6, \"digest\": \"0000000000abcdef\", \"switches\": [], "
            "\"measures\": {}, \"oddities\": [\"it crashed, stopped by signal 6\"]}"));
}
