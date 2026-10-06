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

using kd::test::fixture;

// The digest of a crowd's whole state at each of the given seconds, run there one event at a time, or in islands of a
// window's length on some threads, with the order fuzzer on or off.
std::vector<std::uint64_t> stops(std::int64_t camps, const std::vector<kd::time::Seconds>& at, kd::time::Seconds window,
                                 int threads, std::optional<std::uint64_t> fuzz,
                                 std::vector<kd::world::Record>* history = nullptr) {
    kd::demo::CrowdWorld crowd(11, fixture(), camps);
    kd::world::World& w = crowd.world();
    w.keep_history(history);
    std::optional<kd::run::Workers> workers;
    if (threads > 0) {
        workers.emplace(threads);
    }
    std::vector<std::uint64_t> out;
    std::uint64_t batch = 0;
    for (const kd::time::Seconds goal : at) {
        if (fuzz) {
            w.beings().fuzz(*fuzz + batch++);
        }
        if (workers) {
            w.run_islands(goal, *workers, window);
        } else {
            w.run_to(goal);
        }
        out.push_back(w.digests().whole);
    }
    return out;
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
    void handle(kd::world::Context& /*c*/, const Event& e) override { handled.push_back(e.key); }
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
        void handle(kd::world::Context& c, const Event& /*e*/) override {
            if (!done) {
                done = true;
                c.schedule(other, 0, c.now());
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
TEST_CASE("a world of 1,000 markers gives the same daily digests over 30 game days, however it is cut or scrambled") {
    const std::vector<std::uint64_t> one = daily(40, 30, kd::time::kDay, std::nullopt);
    CHECK(daily(40, 30, 7'919, std::nullopt) == one);
    CHECK(daily(40, 30, kd::time::kDay, 99) == one);
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

// checks: TIM-17
TEST_CASE("markers wake through the morning and rest for different lengths so the crowd never moves in step") {
    kd::demo::CrowdWorld crowd(5, fixture(), 10);
    std::vector<kd::world::Way> ways;
    crowd.world().keep_ways(&ways);
    crowd.world().run_to(2 * kd::time::kDay);
    std::vector<kd::time::Seconds> wakes;
    std::vector<kd::time::Seconds> rests;
    for (const kd::world::Way& w : ways) {
        const kd::world::Activity& a = w.activity;
        if (a.what == static_cast<std::uint8_t>(kd::demo::Doing::sleep) && a.start > kd::time::kDay / 2 &&
            a.start < kd::time::kDay + kd::time::kDay / 2) {
            wakes.push_back(a.end);
            CHECK(crowd.daylight().night_at(a.end - 1) == (a.end == crowd.daylight().next_dawn(a.start)));
        }
        if (a.what == static_cast<std::uint8_t>(kd::demo::Doing::rest)) {
            rests.push_back(a.end - a.start);
        }
    }
    const auto distinct = [](std::vector<kd::time::Seconds> v) {
        std::sort(v.begin(), v.end());
        return static_cast<std::size_t>(std::unique(v.begin(), v.end()) - v.begin());
    };
    // the 250 markers waking at the second dawn, and their rests
    REQUIRE(wakes.size() == 250);
    CHECK(distinct(wakes) > 200);
    REQUIRE(rests.size() > 500);
    CHECK(distinct(rests) > rests.size() / 2);
}

// checks: TIM-17
TEST_CASE("an activity cut short keeps what it reached: a walker stands where it got to") {
    const kd::num::Torus& torus = kd::world::World::kTorus;
    kd::world::Activity walk{1, 1000, 1100, {5000, 5000}, {6000, 4000}};
    CHECK(walk.at(torus, 1050) == kd::num::Point{5500, 4500});
    CHECK(walk.share(1025) == 250'000);
    walk.cut(torus, 1050);
    CHECK(walk.end == 1050);
    CHECK(walk.to == kd::num::Point{5500, 4500});
    CHECK(walk.at(torus, 1080) == kd::num::Point{5500, 4500});
    // across the edge where the map wraps, the short way round
    kd::world::Activity round{1, 0, 10, {torus.width() - 50, 7}, {50, 7}};
    round.cut(torus, 5);
    CHECK(round.to == kd::num::Point{0, 7});
}

// checks: TIM-17
TEST_CASE("a call to someone else lands a second later") {
    const kd::data::Catalogue& cat = fixture();
    kd::world::World w(1, cat);
    struct Caller final : kd::world::System {
        kd::ecs::Id callee;
        std::vector<std::pair<kd::time::Seconds, std::uint32_t>> seen;
        std::string_view name() const override { return "caller"; }
        void handle(kd::world::Context& c, const Event& e) override {
            seen.emplace_back(c.now(), e.slot);
            if (e.slot == kd::world::kActivitySlot && e.key.owner != callee.value) {
                c.schedule(callee, kd::world::kCallSlot, c.now() + 1);
            }
        }
    } caller;
    w.set_system(kd::ecs::Family::marker, caller);
    const kd::ecs::Id a = w.beings().id_of(w.make_being(kd::ecs::Family::marker));
    caller.callee = w.beings().id_of(w.make_being(kd::ecs::Family::marker));
    w.schedule(a, kd::world::kActivitySlot, 100);
    w.run_to(200);
    REQUIRE(caller.seen.size() == 2);
    CHECK(caller.seen[1] == std::pair<kd::time::Seconds, std::uint32_t>{101, kd::world::kCallSlot});
}

// checks: TIM-17
TEST_CASE("driven a second at a time or in big windows, every greeting happens at the same game second") {
    const kd::time::Seconds end = 2 * kd::time::kDay;
    std::vector<kd::world::Record> by_second;
    {
        kd::demo::CrowdWorld crowd(11, fixture(), 12);
        crowd.world().keep_history(&by_second);
        for (kd::time::Seconds t = 1; t <= end; ++t) {
            crowd.world().run_to(t);
        }
    }
    std::vector<kd::world::Record> by_day;
    std::vector<kd::world::Record> in_islands;
    stops(12, {kd::time::kDay, end}, 0, 0, std::nullopt, &by_day);
    stops(12, {kd::time::kDay, end}, 900, 4, std::nullopt, &in_islands);
    REQUIRE(by_second.size() > 20);
    const auto same = [](const std::vector<kd::world::Record>& x, const std::vector<kd::world::Record>& y) {
        return std::equal(x.begin(), x.end(), y.begin(), y.end(), [](const auto& r, const auto& s) {
            return r.key == s.key && r.n == s.n && r.what == s.what && r.a == s.a && r.b == s.b;
        });
    };
    CHECK(same(by_second, by_day));
    CHECK(same(by_second, in_islands));
}

// checks: WLD-13 TIM-17
TEST_CASE("the ways kept for the screen are every walker's every move without a jump and the same in islands") {
    const kd::time::Seconds end = 2 * kd::time::kDay;
    std::vector<kd::world::Activity> first;
    std::vector<kd::world::Activity> last;
    const auto ways_of = [&](kd::time::Seconds window, int threads) {
        kd::demo::CrowdWorld crowd(11, fixture(), 12);
        kd::world::World& w = crowd.world();
        first.clear();
        w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
            if (id.family() == kd::ecs::Family::marker) {
                first.push_back(w.beings().raw().get<kd::world::Activity>(h));
            }
        });
        std::vector<kd::world::Way> ways;
        w.keep_ways(&ways);
        const std::uint64_t before = w.digests().whole;
        if (threads > 0) {
            kd::run::Workers workers(threads);
            w.run_islands(end, workers, window);
        } else {
            w.run_to(end);
        }
        // keeping them changes nothing in the world
        kd::demo::CrowdWorld unkept(11, fixture(), 12);
        CHECK(unkept.world().digests().whole == before);
        unkept.world().run_to(end);
        CHECK(unkept.world().digests().whole == w.digests().whole);
        last.clear();
        w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
            if (id.family() == kd::ecs::Family::marker) {
                last.push_back(w.beings().raw().get<kd::world::Activity>(h));
            }
        });
        return ways;
    };
    const std::vector<kd::world::Way> one = ways_of(0, 0);
    const std::vector<kd::world::Way> islands = ways_of(900, 4);
    const auto same = [](const kd::world::Way& x, const kd::world::Way& y) {
        const kd::world::Activity& a = x.activity;
        const kd::world::Activity& b = y.activity;
        return x.key == y.key && x.n == y.n && x.id == y.id && a.what == b.what && a.start == b.start &&
               a.end == b.end && a.from == b.from && a.to == b.to;
    };
    REQUIRE(one.size() > 1000);
    CHECK(std::equal(one.begin(), one.end(), islands.begin(), islands.end(), same));
    // each way begins at its event, where the walker's way before it had brought it
    const kd::num::Torus& torus = kd::world::World::kTorus;
    std::vector<kd::world::Activity> now = first;
    std::vector<kd::ecs::Id> ids;
    kd::demo::CrowdWorld crowd(11, fixture(), 12);
    crowd.world().beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle /*h*/) {
        if (id.family() == kd::ecs::Family::marker) {
            ids.push_back(id);
        }
    });
    std::size_t jumps = 0;
    for (const kd::world::Way& way : one) {
        const auto i = static_cast<std::size_t>(std::lower_bound(ids.begin(), ids.end(), way.id) - ids.begin());
        REQUIRE(i < ids.size());
        CHECK(way.activity.start == way.key.second);
        jumps += now[i].at(torus, way.activity.start) == way.activity.from ? 0 : 1;
        now[i] = way.activity;
    }
    CHECK(jumps == 0);
    // and the last way each took is what it is doing at the end
    std::size_t unlike = 0;
    for (std::size_t i = 0; i < now.size(); ++i) {
        const kd::world::Activity& a = now[i];
        const kd::world::Activity& b = last[i];
        unlike += a.what == b.what && a.start == b.start && a.end == b.end && a.from == b.from && a.to == b.to ? 0 : 1;
    }
    CHECK(unlike == 0);
}

// checks: RES-05 WLD-13 TIM-17
TEST_CASE("islands give the one-thread world's digest for any window and thread count, stopped anywhere") {
    // stops at seconds chosen by keyed chance over two game days, and at each midnight
    const kd::chance::Draws draws(5, kd::chance::name("test"), 0, 0, kd::chance::name("stops"));
    std::vector<kd::time::Seconds> at;
    for (std::uint64_t i = 0; i < 12; ++i) {
        at.push_back(draws.between(i, 1, 2 * kd::time::kDay));
    }
    at.push_back(kd::time::kDay);
    at.push_back(2 * kd::time::kDay);
    std::sort(at.begin(), at.end());
    at.erase(std::unique(at.begin(), at.end()), at.end());
    const std::vector<std::uint64_t> one = stops(20, at, 0, 0, std::nullopt);
    for (const kd::time::Seconds window : {kd::time::Seconds{60}, kd::time::Seconds{300}, kd::time::Seconds{900}}) {
        for (const int threads : {1, 2, 3, 4}) {
            CAPTURE(window);
            CAPTURE(threads);
            CHECK(stops(20, at, window, threads, 7) == one);
        }
    }
}
