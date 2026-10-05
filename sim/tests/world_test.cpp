#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/chance/chance.hpp"
#include "kd/core/bytes.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/ecs/registry.hpp"
#include "kd/event/queue.hpp"
#include "kd/proof/fixture.hpp"
#include "kd/world/world.hpp"

namespace {

using kd::event::Event;
using kd::event::Key;
using kd::event::Queue;

// Pops every event, in the order they come.
std::vector<Key> drain(Queue& q) {
    std::vector<Key> out;
    while (!q.empty()) {
        out.push_back(q.pop().key);
    }
    return out;
}

const kd::data::Catalogue& fixture() {
    static const kd::data::Catalogue catalogue = [] {
        kd::data::Catalogue c;
        const auto files = kd::proof::fixture_files();
        REQUIRE(c.load(files).empty());
        return c;
    }();
    return catalogue;
}

// Each day's digest of a crowd's whole state, run to each midnight in batches of the given length.
std::vector<std::uint64_t> daily(std::int64_t camps, int days, kd::time::Seconds batch,
                                 std::optional<std::uint64_t> fuzz) {
    kd::demo::CrowdWorld crowd(7, fixture(), camps);
    kd::world::World& w = crowd.world();
    w.set_fuzz(fuzz);
    std::vector<std::uint64_t> out;
    for (int day = 1; day <= days; ++day) {
        const kd::time::Seconds goal = day * kd::time::kDay;
        while (w.frontier() < goal) {
            w.advance(w.frontier(), std::min(goal, w.frontier() + batch));
        }
        out.push_back(w.digests().whole);
    }
    return out;
}

// A system that records what it handles, and may schedule more.
struct Recorder final : kd::world::System {
    std::vector<Key> handled;
    std::string_view name() const override { return "recorder"; }
    void handle(kd::world::World& /*w*/, const Event& e) override { handled.push_back(e.key); }
};

}  // namespace

// checks: TIM-17
TEST_CASE("events at the same second settle by owner, then sequence, whatever order they were pushed in") {
    std::vector<Event> events;
    for (std::uint64_t owner = 1; owner <= 5; ++owner) {
        for (std::uint64_t seq = 1; seq <= 4; ++seq) {
            events.push_back({{100, owner * 1000, seq}, 0});
            events.push_back({{99 + static_cast<std::int64_t>(seq % 3), owner * 1000, seq + 10}, 0});
        }
    }
    std::vector<Key> wanted;
    wanted.reserve(events.size());
    for (const Event& e : events) {
        wanted.push_back(e.key);
    }
    std::sort(wanted.begin(), wanted.end());
    const kd::chance::Draws draws(1, kd::chance::name("test"), 0, 0, kd::chance::name("shuffle"));
    for (std::uint64_t round = 0; round < 20; ++round) {
        std::vector<Event> shuffled = events;
        for (std::size_t i = shuffled.size() - 1; i > 0; --i) {
            std::swap(shuffled[i], shuffled[draws.below(round * 1000 + i, i + 1)]);
        }
        Queue q;
        for (const Event& e : shuffled) {
            q.push(e);
        }
        CHECK(drain(q) == wanted);
    }
}

// checks: TIM-17
TEST_CASE("a queue rebuilt without its dead events, or written and read back, runs on identically") {
    Queue q;
    for (std::uint64_t i = 0; i < 1000; ++i) {
        q.push({{static_cast<std::int64_t>(i % 37), 1000 + i % 11, i}, static_cast<std::uint32_t>(i % 4)});
    }
    // every third event is dead
    const auto live = [](const Event& e) { return e.key.sequence % 3 != 0; };
    Queue rebuilt = q;
    rebuilt.rebuild(live);
    CHECK(rebuilt.size() == 666);
    std::vector<Key> wanted;
    for (Queue copy = q; !copy.empty();) {
        const Event e = copy.pop();
        if (live(e)) {
            wanted.push_back(e.key);
        }
    }
    CHECK(drain(rebuilt) == wanted);
    kd::ByteWriter w;
    q.write(w, live);
    kd::ByteReader r(w.bytes());
    std::optional<Queue> read = Queue::read(r);
    REQUIRE(read.has_value());
    CHECK(r.finished());
    Queue got = read.value_or(Queue{});
    CHECK(drain(got) == wanted);
    // cut short or out of order, it is refused
    std::vector<std::byte> cut = w.bytes();
    cut.pop_back();
    kd::ByteReader short_reader(cut);
    CHECK(!Queue::read(short_reader).has_value());
}

