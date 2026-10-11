#include "kd/demo/living.hpp"
#include <limits>
#include "doctest.h"
#include "helpers.hpp"
#include "kd/chance/chance.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/save/archive.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
namespace {
using namespace kd;
struct One {
    demo::CrowdWorld camp;
    world::World& w;
    ecs::Id id{}, home{};
    world::Beings::Handle h{}, ch{};
    One(std::uint64_t seed = 17)
        : camp(seed, test::camp_fixture(), 1, true),
          w(camp.world()),
          home(camp.camp_ids().front()),
          ch(w.beings().handle(home)) {
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
TEST_CASE("pending place dreams require a live person or a typed ended person of their camp") {
    for (int fault = 0; fault < 8; ++fault) {
        One one;
        one.know();
        one.w.command(0, 2, one.id.value, 0);
        one.w.run_to(1);
        auto& ledger = one.w.beings().raw().get<world::Dreams>(one.ch);
        REQUIRE(ledger.acts.size() == 1);
        if (fault == 1 || fault == 2) {
            const auto item = one.w.make_thing();
            const auto serial = one.w.things().id_of(item).value & ((std::uint64_t{1} << 60U) - 1);
            ledger.acts.front().person = (std::uint64_t{3} << 60U) | serial;
            if (fault == 2) one.w.things().end(one.w.things().id_of(item));
        }
        if (fault >= 3) {
            one.w.end_being(one.id);
            REQUIRE(ledger.ended.size() == 1);
            if (fault == 4) ledger.ended.clear();
            if (fault == 5) ledger.ended.front().id = one.home;
            if (fault == 6) ledger.ended.front().ended_at = 2;
        }
        std::string why;
        auto chunks = one.w.save();
        if (fault == 7)
            for (auto& chunk : chunks)
                if (chunk.tag == save::tag("DRMS")) chunk.version = 2;
        auto opened = demo::CrowdWorld::open(test::camp_fixture(), chunks, why);
        INFO(fault, why);
        if (fault == 0 || fault == 3) {
            REQUIRE(opened);
            CHECK(opened->world().digests().whole == one.w.digests().whole);
            CHECK(save::write_snapshot(opened->world().save()) == save::write_snapshot(one.w.save()));
            one.w.run_to(2);
            opened->world().run_to(2);
            CHECK(opened->world().digests().whole == one.w.digests().whole);
            if (fault == 3) {
                one.w.run_to(time::kHour + 1);
                CHECK(ledger.acts.front().status == 3);
                auto cancelled = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
                INFO(why);
                REQUIRE(cancelled);
                CHECK(cancelled->world().digests().whole == one.w.digests().whole);
            }
        } else {
            CHECK_FALSE(opened);
            CHECK_FALSE(why.empty());
            if (fault == 7) CHECK(why.find("Start a new camp") != std::string::npos);
        }
    }
}
TEST_CASE("a sleeping place dream tips a close autonomous visit but urgent thirst wins") {
    for (const bool urgent : {false, true}) {
        One one;
        one.know();
        one.life().food = 4000000;
        one.life().water = urgent ? 300 : 3000;
        one.life().awake = 0;
        one.life().goal = 2;
        one.set_action(world::LivingAct::rest, one.act().from, 120, 14400);
        CHECK(demo::Living::dream_problem(one.w, one.id, 0).empty());
        one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
        one.w.run_to(121);
        const auto& thought = one.w.beings().raw().get<world::Dream>(one.h);
        CHECK(thought.subject == 0);
        CHECK(thought.at == 0);
        CHECK(thought.until == demo::Living::kDreamLife);
        CHECK(one.life().goal == (urgent ? 1 : 3));
        if (!urgent) {
            CHECK(one.life().explore_at == one.facts().food_at);
            CHECK(thought.decision_pull == demo::Living::kDreamPull);
        } else
            CHECK(thought.decision_pull == 0);
        CHECK(one.life().memory_kind == 2);  // ordinary sleep memory has no sender/request metadata
    }
}
TEST_CASE("awake dreams wait for sleep and keep requested and execution times through reopening") {
    One one;
    one.know();
    one.life().awake = 129590;
    one.life().goal = 3;
    one.set_action(world::LivingAct::watch, one.act().from, 120, 0);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
    one.w.run_to(1);
    auto& records = one.w.beings().raw().get<world::Dreams>(one.ch).acts;
    REQUIRE(records.size() == 1);
    if (records.size() != 1) return;
    CHECK(records[0].requested == 0);
    CHECK(records[0].received == 0);
    CHECK(records[0].executed == -1);
    CHECK(records[0].status == 1);
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
    REQUIRE(copy);
    if (!copy) return;
    one.w.run_to(121);
    copy->world().run_to(121);
    CHECK(records[0].executed == 120);
    CHECK(records[0].status == 2);
    CHECK(one.w.digests().whole == copy->world().digests().whole);
}
TEST_CASE("vanished queued dream subjects cancel without a remembered player trace") {
    One one;
    one.know();
    one.life().awake = 129590;
    one.set_action(world::LivingAct::watch, one.act().from, 120, 0);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
    one.w.run_to(1);
    one.env().food_cap_mg = one.facts().food_mg = 0;
    one.w.run_to(121);
    const auto& records = one.w.beings().raw().get<world::Dreams>(one.ch).acts;
    REQUIRE(records.size() == 1);
    if (records.size() != 1) return;
    CHECK(records[0].status == 3);
    CHECK(records[0].reason == 1);
    CHECK(one.w.beings().raw().get<world::Dream>(one.h).at == -1);
}
TEST_CASE("unknown dream subjects and a fourth queued sleeper are refused with saved caps") {
    demo::CrowdWorld camp(17, test::camp_fixture(), 1, true);
    auto& w = camp.world();
    const auto home = camp.camp_ids().front();
    std::vector<ecs::Id> people;
    w.beings().each([&](ecs::Id id, world::Beings::Handle) {
        if (id.family() == ecs::Family::person) people.push_back(id);
    });
    CHECK_FALSE(demo::Living::dream_problem(w, people[0], 0).empty());
    CHECK_FALSE(demo::Living::dream_problem(w, people[0], 99).empty());
    for (std::size_t i = 0; i < 4; ++i) w.command(0, demo::Living::kPlaceDream, people[i].value, 2);
    w.run_to(1);
    const auto& records = w.beings().raw().get<world::Dreams>(w.beings().handle(home)).acts;
    CHECK(records.size() == 3);
    CHECK_FALSE(demo::Living::dream_problem(w, people[0], 2).empty());
    CHECK_FALSE(demo::Living::dream_problem(w, people[4], 2).empty());
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), w.save(), why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK(copy->world().digests().whole == w.digests().whole);
    CHECK_FALSE(demo::Living::dream_problem(copy->world(), people[4], 2).empty());
}
TEST_CASE("dream caps count execution nights inside one fast batch and survive reopening") {
    One one;
    one.know();
    one.set_action(world::LivingAct::rest, one.facts().shelter_at, 50000, 0);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
    one.w.command(1, demo::Living::kPlaceDream, one.id.value, 1);
    one.w.command(21600, demo::Living::kPlaceDream, one.id.value, 1);
    one.w.command(21601, demo::Living::kPlaceDream, one.id.value, 2);
    one.w.run_to(21602);
    const auto& ledger = one.w.beings().raw().get<world::Dreams>(one.ch);
    REQUIRE(ledger.acts.size() == 2);
    if (ledger.acts.size() != 2) return;
    CHECK(ledger.acts[0].executed == 0);
    CHECK(ledger.acts[1].executed == 21600);
    CHECK(ledger.sent[0] == one.id.value);
    CHECK(ledger.sent[1] == 0);
    const auto& dream = one.w.beings().raw().get<world::Dream>(one.h);
    CHECK(dream.subject == 1);
    CHECK(dream.until == 21600 + demo::Living::kDreamLife);  // replaced, never accumulated
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK_FALSE(demo::Living::dream_problem(copy->world(), one.id, 0).empty());
}
TEST_CASE("checksummed dream state rejects missing caps, invented times and duplicated influence") {
    for (int fault = 0; fault < 12; ++fault) {
        One one;
        one.know();
        one.set_action(world::LivingAct::rest, one.facts().shelter_at, 120, 14400);
        one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
        one.w.run_to(1);
        auto& ledger = one.w.beings().raw().get<world::Dreams>(one.ch);
        auto& thought = one.w.beings().raw().get<world::Dream>(one.h);
        if (fault == 0) ledger.sent.fill(0);
        if (fault == 1) ledger.acts[0].executed = 2;
        if (fault == 2) thought.decision_pull = 120;
        if (fault == 3) ledger.acts[0].number = 99;
        if (fault == 4) ledger.acts[0].choice = 0;
        if (fault == 5) thought.until += time::kDay;
        if (fault == 8) ledger.acts[0].place.x = std::numeric_limits<std::int32_t>::min();
        if (fault == 9) thought.place.y = std::numeric_limits<std::int32_t>::min();
        if (fault == 10) {
            ledger.acts[0].decision_at = 0;
            ledger.acts[0].choice = 0;
            ledger.acts[0].pull = 59;
        }
        if (fault == 11) {
            thought.decision_subject = 0;
            thought.decision_pull = 59;
        }
        auto chunks = one.w.save();
        if (fault == 6) std::erase_if(chunks, [](const auto& c) { return c.tag == save::tag("DRMS"); });
        if (fault == 7) {
            const auto* c = save::find_chunk(chunks, save::tag("DRMS"));
            chunks.push_back(*c);
        }
        std::string why;
        const auto decoded = save::read_snapshot(save::write_snapshot(chunks), why);
        REQUIRE(decoded);
        if (!decoded) return;
        INFO(fault);
        CHECK_FALSE(demo::CrowdWorld::open(test::camp_fixture(), *decoded, why));
    }
}
TEST_CASE("three delivered dreams close the night's cap and a missing sleeper cancels its pending dream") {
    demo::CrowdWorld camp(17, test::camp_fixture(), 1, true);
    auto& w = camp.world();
    std::vector<ecs::Id> people;
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (id.family() != ecs::Family::person) return;
        people.push_back(id);
        auto& activity = w.beings().raw().get<world::Activity>(h);
        const auto at = activity.from;
        activity = {2, 0, 120, at, at};
        w.schedule(id, world::kActivitySlot, 120);
    });
    for (std::size_t i = 0; i < 4; ++i) w.command(0, demo::Living::kPlaceDream, people[i].value, 2);
    w.run_to(1);
    const auto home = camp.camp_ids().front();
    CHECK(w.beings().raw().get<world::Dreams>(w.beings().handle(home)).acts.size() == 3);
    CHECK_FALSE(demo::Living::dream_problem(w, people[3], 2).empty());
    One one;
    one.know();
    one.set_action(world::LivingAct::watch, one.act().from, 120, 0);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 2);
    one.w.run_to(1);
    for (std::uint32_t s = 0; s < 4; ++s) one.w.cancel(one.id, s);
    one.w.beings().end(one.id);
    one.w.run_to(3601);
    const auto& act = one.w.beings().raw().get<world::Dreams>(one.ch).acts.front();
    CHECK(act.status == 3);
    CHECK(act.reason == 3);
}
TEST_CASE("natural and sent place dreams leave identical ordinary thoughts and waking choices") {
    One identity;
    std::uint64_t seed = 1;
    for (; seed < 1000; ++seed) {
        const chance::Draws draws(seed, chance::name("living"), identity.id.value, -1, chance::name("place dream"));
        if (draws.between(0, 0, 59) == 0) break;
    }
    REQUIRE(seed < 1000);
    One natural(seed), sent(seed);
    for (One* one : {&natural, &sent}) {
        one->know();
        one->life().awake = 129600;
        one->life().food = 4000000;
        one->life().water = 3000;
    }
    sent.w.command(1, demo::Living::kPlaceDream, sent.id.value, 2);
    natural.w.run_to(2);
    sent.w.run_to(2);
    CHECK(natural.w.beings().raw().get<world::Dream>(natural.h).subject == 2);
    ByteWriter a, b;
    ecs::write_component(natural.w.beings().raw().get<world::Dream>(natural.h), a);
    ecs::write_component(sent.w.beings().raw().get<world::Dream>(sent.h), b);
    CHECK(a.bytes() == b.bytes());
    natural.w.run_to(7202);
    sent.w.run_to(7202);
    ByteWriter first, second;
    ecs::write_component(natural.life(), first);
    ecs::write_component(sent.life(), second);
    CHECK(first.bytes() == second.bytes());
}
TEST_CASE("sent dreams remain identical across worker counts and saved pending continuation") {
    demo::CrowdWorld reference(17, test::camp_fixture(), 1, true);
    auto& w = reference.world();
    ecs::Id first{};
    w.beings().each([&](ecs::Id id, world::Beings::Handle) {
        if (first.value == 0 && id.family() == ecs::Family::person) first = id;
    });
    w.command(0, demo::Living::kPlaceDream, first.value, 2);
    w.run_to(1);
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), w.save(), why);
    REQUIRE(copy);
    if (!copy) return;
    run::Workers workers(4);
    for (time::Seconds day = 1; day <= 4; ++day) {
        w.run_to(day * time::kDay);
        copy->world().run_islands(day * time::kDay, workers, 600);
        CHECK(w.digests().whole == copy->world().digests().whole);
        copy = demo::CrowdWorld::open(test::camp_fixture(), copy->world().save(), why);
        REQUIRE(copy);
        if (!copy) return;
    }
}
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
    CHECK(one.life().water == 239);  // 100 + 277 ml intake - 138 ml bodily use
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

