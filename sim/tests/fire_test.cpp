#include "kd/demo/fire.hpp"
#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/snapshot.hpp"
namespace {
const kd::data::Catalogue& fire_catalogue() {
    static const auto catalogue = [] {
        kd::data::Catalogue out;
        REQUIRE(out.load(kd::data::read_catalogue(std::string(KD_REPO) + "/data")).empty());
        return out;
    }();
    return catalogue;
}
std::uint32_t fire_entry(std::string_view name) {
    const auto found = fire_catalogue().find("item", name);
    REQUIRE(found);
    return found.value_or(0);
}
struct FireFixture {
    kd::demo::CrowdWorld camp;
    kd::ecs::Id home{}, hearth{}, food{};
    explicit FireFixture(std::uint64_t seed = 91) : camp(seed, fire_catalogue(), 1, true, true) {
        auto& w = camp.world();
        auto& raw = w.beings().raw();
        home = camp.camp_ids().front();
        const auto here = raw.get<kd::world::Camp>(w.beings().handle(home)).shelter_at;
        w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
            if (!raw.all_of<kd::world::Life>(h)) return;
            auto& life = raw.get<kd::world::Life>(h);
            life.food = 4000000;
            life.water = 3000;
            life.awake = 0;
            const auto at = raw.get<kd::world::Place>(h).at;
            raw.get<kd::world::Activity>(h) = {2, 0, 8 * kd::time::kHour, at, at};
            for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(id, slot);
            w.schedule(id, 0, 8 * kd::time::kHour);
        });
        hearth = add(fire_entry("base:dry_stick"), 5000000, here);
        auto& f = w.things().raw().emplace<kd::world::Fire>(w.things().handle(hearth));
        f.hearth = home;
        f.at = here;
        f.heat = 3;
        f.fuel_mg = 4000000;
        f.ash_mg = 1000000;
        f.burn_remainder = 13;
        f.next = f.deadline();
        food = add(fire_entry("base:roots"), 1000000, here);
        auto& t = w.things().raw().emplace<kd::world::HeatTimer>(w.things().handle(food));
        t.item = food;
        t.elapsed = 600;
        t.exposure_heat = 3;
        t.next = kd::time::kHour - 600;
        w.schedule(home, 2, 1);
        w.schedule(home, 3, t.next);
    }
    kd::ecs::Id add(std::uint32_t kind, std::int64_t mass, kd::num::Point at) {
        auto& w = camp.world();
        const auto h = w.make_thing();
        w.things().raw().emplace<kd::world::Place>(h, at);
        auto& i = w.things().raw().emplace<kd::world::Item>(h);
        i.kind = kind;
        i.material = kind;
        i.mass = mass;
        i.length = 300;
        i.home = home;
        return w.things().id_of(h);
    }
    kd::world::Fire& fire() {
        auto& w = camp.world();
        return w.things().raw().get<kd::world::Fire>(w.things().handle(hearth));
    }
};
std::unique_ptr<kd::demo::CrowdWorld> reopen_fire(const kd::world::World& w, std::string& why) {
    const auto chunks = kd::save::read_snapshot(kd::save::write_snapshot(w.save()), why);
    REQUIRE(chunks);
    if (!chunks) return {};
    return kd::demo::CrowdWorld::open(fire_catalogue(), *chunks, why);
}
}  // namespace
TEST_CASE("FIRE1 and THER1 reopen conserved records and pending camp deadlines exactly") {
    FireFixture f;
    auto& w = f.camp.world();
    w.run_to(1200);
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    CHECK(opened->world().digests().whole == w.digests().whole);
    const auto& fire = opened->world().things().raw().get<kd::world::Fire>(opened->world().things().handle(f.hearth));
    CHECK(fire.fuel_mg == 4000000);
    CHECK(fire.ash_mg == 1000000);
    CHECK(fire.burn_remainder == 13);
    const auto& timer =
        opened->world().things().raw().get<kd::world::HeatTimer>(opened->world().things().handle(f.food));
    CHECK(timer.elapsed == 601);
    CHECK(timer.next == 3000);
    opened->world().beings().fuzz(73);
    opened->world().things().fuzz(91);
    kd::run::Workers pool(4);
    opened->world().run_islands(1500, pool, 1);
    w.run_to(1500);
    CHECK(opened->world().digests().whole == w.digests().whole);
    CHECK(kd::save::write_snapshot(opened->world().save()) == kd::save::write_snapshot(w.save()));
}
TEST_CASE("fire readers refuse negative fuel broken references and lost deadlines") {
    for (int fault = 0; fault < 6; ++fault) {
        FireFixture f;
        auto& w = f.camp.world();
        if (fault == 0) f.fire().fuel_mg = -1;
        if (fault == 1) f.fire().ash_mg += 1;
        if (fault == 2) f.fire().owner = kd::ecs::Id{999};
        if (fault == 3) w.cancel(f.home, 2);
        if (fault == 4) w.cancel(f.home, 3);
        if (fault == 5) {
            f.fire().next += 1;
            w.schedule(f.home, 2, f.fire().next);
        }
        std::string why;
        CHECK_FALSE(reopen_fire(w, why));
        CHECK_FALSE(why.empty());
    }
}
TEST_CASE("fire feature requires unique critical FIRE1 THER1 chunks and complete thermal coverage") {
    for (int fault = 0; fault < 4; ++fault) {
        FireFixture f;
        auto chunks = f.camp.world().save();
        const auto tag = kd::save::tag(fault == 0 ? "FIRE" : "THER");
        const auto where = std::find_if(chunks.begin(), chunks.end(), [&](const auto& c) { return c.tag == tag; });
        REQUIRE(where != chunks.end());
        if (fault == 0) chunks.erase(where);
        if (fault == 1) {
            const auto copy = *where;
            chunks.push_back(copy);
        }
        if (fault == 2) where->critical = false;
        if (fault == 3) where->data[0] = std::byte{0};
        std::string why;
        CHECK_FALSE(kd::demo::CrowdWorld::open(fire_catalogue(), chunks, why));
        CHECK_FALSE(why.empty());
    }
}
TEST_CASE("mild ambient transitions use saved events without replacing hourly renewal") {
    kd::demo::CrowdWorld camp(92, fire_catalogue(), 1, true, true);
    auto& w = camp.world();
    const auto h = w.beings().handle(camp.camp_ids().front());
    CHECK(w.beings().raw().get<kd::world::Ambient>(h).milli_c == 18000);
    w.run_to(6 * kd::time::kHour + 1);
    CHECK(w.beings().raw().get<kd::world::Ambient>(h).milli_c == 24000);
    CHECK(w.beings().raw().get<kd::world::Ambient>(h).next == 20 * kd::time::kHour);
    CHECK(w.beings().raw().get<kd::world::Habitat>(h).renewed_at == 6 * kd::time::kHour);
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    w.run_to(20 * kd::time::kHour + 1);
    opened->world().run_to(20 * kd::time::kHour + 1);
    CHECK(w.beings().raw().get<kd::world::Ambient>(h).milli_c == 18000);
    CHECK(opened->world().digests().whole == w.digests().whole);
}

