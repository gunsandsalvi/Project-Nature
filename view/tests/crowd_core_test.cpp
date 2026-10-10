#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <set>
#include <thread>
#include <vector>

#include "crowd_core.hpp"
#include "doctest.h"
#include "heat.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/fire.hpp"
#include "kd/demo/kept.hpp"
#include "kd/ecs/component.hpp"
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

TEST_CASE("shared display knowledge reconstructs every scalar and immutable evidence record exactly") {
    kd::world::Knowledge source;
    source.familiar.resize(8);
    for (std::uint64_t n = 1; n <= 200; ++n) {
        kd::world::Memory memory;
        memory.id = n;
        memory.inputs = source.familiar;
        memory.participants.push_back({kd::ecs::Id{n}});
        source.memories.push_back(memory);
    }
    source.skills.resize(3);
    source.hunches.resize(2);
    source.reasons.resize(3);
    source.peers.resize(2);
    source.observations.resize(2);
    kd::world::KnowledgeView previous;
    for (std::uint64_t n = 0; n < 128; ++n) {
        source.settled = -static_cast<std::int64_t>(n + 1);
        source.curiosity_remainder = static_cast<std::int64_t>(n * 191);
        source.choice = n + 1000;
        source.watching = kd::ecs::Id{n};
        source.sectors[n % 15].last_use = -static_cast<std::int64_t>(n + 1);
        source.sectors[n % 15].fraction = static_cast<std::int64_t>(n * 12345);
        source.skills[n % 3].practice.decay_level = static_cast<std::int64_t>(n);
        source.familiar[n % 8].source_events[n % 18] = n;
        source.memories[n % 200].strength = static_cast<std::uint8_t>(n);
        // Fixtures may present a noncanonical record order; sharing must still
        // reconstruct exactly, without assuming sorted memory identities.
        if (n == 64) std::reverse(source.memories.begin(), source.memories.end());
        const auto captured = kd::world::KnowledgeView::capture(source, previous);
        kd::ByteWriter actual, expected;
        kd::ecs::write_component(*captured.get(), actual);
        kd::ecs::write_component(source, expected);
        const auto held = actual.take();
        CHECK(held == expected.take());
        CHECK(kd::world::KnowledgeView::capture(source, captured) == captured);
        source.memories[n % 200].certainty ^= 1;
        kd::ByteWriter old;
        kd::ecs::write_component(*captured.get(), old);
        CHECK(held == old.take());
        previous = captured;
    }
}

TEST_CASE("concurrent lazy knowledge readers publish one immutable component with retained ownership") {
    kd::world::Knowledge source;
    source.choice = 812;
    source.settled = -19;
    source.familiar.resize(5);
    source.memories.resize(3);
    for (auto& memory : source.memories) memory.inputs = source.familiar;
    const auto captured = kd::world::KnowledgeView::capture(source);
    std::atomic<bool> begin{false};
    std::array<const kd::world::Knowledge*, 4> pointers{};
    std::array<kd::save::Bytes, 4> bytes{};
    std::array<bool, 4> stable{};
    std::vector<std::thread> readers;
    for (std::size_t n = 0; n < pointers.size(); ++n)
        readers.emplace_back([&, captured, n] {
            while (!begin.load(std::memory_order_acquire)) std::this_thread::yield();
            pointers[n] = captured.get();
            stable[n] = true;
            for (unsigned count = 0; count < 1000; ++count)
                if (captured.get() != pointers[n]) stable[n] = false;
            kd::ByteWriter writer;
            kd::ecs::write_component(*pointers[n], writer);
            bytes[n] = writer.take();
        });
    begin.store(true, std::memory_order_release);
    for (auto& reader : readers) reader.join();
    kd::ByteWriter expected;
    kd::ecs::write_component(source, expected);
    for (std::size_t n = 0; n < pointers.size(); ++n) {
        CHECK(stable[n]);
        CHECK(pointers[n] == pointers[0]);
        CHECK(bytes[n] == expected.bytes());
    }
}

