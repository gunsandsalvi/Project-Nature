#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

#include "crowd_core.hpp"
#include "doctest.h"
#include "heat.hpp"
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
