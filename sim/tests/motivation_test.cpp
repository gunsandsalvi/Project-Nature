// A19.3: labelled law tests are not discovery gates; ordinary continuation stays unscripted.
#include "kd/demo/motivation.hpp"
#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/run/workers.hpp"
#include "kd/world/knowledge_view.hpp"
namespace {
const kd::data::Catalogue& motivation_catalogue() {
    static const auto catalogue = [] {
        kd::data::Catalogue value;
        REQUIRE(value.load(kd::data::read_folder(KD_REPO "/data")).empty());
        return value;
    }();
    return catalogue;
}
void experienced_day(kd::world::Motivation& state, std::int64_t day, std::uint64_t deficit, std::uint64_t relief,
                     std::size_t channel = 0) {
    const auto at = day * kd::time::kDay;
    auto& row = kd::demo::Motivation::day(state, at);
    row.opportunities[channel] = 1;
    row.deficits[channel] = deficit;
    row.relief[channel] = relief;
    kd::demo::Motivation::dawn(state, at + 6 * kd::time::kHour);
}
}  // namespace
TEST_CASE("own eligible shortage raises bounded pressure and seven-day relief relaxes it") {
    kd::world::Motivation state;
    for (std::int64_t day = 0; day < 20; ++day) {
        const auto before = state.pressure[0];
        experienced_day(state, day, 100, 0);
        CHECK(state.pressure[0] >= before);
        CHECK(state.pressure[0] - before <= kd::world::Motivation::kUnit / 16);
        CHECK(state.pressure[0] <= kd::world::Motivation::kUnit);
        CHECK(state.pressure[1] == 0);
    }
    CHECK(state.pressure[0] == kd::world::Motivation::kUnit);
    CHECK(state.update_remainder[0] == 0);
    for (std::int64_t day = 20; day < 45; ++day) {
        const auto before = state.pressure[0];
        experienced_day(state, day, 0, 100);
        CHECK(std::abs(state.pressure[0] - before) <= kd::world::Motivation::kUnit / 16);
    }
    CHECK(state.pressure[0] == 0);
    CHECK(kd::demo::Motivation::valid(state, 45 * kd::time::kDay));
}
TEST_CASE("requests without materials or eligible opportunities cannot accumulate pressure") {
    kd::world::Motivation state;
    for (std::int64_t day = 0; day < 20; ++day) {
        auto& row = kd::demo::Motivation::day(state, day * kd::time::kDay);
        row.requests[1] = 100;
        kd::demo::Motivation::dawn(state, day * kd::time::kDay + 6 * kd::time::kHour);
        CHECK(state.pressure[0] == 0);
        CHECK(state.pressure[1] == 0);
    }
    experienced_day(state, 20, 100, 0, 1);
    const auto raised = state.pressure[1];
    CHECK(raised == kd::world::Motivation::kUnit / 16);
    kd::demo::Motivation::dawn(state, 21 * kd::time::kDay + 6 * kd::time::kHour);
    CHECK(state.pressure[1] == raised);  // yesterday's eligible opportunity cannot cause today's wind-up
}
TEST_CASE("dawn update is once-only and malformed saved motivation is rejected") {
    kd::world::Motivation state;
    experienced_day(state, 0, 33, 0);
    const auto after = state;
    kd::demo::Motivation::dawn(state, 6 * kd::time::kHour);
    CHECK(state == after);
    kd::demo::Motivation::dawn(state, 7 * kd::time::kHour);
    CHECK(state == after);
    CHECK(kd::demo::Motivation::valid(state, kd::time::kDay));
    for (int fault = 0; fault < 7; ++fault) {
        auto broken = state;
        if (fault == 0) broken.pressure[0] = kd::world::Motivation::kUnit + 1;
        if (fault == 1) broken.update_remainder[0] = 16;
        if (fault == 2) broken.window[0].deficits[0] = 101;
        if (fault == 3) broken.window[0].day = 7;
        if (fault == 4) broken.dawn = 7 * kd::time::kHour;
        if (fault == 5) broken.actions[0].failures = 16;
        if (fault == 6) broken.relief_remainder[0] = broken.denominator[0];
        CHECK_FALSE(kd::demo::Motivation::valid(broken, kd::time::kDay));
    }
}
TEST_CASE("ordinary dawn motivation and reasons reopen exactly across steps and four workers") {
    kd::demo::CrowdWorld camp(8806, motivation_catalogue(), 1, true, true);
    auto& w = camp.world();
    std::string why;
    auto other = kd::demo::CrowdWorld::open(motivation_catalogue(), w.save(), why);
    REQUIRE_MESSAGE(other, why);
    if (!other) return;
    kd::run::Workers workers(4);
    for (const auto at : {6 * kd::time::kHour - 1, 6 * kd::time::kHour + 1, kd::time::kDay + 6 * kd::time::kHour + 1,
                          3 * kd::time::kDay}) {
        w.run_to(at);
        other->world().run_islands(at, workers, 1);
        CHECK(w.digests().whole == other->world().digests().whole);
        auto opened = kd::demo::CrowdWorld::open(motivation_catalogue(), w.save(), why);
        REQUIRE_MESSAGE(opened, why);
        if (!opened) return;
        CHECK(opened->world().digests().whole == w.digests().whole);
        other = std::move(opened);
    }
    const auto& constant = std::as_const(w);
    std::size_t people = 0, motivated = 0;
    constant.beings().each([&](kd::ecs::Id, auto h) {
        const auto* mind = constant.beings().raw().try_get<kd::world::Knowledge>(h);
        if (!mind) return;
        ++people;
        CHECK(mind->motivation.dawn == 2 * kd::time::kDay + 6 * kd::time::kHour);
        CHECK(kd::demo::Motivation::valid(mind->motivation, w.frontier()));
        if (mind->motivation.pressure[0] || mind->motivation.pressure[1]) ++motivated;
        const auto captured = kd::world::KnowledgeView::capture(*mind);
        REQUIRE(captured);
        CHECK(*captured.get() == *mind);
    });
    CHECK(people == 25);
    CHECK(motivated > 0);
}