// checks: PLT-10 PLT-07 TIM-05
TEST_CASE("game time checkpoints advance the fallback frontier and recover the exact older snapshot") {
    kd::save::FakeFiles files;
    kd::save::Keeper keeper(files);
    auto kept = kd::demo::keep_crowd(keeper, fixture(), 17, 1);
    REQUIRE(kept.crowd);
    auto& world = kept.crowd->world();
    kd::view::CrowdStepper stepper(*kept.crowd);
    stepper.keep(&keeper);
    const auto goal = 3 * kd::time::kYear + kd::time::kHour;
    stepper.set_screen(static_cast<double>(goal));
    auto frontier = world.frontier();
    while (frontier < goal) {
        frontier = stepper.advance(frontier, goal);
        stepper.snapshots().take();
    }
    keeper.flush();
    auto saves = files.list("snapshots");
    REQUIRE(saves.size() == 2);
    CHECK(saves.front() > "00000000000000000000.kds");
    CHECK(keeper.last_snapshot() >= 2 * kd::time::kYear);
    kd::demo::CrowdWorld reference(17, fixture(), 1);
    reference.world().run_to(goal);
    CHECK(world.digests().whole == reference.world().digests().whole);
    auto damaged = files;
    damaged.raw("snapshots/" + saves.back()).resize(30);
    kd::save::Keeper reopened(damaged);
    auto again = kd::demo::keep_crowd(reopened, fixture(), 17, 1);
    REQUIRE_MESSAGE(again.crowd, again.problem);
    CHECK(again.snapshot == saves.front());
    std::vector<kd::world::Record> records;
    again.crowd->world().keep_history(&records);
    again.crowd->world().run_to(goal);
    reopened.history(records);
    reopened.flush();
    CHECK(reopened.mismatches() == 0);
    CHECK(again.crowd->world().digests().whole == reference.world().digests().whole);
    again.crowd->world().keep_history(nullptr);
}

TEST_CASE("unchanged personal knowledge shares notifications while changed evidence preserves older views") {
    using namespace kd;
    struct Notices final : world::System {
        std::string_view name() const override { return "display notices"; }
        void handle(world::Context& c, const event::Event& e) override {
            const ecs::Id id{e.key.owner};
            c.moved(id);
            const auto first = c.world().digests().whole;
            c.moved(id);
            CHECK(c.world().digests().whole == first);
            auto& know = c.world().beings().raw().get<world::Knowledge>(c.world().beings().handle(id));
            ++know.kindness;
            c.moved(id);
            c.moved(id);
            know.familiar.front().source_events[3] = 123;
            c.moved(id);
            c.moved(id);
        }
        void digest(num::Digest&) const override {}
    } notices;
    world::World w(8807, fixture());
    w.set_system(ecs::Family::person, notices);
    const auto h = w.make_being(ecs::Family::person);
    const auto id = w.beings().id_of(h);
    w.beings().raw().emplace<world::Activity>(h);
    auto& know = w.beings().raw().emplace<world::Knowledge>(h);
    know.familiar.push_back({});
    std::vector<world::Way> ways;
    w.keep_ways(&ways);
    w.schedule(id, 0, 1);
    w.run_to(2);
    REQUIRE(ways.size() == 6);
    if (ways.size() != 6) return;
    CHECK(ways[0].knowledge == ways[1].knowledge);
    CHECK(ways[2].knowledge == ways[3].knowledge);
    CHECK(ways[4].knowledge == ways[5].knowledge);
    CHECK(ways[0].knowledge != ways[2].knowledge);
    CHECK(ways[2].knowledge != ways[4].knowledge);
    CHECK(ways[0].knowledge->kindness == 50);
    CHECK(ways[2].knowledge->kindness == 51);
    CHECK(ways[2].knowledge->familiar.front().source_events[3] == 0);
    CHECK(ways[4].knowledge->familiar.front().source_events[3] == 123);
}

TEST_CASE("display actor rows share sealed trails and preserve earlier frontiers after pruning") {
    kd::Pages<kd::world::Activity> trail;
    for (kd::time::Seconds n = 0; n < 1537; ++n) trail.push_back({1, n, n + 1, {}, {}});
    kd::view::SharedRows<kd::world::Activity> before;
    before.append(trail);
    trail.push_back({1, 1537, 1538, {}, {}});
    kd::view::SharedRows<kd::world::Activity> after;
    after.append(trail);
    CHECK(&before.front() == &after.front());
    CHECK(before.size() == 1537);
    CHECK(after.size() == 1538);
    trail.drop_prefix(513);
    kd::view::SharedRows<kd::world::Activity> pruned;
    pruned.append(trail);
    CHECK(pruned.front().start == 513);
    CHECK(&pruned[511] == &before[1024]);
    CHECK(before.front().start == 0);
    CHECK(before[1536].start == 1536);
}