TEST_CASE("fire storage survives labelled removal of all observers") {
    FireFixture f;
    auto& w = f.camp.world();
    std::vector<kd::ecs::Id> people;
    w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
        if (w.beings().raw().all_of<kd::world::Person>(h)) people.push_back(id);
    });
    for (const auto id : people) {
        for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(id, slot);
        w.beings().end(id);
    }
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    CHECK(opened->world().things().raw().all_of<kd::world::Fire>(opened->world().things().handle(f.hearth)));
    CHECK(opened->world().digests().whole == w.digests().whole);
}

TEST_CASE("fire build refuses format three camps with the plain older-build message") {
    FireFixture f;
    auto bytes = kd::save::write_snapshot(f.camp.world().save());
    REQUIRE(bytes.size() > 12);
    bytes[8] = std::byte{3};
    std::string why;
    CHECK_FALSE(kd::save::read_snapshot(bytes, why));
    CHECK(why == kd::save::kOlderSave);
}
namespace {
struct FireOperations final : kd::world::System {
    FireFixture fixture;
    kd::ecs::Id input{}, carried{};
    bool applied = false;
    explicit FireOperations(std::uint64_t seed = 91) : fixture(seed) {
        auto& w = fixture.camp.world();
        w.things().raw().remove<kd::world::HeatTimer>(w.things().handle(fixture.food));
        w.cancel(fixture.home, 3);
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (!w.beings().raw().all_of<kd::world::Life>(h)) return;
            auto& activity = w.beings().raw().get<kd::world::Activity>(h);
            activity.end = kd::time::kDay;
            w.schedule(id, 0, activity.end);
        });
        // Labelled mechanics fixture keeps people asleep and starts from exact five-kg fire accounting.
        auto& f = fixture.fire();
        f.fuel_mg = 5000000;
        f.ash_mg = 0;
        f.burn_remainder = 0;
        f.next = 3600;
        w.schedule(fixture.home, 2, 1);  // the fresh scene's lightning ignition is due first
        w.set_command_taker(*this);
    }
    std::string_view name() const override { return "labelled fire mechanics"; }
    void handle(kd::world::Context&, const kd::event::Event&) override {}
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        if (cmd.what == 910)
            applied = kd::demo::FireRules::feed(c, fixture.hearth, input, static_cast<std::int64_t>(cmd.b));
        if (cmd.what == 911) applied = kd::demo::FireRules::blow(c, fixture.hearth);
        if (cmd.what == 912) applied = kd::demo::FireRules::bank(c, fixture.hearth);
        if (cmd.what == 913) kd::demo::FireRules::ember(c, input);
    }
    void request(kd::time::Seconds at, std::uint32_t what, std::uint64_t mass = 0) {
        auto& w = fixture.camp.world();
        (void)w.command(at, what, 0, mass);
        w.run_to(at + 1);
    }
};
}  // namespace
TEST_CASE("five_kg_hour transfers finite fuel to ash with its exact carried remainder") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    w.run_to(3600);
    CHECK(ops.fixture.fire().fuel_mg == 5000000);
    w.run_to(3601);
    CHECK(ops.fixture.fire().fuel_mg == 0);
    CHECK(ops.fixture.fire().ash_mg == 5000000);
    CHECK(ops.fixture.fire().heat == 1);
    CHECK(ops.fixture.fire().embers_until == 14400);
    w.run_to(14401);
    CHECK(ops.fixture.fire().heat == 0);
    std::string why;
    CHECK(reopen_fire(w, why));
}
TEST_CASE("dry tinder blows an ember to heat two in sixty seconds and wet never lights") {
    for (const bool wet : {false, true}) {
        FireOperations ops;
        auto& w = ops.fixture.camp.world();
        const auto here = ops.fixture.fire().at;
        ops.input = ops.fixture.add(fire_entry("base:ember"), 1000, here);
        ops.request(0, 913);
        const auto ember = ops.input;
        ops.fixture.hearth = ember;
        ops.input = ops.fixture.add(fire_entry(wet ? "base:green_stick" : "base:tinder"), 100000, here);
        ops.request(1, 910, 100000);
        CHECK(ops.applied);
        if (wet) {
            CHECK(ops.fixture.fire().heat <= 1);
            CHECK(ops.fixture.fire().fuel_mg == 1000);
            CHECK(ops.fixture.fire().evaporated_mg == 100000);
        } else {
            ops.request(2, 911);
            CHECK(ops.applied);
            w.run_to(62);
            CHECK(ops.fixture.fire().heat == 1);
            w.run_to(63);
            CHECK(ops.fixture.fire().heat == 2);
            CHECK(ops.fixture.fire().fuel_mg == 101000);
        }
    }
}
TEST_CASE("one wet kilogram damps one level and conserves its vapour ledger") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    ops.input = ops.fixture.add(fire_entry("base:green_board"), 1000000, ops.fixture.fire().at);
    ops.request(10, 910, 500000);
    CHECK(ops.applied);
    CHECK(ops.fixture.fire().heat == 3);
    ops.request(20, 910, 500000);
    CHECK(ops.applied);
    CHECK(ops.fixture.fire().heat == 2);
    CHECK(ops.fixture.fire().damp_remainder == 0);
    CHECK(ops.fixture.fire().evaporated_mg == 1000000);
    CHECK(w.things().raw().get<kd::world::Item>(w.things().handle(ops.input)).mass == 0);
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    w.run_to(1800);
    opened->world().run_to(1800);
    CHECK(w.digests().whole == opened->world().digests().whole);
}
TEST_CASE("banked_overnight uses real ash cover and expires twelve hours after banking") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    ops.request(300, 912);
    CHECK(ops.applied);
    CHECK(ops.fixture.fire().ash_mg >= 100000);
    CHECK(ops.fixture.fire().heat == 1);
    CHECK(ops.fixture.fire().banked_until == 43500);
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    w.run_to(43500);
    opened->world().run_to(43500);
    CHECK(ops.fixture.fire().heat == 1);
    w.run_to(43501);
    opened->world().run_to(43501);
    CHECK(ops.fixture.fire().heat == 0);
    CHECK(w.digests().whole == opened->world().digests().whole);
}
TEST_CASE("local ignition follows physical distance and wet material rather than stock immunity") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    const auto here = ops.fixture.fire().at;
    const auto nearby = ops.fixture.add(fire_entry("base:dry_stick"), 250000, w.torus().moved(here, {100, 0}));
    const auto far = ops.fixture.add(fire_entry("base:dry_stick"), 250000, w.torus().moved(here, {101, 0}));
    const auto wet = ops.fixture.add(fire_entry("base:green_stick"), 250000, here);
    w.run_to(2);
    CHECK(w.things().raw().all_of<kd::world::Fire>(w.things().handle(nearby)));
    CHECK_FALSE(w.things().raw().all_of<kd::world::Fire>(w.things().handle(far)));
    CHECK_FALSE(w.things().raw().all_of<kd::world::Fire>(w.things().handle(wet)));
}