TEST_CASE("actions begun above the fatigue threshold stop at thirty-six awake hours and reopen exactly") {
    One one;
    one.know();
    one.life().food = 0;
    one.life().water = 3000;
    one.life().awake = 129000;  // 35 hours 50 minutes; hunger wins before the hard sleep limit.
    one.w.run_to(590);
    CHECK(one.act().what == static_cast<std::uint8_t>(world::LivingAct::gather));
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
    REQUIRE(copy);
    if (!copy) return;
    one.w.run_to(601);
    copy->world().run_to(601);
    CHECK(one.act().what == static_cast<std::uint8_t>(world::LivingAct::rest));
    CHECK(one.act().start == 600);
    CHECK(one.life().goal == 2);
    CHECK(one.life().carried_food > 0);
    CHECK(one.life().carried_food < 1000000);
    CHECK(one.act().from == one.act().to);
    CHECK(copy->world().digests().whole == one.w.digests().whole);
}

TEST_CASE("intake and bodily depletion combine before clamping depleted meals and drinks") {
    for (const auto act : {world::LivingAct::eat, world::LivingAct::drink}) {
        One one;
        one.life().food = one.life().water = 0;
        const auto end = act == world::LivingAct::eat ? 1200 : 120;
        const auto portion = act == world::LivingAct::eat ? 1000000 : 500;
        one.life().carried_food = act == world::LivingAct::eat ? portion : 0;
        one.life().allocated_water = act == world::LivingAct::drink ? portion : 0;
        one.set_action(act, one.act().from, end, portion);
        const auto partial = one.camp.living()->sample(one.life(), one.act(), end / 2);
        const auto full = one.camp.living()->sample(one.life(), one.act(), end);
        INFO(static_cast<int>(act));
        if (act == world::LivingAct::eat) {
            CHECK(partial.food == 472223);  // 500000 intake - 27777 depletion
            CHECK(full.food == 944445);     // 1000000 intake - 55555 depletion
            CHECK(partial.water == 380);    // berry water also pays bodily consumption
            CHECK(full.water == 759);
            CHECK(full.carried_food == 0);
        } else {
            CHECK(partial.water == 248);  // 250 intake - 2 depletion
            CHECK(full.water == 496);
            CHECK(full.food == 0);
            CHECK(full.allocated_water == 0);
        }
        CHECK(one.life().food == 0);  // display sampling never settles the saved body
        CHECK(one.life().water == 0);
    }
}

