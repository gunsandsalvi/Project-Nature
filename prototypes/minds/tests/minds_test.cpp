// P6's tests (IMPLEMENTATION α0.4b): the land, the paths in levels, choices with their reasons, minds within their
// caps, talk passing what people know, and one thread and four ending every day the same. Pre-production code
// (research 00).
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <algorithm>
#include <memory>

#include "minds.hpp"

namespace {

constexpr int kDays = 40;

// One world, made once and run for 40 days, which most tests read: long enough for minds to fill to their caps.
const minds::World& forty_days() {
    static const std::unique_ptr<minds::World> world = [] {
        minds::Settings settings;
        settings.threads = 4;
        auto w = std::make_unique<minds::World>(settings);
        for (int d = 0; d < kDays; ++d) {
            w->run_day();
        }
        return w;
    }();
    return *world;
}

}  // namespace

// checks: TIM-17
TEST_CASE("the land: its river crossed only at fords, which set walks' routes, and every camp near water") {
    const minds::Land& land = forty_days().land();
    CHECK(land.camps.size() == 40);
    std::vector<std::int32_t> near;
    for (const minds::Camp& c : land.camps) {
        land.spots_near(c.cell, 200.0, &near);
        CHECK(std::any_of(near.begin(), near.end(), [&land](std::int32_t s) {
            return land.spots[static_cast<std::size_t>(s)].kind == minds::Kind::kWater;
        }));
    }
    int water = 0;
    int fords = 0;
    for (int x = 0; x < minds::kSide; ++x) {
        bool wet = false;
        for (int y = 0; y < minds::kSide; ++y) {
            wet = wet || land.ground(minds::cell_at(x, y)) == minds::Ground::kWater;
        }
        water += wet ? 1 : 0;
        fords += (x % 96) < 4 ? 1 : 0;
    }
    CHECK(water == minds::kSide - fords);
}

// checks: TIM-17, MND-15
TEST_CASE("paths in levels: an impossible trip fails at once, and a cached trip is the trip found fresh") {
    const minds::World& w = forty_days();
    const minds::Land& land = w.land();
    const minds::Paths paths(land);  // its own cache, empty
    minds::PathScratch a(paths.entrances());
    minds::PathScratch b(paths.entrances());
    minds::Trip first;
    minds::Trip again;
    minds::Trip fresh;
    int tried = 0;
    for (std::size_t i = 0; i + 1 < land.camps.size(); i += 3) {
        const int from = land.camps[i].cell;
        const int to = land.camps[i + 1].cell;
        if (paths.region(from) != paths.region(to)) {
            continue;
        }
        REQUIRE(paths.trip(from, to, &a, &first));
        REQUIRE(paths.trip(from, to, &a, &again));  // from the cache
        REQUIRE(paths.trip(from, to, &b, &fresh));  // another thread's empty cache
        CHECK(first.metres == again.metres);
        CHECK(first.metres == fresh.metres);
        CHECK(first.via == fresh.via);
        // a trip from a step away takes the path the first trip cached, as a fresh search would find it
        for (const int beside : {from + 1, from - 1, from + minds::kSide, from - minds::kSide}) {
            if (land.passable(beside) && minds::block_of(beside) == minds::block_of(from)) {
                minds::PathScratch c(paths.entrances());
                minds::Trip near_cached;
                minds::Trip near_fresh;
                REQUIRE(paths.trip(beside, to, &a, &near_cached));
                REQUIRE(paths.trip(beside, to, &c, &near_fresh));
                CHECK(near_cached.metres == near_fresh.metres);
                CHECK(near_cached.via == near_fresh.via);
                break;
            }
        }
        // no shorter than the straight line, and not absurdly longer
        CHECK(first.metres >= static_cast<float>(minds::metres(from, to)) - 1.0F);
        CHECK(first.metres < 3.0F * static_cast<float>(minds::metres(from, to)) + 100.0F);
        ++tried;
    }
    CHECK(tried >= 5);
    CHECK(a.counts.cached >= 1);
    // a cell no one can stand on has no trip to it
    int blocked = -1;
    for (int c = 0; c < minds::kCells && blocked < 0; ++c) {
        blocked = land.passable(c) ? -1 : c;
    }
    REQUIRE(blocked >= 0);
    CHECK_FALSE(paths.trip(land.camps[0].cell, blocked, &a, &first));
}

// checks: MND-09, PRN-13
TEST_CASE("every choice keeps three reasons and the options it beat") {
    const minds::World& w = forty_days();
    int explained = 0;
    for (std::size_t i = 0; i < w.people().size(); ++i) {
        const minds::Activity& a = w.people()[i].now;
        if (a.start == 0) {
            continue;  // still the first rest, which no choice made
        }
        for (const std::uint8_t r : a.reasons) {
            CHECK(r < minds::kNeeds + 3);
        }
        CHECK(a.beaten[0] >= 0);
        CHECK(a.beaten[0] != a.action);
        ++explained;
    }
    CHECK(explained > 900);
    CHECK(w.explain(0).find(", because: ") != std::string::npos);
}

// checks: MND-14, TIM-17
TEST_CASE("no mind passes its caps, activities end at events and people keep alive their bodies' needs") {
    const minds::World& w = forty_days();
    const double per_person_day = static_cast<double>(w.decisions()) / (kDays * static_cast<double>(w.people().size()));
    CHECK(per_person_day >= 10.0);  // TIM-17: about 10 to 30 activities a day
    CHECK(per_person_day <= 40.0);
    // some activities end early, when a need they don't meet falls below 20, and most don't
    CHECK(w.cut_short() > 0);
    CHECK(w.cut_short() * 10 < w.decisions());
    int starving = 0;
    int most_ties = 0;
    for (const minds::Person& p : w.people()) {
        CHECK(p.places <= minds::kPlacesHeld);
        CHECK(p.ties <= minds::kTiesHeld);
        CHECK(p.memories <= minds::kMemoriesHeld);
        most_ties = std::max(most_ties, p.ties);
        CHECK(p.now.end - p.now.start >= minds::kWindow);
        CHECK(p.now.end <= p.now.full_end);
        starving += (p.need[minds::kHunger] <= 0.0F || p.need[minds::kThirst] <= 0.0F) ? 1 : 0;
    }
    CHECK(starving * 20 < static_cast<int>(w.people().size()));
    CHECK(most_ties == minds::kTiesHeld);  // talk has filled someone's ties, and the cap held them
}

// checks: MND-33, MND-23
TEST_CASE("talk passes places on with who told them") {
    const minds::World& w = forty_days();
    int told = 0;
    for (const minds::Person& p : w.people()) {
        for (int i = 0; i < p.places; ++i) {
            told += p.place[static_cast<std::size_t>(i)].told_by >= 0 ? 1 : 0;
        }
    }
    CHECK(told > 100);
}

// checks: RES-05, TIM-16
TEST_CASE("one thread and four end every day with the same state") {
    minds::Settings one;
    minds::Settings four;
    four.threads = 4;
    CHECK(minds::run(one, 2) == minds::run(four, 2));
}
