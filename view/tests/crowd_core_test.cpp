#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

#include "crowd_core.hpp"
#include "doctest.h"
#include "heat.hpp"
#include "kd/proof/camp_cases.hpp"
#include "kd/proof/fixture.hpp"
#include "triple.hpp"

namespace {

// A slot whose every value must be the same, so a torn read shows.
struct Slot {
    std::uint64_t serial = 0;
    std::vector<std::uint64_t> values = std::vector<std::uint64_t>(256, 0);
};

const kd::data::Catalogue& fixture() {
    static const kd::data::Catalogue catalogue = [] {
        kd::data::Catalogue c;
        const auto files = kd::proof::fixture_files();
        REQUIRE(c.load(files).empty());
        return c;
    }();
    return catalogue;
}

}  // namespace

TEST_CASE("marker trails retain no living state while living trails follow each activity") {
    kd::demo::CrowdWorld markers(3, fixture(), 1);
    kd::view::CrowdStepper marker_stepper(markers);
    REQUIRE(marker_stepper.snapshots().take());
    CHECK(marker_stepper.snapshots().front().lives.empty());
    CHECK(marker_stepper.snapshots().front().changed_at.empty());
    kd::time::Seconds frontier = 0;
    while (frontier < kd::time::kDay) frontier = marker_stepper.advance(frontier, kd::time::kDay);
    REQUIRE(marker_stepper.snapshots().take());
    CHECK(marker_stepper.snapshots().front().lives.empty());
    CHECK(marker_stepper.snapshots().front().changed_at.empty());
    CHECK(marker_stepper.snapshots().front().ways.size() > markers.world().beings().size());

    kd::data::Catalogue catalogue;
    REQUIRE(catalogue.load(kd::proof::camp_files()).empty());
    kd::demo::CrowdWorld camp(3, catalogue, 1, true);
    kd::demo::CrowdWorld reference(3, catalogue, 1, true);
    kd::view::CrowdStepper camp_stepper(camp);
    frontier = 0;
    for (kd::time::Seconds screen = 1; screen < kd::time::kDay; screen += 997) {
        camp_stepper.set_screen(static_cast<double>(screen));
        while (frontier < screen + 3600) frontier = camp_stepper.advance(frontier, screen + 3600);
        REQUIRE(camp_stepper.snapshots().take());
        const auto& snapshot = camp_stepper.snapshots().front();
        REQUIRE(snapshot.lives.size() == snapshot.ways.size());
        reference.world().run_to(screen + 1);
        for (std::size_t i = 0; i < snapshot.walkers.size(); ++i) {
            const auto k = snapshot.way_index(i, static_cast<double>(screen));
            const auto& life = snapshot.lives[k];
            REQUIRE(life);
            if (!life) return;
            const auto h = reference.world().beings().handle(kd::ecs::Id{snapshot.walkers[i].id});
            const auto& raw = reference.world().beings().raw();
            const auto& activity = raw.get<kd::world::Activity>(h);
            const auto expected = reference.living()->sample(raw.get<kd::world::Life>(h), activity, screen);
            const auto drawn = camp.living()->sample(*life, snapshot.ways[k], screen);
            CHECK(snapshot.ways[k].what == activity.what);
            CHECK(kd::demo::Living::needs(drawn) == kd::demo::Living::needs(expected));
            CHECK(drawn.goal == expected.goal);
        }
    }
}

TEST_CASE("a dream sent during sleep is visible at its event without interrupting or rewriting earlier sleep") {
    using namespace kd;
    data::Catalogue catalogue;
    REQUIRE(catalogue.load(proof::camp_files()).empty());
    demo::CrowdWorld camp(17, catalogue, 1, true);
    view::CrowdStepper stepper(camp);
    time::Seconds frontier = 0;
    while (frontier < 36000) frontier = stepper.advance(frontier, 36000);
    REQUIRE(stepper.snapshots().take());
    const auto& before = stepper.snapshots().front();
    std::size_t sleeper = before.walkers.size();
    for (std::size_t i = 0; i < before.walkers.size(); ++i) {
        const auto& activity = before.way_at(i, static_cast<double>(frontier));
        if (activity.what == static_cast<std::uint8_t>(world::LivingAct::rest) && activity.end > frontier + 60) {
            sleeper = i;
            break;
        }
    }
    REQUIRE(sleeper < before.walkers.size());
    if (sleeper >= before.walkers.size()) return;
    const auto& old_thought = before.dreams[before.way_index(sleeper, static_cast<double>(frontier))];
    REQUIRE(old_thought);
    if (!old_thought) return;
    const auto old_at = old_thought->at;
    const auto old_activity = before.way_at(sleeper, static_cast<double>(frontier));
    const ecs::Id id{before.walkers[sleeper].id};
    camp.world().command(frontier, demo::Living::kPlaceDream, id.value, 2);
    const auto asked = frontier;
    frontier = stepper.advance(frontier, frontier + 1);
    REQUIRE(stepper.snapshots().take());
    const auto& after = stepper.snapshots().front();
    const auto now = after.way_index(sleeper, static_cast<double>(asked));
    const auto& thought = after.dreams[now];
    REQUIRE(thought);
    if (!thought) return;
    CHECK(thought->at == asked);
    CHECK(thought->subject == 2);
    const auto past = after.way_index(sleeper, static_cast<double>(asked - 1));
    const auto& past_thought = after.dreams[past];
    REQUIRE(past_thought);
    if (!past_thought) return;
    CHECK(past_thought->at == old_at);
    CHECK(after.ways[now].start == old_activity.start);
    CHECK(after.ways[now].end == old_activity.end);
    CHECK(after.ways[now].what == old_activity.what);
    CHECK(after.ways[now].from == old_activity.from);
    CHECK(after.ways[now].to == old_activity.to);
    const auto h = camp.world().beings().handle(id);
    const auto& raw = camp.world().beings().raw();
    CHECK(thought->at == raw.get<world::Dream>(h).at);
    const auto& life = after.lives[now];
    REQUIRE(life);
    if (!life) return;
    const auto drawn = camp.living()->sample(*life, after.ways[now], asked);
    const auto expected = camp.living()->sample(raw.get<world::Life>(h), raw.get<world::Activity>(h), asked);
    CHECK(demo::Living::needs(drawn) == demo::Living::needs(expected));
    std::string why;
    auto reopened = demo::CrowdWorld::open(catalogue, camp.world().save(), why);
    INFO(why);
    REQUIRE(reopened);
    if (!reopened) return;
    view::CrowdStepper again(*reopened);
    REQUIRE(again.snapshots().take());
    const auto& saved = again.snapshots().front();
    const auto& saved_thought = saved.dreams[saved.way_index(sleeper, static_cast<double>(frontier))];
    REQUIRE(saved_thought);
    if (!saved_thought) return;
    CHECK(saved_thought->at == thought->at);
}