TEST_CASE("expired dream influence stops drawing a visit even when all needs can wait") {
    One one;
    one.know();
    one.set_action(world::LivingAct::rest, one.facts().shelter_at, demo::Living::kDreamLife + 1, 0);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
    one.w.run_to(demo::Living::kDreamLife);
    one.life().food = 4000000;
    one.life().water = 3000;
    one.life().awake = 0;
    one.life().settled = demo::Living::kDreamLife;
    one.act().start = demo::Living::kDreamLife;
    one.w.run_to(demo::Living::kDreamLife + 2);
    const auto& thought = one.w.beings().raw().get<world::Dream>(one.h);
    CHECK(thought.at == 0);
    CHECK(thought.until == demo::Living::kDreamLife);
    CHECK(thought.decision_pull == 0);
    CHECK(one.life().scores[3] == 0);
}
TEST_CASE("a queued water dream in seed seventeen tips Ari's ordinary visit after sleep") {
    demo::CrowdWorld normal(17, test::camp_fixture(), 1, true), dreamt(17, test::camp_fixture(), 1, true);
    const ecs::Id ari{3458764513820540930ULL};
    normal.world().run_to(25200);
    dreamt.world().run_to(25200);
    CHECK(demo::Living::dream_problem(dreamt.world(), ari, 1).empty());
    dreamt.world().command(25200, demo::Living::kPlaceDream, ari.value, 1);
    normal.world().run_to(42601);
    dreamt.world().run_to(42601);
    const auto h = dreamt.world().beings().handle(ari);
    const auto& thought = dreamt.world().beings().raw().get<world::Dream>(h);
    const auto& life = dreamt.world().beings().raw().get<world::Life>(h);
    const auto& control = normal.world().beings().raw().get<world::Life>(normal.world().beings().handle(ari));
    CHECK(life.goal == 3);
    CHECK(thought.decision_pull == 60);
    CHECK(life.explore_at == life.known_at[1]);
    CHECK(control.explore_at != life.explore_at);
    CHECK(thought.visit_at >= thought.at);
    const auto home = dreamt.camp_ids().front();
    const auto& act =
        dreamt.world().beings().raw().get<world::Dreams>(dreamt.world().beings().handle(home)).acts.front();
    CHECK(act.requested == 25200);
    CHECK(act.executed > act.requested);
    CHECK(act.pull == 60);
    CHECK(act.visited_at == thought.visit_at);
}