TEST_CASE("200 low high banking and carrying trials use maker chance without free failure embers") {
    // Before results: level two at night is 50%, level ten 95%; same 99% bounds as other craft trials.
    for (const auto operation : {3, 4}) {
        for (const auto level : {2, 10}) {
            std::size_t successes = 0;
            for (std::uint64_t seed = 0; seed < 200; ++seed) {
                FireOperations ops(seed);
                auto& w = ops.fixture.camp.world();
                w.run_to(2);  // settle the scene's initial ignition before placing the trial container
                const auto here = ops.fixture.fire().at;
                kd::ecs::Id person{};
                w.beings().each([&](kd::ecs::Id id, auto h) {
                    if (!person.value && w.beings().raw().all_of<kd::world::Person>(h)) person = id;
                });
                const auto h = w.beings().handle(person);
                auto& raw = w.beings().raw();
                const auto recipe =
                    fire_catalogue().find("blueprint", operation == 3 ? "base:bank_fire" : "base:carry_ember");
                REQUIRE(recipe);
                auto& know = raw.get<kd::world::Knowledge>(h);
                for (auto& skill : know.skills)
                    if (skill.recipe == *recipe) skill.practice = {level * 1000, level * 1000, 0, -1};
                know.sectors[2] = {level * 1000, level * 1000, 0, -1};
                auto& t = raw.get<kd::world::Thermal>(h);
                t.tending = static_cast<std::uint8_t>(operation);
                t.tending_phase = 3;
                t.tending_started = 2;
                t.tending_fire = ops.fixture.hearth;
                if (operation == 4) {
                    t.tending_input = ops.fixture.add(fire_entry("base:rotten_wood"), 100000, here);
                    t.tending_mass = 100000;
                }
                raw.get<kd::world::Place>(h).at = here;
                const auto seconds = operation == 3 ? 300 : 60;
                raw.get<kd::world::Life>(h).settled = 2;
                raw.get<kd::world::Activity>(h) = {12, 2, 2 + seconds, here, here};
                w.schedule(person, 0, 2 + seconds);
                std::size_t before = 0;
                w.things().each([&](kd::ecs::Id, auto th) {
                    if (w.things().raw().all_of<kd::world::Fire>(th)) ++before;
                });
                std::int64_t mass_before = 0;
                w.things().each(
                    [&](kd::ecs::Id, auto th) { mass_before += w.things().raw().get<kd::world::Item>(th).mass; });
                w.run_to(seconds + 3);
                std::size_t after = 0;
                w.things().each([&](kd::ecs::Id, auto th) {
                    if (w.things().raw().all_of<kd::world::Fire>(th)) ++after;
                });
                if (operation == 3 ? ops.fixture.fire().banked_until != 0 : after > before) ++successes;
                std::int64_t mass_after = 0;
                w.things().each(
                    [&](kd::ecs::Id, auto th) { mass_after += w.things().raw().get<kd::world::Item>(th).mass; });
                CHECK(mass_after == mass_before);
                std::string why;
                INFO(why);
                CHECK(reopen_fire(w, why));
            }
            MESSAGE("operation ", operation, " level ", level, ": ", successes, "/200");
            CHECK(successes >= (level == 10 ? 181 : 100));
            CHECK(successes <= (level == 10 ? 197 : 140));
        }
    }
}

