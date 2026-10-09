#include "kd/demo/living.hpp"
#include "doctest.h"
#include "helpers.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
namespace {
using namespace kd;
struct One {
    demo::CrowdWorld camp{17, test::camp_fixture(), 1, true};
    world::World& w = camp.world();
    ecs::Id id{}, home = camp.camp_ids().front();
    world::Beings::Handle h{}, ch = w.beings().handle(home);
    One() {
        std::vector<ecs::Id> remove;
        w.beings().each([&](ecs::Id other, world::Beings::Handle handle) {
            if (other.family() != ecs::Family::person) return;
            if (id.value == 0) {
                id = other;
                h = handle;
            } else
                remove.push_back(other);
        });
        for (const auto other : remove) {
            for (std::uint32_t s = 0; s < 4; ++s) w.cancel(other, s);
            w.beings().end(other);
        }
    }
    world::Life& life() { return w.beings().raw().get<world::Life>(h); }
    world::Camp& facts() { return w.beings().raw().get<world::Camp>(ch); }
    world::Habitat& env() { return w.beings().raw().get<world::Habitat>(ch); }
    world::Activity& act() { return w.beings().raw().get<world::Activity>(h); }
    void know() {
        life().known_at = {facts().food_at, facts().water_at, facts().shelter_at};
        life().known_amount = {facts().food_mg, facts().water_ml, 1};
        life().seen = {0, 0, 0};
        life().source = {1, 1, 2};
    }
    void set_action(world::LivingAct what, num::Point at, time::Seconds end, std::int64_t portion) {
        w.beings().raw().get<world::Place>(h).at = at;
        act() = {static_cast<std::uint8_t>(what), 0, end, at, at};
        life().settled = 0;
        life().portion = portion;
        life().applied = 0;
        w.schedule(id, world::kActivitySlot, end);
    }
};
}  // namespace
TEST_CASE("living choices favour a thirstier otherwise identical person using recorded knowledge") {
    One fed, thirsty;
    fed.know();
    thirsty.know();
    fed.life().food = thirsty.life().food = 2800000;
    fed.life().water = 2700;
    thirsty.life().water = 900;
    fed.life().awake = thirsty.life().awake = 0;
    fed.w.run_to(2);
    thirsty.w.run_to(2);
    CHECK(fed.life().goal == 0);
    CHECK(thirsty.life().goal == 1);
    CHECK(thirsty.life().scores[1] > fed.life().scores[1]);
    CHECK(thirsty.life().decision_needs[1] < fed.life().decision_needs[1]);
}
TEST_CASE("interrupted meals retain exactly their elapsed share and cannot eat the same food twice") {
    One one;
    one.know();
    one.life().food = 1000000;
    one.life().water = 100;
    one.life().carried_food = 1000000;
    one.life().goal = 0;
    one.set_action(world::LivingAct::eat, one.facts().shelter_at, 1200, 1000000);
    one.w.schedule(one.id, world::kCallSlot, 600);
    one.w.run_to(601);
    CHECK(one.life().carried_food == 500000);
    CHECK(one.life().food == 1000000 - 27777 + 500000);
    CHECK(one.life().water == 100 - 20 + 400);
    const auto carried = one.life().carried_food;
    auto snapshot = one.w.save();
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), snapshot, why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK(copy->world().beings().raw().get<world::Life>(copy->world().beings().handle(one.id)).carried_food == carried);
    one.w.run_to(10000);
    copy->world().run_to(10000);
    CHECK(copy->world().digests().whole == one.w.digests().whole);
}
TEST_CASE("interrupted drinking returns unused allocation without duplicating water, even after pool renewal") {
    One one;
    one.know();
    one.life().water = 100;
    one.life().goal = 1;
    one.facts().water_ml -= 500;
    one.env().water_taken = 500;
    one.life().allocated_water = 500;
    one.set_action(world::LivingAct::drink, one.facts().water_at, 7200, 500);
    one.w.schedule(one.id, world::kCallSlot, 4000);
    one.w.run_to(4001);
    CHECK(one.env().water_spilled == 223);
    CHECK(one.life().allocated_water == 500);
    CHECK(one.env().water_taken == 1000);
    CHECK(one.life().water == 277);  // initial water depleted; actual elapsed share only
    CHECK(one.facts().water_ml <= one.env().water_cap_ml);
    std::string why;
    CHECK(demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why));
}
TEST_CASE("interrupted gathering removes only earned available berries once and carries the result") {
    One one;
    one.know();
    one.life().goal = 0;
    one.set_action(world::LivingAct::gather, one.facts().food_at, 1800, 1000000);
    one.w.schedule(one.id, world::kCallSlot, 900);
    one.w.run_to(901);
    CHECK(one.life().carried_food == 500000);
    CHECK(one.env().food_taken == 500000);
    CHECK(one.facts().food_mg == 24500000);
    CHECK(one.act().what == static_cast<std::uint8_t>(world::LivingAct::carry));
    one.w.run_to(1801);
    CHECK(one.env().food_taken == 500000);
}
TEST_CASE("absent food stays absent and picked fruit never exceeds bounded renewal inputs") {
    One absent;
    absent.facts().food_mg = 0;
    absent.env().food_cap_mg = 0;
    absent.know();
    absent.w.run_to(3 * time::kDay);
    CHECK(absent.facts().food_mg == 0);
    CHECK(absent.env().food_grown == 0);
    CHECK(absent.life().carried_food == 0);
    CHECK(absent.life().food == 0);
    One grown;
    grown.facts().food_mg = 0;
    grown.env().root_water_ml = 1000;
    grown.env().crop_budget_mg = 3000000;
    grown.w.cancel(grown.id, world::kActivitySlot);
    grown.w.run_to(7201);
    CHECK(grown.env().food_grown == 1000000);
    CHECK(grown.facts().food_mg == 1000000);
    CHECK(grown.env().root_water_ml == 0);
    CHECK(grown.env().crop_budget_mg == 2000000);
}
TEST_CASE("blocked water gives an honest rejected option and never teleports through rock") {
    One one;
    one.know();
    one.life().water = 1;
    const auto centre = one.w.beings().raw().get<world::Place>(one.ch).at;
    one.env().rock_west = 900;
    one.env().rock_east = 1300;
    one.env().rock_south = 1000;
    one.env().rock_north = 1400;
    CHECK(demo::Living::route(one.w, one.home, one.act().from, one.facts().water_at).empty());
    one.w.run_to(2);
    CHECK(one.life().goal != 1);
    CHECK(one.life().scores[1] == -1000000);
    CHECK(one.life().blocked_until[1] > one.w.frontier());
    one.w.run_to(7200);
    const auto at = one.act().at(one.w.torus(), one.w.frontier());
    CHECK(at != one.facts().water_at);
    CHECK(one.w.torus().distance(at, centre) <= 2400);
}
TEST_CASE("sight respects rock, darkness and hourly noticing; a remembered source survives out of sight") {
    One one;
    const auto centre = one.w.beings().raw().get<world::Place>(one.ch).at;
    const auto from = one.w.torus().moved(centre, {-400, 0});
    const auto to = one.w.torus().moved(centre, {400, 0});
    CHECK_FALSE(demo::Living::visible(one.w, one.home, from, to));
    CHECK(one.life().source[0] == 0);
    CHECK(one.life().source[1] == 0);
    one.w.run_to(6 * time::kHour + 7200);
    CHECK(one.life().source[0] > 0);
    CHECK(one.life().source[1] > 0);
    CHECK(one.life().seen[0] <= one.w.frontier());
    CHECK(one.life().seen[1] <= one.w.frontier());
    CHECK(one.life().source[2] > 0);
}
TEST_CASE("old idle camp snapshot migrates once and all new action phases resume identically") {
    std::string why;
    const auto bytes = kd::save::DiskFiles(std::string(KD_REPO) + "/sim/tests/fixtures").read("camp-31301.kds");
    REQUIRE(bytes);
    if (!bytes) return;
    const auto chunks = save::read_snapshot(*bytes, why);
    REQUIRE(chunks);
    if (!chunks) return;
    auto old = demo::CrowdWorld::open(test::camp_fixture(), *chunks, why);
    REQUIRE(old);
    if (!old) return;
    CHECK(old->world().frontier() == 25200);
    auto second = demo::CrowdWorld::open(test::camp_fixture(), old->world().save(), why);
    REQUIRE(second);
    if (!second) return;
    CHECK(old->world().digests().whole == second->world().digests().whole);
    std::array<bool, 8> covered{};
    for (std::int64_t t = 25201; t < 2 * time::kDay; t += 61) {
        old->world().run_to(t);
        bool novel = false;
        old->world().beings().each([&](ecs::Id id, world::Beings::Handle h) {
            if (id.family() != ecs::Family::person) return;
            const auto act = old->world().beings().raw().get<world::Activity>(h).what;
            if (!covered[act]) {
                covered[act] = true;
                novel = true;
            }
        });
        if (!novel) continue;
        auto copy = demo::CrowdWorld::open(test::camp_fixture(), old->world().save(), why);
        REQUIRE(copy);
        if (!copy) return;
        copy->world().run_to(t + 3600);
        second->world().run_to(t + 3600);
        CHECK(copy->world().digests().whole == second->world().digests().whole);
    }
    for (const auto act : {0, 1, 2, 4, 5, 6, 7}) CHECK(covered[static_cast<std::size_t>(act)]);
}
TEST_CASE("five fixed camp seeds live for seven days with identical worker counts and reopening") {
    for (const auto seed : {1U, 3U, 17U, 42U, 91U}) {
        demo::CrowdWorld reference(seed, test::camp_fixture(), 1, true);
        std::string why;
        auto copy = demo::CrowdWorld::open(test::camp_fixture(), reference.world().save(), why);
        REQUIRE(copy);
        if (!copy) return;
        run::Workers workers(4);
        std::array<std::int64_t, 3> minimum{100, 100, 100};
        for (std::int64_t day = 1; day <= 7; ++day) {
            reference.world().run_to(day * time::kDay);
            copy->world().run_islands(day * time::kDay, workers, 3600);
            CHECK(reference.world().digests().whole == copy->world().digests().whole);
            const auto ch = reference.world().beings().handle(reference.camp_ids().front());
            const auto& facts = reference.world().beings().raw().get<world::Camp>(ch);
            const auto& env = reference.world().beings().raw().get<world::Habitat>(ch);
            CHECK(env.food_cap_mg + env.food_grown == facts.food_mg + env.food_taken);
            CHECK(env.water_cap_ml + env.water_added == facts.water_ml + env.water_taken);
            reference.world().beings().each([&](ecs::Id id, world::Beings::Handle h) {
                if (id.family() != ecs::Family::person) return;
                const auto l = reference.living()->sample(reference.world().beings().raw().get<world::Life>(h),
                                                          reference.world().beings().raw().get<world::Activity>(h),
                                                          day * time::kDay);
                const auto needs = demo::Living::needs(l);
                for (std::size_t i = 0; i < 3; ++i) minimum[i] = std::min(minimum[i], needs[i]);
            });
        }
        INFO(seed);
        CHECK(minimum[0] > 20);
        CHECK(minimum[1] > 20);
        CHECK(minimum[2] > 20);
    }
}