TEST_CASE("display publications share immutable history and spent pages while preserving older serial lookups") {
    kd::data::Catalogue cat;
    REQUIRE(cat.load(kd::data::read_catalogue(KD_REPO "/data")).empty());
    kd::demo::CrowdWorld camp(333, cat, 1, true, true);
    auto& w = camp.world();
    const auto home = camp.camp_ids().front();
    w.retain_records(0);
    const auto original_archive = w.item_archive().size();
    const auto spent = [&] {
        const auto h = w.make_thing();
        const auto id = w.things().id_of(h);
        w.things().raw().emplace<kd::world::Place>(h);
        auto& item = w.things().raw().emplace<kd::world::Item>(h);
        item.home = home;
        item.kind = item.material = *cat.find("item", "base:flint");
        item.state = 4;
        item.length = 100;
        return id;
    };
    for (int n = 0; n < 513; ++n) (void)spent();
    w.retain_records(0);
    auto& history = w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(home));
    // Labelled publication load; these records never enter an acceptance observer.
    for (std::uint64_t n = 1; n <= 100001; ++n) {
        kd::world::Result event;
        event.id = n;
        event.kind = 2;
        history.events.push_back(event);
    }
    history.next = 100002;
    kd::view::CrowdStepper stepper(camp);
    REQUIRE(stepper.snapshots().take());
    const auto before = stepper.snapshots().front();
    REQUIRE(before.craft_history.size() == 1);
    CHECK(before.craft_history.front().events.pages()[0] == history.events.pages()[0]);
    CHECK(before.item_archive.pages()[0] == w.item_archive().pages()[0]);
    const auto next = spent();
    w.retain_records(0);
    kd::world::Result added;
    added.id = history.next++;
    added.kind = 2;
    history.events.push_back(added);
    stepper.refresh();
    REQUIRE(stepper.snapshots().take());
    const auto& after = stepper.snapshots().front();
    CHECK(before.craft_history.front().events.size() == 100001);
    CHECK(after.craft_history.front().events.size() == 100002);
    CHECK(after.craft_history.front().events.pages()[0] == before.craft_history.front().events.pages()[0]);
    CHECK(after.craft_history.front().public_index().size() == 100002);
    const auto serial = next.value & ((std::uint64_t{1} << 60U) - 1);
    CHECK(before.archive_index.get(serial) == kd::RecordIndex::kMissing);
    CHECK(after.archive_index.get(serial) == original_archive + 513);
    CHECK(before.item_archive.size() == original_archive + 513);
    CHECK(after.item_archive.size() == original_archive + 514);
    CHECK(&before.items.front() == &after.items.front());
}

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