TEST_CASE("unblown embers use one saved half-chance after 180 seconds") {
    // Before results: 200 independent seeds, 50% death; the ordinary 99% binomial bounds.
    std::size_t died = 0;
    for (std::uint64_t seed = 0; seed < 200; ++seed) {
        FireOperations ops(seed);
        auto& w = ops.fixture.camp.world();
        ops.input = ops.fixture.add(fire_entry("base:ember"), 1000, ops.fixture.fire().at);
        ops.request(0, 913);
        w.run_to(180);
        const auto h = w.things().handle(ops.input);
        CHECK(w.things().raw().get<kd::world::Fire>(h).heat == 1);
        w.run_to(181);
        const auto& f = w.things().raw().get<kd::world::Fire>(h);
        CHECK(f.unblown_checked == 1);
        if (!f.heat) ++died;
        std::string why;
        auto opened = reopen_fire(w, why);
        INFO(why);
        REQUIRE(opened);
        w.run_to(1000);
        opened->world().run_to(1000);
        CHECK(w.digests().whole == opened->world().digests().whole);
    }
    CHECK(died >= 82);
    CHECK(died <= 118);
}

TEST_CASE("extinguishing leaves physical charcoal and ash with conserved vapour") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    ops.input = ops.fixture.add(fire_entry("base:green_board"), 3000000, ops.fixture.fire().at);
    std::int64_t before = 0;
    w.things().each([&](kd::ecs::Id, auto h) { before += w.things().raw().get<kd::world::Item>(h).mass; });
    ops.request(10, 910, 3000000);
    CHECK(ops.applied);
    CHECK(ops.fixture.fire().heat == 0);
    CHECK(ops.fixture.fire().fuel_mg == 0);
    std::int64_t after = ops.fixture.fire().evaporated_mg, charcoal = 0;
    w.things().each([&](kd::ecs::Id, auto h) {
        const auto& item = w.things().raw().get<kd::world::Item>(h);
        after += item.mass;
        if (item.kind == fire_entry("base:charcoal")) charcoal += item.mass;
    });
    CHECK(charcoal > 0);
    CHECK(after == before);
    std::string why;
    CHECK(reopen_fire(w, why));
}
TEST_CASE("heat two consumes one kg per hour and finite feeding cannot debit a source twice") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    auto& f = ops.fixture.fire();
    f.heat = 2;
    f.fuel_mg = 1000000;
    f.next = f.deadline();
    w.things().raw().get<kd::world::Item>(w.things().handle(ops.fixture.hearth)).mass = 1000000;
    w.run_to(3601);
    CHECK(f.fuel_mg == 0);
    CHECK(f.ash_mg == 1000000);
    ops.input = ops.fixture.add(fire_entry("base:tinder"), 100000, ops.fixture.fire().at);
    ops.request(3602, 910, 100000);
    CHECK(ops.applied);
    ops.request(3603, 910, 100000);
    CHECK_FALSE(ops.applied);
    CHECK(w.things().raw().get<kd::world::Item>(w.things().handle(ops.input)).mass == 0);
}