// checks: WLD-13
TEST_CASE("each walker is drawn where the world has it doing what it does however far ahead the world has run") {
    const kd::num::Torus torus = kd::world::World::kTorus;
    kd::demo::CrowdWorld ahead(3, fixture(), 6);
    kd::demo::CrowdWorld reference(3, fixture(), 6);
    kd::view::CrowdStepper stepper(ahead);
    const kd::num::Point origin = ahead.square().south_west;
    kd::time::Seconds frontier = 0;
    std::uint64_t checked = 0;
    std::uint64_t wrong = 0;
    // the screen at a second, and the world run on to a lead ahead of it, as the speed loop asks at camp speed and up
    for (kd::time::Seconds screen = 1; screen < 2 * kd::time::kDay; screen += 97) {
        stepper.set_screen(static_cast<double>(screen));
        const kd::time::Seconds goal = screen + 1'800;
        while (frontier < goal) {
            frontier = stepper.advance(frontier, goal);
        }
        REQUIRE(stepper.snapshots().take());
        const kd::view::Snapshot& s = stepper.snapshots().front();
        // the reference with every event at the screen's second done, so it is doing what began by then
        reference.world().run_to(screen + 1);
        std::size_t i = 0;
        reference.world().beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
            if (id.family() != kd::ecs::Family::marker) {
                return;
            }
            const kd::world::Activity& a = reference.world().beings().raw().get<kd::world::Activity>(h);
            const kd::num::Offset want = torus.offset(origin, a.at(torus, screen));
            const kd::world::Activity& way = s.way_at(i, static_cast<double>(screen));
            double east = 0.0;
            double north = 0.0;
            kd::view::place(torus, way, static_cast<double>(screen), origin, east, north);
            const bool same = s.walkers[i].id == id.value && way.what == a.what &&
                              east == static_cast<double>(want.dx) && north == static_cast<double>(want.dy);
            wrong += same ? 0 : 1;
            ++checked;
            ++i;
        });
    }
    CHECK(checked > 100'000);
    CHECK(wrong == 0);
}

// checks: WLD-13
TEST_CASE("the triple buffer never tears under a producer at full speed") {
    kd::view::TripleBuffer<Slot> buffer;
    std::atomic<bool> stop{false};
    std::thread producer([&] {
        for (std::uint64_t n = 1; !stop.load(std::memory_order_relaxed); ++n) {
            Slot& s = buffer.back();
            s.serial = n;
            for (std::uint64_t& v : s.values) {
                v = n;
            }
            buffer.publish();
        }
    });
    std::uint64_t last = 0;
    std::uint64_t taken = 0;
    std::uint64_t torn = 0;
    std::uint64_t backward = 0;
    // until enough slots are taken, or a few seconds if the buffer never hands one over
    const auto give_up = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (taken < 20'000 && std::chrono::steady_clock::now() < give_up) {
        if (!buffer.take()) {
            continue;
        }
        ++taken;
        const Slot& s = buffer.front();
        for (const std::uint64_t v : s.values) {
            torn += v == s.serial ? 0 : 1;
        }
        // the newest only ever moves forward
        backward += s.serial > last ? 0 : 1;
        last = s.serial;
    }
    stop.store(true);
    producer.join();
    CHECK(taken == 20'000);
    CHECK(torn == 0);
    CHECK(backward == 0);
}

// checks: PLT-01
TEST_CASE("the heat governor cuts the share within one reading and gives it back only after a calm minute") {
    kd::view::HeatGovernor heat({0.85, 0.5, 0.25, 30, 0.05});
    CHECK(heat.read(0.5) == 1.0);
    // a rising forecast: cut at the first reading that nears throttling, and again while it stays near
    CHECK(heat.read(0.86) == doctest::Approx(0.5));
    CHECK(heat.read(0.9) == doctest::Approx(0.25));
    CHECK(heat.read(0.95) == doctest::Approx(0.25));
    // a minute of calm readings, every 2 s, gives nothing back
    for (int i = 0; i < 30; ++i) {
        CHECK(heat.read(0.6) == doctest::Approx(0.25));
    }
    // after it, a little each reading, up to the whole
    CHECK(heat.read(0.6) == doctest::Approx(0.30));
    for (int i = 0; i < 40; ++i) {
        heat.read(0.6);
    }
    CHECK(heat.share() == 1.0);
    // a near reading starts the minute again
    heat.read(0.9);
    for (int i = 0; i < 30; ++i) {
        heat.read(0.6);
    }
    CHECK(heat.share() == doctest::Approx(0.5));
}