TEST_CASE("local pressure changes learning value without overpowering survival or protected plans") {
    kd::world::Motivation state;
    state.pressure[0] = kd::world::Motivation::kUnit;
    state.pressure[1] = kd::world::Motivation::kUnit;
    kd::world::CraftReason trial;
    trial.kind = 1;
    trial.need = 3;
    trial.action = 2;
    trial.parts = {0, 500, -5};
    trial.score = 495;
    auto encouraged = trial;
    kd::demo::Motivation::value(state, encouraged, 0);
    CHECK(encouraged.score > trial.score);
    CHECK(encouraged.parts[2] == trial.parts[2]);
    auto repeated = trial;
    kd::demo::Motivation::value(state, repeated, 15);
    CHECK(repeated.score < trial.score);
    CHECK(repeated.learning_progress_ppm >= 62500);
    auto fresh_evidence = trial;
    kd::demo::Motivation::value(state, fresh_evidence, 0);
    CHECK(fresh_evidence.score == encouraged.score);
    auto disabled = trial;
    kd::demo::Motivation::value(state, disabled, 15, false);
    CHECK(disabled.score == trial.score);
    auto body = trial;
    body.kind = 2;
    body.need = 0;
    body.score = 800000;
    body.parts = {800000, 0, 0};
    const auto urgent = body;
    kd::demo::Motivation::value(state, body, 15);
    CHECK(body == urgent);
    CHECK(body.score > encouraged.score);
    auto plan = trial;
    plan.parts = {0, 90000, 0};
    plan.score = 90000;
    const auto protected_plan = plan;
    kd::demo::Motivation::value(state, plan, 15);
    CHECK(plan == protected_plan);
    auto enquiry = trial;
    enquiry.kind = 5;
    enquiry.confidence = 0;
    const auto unanswered = enquiry;
    kd::demo::Motivation::value(state, enquiry, 0);
    CHECK(enquiry == unanswered);
    // Observed inability is an eligible teaching belief, without consulting the learner's mind.
    enquiry.confidence = 100;
    kd::demo::Motivation::value(state, enquiry, 0);
    CHECK(enquiry.motivation == 2);
    CHECK(enquiry.score > unanswered.score);
}

TEST_CASE("fuel operation two and striking action two retain independent own feedback through reopen") {
    kd::demo::CrowdWorld camp(8812, motivation_catalogue(), 1, true, true);
    auto& w = camp.world();
    kd::world::Motivation* state = nullptr;
    w.beings().each([&](kd::ecs::Id, auto h) {
        if (!state && w.beings().raw().all_of<kd::world::Knowledge>(h))
            state = &w.beings().raw().get<kd::world::Knowledge>(h).motivation;
    });
    REQUIRE(state);
    if (!state) return;
    kd::world::CraftReason strike;
    strike.kind = 1;
    strike.action = 2;
    strike.need = 3;
    strike.score = 495;
    strike.parts = {0, 500, -5};
    auto fuel = strike;
    fuel.kind = 3;
    fuel.confidence = 0;
    auto& striking = kd::demo::Motivation::feedback(*state, strike);
    auto& feeding = kd::demo::Motivation::feedback(*state, fuel);
    striking = {123, 1000, 15};
    feeding = {456, 2000, 0};
    CHECK(striking.failures == 15);
    CHECK(striking.evidence == 123);
    state->pressure[0] = kd::world::Motivation::kUnit;
    kd::demo::Motivation::value(*state, strike, striking.failures);
    kd::demo::Motivation::value(*state, fuel, feeding.failures);
    CHECK(strike.score < 495);
    CHECK(fuel.score > 495);
    feeding.failures = 7;
    CHECK(striking.failures == 15);
    CHECK(kd::demo::Motivation::valid(*state, 0));
    auto invalid = *state;
    invalid.fire_actions[1].failures = 16;
    CHECK_FALSE(kd::demo::Motivation::valid(invalid, 0));
    std::string why;
    auto reopened = kd::demo::CrowdWorld::open(motivation_catalogue(), w.save(), why);
    REQUIRE_MESSAGE(reopened, why);
    if (!reopened) return;
    CHECK(w.digests().whole == reopened->world().digests().whole);
    std::size_t matching = 0;
    reopened->world().beings().each([&](kd::ecs::Id, auto h) {
        if (const auto* mind = reopened->world().beings().raw().try_get<kd::world::Knowledge>(h);
            mind && mind->motivation == *state)
            ++matching;
    });
    CHECK(matching == 1);
}