TEST_CASE("rebanking the same ember cannot extend its saved lifetime") {
    FireOperations ops;
    ops.request(300, 912);
    REQUIRE(ops.applied);
    const auto until = ops.fixture.fire().banked_until;
    ops.request(600, 912);
    CHECK_FALSE(ops.applied);
    CHECK(ops.fixture.fire().banked_until == until);
}
TEST_CASE("warmth_two_metres follows physical distance and working comfort") {
    FireFixture f;
    auto& w = f.camp.world();
    kd::ecs::Id person{};
    w.beings().each([&](kd::ecs::Id id, auto h) {
        if (!person.value && w.beings().raw().all_of<kd::world::Person>(h)) person = id;
    });
    const auto h = w.beings().handle(person);
    auto& raw = w.beings().raw();
    for (const auto distance : {200, 201}) {
        const auto place = w.torus().moved(f.fire().at, {distance, 0});
        raw.get<kd::world::Place>(h).at = place;
        raw.get<kd::world::Activity>(h) = {0, 0, kd::time::kDay, place, place};
        raw.get<kd::world::Thermal>(h) = {};
        const auto t = kd::demo::FireRules::sample_thermal(w, h, 3600);
        CHECK(t.felt_milli_c == (distance == 200 ? 33000 : 18000));
        CHECK(t.warmth == (distance == 200 ? 100 : 70));
    }
    CHECK(kd::demo::FireRules::warmth(18000, kd::world::LivingAct::craft) == 100);
    CHECK(kd::demo::FireRules::warmth(18000, kd::world::LivingAct::rest) == 70);
    CHECK(kd::demo::FireRules::warmth(5000, kd::world::LivingAct::rest) < 20);
}
TEST_CASE("thermal water rate keeps every fraction across tiny settlements and net intake") {
    FireFixture f;
    auto& w = f.camp.world();
    auto& raw = w.beings().raw();
    raw.get<kd::world::Ambient>(w.beings().handle(f.home)).milli_c = 24000;
    kd::ecs::Id person{};
    w.beings().each([&](kd::ecs::Id id, auto h) {
        if (!person.value && raw.all_of<kd::world::Person>(h)) person = id;
    });
    const auto h = w.beings().handle(person);
    const auto here = f.fire().at;
    raw.get<kd::world::Place>(h).at = here;
    raw.get<kd::world::Activity>(h) = {0, 0, kd::time::kDay, here, here};
    raw.get<kd::world::Thermal>(h) = {};
    const auto direct = kd::demo::FireRules::sample_thermal(w, h, kd::time::kDay);
    const auto rate = w.catalogue().kind<kd::demo::LivingRules>()[0].water_day;
    CHECK(direct.water_used_ml == rate * 14 / 100);
    CHECK(direct.water_due_ml == direct.water_used_ml);
    for (kd::time::Seconds second = 1; second <= kd::time::kDay; ++second)
        raw.get<kd::world::Thermal>(h) = kd::demo::FireRules::sample_thermal(w, h, second);
    kd::ByteWriter a, b;
    kd::ecs::write_component(direct, a);
    kd::ecs::write_component(raw.get<kd::world::Thermal>(h), b);
    CHECK(a.take() == b.take());
    kd::demo::Living living(w);
    kd::world::Life body;
    body.water = 3000;
    body.portion = 3000;
    body.allocated_water = 3000;
    const kd::world::Activity drink{6, 0, kd::time::kHour, here, here};
    const auto extra_hour = rate * 14 * kd::time::kHour / (100 * kd::time::kDay);
    CHECK(living.sample(body, drink, kd::time::kHour, extra_hour).water == 3000);
}
TEST_CASE("thermal readers refuse negative deferred water and invalid warming plans") {
    for (int fault = 0; fault < 4; ++fault) {
        FireFixture f;
        auto& w = f.camp.world();
        kd::ecs::Id person{};
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (!person.value && w.beings().raw().all_of<kd::world::Person>(h)) person = id;
        });
        auto& t = w.beings().raw().get<kd::world::Thermal>(w.beings().handle(person));
        if (fault == 0) t.water_due_ml = -1;
        if (fault == 1) t.water_due_ml = 1;
        if (fault == 2) {
            t.warm_phase = 1;
            t.warm_fire = {999};
        }
        if (fault == 3) {
            t.warm_phase = 2;
            t.warm_fire = f.hearth;
        }
        std::string why;
        CHECK_FALSE(reopen_fire(w, why));
        CHECK_FALSE(why.empty());
    }
}