TEST_CASE("checksummed living snapshots reject missing, duplicate, truncated and impossible saved work") {
    for (int fault = 0; fault < 8; ++fault) {
        One one;
        one.know();
        if (fault == 3) one.life().awake = -1;
        if (fault == 4) one.life().applied = one.life().portion + 1;
        if (fault == 5) one.life().allocated_water = 10;
        if (fault == 6) one.life().known_at[1].x += one.w.torus().width();
        if (fault == 7) {
            const auto centre = one.w.beings().raw().get<world::Place>(one.ch).at;
            const auto from = one.w.torus().moved(centre, {0, 0});
            one.set_action(world::LivingAct::walk, from, 100, 0);
            one.act().to = one.w.torus().moved(centre, {600, 0});
        }
        auto chunks = one.w.save();
        if (fault == 0) std::erase_if(chunks, [](const auto& c) { return c.tag == save::tag("LIFE"); });
        if (fault == 1) chunks.push_back(*save::find_chunk(chunks, save::tag("LIFE")));
        if (fault == 2)
            for (auto& c : chunks)
                if (c.tag == save::tag("LIFE")) c.data.pop_back();
        std::string why;
        const auto decoded = save::read_snapshot(save::write_snapshot(chunks), why);
        REQUIRE(decoded);
        if (!decoded) return;
        CHECK_FALSE(demo::CrowdWorld::open(test::camp_fixture(), *decoded, why));
    }
}
TEST_CASE("checksummed living snapshots reject lost, mistimed or invalid queued action events") {
    for (int fault = 0; fault < 5; ++fault) {
        One one;
        auto chunks = one.w.save();
        auto events = one.w.queue().live_in_order([](const auto&) { return true; });
        event::Queue damaged;
        for (auto e : events) {
            if (e.key.owner == one.id.value) {
                if (fault == 0) e.slot = 100;
                if (fault == 1) ++e.key.second;
                if (fault == 2) continue;
                if (fault == 3) e.key.owner += 1000;
            }
            if (fault == 4 && e.key.owner == one.home.value) ++e.key.second;
            damaged.push(e);
        }
        ByteWriter bytes;
        damaged.write(bytes, [](const auto&) { return true; });
        for (auto& c : chunks)
            if (c.tag == save::tag("QUEU")) c.data = bytes.bytes();
        std::string why;
        const auto decoded = save::read_snapshot(save::write_snapshot(chunks), why);
        REQUIRE(decoded);
        if (!decoded) return;
        CHECK_FALSE(demo::CrowdWorld::open(test::camp_fixture(), *decoded, why));
    }
}
TEST_CASE("walking and resting retain actual elapsed progress when called away") {
    One walk;
    walk.know();
    const auto from = walk.facts().shelter_at;
    walk.set_action(world::LivingAct::walk, from, 100, 0);
    walk.act().to = walk.w.torus().moved(from, {400, 0});
    walk.w.schedule(walk.id, world::kCallSlot, 50);
    walk.w.run_to(51);
    CHECK(walk.act().from == walk.w.torus().moved(from, {200, 0}));
    One rest;
    rest.know();
    rest.life().awake = 60000;
    rest.set_action(world::LivingAct::rest, rest.facts().shelter_at, 7200, 14400);
    rest.w.schedule(rest.id, world::kCallSlot, 3600);
    rest.w.run_to(3601);
    CHECK(rest.life().awake == 52800);
    CHECK(rest.life().memory_kind == 2);
    CHECK(rest.life().memory_amount == 3600);
}