TEST_CASE("dream arrival records the actual use spot near remembered food rather than demanding its centre") {
    One one;
    one.know();
    one.life().food = 2800000;
    one.life().water = 3000;
    one.life().awake = 0;
    const auto near = one.w.torus().moved(one.facts().food_at, {-100, 0});
    one.set_action(world::LivingAct::rest, near, 120, 14400);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
    one.w.run_to(300);
    CHECK(one.life().goal == 0);
    CHECK(one.act().what == static_cast<std::uint8_t>(world::LivingAct::gather));
    CHECK(one.act().from != one.facts().food_at);
    const auto& thought = one.w.beings().raw().get<world::Dream>(one.h);
    CHECK(thought.decision_pull == 60);
    CHECK(thought.visit_at >= 120);
    CHECK(one.w.beings().raw().get<world::Dreams>(one.ch).acts[0].visited_at == thought.visit_at);
}

TEST_CASE("a food dream keeps its site arrival after carrying the berries home to eat") {
    One one;
    one.know();
    one.life().food = 2800000;
    one.life().water = 3000;
    one.life().awake = 0;
    const auto near = one.w.torus().moved(one.facts().food_at, {-100, 0});
    one.set_action(world::LivingAct::rest, near, 120, 14400);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
    one.w.run_to(300);
    REQUIRE(one.act().what == static_cast<std::uint8_t>(world::LivingAct::gather));
    const auto arrival = one.w.beings().raw().get<world::Dream>(one.h).visit_at;
    REQUIRE(arrival >= 120);
    bool carried = false;
    for (int step = 0; step < 20 && one.act().what != static_cast<std::uint8_t>(world::LivingAct::eat); ++step) {
        one.w.run_to(one.act().end + 1);
        carried = carried || one.act().what == static_cast<std::uint8_t>(world::LivingAct::carry);
    }
    CHECK(carried);
    REQUIRE(one.act().what == static_cast<std::uint8_t>(world::LivingAct::eat));
    CHECK(one.w.torus().distance(one.act().from, one.facts().food_at) > 200);
    CHECK(one.w.beings().raw().get<world::Dream>(one.h).visit_at == arrival);
    CHECK(one.w.beings().raw().get<world::Dreams>(one.ch).acts[0].visited_at == arrival);
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
    INFO(why);
    REQUIRE(copy);
    if (!copy) return;
    const auto h = copy->world().beings().handle(one.id);
    const auto ch = copy->world().beings().handle(one.home);
    CHECK(copy->world().beings().raw().get<world::Dream>(h).visit_at == arrival);
    CHECK(copy->world().beings().raw().get<world::Dreams>(ch).acts[0].visited_at == arrival);
}