// checks: TIM-17
TEST_CASE("a cancelled event never runs, and a handler may schedule only after its own key") {
    const kd::data::Catalogue& cat = fixture();
    kd::world::World w(1, cat);
    Recorder recorder;
    w.set_system(kd::ecs::Family::marker, recorder);
    const kd::ecs::Id a = w.beings().id_of(w.make_being(kd::ecs::Family::marker));
    const kd::ecs::Id b = w.beings().id_of(w.make_being(kd::ecs::Family::marker));
    w.schedule(a, 0, 50);
    w.schedule(b, 0, 50);
    w.schedule(a, 1, 60);
    w.cancel(b, 0);
    w.schedule(a, 1, 70);  // replaces the event at 60
    w.run_to(100);
    REQUIRE(recorder.handled.size() == 2);
    CHECK(recorder.handled[0] == Key{50, a.value, 1});
    CHECK(recorder.handled[1] == Key{70, a.value, 3});
    CHECK(w.queue().empty());
    // an event on another owner within the same second, or anything before the frontier, stops the run
    struct Hasty final : kd::world::System {
        kd::ecs::Id other;
        bool done = false;
        std::string_view name() const override { return "hasty"; }
        void handle(kd::world::World& world, const Event& /*e*/) override {
            if (!done) {
                done = true;
                world.schedule(other, 0, world.now());
            }
        }
    };
    CHECK(kd::test::stops([&] {
        kd::world::World v(1, cat);
        Hasty hasty;
        v.set_system(kd::ecs::Family::marker, hasty);
        const kd::ecs::Id x = v.beings().id_of(v.make_being(kd::ecs::Family::marker));
        hasty.other = v.beings().id_of(v.make_being(kd::ecs::Family::marker));
        v.schedule(x, 0, 5);
        v.run_to(10);
    }));
    CHECK(kd::test::stops([&] {
        kd::world::World v(1, cat);
        v.run_to(10);
        v.schedule(kd::ecs::owners::daylight, 0, 5);
    }));
}

// checks: RES-05
TEST_CASE("ids are never reused through a million makes and ends") {
    kd::ecs::IdMaker maker;
    kd::world::Beings beings;
    std::vector<kd::ecs::Id> live;
    kd::ecs::Id last{};
    const kd::chance::Draws draws(3, kd::chance::name("test"), 0, 0, kd::chance::name("ends"));
    for (std::uint64_t i = 0; i < 1'000'000; ++i) {
        const kd::ecs::Id id = maker.make(i % 3 == 0 ? kd::ecs::Family::place : kd::ecs::Family::marker);
        REQUIRE((id.value & ((std::uint64_t{1} << 60U) - 1)) > (last.value & ((std::uint64_t{1} << 60U) - 1)));
        last = id;
        beings.make(id);
        live.push_back(id);
        // end about as many as are made, chosen by keyed chance
        if (live.size() > 1000) {
            const std::size_t at = draws.below(i, live.size());
            beings.end(live[at]);
            live[at] = live.back();
            live.pop_back();
        }
    }
    CHECK(beings.size() == live.size());
    for (const kd::ecs::Id id : live) {
        CHECK(beings.find(id).has_value());
    }
}

// checks: RES-05
TEST_CASE("the state's digest does not depend on EnTT's order, with the order fuzzer on") {
    kd::demo::CrowdWorld crowd(3, fixture(), 8);
    crowd.world().run_to(kd::time::kDay / 2);
    const kd::world::Digests before = crowd.world().digests();
    for (std::uint64_t key = 1; key <= 5; ++key) {
        crowd.world().beings().fuzz(key);
        CHECK(crowd.world().digests().whole == before.whole);
    }
}

// checks: TIM-16 TIM-17 RES-05
TEST_CASE("a world of 1,000 markers gives the same daily digests over 60 game days, however it is cut or scrambled") {
    const std::vector<std::uint64_t> one = daily(40, 60, kd::time::kDay, std::nullopt);
    CHECK(daily(40, 60, 7'919, std::nullopt) == one);
    CHECK(daily(40, 60, kd::time::kDay, 99) == one);
    CHECK(one.front() != one.back());
}

// checks: TIM-17
TEST_CASE("markers walk, rest and sleep, every move an activity ending at its event") {
    kd::demo::CrowdWorld crowd(5, fixture(), 10);
    kd::world::World& w = crowd.world();
    for (int day = 1; day <= 3; ++day) {
        for (const kd::time::Seconds hour : {kd::time::Seconds{9}, kd::time::Seconds{24}}) {
            w.run_to((day - 1) * kd::time::kDay + hour * kd::time::kHour);
            const std::vector<Event> live = w.queue().live_in_order([&](const Event& e) {
                const auto h = w.beings().find(kd::ecs::Id{e.key.owner});
                return e.key.owner < kd::ecs::owners::count ||
                       (h && w.beings().raw().get<kd::world::Schedule>(*h).expected[e.slot] == e.key.sequence);
            });
            std::size_t walking = 0;
            std::size_t sleeping = 0;
            w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
                if (id.family() != kd::ecs::Family::marker) {
                    return;
                }
                const auto& a = w.beings().raw().get<kd::world::Activity>(h);
                const bool at_its_end = std::any_of(live.begin(), live.end(), [&](const Event& e) {
                    return e.key.owner == id.value && e.key.second == a.end;
                });
                CHECK(at_its_end);
                walking += a.what == static_cast<std::uint8_t>(kd::demo::Doing::walk) ? 1 : 0;
                sleeping += a.what == static_cast<std::uint8_t>(kd::demo::Doing::sleep) ? 1 : 0;
            });
            if (hour == 24) {
                CHECK(sleeping == 250);  // at midnight, every marker sleeps
            } else {
                CHECK(sleeping == 0);
                CHECK(walking > 0);
            }
        }
    }
}