TEST_CASE("thirty-six awake hours force sleep where the exhausted person stands") {
    One one;
    one.know();
    one.life().awake = 129600;
    one.life().food = one.life().water = 0;
    const auto here = one.w.beings().raw().get<world::Place>(one.h).at;
    one.w.run_to(2);
    CHECK(one.life().goal == 2);
    CHECK(one.act().what == 2);
    CHECK(one.act().from == here);
    CHECK(one.act().to == here);
}

TEST_CASE("same-second first living save recovers its migration baseline after corruption") {
    std::string why;
    const auto bytes = save::DiskFiles(std::string(KD_REPO) + "/sim/tests/fixtures").read("camp-31301.kds");
    REQUIRE(bytes);
    if (!bytes) return;
    auto older = save::read_snapshot(*bytes, why);
    REQUIRE(older);
    if (!older) return;
    // A second genuine idle snapshot, thirty seconds earlier, with a fresh whole-file checksum.
    for (auto& c : *older) {
        if (c.tag != save::tag("WRLD")) continue;
        ByteWriter at;
        at.i64(25170);
        std::copy(at.bytes().begin(), at.bytes().end(), c.data.begin() + 8);
    }
    save::FakeFiles files;
    REQUIRE(files.write_whole("snapshots/00000000000000025170.kds", save::write_snapshot(*older)));
    REQUIRE(files.write_whole("snapshots/00000000000000025200.kds", *bytes));
    std::uint64_t expected = 0;
    {
        save::Keeper keeper(files, "31302-test");
        auto kept = demo::keep_crowd(keeper, test::camp_fixture(), 17, 1, {}, true);
        REQUIRE(kept.crowd);
        if (!kept.crowd) return;
        auto& w = kept.crowd->world();
        keeper.snapshot(w);  // Pause/save before even one second of the new rules has run.
        keeper.flush();
        std::vector<world::Record> history;
        w.keep_history(&history);
        w.run_to(27000);
        keeper.history(history);
        keeper.pause_mark(w.frontier());
        keeper.flush();
        expected = w.digests().whole;
    }
    files.raw("snapshots/00000000000000025200.kds").back() ^= std::byte{1};
    save::Keeper keeper(files, "31302-test");
    auto kept = demo::keep_crowd(keeper, test::camp_fixture(), 17, 1, {}, true);
    REQUIRE(kept.crowd);
    if (!kept.crowd) return;
    CHECK_FALSE(kept.damaged.empty());
    CHECK(kept.was_at == 27000);
    std::vector<world::Record> history;
    auto& w = kept.crowd->world();
    w.keep_history(&history);
    w.run_to(kept.was_at);
    keeper.history(history);
    keeper.flush();
    CHECK(w.digests().whole == expected);
    CHECK(keeper.mismatches() == 0);
}