TEST_CASE("dream requests scheduled out of number order still reopen with the same caps") {
    One one;
    one.know();
    one.set_action(world::LivingAct::rest, one.facts().shelter_at, 50000, 0);
    one.w.command(21600, demo::Living::kPlaceDream, one.id.value, 1);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 0);
    one.w.run_to(21601);
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
    INFO(why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK(copy->world().digests().whole == one.w.digests().whole);
    CHECK_FALSE(demo::Living::dream_problem(copy->world(), one.id, 2).empty());
}

TEST_CASE("an urgent water dream records the real arrival without claiming a dream pull") {
    One one;
    one.know();
    one.life().food = 4000000;
    one.life().water = 300;
    one.life().awake = 0;
    one.life().goal = 2;
    one.set_action(world::LivingAct::rest, one.facts().shelter_at, 120, 14400);
    one.w.command(0, demo::Living::kPlaceDream, one.id.value, 1);
    one.w.run_to(121);
    CHECK(one.life().goal == 1);
    CHECK(one.act().what == static_cast<std::uint8_t>(world::LivingAct::walk));
    CHECK(one.w.beings().raw().get<world::Dream>(one.h).visit_at == -1);
    for (int step = 0; step < 20 && one.act().what != static_cast<std::uint8_t>(world::LivingAct::drink); ++step)
        one.w.run_to(one.act().end + 1);
    REQUIRE(one.act().what == static_cast<std::uint8_t>(world::LivingAct::drink));
    const auto& thought = one.w.beings().raw().get<world::Dream>(one.h);
    CHECK(thought.decision_pull == 0);
    CHECK(thought.visit_at == one.act().start);
    CHECK(thought.visit_at > 120);
    const auto& records = one.w.beings().raw().get<world::Dreams>(one.ch).acts;
    REQUIRE(records.size() == 1);
    if (records.size() != 1) return;
    CHECK(records[0].choice == 1);
    CHECK(records[0].pull == 0);
    CHECK(records[0].visited_at == thought.visit_at);
    std::string why;
    auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
    INFO(why);
    REQUIRE(copy);
    if (!copy) return;
    const auto h = copy->world().beings().handle(one.id);
    const auto ch = copy->world().beings().handle(one.home);
    CHECK(copy->world().beings().raw().get<world::Dream>(h).visit_at == one.act().start);
    CHECK(copy->world().beings().raw().get<world::Dreams>(ch).acts[0].visited_at == one.act().start);
    CHECK(copy->world().beings().raw().get<world::Dreams>(ch).acts[0].pull == 0);
}