TEST_CASE("ordinary warming preserves its walk and reopen and records only actual experienced warmth") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    auto& raw = w.beings().raw();
    kd::ecs::Id person{};
    w.beings().each([&](kd::ecs::Id id, auto h) {
        if (!raw.all_of<kd::world::Knowledge>(h)) return;
        CHECK(raw.get<kd::world::Knowledge>(h).memories.empty());
        if (!person.value) person = id;
    });
    const auto h = w.beings().handle(person);
    const auto cold = w.torus().moved(ops.fixture.fire().at, {201, 0});
    raw.get<kd::world::Place>(h).at = cold;
    raw.get<kd::world::Activity>(h) = {0, 0, 60, cold, cold};
    w.schedule(person, 0, 60);
    w.run_to(61);
    CHECK(raw.get<kd::world::Thermal>(h).warm_phase == 1);
    CHECK(raw.get<kd::world::Activity>(h).what == 1);
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    w.run_to(62);
    opened->world().run_to(62);
    CHECK(raw.get<kd::world::Thermal>(h).warm_phase == 2);
    CHECK(raw.get<kd::world::Activity>(h).what == 11);
    CHECK(w.digests().whole == opened->world().digests().whole);
    kd::run::Workers workers(4);
    w.run_to(1862);
    opened->world().run_islands(1862, workers, 1);
    CHECK(w.digests().whole == opened->world().digests().whole);
    const auto& know = raw.get<kd::world::Knowledge>(h);
    const auto memory = std::find_if(know.memories.begin(), know.memories.end(),
                                     [](const auto& m) { return m.action == 12 && m.sign == 13; });
    REQUIRE(memory != know.memories.end());
    CHECK(memory->at > 0);
    CHECK(memory->result.value != 0);
    REQUIRE_FALSE(memory->inputs.empty());
    CHECK(memory->inputs[0].values[12] == 3);
    CHECK((know.performed & (1U << 12U)) == 0);  // feeling heat is not performing the heat action
}
namespace {
struct CookingOperations final : kd::world::System {
    FireOperations fires;
    kd::ecs::Id portion{}, viewer{};
    kd::num::Point destination{};
    explicit CookingOperations(std::uint64_t seed = 91) : fires(seed) {
        auto& w = fires.fixture.camp.world();
        portion = fires.fixture.food;
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (!viewer.value && w.beings().raw().all_of<kd::world::Person>(h)) viewer = id;
        });
        auto& f = fires.fixture.fire();
        f.fuel_mg = 20000000;
        w.things().raw().get<kd::world::Item>(w.things().handle(fires.fixture.hearth)).mass = f.fuel_mg;
        f.next = f.deadline();
        w.set_command_taker(*this);
    }
    std::string_view name() const override { return "labelled cooking mechanics"; }
    void handle(kd::world::Context&, const kd::event::Event&) override {}
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        auto& w = c.world();
        if (cmd.a) portion = kd::ecs::Id{cmd.a};
        if (cmd.what == 921) kd::demo::FireRules::food_changed(c, portion);
        if (cmd.what == 922) {
            w.things().raw().get<kd::world::Place>(w.things().handle(portion)).at = destination;
            c.item_changed(portion);
        }
        if (cmd.what == 923) kd::demo::FireRules::notice_food(c, w.beings().handle(viewer));
        if (cmd.what == 924) {
            kd::demo::FireRules::settle_fire(c, fires.fixture.hearth);
            fires.fixture.fire().heat = 4;
            fires.fixture.fire().next = fires.fixture.fire().deadline();
            c.item_changed(fires.fixture.hearth);
            kd::demo::FireRules::deadlines(c, fires.fixture.home);
        }
        if (cmd.what == 925) kd::demo::FireRules::food_intent(c, portion, viewer, true);
        if (cmd.what == 927) {
            const auto ph = w.beings().handle(viewer);
            const auto here = w.torus().moved(fires.fixture.fire().at, {1000, 0});
            w.beings().raw().get<kd::world::Place>(ph).at = here;
            w.beings().raw().get<kd::world::Activity>(ph) = {1, 0, 200, here, fires.fixture.fire().at};
            item().owner = viewer;
            c.schedule(viewer, 0, 200);
            kd::demo::FireRules::carried_food(c, viewer);
        }
        if (cmd.what == 926) {
            kd::demo::FireRules::food_changed(c, portion);
            const auto old = portion;
            auto copy = item();
            copy.mass /= 2;
            item().mass -= copy.mass;
            c.item_changed(old);
            copy.parents = {{old}};
            copy.made_at = c.now();
            const auto h = w.make_thing();
            w.things().raw().emplace<kd::world::Item>(h, copy);
            w.things().raw().emplace<kd::world::Place>(h, fires.fixture.fire().at);
            portion = w.things().id_of(h);
            c.item_changed(portion);
        }
    }
    void request(kd::time::Seconds at, std::uint32_t what) {
        auto& w = fires.fixture.camp.world();
        (void)w.command(at, what, 0, 0);
        w.run_to(at + 1);
    }
    kd::world::HeatTimer& timer() {
        auto& w = fires.fixture.camp.world();
        return w.things().raw().get<kd::world::HeatTimer>(w.things().handle(portion));
    }
    kd::world::Item& item() {
        auto& w = fires.fixture.camp.world();
        return w.things().raw().get<kd::world::Item>(w.things().handle(portion));
    }
};
}  // namespace
TEST_CASE("cook_one_hour_burn_two happens unseen, preserves mass and never retries the first chance") {
    for (std::uint64_t seed = 1; seed <= 20; ++seed) {
        CookingOperations ops(seed);
        auto& w = ops.fires.fixture.camp.world();
        const auto recipe = *fire_catalogue().find("blueprint", "base:roast_food");
        ops.request(0, 921);
        CHECK(ops.timer().next == 3600);
        const auto history_before =
            w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(ops.fires.fixture.home)).events.size();
        w.run_to(3601);
        CHECK(ops.timer().tried == 1);
        CHECK(ops.timer().elapsed == 3600);
        CHECK(ops.item().mass == 1000000);
        CHECK(ops.item().state <= 1);
        CHECK(kd::demo::Crafting::characteristics(fire_catalogue(), ops.item())[8] == (ops.item().state == 1 ? 3 : 2));
        CHECK(w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(ops.fires.fixture.home)).events.size() ==
              history_before);
        w.beings().each([&](kd::ecs::Id, auto h) {
            if (const auto* know = w.beings().raw().try_get<kd::world::Knowledge>(h))
                CHECK_FALSE(kd::demo::Learning::knows(*know, recipe));
        });
        const auto first = ops.item().state;
        ops.destination = w.torus().moved(ops.fires.fixture.fire().at, {1000, 0});
        ops.request(4000, 922);
        CHECK(ops.timer().next == 0);
        CHECK(ops.timer().elapsed == 4000);
        std::string why;
        CHECK(reopen_fire(w, why));
        ops.request(6000, 921);
        CHECK(ops.timer().elapsed == 4000);
        CHECK(ops.item().state == first);
        ops.destination = ops.fires.fixture.fire().at;
        ops.request(7000, 922);
        CHECK(ops.timer().next == 10200);
        w.run_to(10201);
        CHECK(ops.item().state == 2);
        CHECK(ops.timer().completed == 1);
        CHECK(ops.timer().next == 0);
        CHECK(kd::demo::Crafting::characteristics(fire_catalogue(), ops.item())[8] == 0);
        CHECK(ops.item().mass == 1000000);
        CHECK(reopen_fire(w, why));
    }
}
TEST_CASE("one hour at heat four burns food without a cooking reroll") {
    CookingOperations ops;
    auto& w = ops.fires.fixture.camp.world();
    ops.request(0, 924);
    w.run_to(3601);
    CHECK(ops.item().state == 2);
    CHECK(ops.timer().hot_elapsed == 3600);
    CHECK(ops.timer().tried == 0);
}
TEST_CASE("unseen cooked food credits an actual noticer and preserves evidence across reopen") {
    int cooked = 0;
    for (std::uint64_t seed = 1; seed <= 20; ++seed) {
        CookingOperations ops(seed);
        auto& w = ops.fires.fixture.camp.world();
        ops.request(0, 921);
        w.run_to(3601);
        if (ops.item().state != 1) continue;
        ++cooked;
        const auto home = ops.fires.fixture.home;
        auto& history = w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(home));
        const auto before = history.events.size();
        auto& a = w.beings().raw().get<kd::world::Activity>(w.beings().handle(ops.viewer));
        a.what = 0;
        a.from = a.to = w.torus().moved(ops.fires.fixture.fire().at, {600, 0});
        w.beings().raw().get<kd::world::Place>(w.beings().handle(ops.viewer)).at = a.from;
        ops.request(3602, 923);
        CHECK(history.events.size() == before);
        a.from = a.to = ops.fires.fixture.fire().at;
        w.beings().raw().get<kd::world::Place>(w.beings().handle(ops.viewer)).at = a.from;
        ops.request(3603, 923);
        REQUIRE(history.events.size() == before + 1);
        CHECK(history.events.back().actor == ops.viewer);
        CHECK(history.events.back().route == 1);
        CHECK(ops.timer().notices.size() == 1);
        ops.request(3604, 923);
        CHECK(history.events.size() == before + 1);
        std::string why;
        auto reopened = reopen_fire(w, why);
        INFO(why);
        CHECK(reopened);
    }
    CHECK(cooked > 0);
}
TEST_CASE("cooking uses the normal low and high maker chance in two hundred roots and meat trials") {
    for (const auto name : {"base:roots", "base:meat"}) {
        for (const bool high : {false, true}) {
            int successes = 0;
            for (std::uint64_t trial = 1; trial <= 200; ++trial) {
                CookingOperations ops(trial);
                auto& w = ops.fires.fixture.camp.world();
                const auto recipe = *fire_catalogue().find("blueprint", "base:roast_food");
                auto& know = w.beings().raw().get<kd::world::Knowledge>(w.beings().handle(ops.viewer));
                kd::world::Skill skill;
                skill.recipe = recipe;
                skill.known = 1;
                skill.practice = {high ? 10000 : 0, high ? 10000 : 0, 0, 0};
                know.skills.push_back(skill);
                know.sectors[5] = skill.practice;
                const auto ph = w.beings().handle(ops.viewer);
                w.beings().raw().get<kd::world::Place>(ph).at = ops.fires.fixture.fire().at;
                auto& activity = w.beings().raw().get<kd::world::Activity>(ph);
                activity.from = activity.to = ops.fires.fixture.fire().at;
                ops.item().kind = ops.item().material = fire_entry(name);
                ops.request(0, 925);
                w.run_to(3601);
                CHECK(ops.item().mass == 1000000);
                CHECK(ops.item().state <= 1);
                successes += ops.item().state == 1;
            }
            INFO(std::string(name), " high=", high, " successes=", successes);
            MESSAGE(std::string(name), " high=", high, " successes=", successes, "/200");
            CHECK(successes >= (high ? 181 : 62));
            CHECK(successes <= (high ? 197 : 98));
        }
    }
}
TEST_CASE("splitting a heated portion preserves elapsed exposure and its original chance") {
    CookingOperations ops;
    auto& w = ops.fires.fixture.camp.world();
    ops.request(0, 921);
    const auto original = ops.portion;
    ops.request(1800, 926);
    CHECK(ops.timer().elapsed == 1800);
    CHECK(ops.timer().chance_source == original);
    CHECK(ops.timer().next == 3600);
    std::string why;
    REQUIRE(reopen_fire(w, why));
    w.run_to(3601);
    const auto& first = w.things().raw().get<kd::world::Item>(w.things().handle(original));
    CHECK(ops.item().state == first.state);
    CHECK(ops.item().mass + first.mass == 1000000);
    CHECK(ops.timer().tried == 1);
}
TEST_CASE("cooking readers reject lost deadlines duplicate noticers and repeated cooked chances") {
    for (int fault = 0; fault < 6; ++fault) {
        CookingOperations ops;
        auto& w = ops.fires.fixture.camp.world();
        ops.request(0, 921);
        if (fault == 0) ops.timer().exposure_heat = 6;
        if (fault == 1) {
            ops.timer().next = 0;
            w.cancel(ops.fires.fixture.home, 3);
        }
        if (fault == 2) ops.timer().notices = {{ops.viewer}, {ops.viewer}};
        if (fault == 3) {
            ops.item().state = 1;
            ops.timer().tried = 0;
        }
        if (fault == 4) {
            ops.timer().elapsed = kd::time::kHour;
            ops.timer().tried = 0;
        }
        if (fault == 5) {
            ops.timer().next = 7200;
            w.schedule(ops.fires.fixture.home, 3, 7200);
        }
        std::string why;
        CHECK_FALSE(reopen_fire(w, why));
        CHECK_FALSE(why.empty());
    }
}
TEST_CASE("cooking and burning reopen exactly through shuffled storage and four workers") {
    CookingOperations ops;
    auto& w = ops.fires.fixture.camp.world();
    ops.request(0, 921);
    w.run_to(1801);
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    opened->world().beings().fuzz(823);
    opened->world().things().fuzz(824);
    kd::run::Workers pool(4);
    opened->world().run_islands(7201, pool, 1);
    w.run_to(7201);
    CHECK(ops.item().state == 2);
    CHECK(w.digests().whole == opened->world().digests().whole);
    CHECK(kd::save::write_snapshot(w.save()) == kd::save::write_snapshot(opened->world().save()));
}
TEST_CASE("carried raw food schedules its actual entry into heat before the walk ends") {
    CookingOperations ops;
    auto& w = ops.fires.fixture.camp.world();
    ops.request(0, 927);
    CHECK(ops.timer().exposure_heat == 0);
    CHECK(ops.timer().next == 180);
    w.run_to(180);
    CHECK(ops.timer().elapsed == 0);
    w.run_to(181);
    CHECK(ops.timer().exposure_heat == 3);
    CHECK(ops.timer().next == 3780);
    std::string why;
    CHECK(reopen_fire(w, why));
}
TEST_CASE("actual fire task light removes darkness cost and walls block it") {
    FireOperations ops;
    auto& w = ops.fixture.camp.world();
    const auto here = ops.fixture.fire().at;
    const auto edge = w.torus().moved(here, {200, 0});
    CHECK(kd::demo::FireRules::task_light(w, ops.fixture.home, here, edge, 0));
    CHECK_FALSE(kd::demo::FireRules::task_light(w, ops.fixture.home, here, w.torus().moved(here, {201, 0}), 0));
    kd::ecs::Id person{};
    w.beings().each([&](kd::ecs::Id id, auto h) {
        if (!person.value && w.beings().raw().all_of<kd::world::Person>(h)) person = id;
    });
    const auto h = w.beings().handle(person);
    auto& activity = w.beings().raw().get<kd::world::Activity>(h);
    activity.from = activity.to = here;
    const auto recipe = *fire_catalogue().find("blueprint", "base:sharp_flake");
    const auto lit = kd::demo::Crafting::success(w, h, recipe, {});
    CHECK(kd::demo::Crafting::time_cost(w, h, 100) == 100);
    activity.from = activity.to = w.torus().moved(here, {500, 0});
    CHECK(kd::demo::Crafting::success(w, h, recipe, {}) == lit - 100000);
    CHECK(kd::demo::Crafting::time_cost(w, h, 100) == 125);
    auto& rock = w.beings().raw().get<kd::world::Habitat>(w.beings().handle(ops.fixture.home));
    rock.rock_west = -1010;
    rock.rock_east = -990;
    rock.rock_south = 1500;
    rock.rock_north = 1700;
    CHECK_FALSE(kd::demo::FireRules::task_light(w, ops.fixture.home, edge, edge, 0));
}
TEST_CASE("fresh out-fire scene has the same finite stock and skills without fire or invented warmth") {
    kd::demo::CrowdWorld cold(93, fire_catalogue(), 1, true, true, true);
    auto& w = cold.world();
    std::int64_t mass = 0, ash = 0;
    w.things().each([&](kd::ecs::Id, auto h) {
        mass += w.things().raw().get<kd::world::Item>(h).mass;
        if (const auto* f = w.things().raw().try_get<kd::world::Fire>(h)) {
            CHECK(f->heat == 0);
            CHECK(f->fuel_mg == 0);
            CHECK(f->next == 0);
            ash += f->ash_mg;
        }
    });
    CHECK(mass == 450000000);
    CHECK(ash == 5000000);
    w.beings().each([&](kd::ecs::Id, auto h) {
        if (const auto* know = w.beings().raw().try_get<kd::world::Knowledge>(h)) {
            CHECK(know->skills.size() == 5);
            CHECK(know->memories.empty());
        }
    });
    std::string why;
    CHECK(reopen_fire(w, why));
}

