#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

#include "crowd_core.hpp"
#include "doctest.h"
#include "heat.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/fire.hpp"
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