TEST_CASE("immutable item trails and personal knowledge do not expose a future frontier to the screen") {
    auto files = kd::data::read_folder(KD_REPO "/data");
    kd::data::Catalogue c;
    REQUIRE(c.load(files).empty());
    kd::demo::CrowdWorld camp(83, c, 1, true, true);
    kd::view::CrowdStepper stepper(camp);
    REQUIRE(stepper.snapshots().take());
    const auto initial = stepper.snapshots().front();
    auto frontier = kd::time::Seconds{0};
    while (frontier < 90000) frontier = stepper.advance(frontier, 90000);
    REQUIRE(stepper.snapshots().take());
    const auto future = stepper.snapshots().front();
    // Output-only footprint diagnosis, not an acceptance scene or altered law.
    std::set<const void*> unique, payloads;
    std::uint64_t knowledge_bytes = 0, work_bytes = 0, item_bytes = 0, heat_bytes = 0;
    for (std::size_t n = 0; n < future.knowledge.size(); ++n) {
        const auto& k = future.knowledge[n];
        if (!k || !unique.insert(k.identity()).second) continue;
        knowledge_bytes += k.retained_bytes(payloads);
    }
    for (std::size_t n = 0; n < future.works.size(); ++n)
        if (const auto& work = future.works[n]; work)
            work_bytes += work->inputs.capacity() * sizeof(kd::world::Reservation);
    for (std::size_t n = 0; n < future.items.size(); ++n) {
        const auto& item = future.items[n];
        item_bytes += sizeof(item) + item.item.parents.capacity() * sizeof(kd::world::Link);
        if (item.timer) heat_bytes += item.timer->heat_sources.capacity() * sizeof(kd::world::HeatCredit);
    }
    std::fprintf(stderr,
                 "DISPLAY-COST frontier=%lld ways=%zu unique_knowledge=%zu knowledge_bytes=%llu "
                 "work_bytes=%llu item_bytes=%llu heat_credit_bytes=%llu\n",
                 static_cast<long long>(future.frontier), future.ways.size(), unique.size(),
                 static_cast<unsigned long long>(knowledge_bytes), static_cast<unsigned long long>(work_bytes),
                 static_cast<unsigned long long>(item_bytes), static_cast<unsigned long long>(heat_bytes));
    CHECK(future.frontier == 90000);
    CHECK(future.item_first.size() >= initial.item_first.size());
    std::int64_t mass = 0;
    std::size_t visible = 0;
    for (std::size_t i = 0; i + 1 < future.item_first.size(); ++i) {
        const auto* item = future.item_at(i, 0);
        if (!item) continue;
        ++visible;
        mass += item->item.mass;
        CHECK(item->item.made_at == (item->fire ? 0 : -1));
    }
    CHECK(visible + 1 == initial.item_first.size());
    CHECK(mass == 450000000);
    for (std::size_t i = 0; i < future.walkers.size(); ++i) {
        const auto at = future.way_index(i, 0);
        REQUIRE(future.knowledge[at]);
        CHECK(future.knowledge[at]->skills.size() == 5);
        CHECK(future.knowledge[at]->memories.empty());
        const auto& work = future.works[at];
        REQUIRE(work.has_value());
        if (work.has_value()) CHECK(work->state == 0);
    }
    const auto digest = camp.world().digests().whole;
    stepper.set_screen(90000);
    stepper.refresh();
    REQUIRE(stepper.snapshots().take());
    CHECK(camp.world().digests().whole == digest);
    // The old display-owned value survives slot recycling and producer pruning.
    CHECK(initial.items.front().item.mass > 0);
    CHECK(initial.items.front().item.made_at == -1);
}
TEST_CASE("fire and comfort snapshots sample the display time while the producer runs ahead") {
    kd::data::Catalogue c;
    REQUIRE(c.load(kd::data::read_folder(KD_REPO "/data")).empty());
    kd::demo::CrowdWorld ahead(93, c, 1, true, true);
    kd::demo::CrowdWorld reference(93, c, 1, true, true);
    kd::view::CrowdStepper stepper(ahead);
    auto frontier = kd::time::Seconds{0};
    while (frontier < 36000) frontier = stepper.advance(frontier, 36000);
    REQUIRE(stepper.snapshots().take());
    const auto snapshot = stepper.snapshots().front();
    const auto digest = ahead.world().digests().whole;
    for (const kd::time::Seconds screen : {199, 599, 1199, 2999, 4999, 21999, 29999}) {
        reference.world().run_to(screen + 1);
        for (std::size_t i = 0; i < snapshot.walkers.size(); ++i) {
            const auto id = kd::ecs::Id{snapshot.walkers[i].id};
            const auto h = reference.world().beings().handle(id);
            const auto sampled =
                snapshot.thermal_at(i, static_cast<double>(screen), reference.living()->rules().water_day);
            REQUIRE(sampled);
            const auto actual = kd::demo::FireRules::sample_thermal(reference.world(), h, screen);
            kd::ByteWriter a, b;
            kd::ecs::write_component(sampled.value_or(kd::world::Thermal{}), a);
            kd::ecs::write_component(actual, b);
            CHECK(a.take() == b.take());
            const auto k = snapshot.way_index(i, static_cast<double>(screen));
            REQUIRE(snapshot.lives[k]);
            const auto shown =
                reference.living()->sample(snapshot.lives[k].value_or(kd::world::Life{}), snapshot.ways[k], screen,
                                           sampled.value_or(kd::world::Thermal{}).water_due_ml);
            const auto& body = reference.world().beings().raw().get<kd::world::Life>(h);
            const auto& act = reference.world().beings().raw().get<kd::world::Activity>(h);
            const auto real = reference.living()->sample(body, act, screen, actual.water_due_ml);
            CHECK(shown.water == real.water);
            CHECK(shown.awake == real.awake);
        }
        for (std::size_t i = 0; i + 1 < snapshot.item_first.size(); ++i) {
            const auto* item = snapshot.item_at(i, static_cast<double>(screen));
            if (!item || !item->fire) continue;
            const auto rh = reference.world().things().handle(item->id);
            CHECK(item->fire->heat == reference.world().things().raw().get<kd::world::Fire>(rh).heat);
        }
    }
    CHECK(ahead.world().digests().whole == digest);
}

TEST_CASE("recovery skips unseen knowledge trails through a long camp interval without changing its digest") {
    using namespace kd;
    data::Catalogue catalogue;
    REQUIRE(catalogue.load(data::read_folder(KD_REPO "/data")).empty());
    demo::CrowdWorld camp(17, catalogue, 1, true, true);
    demo::CrowdWorld reference(17, catalogue, 1, true, true);
    view::CrowdStepper stepper(camp);
    const auto goal = 2 * time::kDay;
    stepper.set_screen(static_cast<double>(goal));
    time::Seconds frontier = 0;
    while (frontier < goal) {
        frontier = stepper.advance(frontier, goal);
        REQUIRE(stepper.snapshots().take());
        const auto& snapshot = stepper.snapshots().front();
        CHECK(snapshot.knowledge.size() == snapshot.walkers.size());
        CHECK(snapshot.ways.size() == snapshot.walkers.size());
    }
    reference.world().run_to(goal);
    CHECK(camp.world().digests().whole == reference.world().digests().whole);
}