TEST_CASE("an active cooking demonstration credits nearby observers through the shared learning path") {
    CookingOperations ops;
    auto& w = ops.fires.fixture.camp.world();
    auto& raw = w.beings().raw();
    const auto recipe = *fire_catalogue().find("blueprint", "base:roast_food");
    const auto maker = w.beings().handle(ops.viewer);
    auto& work = raw.get<kd::world::Work>(maker);
    work.number = 1;
    work.state = 2;
    work.action = 12;
    work.recipe = recipe;
    work.intended = 1;
    work.try_seconds = work.next_try = work.end = 3600;
    const auto here = ops.fires.fixture.fire().at;
    raw.get<kd::world::Activity>(maker) = {static_cast<std::uint8_t>(kd::world::LivingAct::craft), 0, 7200, here, here};
    kd::ecs::Id observer{};
    w.beings().each([&](kd::ecs::Id id, auto h) {
        if (observer.value || id == ops.viewer || !raw.all_of<kd::world::Knowledge>(h)) return;
        observer = id;
        raw.get<kd::world::Activity>(h) = {0, 0, 7200, here, here};
        raw.get<kd::world::Life>(h).awake = 1;
    });
    REQUIRE(observer.value);
    ops.request(0, 925);
    w.run_to(3601);
    const auto& mind = raw.get<kd::world::Knowledge>(w.beings().handle(observer));
    const auto skill =
        std::find_if(mind.skills.begin(), mind.skills.end(), [&](const auto& s) { return s.recipe == recipe; });
    REQUIRE(skill != mind.skills.end());
    CHECK(skill->observation_quarters == 1);
    CHECK_FALSE(skill->known);
    CHECK(mind.last_observed_event > 0);
}