TEST_CASE("migration sealing survives a power cut between every storage call and reports failures") {
    std::string why;
    const auto bytes = save::DiskFiles(std::string(KD_REPO) + "/sim/tests/fixtures").read("camp-31301.kds");
    REQUIRE(bytes);
    if (!bytes) return;
    const auto chunks = save::read_snapshot(*bytes, why);
    REQUIRE(chunks);
    if (!chunks) return;
    auto converted = demo::CrowdWorld::open(test::camp_fixture(), *chunks, why);
    REQUIRE(converted);
    if (!converted) return;
    auto& w = converted->world();
    save::FakeFiles original;
    REQUIRE(original.write_whole("snapshots/00000000000000025200.kds", *bytes));
    std::uint64_t calls = 0;
    {
        save::FakeFiles files = original;
        save::Keeper keeper(files, "31302-test");
        const auto found = keeper.open();
        CHECK(keeper.begin(found, test::camp_fixture()) == save::Update::small);
        const auto before = files.calls();
        REQUIRE(keeper.seal_camp_start(w));
        calls = files.calls() - before;
    }
    REQUIRE(calls > 0);
    for (std::uint64_t cut = 0; cut <= calls; ++cut) {
        INFO(cut);
        save::FakeFiles files = original;
        {
            save::Keeper keeper(files, "31302-test");
            const auto found = keeper.open();
            CHECK(keeper.begin(found, test::camp_fixture()) == save::Update::small);
            files.stop_after(cut);
            const bool sealed = keeper.seal_camp_start(w);
            CHECK(sealed == !keeper.failed());
            if (cut < calls) CHECK_FALSE(sealed);
        }
        files.restart();
        files.power_cut();
        save::Keeper recovered(files, "31302-test");
        const auto found = recovered.open();
        REQUIRE(found.snapshot);
        if (!found.snapshot) return;
        auto same = demo::CrowdWorld::open(test::camp_fixture(), *found.snapshot, why);
        REQUIRE(same);
        if (!same) return;
        CHECK(same->world().digests().whole == w.digests().whole);
    }
}