TEST_CASE("rounded walking routes stay outside inclusive rock when interrupted") {
    One one;
    const auto centre = one.w.beings().raw().get<world::Place>(one.ch).at;
    const auto from = one.w.torus().moved(centre, {400, 1000});
    const auto target = one.w.torus().moved(centre, {401, 800});
    REQUIRE(world::camp_line_clear({400, 1000}, {401, 800}, one.env()));
    const auto path = demo::Living::route(one.w, one.home, from, target);
    REQUIRE_FALSE(path.empty());
    if (path.empty()) return;
    auto origin = from;
    for (const auto destination : path) {
        const world::Activity segment{1, 0, 20, origin, destination};
        for (time::Seconds second = 0; second <= 20; ++second) {
            const auto point = one.w.torus().offset(centre, segment.at(one.w.torus(), second));
            CHECK(world::camp_line_clear(point, point, one.env()));
        }
        origin = destination;
    }
    CHECK(origin == target);
    one.set_action(world::LivingAct::walk, from, 20, 0);
    one.act().to = path.front();
    const auto interrupted = one.act().at(one.w.torus(), 10);
    one.w.run_to(10);
    one.set_action(world::LivingAct::watch, interrupted, 70, 0);
    one.act().start = 10;
    one.life().settled = 10;
    one.life().explore_at = interrupted;
    one.life().use_at = target;
    std::string why;
    const auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
    INFO(why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK(copy->world().digests().whole == one.w.digests().whole);
}

TEST_CASE("rounded routes protect every rock edge in either direction") {
    One one;
    const auto centre = one.w.beings().raw().get<world::Place>(one.ch).at;
    const std::array<std::pair<num::Offset, num::Offset>, 8> pairs{{{{400, 1000}, {401, 800}},
                                                                    {{200, 1000}, {199, 800}},
                                                                    {{400, -600}, {401, -400}},
                                                                    {{200, -600}, {199, -400}},
                                                                    {{100, 900}, {300, 901}},
                                                                    {{500, 900}, {300, 901}},
                                                                    {{100, -500}, {300, -501}},
                                                                    {{500, -500}, {300, -501}}}};
    for (const auto& [first, last] : pairs) {
        for (bool reversed : {false, true}) {
            auto origin = one.w.torus().moved(centre, reversed ? last : first);
            const auto target = one.w.torus().moved(centre, reversed ? first : last);
            const auto path = demo::Living::route(one.w, one.home, origin, target);
            REQUIRE_FALSE(path.empty());
            if (path.empty()) continue;
            CHECK(path == demo::Living::route(one.w, one.home, origin, target));
            for (const auto destination : path) {
                const world::Activity segment{1, 0, 20, origin, destination};
                for (time::Seconds second = 0; second <= 20; ++second) {
                    const auto point = one.w.torus().offset(centre, segment.at(one.w.torus(), second));
                    CHECK(world::camp_line_clear(point, point, one.env()));
                }
                origin = destination;
            }
            CHECK(origin == target);
        }
    }
}

TEST_CASE("saved motion rejects actual rounded collisions and keeps legal edge trajectories") {
    for (bool legal : {false, true}) {
        One one;
        const auto centre = one.w.beings().raw().get<world::Place>(one.ch).at;
        const auto first = num::Offset{400, legal ? 901 : 1000};
        const auto last = num::Offset{401, legal ? 900 : 800};
        REQUIRE(world::camp_line_clear(first, last, one.env()));
        one.set_action(world::LivingAct::walk, one.w.torus().moved(centre, first), 20, 0);
        one.act().to = one.w.torus().moved(centre, last);
        std::string why;
        const auto copy = demo::CrowdWorld::open(test::camp_fixture(), one.w.save(), why);
        INFO(legal, why);
        CHECK(bool(copy) == legal);
        if (copy) CHECK(copy->world().digests().whole == one.w.digests().whole);
    }
}
