#include "kd/demo/fire.hpp"
#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
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
    CHECK(timer.elapsed == 600);
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
            CHECK(successes >= (level == 10 ? 181 : 82));
            CHECK(successes <= (level == 10 ? 197 : 118));
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
