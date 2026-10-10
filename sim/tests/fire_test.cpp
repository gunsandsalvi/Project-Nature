#include "kd/demo/fire.hpp"
#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/choice.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/snapshot.hpp"
namespace {
std::uint32_t required_entry(const kd::data::Catalogue& catalogue, std::string_view folder, std::string_view name) {
    const auto found = catalogue.find(folder, name);
    KD_CHECK(found.has_value(), "Fire requires its validated catalogue entry");
    return found.value_or(0);
}
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
    explicit FireFixture(std::uint64_t seed = 91, const kd::data::Catalogue& catalogue = fire_catalogue())
        : camp(seed, catalogue, 1, true, true) {
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
        hearth = add(required_entry(catalogue, "item", "base:dry_stick"), 5000000, here);
        auto& f = w.things().raw().emplace<kd::world::Fire>(w.things().handle(hearth));
        f.hearth = home;
        f.at = here;
        f.heat = 3;
        f.fuel_mg = 4000000;
        f.ash_mg = 1000000;
        f.burn_remainder = 13;
        f.next = f.deadline();
        food = add(required_entry(catalogue, "item", "base:roots"), 1000000, here);
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
    REQUIRE(w.archived_item(ops.input));
    CHECK(w.archived_item(ops.input)->item.mass == 0);
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
                    if (skill.recipe == recipe.value_or(0)) skill.practice = {level * 1000LL, level * 1000LL, 0, -1};
                know.sectors[2] = {level * 1000LL, level * 1000LL, 0, -1};
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
    REQUIRE(w.archived_item(ops.input));
    CHECK(w.archived_item(ops.input)->item.mass == 0);
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
        const auto recipe = required_entry(fire_catalogue(), "blueprint", "base:roast_food");
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
                const auto recipe = required_entry(fire_catalogue(), "blueprint", "base:roast_food");
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
    const auto recipe = required_entry(fire_catalogue(), "blueprint", "base:sharp_flake");
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
    const auto recipe = required_entry(fire_catalogue(), "blueprint", "base:roast_food");
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

TEST_CASE("fire readers reject disagreement with physical item ownership including an unowned fire") {
    for (const int fault : {0, 1, 2}) {
        FireFixture f;
        auto& w = f.camp.world();
        std::vector<kd::ecs::Id> people;
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (w.beings().raw().all_of<kd::world::Person>(h)) people.push_back(id);
        });
        REQUIRE(people.size() >= 2);
        auto& item = w.things().raw().get<kd::world::Item>(w.things().handle(f.hearth));
        item.owner = people[0];
        f.fire().owner = people[0];
        std::string why;
        REQUIRE(reopen_fire(w, why));
        if (fault == 0) f.fire().owner = {};
        if (fault == 1) item.owner = {};
        if (fault == 2) f.fire().owner = people[1];
        INFO(fault);
        CHECK_FALSE(reopen_fire(w, why));
        CHECK(why == "fire carrier disagrees with physical item owner");
    }
}

namespace {
const kd::data::Catalogue& renamed_fire_catalogue() {
    static const auto catalogue = [] {
        auto files = kd::data::read_catalogue(std::string(KD_REPO) + "/data");
        std::vector<kd::data::SourceFile> copies;
        for (auto& file : files) {
            if (file.path == "base/item/roots.toml") copies.push_back({"base/item/review_roots.toml", file.text});
            if (file.path == "base/item/dry_stick.toml") copies.push_back({"base/item/review_fuel.toml", file.text});
            if (file.path == "base/blueprint/bank_fire.toml") file.path = "base/blueprint/review_bank.toml";
            if (file.path == "base/blueprint/carry_ember.toml") file.path = "base/blueprint/review_carry.toml";
            if (file.path == "base/blueprint/roast_food.toml") file.path = "base/blueprint/review_roast.toml";
        }
        files.insert(files.end(), copies.begin(), copies.end());
        kd::data::Catalogue out;
        REQUIRE(out.load(files).empty());
        return out;
    }();
    return catalogue;
}
struct GenericFoodOperation : kd::world::System {
    FireFixture fixture;
    kd::ecs::Id person{};
    bool prepared = false;
    explicit GenericFoodOperation(bool renamed) : fixture(91, renamed_fire_catalogue()) {
        auto& w = fixture.camp.world();
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (!person.value && w.beings().raw().all_of<kd::world::Person>(h)) person = id;
        });
        const auto ph = w.beings().handle(person);
        w.beings().raw().get<kd::world::Place>(ph).at = fixture.fire().at;
        const auto at = fixture.fire().at;
        w.beings().raw().get<kd::world::Activity>(ph) = {2, 0, 8 * kd::time::kHour, at, at};
        auto& item = w.things().raw().get<kd::world::Item>(w.things().handle(fixture.food));
        item.kind = item.material = required_entry(w.catalogue(), "item", renamed ? "base:review_roots" : "base:roots");
        item.owner = person;
        auto& fire = fixture.fire();
        fire.fuel_mg = 30000000;
        fire.ash_mg = 1000000;
        w.things().raw().get<kd::world::Item>(w.things().handle(fixture.hearth)).mass = fire.fuel_mg + fire.ash_mg;
        fire.next = fire.deadline();
        w.set_command_taker(*this);
    }
    std::string_view name() const override { return "generic food regression"; }
    void handle(kd::world::Context&, const kd::event::Event&) override {}
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        auto& w = c.world();
        const auto h = w.beings().handle(person);
        if (cmd.what == 930) {
            kd::demo::Discovery::learn(c, h, fixture.food, 1U << 8U, 3);
            const auto roast = required_entry(w.catalogue(), "blueprint", "base:review_roast");
            prepared = kd::demo::Crafting::prepare_lesson(c, h, h, roast, 7);
            w.beings().raw().get<kd::world::Work>(h) = {};
            kd::demo::FireRules::carried_food(c, person);
        }
        if (cmd.what == 931) kd::demo::FireRules::notice_food(c, h);
    }
};
}  // namespace
TEST_CASE("identical renamed food prepares cooking and retains the same thermal outcome") {
    std::array<std::int64_t, 2> state{}, elapsed{}, nourishment{};
    for (int variant = 0; variant < 2; ++variant) {
        GenericFoodOperation op(variant != 0);
        auto& w = op.fixture.camp.world();
        (void)w.command(0, 930, 0, 0);
        w.run_to(1);
        CHECK(op.prepared);
        (void)w.command(kd::time::kHour + 1, 931, 0, 0);
        w.run_to(kd::time::kHour + 2);
        const auto h = w.things().handle(op.fixture.food);
        const auto& item = w.things().raw().get<kd::world::Item>(h);
        state[variant] = item.state;
        elapsed[variant] = w.things().raw().get<kd::world::HeatTimer>(h).elapsed;
        nourishment[variant] = kd::demo::Crafting::characteristics(w.catalogue(), item)[8];
    }
    CHECK(state[0] == state[1]);
    CHECK(elapsed[0] == elapsed[1]);
    CHECK(nourishment[0] == nourishment[1]);
}

namespace {
struct GenericFuelOperation : GenericFoodOperation {
    kd::ecs::Id fuel{};
    bool chosen = false, learned = true;
    explicit GenericFuelOperation(bool renamed) : GenericFoodOperation(false) {
        auto& w = fixture.camp.world();
        const auto kind = required_entry(w.catalogue(), "item", renamed ? "base:review_fuel" : "base:dry_stick");
        fuel = fixture.add(kind, 1000000, fixture.fire().at);
        w.things().each([&](kd::ecs::Id id, auto h) {
            if (id == fuel || id == fixture.hearth) return;
            auto& item = w.things().raw().get<kd::world::Item>(h);
            item.mass = 0;
            item.state = 4;
            if (auto* f = w.things().raw().try_get<kd::world::Fire>(h)) {
                *f = {};
                f->hearth = item.home;
                f->at = w.things().raw().get<kd::world::Place>(h).at;
            }
        });
        fixture.fire().heat = 2;
        fixture.fire().fuel_mg = 1000000;
        fixture.fire().ash_mg = 4000000;
        w.things().raw().get<kd::world::Item>(w.things().handle(fixture.hearth)).mass = 5000000;
    }
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        auto& w = c.world();
        const auto h = w.beings().handle(person);
        auto& life = w.beings().raw().get<kd::world::Life>(h);
        life.decision_needs = {100, 100, 100};
        life.scores = {-1000000, -1000000, -1000000, -1000000};
        life.goal = 3;
        if (learned)
            kd::demo::Discovery::learn(c, h, fuel, (1U << 6U) | (1U << 7U) | (1U << 9U), 3);
        else {
            w.beings().raw().get<kd::world::Knowledge>(h).familiar.clear();
            kd::demo::Discovery::learn(c, h, fuel, kd::demo::Discovery::kSight, 1);
        }
        if (cmd.what == 938)
            w.beings().raw().get<kd::world::Knowledge>(h).hourly_draw =
                static_cast<std::uint64_t>(c.now() / kd::time::kHour + 1);
        if (cmd.what == 933) {
            fixture.fire().heat = 2;
            fixture.fire().fuel_mg = 1000000;
            fixture.fire().ash_mg = 4000000;
            w.things().raw().get<kd::world::Item>(w.things().handle(fixture.hearth)).mass = 5000000;
            w.beings().raw().get<kd::world::Work>(h) = {};
            w.beings().raw().get<kd::world::Thermal>(h) = {};
            w.beings().raw().get<kd::world::Place>(h).at = fixture.fire().at;
            w.beings().raw().get<kd::world::Activity>(h) = {0, c.now(), c.now() + 300, fixture.fire().at,
                                                            fixture.fire().at};
        }
        if (cmd.what == 935) {
            auto& fire = fixture.fire();
            fire.heat = 2;
            fire.fuel_mg = 1000000;
            fire.ash_mg = 4000000;
            fire.settled_at = c.now();
            fire.air_until = 0;
            fire.next = fire.deadline();
            w.things().raw().get<kd::world::Item>(w.things().handle(fixture.hearth)).mass = 5000000;
            auto& activity = w.beings().raw().get<kd::world::Activity>(h);
            activity = {0, c.now(), c.now() + 300, fixture.fire().at, fixture.fire().at};
            life.food = 4000000;
            life.water = 3000;
            life.awake = life.portion = life.applied = life.allocated_water = 0;
            life.settled = c.now();
            w.beings().raw().get<kd::world::Place>(h).at = fixture.fire().at;
            w.beings().raw().get<kd::world::Thermal>(h) = {};
            kd::demo::FireRules::deadlines(c, fixture.home);
        }
        auto& living = *const_cast<kd::demo::Living*>(fixture.camp.living());
        if (cmd.what == 936 || cmd.what == 937) {
            auto& thermal = w.beings().raw().get<kd::world::Thermal>(h);
            thermal.warmth = 70;
            thermal.felt_milli_c = 18000;
            life.scores = {0, 0, 0, cmd.what == 936 ? 100000 : 0};
            kd::demo::ChoiceSet options;
            for (std::uint8_t goal = 0; goal < 4; ++goal)
                options.add(kd::demo::Choices::body(life, goal), [&life, goal](std::uint64_t) {
                    life.goal = goal;
                    return false;
                });
            kd::demo::FireRules::choose_warm(living, c, h, &options);
            kd::demo::FireRules::choose(living, c, h, &options);
            CHECK(thermal.tending == 0);
            CHECK(life.meal_item.value == 0);
            CHECK(w.things().raw().get<kd::world::Item>(w.things().handle(fuel)).owner.value == 0);
            kd::world::CraftReason craft;
            craft.kind = 1;
            craft.need = 3;
            craft.score = 50000;
            options.add(craft, [this](std::uint64_t) {
                chosen = true;
                return true;
            });
            const auto started = options.commit(c, h);
            CHECK(started == (cmd.what == 937));
            return;
        }
        if (cmd.what == 934) {
            auto& thermal = w.beings().raw().get<kd::world::Thermal>(h);
            thermal.felt_milli_c = 18000;
            thermal.warmth = 70;
            chosen = kd::demo::FireRules::choose_warm(living, c, h);
        } else
            chosen = kd::demo::FireRules::choose(living, c, h);
    }
};
}  // namespace
TEST_CASE("identical renamed fuel receives the same autonomous tending choice and conserved feed") {
    std::array<std::int64_t, 2> fuel{};
    for (int variant = 0; variant < 2; ++variant) {
        GenericFuelOperation op(variant != 0);
        auto& w = op.fixture.camp.world();
        (void)w.command(0, 932, 0, 0);
        w.run_to(1);
        CHECK(op.chosen);
        CHECK(w.beings().raw().get<kd::world::Thermal>(w.beings().handle(op.person)).tending == 1);
        CHECK(op.fixture.fire().fuel_mg == 1000000);  // Selected plan has not read hidden physical results.
        w.run_to(61);
        fuel[variant] = op.fixture.fire().fuel_mg;
        CHECK(op.fixture.fire().fuel_mg + op.fixture.fire().ash_mg == 6000000);
    }
    CHECK(fuel[0] == fuel[1]);
    CHECK(fuel[0] == 1983334);
    CHECK_FALSE(renamed_fire_catalogue().find("blueprint", "base:bank_fire"));
    CHECK_FALSE(renamed_fire_catalogue().find("blueprint", "base:carry_ember"));
    CHECK_FALSE(renamed_fire_catalogue().find("blueprint", "base:roast_food"));
}

TEST_CASE("shared tending fuel igniting beside the hearth releases both physical owners") {
    GenericFuelOperation op(false);
    auto& w = op.fixture.camp.world();
    op.fixture.fire().air_until = 2;
    op.fixture.fire().next = op.fixture.fire().deadline();
    (void)w.command(0, 932, 0, 0);
    w.run_to(1);
    REQUIRE(op.chosen);
    const auto input = w.beings().raw().get<kd::world::Thermal>(w.beings().handle(op.person)).tending_input;
    REQUIRE(input.value);
    // A real ambient event ignites the carried dry fuel during the finite tending activity.
    w.schedule(op.fixture.home, 2, 2);
    w.run_to(3);
    const auto h = w.things().handle(input);
    REQUIRE(w.things().raw().all_of<kd::world::Fire>(h));
    if (!w.things().raw().all_of<kd::world::Fire>(h)) return;
    CHECK(w.things().raw().get<kd::world::Fire>(h).owner == op.person);
    w.run_to(61);
    const auto& item = w.things().raw().get<kd::world::Item>(h);
    const auto& fire = w.things().raw().get<kd::world::Fire>(h);
    CHECK(item.owner.value == 0);
    CHECK(fire.owner == item.owner);
    CHECK(fire.at == w.things().raw().get<kd::world::Place>(h).at);
    CHECK(item.mass == fire.fuel_mg + fire.ash_mg);
}

TEST_CASE("renamed known banking affordance is selected without the original blueprint name") {
    GenericFuelOperation op(false);
    auto& w = op.fixture.camp.world();
    // This is a maintenance-choice fixture, not an autonomous long-run scene.
    w.beings().each([&](kd::ecs::Id id, auto h) {
        if (!w.beings().raw().all_of<kd::world::Person>(h)) return;
        for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(id, slot);
        auto& activity = w.beings().raw().get<kd::world::Activity>(h);
        activity.what = 2;
        activity.end = 21 * kd::time::kHour;
    });
    (void)w.command(20 * kd::time::kHour, 933, 0, 0);
    w.run_to(20 * kd::time::kHour + 1);
    CHECK(op.chosen);
    CHECK(w.beings().raw().get<kd::world::Thermal>(w.beings().handle(op.person)).tending == 3);
}

TEST_CASE("unlearned hidden burn and fuel properties cannot change the person's tending choice") {
    for (const std::uint8_t property : {6, 7}) {
        std::array<bool, 2> chosen{};
        for (int variant = 0; variant < 2; ++variant) {
            GenericFuelOperation op(false);
            op.learned = false;
            auto& w = op.fixture.camp.world();
            auto& fuel = w.things().raw().get<kd::world::Item>(w.things().handle(op.fuel));
            fuel.changed_mask = 1U << property;
            fuel.changed[property] = variant == 0 ? 0 : 3;
            (void)w.command(0, 932, 0, 0);
            w.run_to(1);
            const auto& know = w.beings().raw().get<kd::world::Knowledge>(w.beings().handle(op.person));
            auto perceived = fuel;
            perceived.state = 0;  // Selection may have split the source; no real trial has finished yet.
            const auto* evidence = kd::demo::Discovery::familiar(know, perceived);
            REQUIRE(evidence);
            CHECK((evidence->mask & (1U << property)) == 0);
            chosen[variant] = op.chosen;
        }
        CHECK(chosen[0] == chosen[1]);
        CHECK(chosen[0]);  // The same uncertain plan is allowed in both worlds; physics still decides its result.
    }
}

TEST_CASE("a goal directed fuel trial survives spent hourly curiosity and learns only from actual contact") {
    for (const std::uint8_t burn : {0, 4}) {
        GenericFuelOperation op(false);
        op.learned = false;
        auto& w = op.fixture.camp.world();
        auto& physical = w.things().raw().get<kd::world::Item>(w.things().handle(op.fuel));
        physical.changed_mask = 1U << 6U;
        physical.changed[6] = burn;
        const auto perceived = physical;
        (void)w.command(0, 938, 0, 0);  // Labelled choice fixture, not an ordinary acceptance scene.
        w.run_to(1);
        REQUIRE(op.chosen);
        const auto h = w.beings().handle(op.person);
        const auto& mind = w.beings().raw().get<kd::world::Knowledge>(h);
        const auto* before = kd::demo::Discovery::familiar(mind, perceived);
        REQUIRE(before);
        CHECK((before->mask & (1U << 6U)) == 0);
        REQUIRE(mind.reasons.size() == 3);
        CHECK(mind.reasons.front().confidence == 0);
        CHECK(mind.reasons.front().parts[2] < 0);
        w.run_to(61);
        const auto* after = kd::demo::Discovery::familiar(mind, perceived);
        REQUIRE(after);
        CHECK((after->mask & (1U << 6U)) != 0);
        CHECK(after->values[6] == burn);
        CHECK(op.fixture.fire().fuel_mg + op.fixture.fire().ash_mg == (burn ? 6000000 : 5000000));
    }
}

TEST_CASE("fire tending keeps its own winner and two actual rejected body options through reopen") {
    GenericFuelOperation op(false);
    auto& w = op.fixture.camp.world();
    (void)w.command(0, 932, 0, 0);
    w.run_to(1);
    REQUIRE(op.chosen);
    const auto ph = w.beings().handle(op.person);
    const auto& mind = w.beings().raw().get<kd::world::Knowledge>(ph);
    const auto& thermal = w.beings().raw().get<kd::world::Thermal>(ph);
    REQUIRE(mind.reasons.size() == 3);
    CHECK(mind.reasons[0].kind == 3);
    CHECK(mind.reasons[0].action == 1);
    CHECK(mind.reasons[0].observed_heat == 2);
    CHECK(mind.reasons[0].observed_fuel_mg == 1000000);
    CHECK(mind.reasons[0].confidence == 100);
    CHECK(mind.reasons[1].kind == 2);
    CHECK(mind.reasons[2].kind == 2);
    CHECK(thermal.tending_choice == mind.choice);
    // The source fixture deliberately changes other people's schedules: test the component's
    // exact persistence independently of those unrelated mechanical fixture arrangements.
    kd::ByteWriter bytes;
    kd::ecs::write_component(mind, bytes);
    const auto saved = bytes.take();
    kd::ByteReader reader(saved);
    kd::world::Knowledge copy;
    REQUIRE(kd::ecs::read_component(copy, reader,
                                    [](std::string_view, std::uint32_t n) { return std::optional<std::uint32_t>(n); }));
    CHECK(copy.choice == mind.choice);
    REQUIRE(copy.reasons.size() == 3);
    CHECK(copy.reasons[0].observed_fuel_mg == mind.reasons[0].observed_fuel_mg);
    CHECK(copy.reasons[2].score == mind.reasons[2].score);
}

TEST_CASE("ordinary timed fire and craft choices retain three reasons and reject identity tampering") {
    kd::demo::CrowdWorld camp(713, fire_catalogue(), 1, true, true);
    auto& w = camp.world();
    w.run_to(2 * kd::time::kDay);
    const auto home = camp.camp_ids().front();
    auto& history = w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(home));
    REQUIRE_FALSE(history.choices.empty());
    bool craft = false, warmth = false;
    for (const auto& choice : history.choices) {
        REQUIRE(choice.reasons.size() == 3);
        for (const auto& reason : choice.reasons) {
            REQUIRE_MESSAGE(reason.parts[0] + reason.parts[1] + reason.parts[2] == reason.score, "choice ", choice.id,
                            " kind ", int(reason.kind), " score ", reason.score, " parts ", reason.parts[0], ",",
                            reason.parts[1], ",", reason.parts[2]);
        }
        craft |= choice.reasons.front().kind == 0 || choice.reasons.front().kind == 1;
        warmth |= choice.reasons.front().kind == 4;
    }
    CHECK(craft);
    (void)warmth;
    bool linked = false;
    for (const auto& event : history.events) {
        if (!event.choice) continue;
        linked = true;
        const auto chosen = std::find_if(history.choices.begin(), history.choices.end(),
                                         [&](const auto& x) { return x.id == event.choice; });
        REQUIRE(chosen != history.choices.end());
        CHECK(chosen->reasons.size() == 3);
    }
    CHECK(linked);
    std::string why;
    REQUIRE(reopen_fire(w, why));
    // Forge a new immutable fixture page rather than modifying a published page.
    const auto forge = [&](auto change) {
        kd::Pages<kd::world::Choice> forged;
        for (std::size_t i = 0; i < history.choices.size(); ++i) {
            auto choice = history.choices[i];
            if (i == 0) change(choice);
            forged.push_back(std::move(choice));
        }
        history.choices = std::move(forged);
    };
    const auto original = history.choices.front().actor;
    forge([&](auto& choice) { choice.actor = home; });
    CHECK_FALSE(reopen_fire(w, why));
    forge([&](auto& choice) { choice.actor = original; });
    REQUIRE(reopen_fire(w, why));
    forge([](auto& choice) { choice.reasons.front().confidence = 101; });
    CHECK_FALSE(reopen_fire(w, why));
    forge([](auto& choice) { choice.reasons.front().confidence = 100; });
    REQUIRE(reopen_fire(w, why));
    forge([](auto& choice) { ++choice.reasons.front().parts[0]; });
    CHECK_FALSE(reopen_fire(w, why));
}

TEST_CASE("warming replaces earlier craft reasons with its own recorded winner and rejected options") {
    GenericFuelOperation op(false);
    auto& w = op.fixture.camp.world();
    const auto ph = w.beings().handle(op.person);
    auto& mind = w.beings().raw().get<kd::world::Knowledge>(ph);
    mind.reasons = {{1, 0, 2, 3, kd::world::kNoRecipe, 8, 1, 300, {}}};
    (void)w.command(0, 934, 0, 0);
    w.run_to(1);
    REQUIRE(op.chosen);
    REQUIRE(mind.reasons.size() == 3);
    CHECK(mind.reasons[0].kind == 4);
    CHECK(mind.reasons[0].need_met == 70);
    CHECK(mind.reasons[0].benefit == 30);
    CHECK(mind.reasons[1].kind == 2);
    CHECK(mind.reasons[2].kind == 2);
    CHECK(mind.choice == w.beings().raw().get<kd::world::Thermal>(ph).warm_choice);
}

#include "kd/run/workers.hpp"
TEST_CASE("kept choices and resumed plans match serial and four-worker islands") {
    kd::demo::CrowdWorld serial(714, fire_catalogue(), 2, true, true);
    kd::demo::CrowdWorld parallel(714, fire_catalogue(), 2, true, true);
    kd::run::Workers workers(4);
    const auto end = 2 * kd::time::kDay;
    serial.world().run_to(end);
    parallel.world().run_islands(end, workers, 32);
    CHECK(serial.world().digests().whole == parallel.world().digests().whole);
    CHECK(kd::save::write_snapshot(serial.world().save()) == kd::save::write_snapshot(parallel.world().save()));
    std::string why;
    CHECK(reopen_fire(serial.world(), why));
    CHECK(reopen_fire(parallel.world(), why));
}

#include "kd/proof/fire_cases.hpp"
TEST_CASE("autonomous cold-hearth chain starts without an action or memory and bounds its proof trace") {
    for (const bool control : {false, true}) {
        const auto run = kd::proof::fire_chain(fire_catalogue(), 715, control, kd::time::kDay);
        CHECK(run.ordinary_setup);
        CHECK(run.ended <= kd::time::kDay);
        CHECK(run.reopen_failures == 0);
        CHECK(run.peak_trace_records < 100000);
        CHECK(run.choice_count > 0);
        CHECK(run.choice_wire_bytes >= 204 * run.choice_count);
        CHECK(run.snapshot_bytes > 0);
        CHECK_FALSE(run.digest.empty());
        if (control) {
            CHECK(run.ember_at == -1);
            CHECK(run.cooked_at == -1);
            CHECK_FALSE(run.complete);
        }
    }
}

TEST_CASE("tentative fire trials use the same visible evidence regardless of hidden burn and resolve after work") {
    std::array<bool, 2> selected{};
    for (int variant = 0; variant < 2; ++variant) {
        GenericFuelOperation op(false);
        op.learned = false;
        auto& w = op.fixture.camp.world();
        auto& raw = w.beings().raw();
        auto& mind = raw.get<kd::world::Knowledge>(w.beings().handle(op.person));
        mind.curiosity = 80;
        std::uint64_t hour = 1;
        for (; hour < 1000; ++hour) {
            const kd::chance::Draws draws(w.seed(), kd::chance::name("curious hour"), op.person.value,
                                          static_cast<std::int64_t>(hour),
                                          kd::chance::name("attempt and familiar action"));
            if (draws.below(0, 24) == 0) break;
        }
        REQUIRE(hour < 1000);
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (!raw.all_of<kd::world::Person>(h)) return;
            for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(id, slot);
        });
        auto& fuel = w.things().raw().get<kd::world::Item>(w.things().handle(op.fuel));
        fuel.changed_mask |= 1U << 6U;
        fuel.changed[6] = variant == 0 ? 0 : 3;
        const auto seen_fuel = fuel;
        const auto at = static_cast<std::int64_t>(hour - 1) * kd::time::kHour;
        (void)w.command(at, 935, 0, 0);
        w.run_to(at + 1);
        selected[variant] = op.chosen;
        REQUIRE(op.chosen);
        REQUIRE(mind.reasons.size() == 3);
        CHECK(mind.reasons[0].confidence == 0);
        CHECK(mind.reasons[0].need == 3);
        CHECK(op.fixture.fire().fuel_mg == 1000000);
        const auto* evidence = kd::demo::Discovery::familiar(mind, seen_fuel);
        REQUIRE(evidence);
        CHECK((evidence->mask & (1U << 6U)) == 0);
        w.run_to(at + 61);
        CHECK(op.fixture.fire().fuel_mg + op.fixture.fire().ash_mg == (variant == 0 ? 5000000 : 6000000));
    }
    CHECK(selected[0] == selected[1]);
}

TEST_CASE("fire selection ignores spent distractions and keeps the lowest eligible ID after pool shuffling") {
    std::array<std::string, 2> digests;
    for (int variant = 0; variant < 2; ++variant) {
        GenericFuelOperation op(false);
        auto& w = op.fixture.camp.world();
        const auto copy = op.fixture.fire();
        const auto second = op.fixture.add(fire_entry("base:dry_stick"), 5000000, copy.at);
        w.things().raw().emplace<kd::world::Fire>(w.things().handle(second), copy);
        for (int n = 0; n < 10000; ++n) {
            const auto spent = op.fixture.add(fire_entry("base:crumb"), 0, copy.at);
            w.things().raw().get<kd::world::Item>(w.things().handle(spent)).state = 4;
        }
        if (variant) w.things().fuzz(94);
        (void)w.command(0, 932, 0, 0);
        w.run_to(1);
        REQUIRE(op.chosen);
        CHECK(w.beings().raw().get<kd::world::Thermal>(w.beings().handle(op.person)).tending_fire == op.fixture.hearth);
        digests[variant] = kd::num::to_hex(w.digests().whole);
    }
    CHECK(digests[0] == digests[1]);
}

TEST_CASE("body and craft can beat earlier warmth and tending without reserving losing inputs") {
    for (const auto command : {936U, 937U}) {
        GenericFuelOperation op(false);
        auto& w = op.fixture.camp.world();
        (void)w.command(0, command, 0, 0);
        w.run_to(1);
        const auto h = w.beings().handle(op.person);
        const auto& mind = w.beings().raw().get<kd::world::Knowledge>(h);
        REQUIRE(mind.reasons.size() == 3);
        CHECK(mind.reasons[0].kind == (command == 936 ? 2 : 1));
        CHECK(mind.reasons[0].score >= mind.reasons[1].score);
        CHECK(mind.reasons[1].score >= mind.reasons[2].score);
        for (const auto& reason : mind.reasons)
            CHECK(reason.parts[0] + reason.parts[1] + reason.parts[2] == reason.score);
        CHECK(w.beings().raw().get<kd::world::Thermal>(h).tending == 0);
        CHECK(op.chosen == (command == 937));
        CHECK(w.things().raw().get<kd::world::Item>(w.things().handle(op.fuel)).mass == 1000000);
    }
}
TEST_CASE("making and teaching share eight blueprint slots with stable equal-score order") {
    kd::demo::ChoiceSet options;
    for (std::uint32_t n = 0; n < 8; ++n) {
        kd::world::CraftReason reason;
        reason.intended = 1;
        reason.recipe = n;
        reason.score = 10;
        options.add(reason, [](std::uint64_t) { return false; });
    }
    kd::world::CraftReason teaching;
    teaching.kind = 5;
    teaching.intended = 1;
    teaching.recipe = 8;
    teaching.score = 10;
    options.add(teaching, [](std::uint64_t) { return false; });
    REQUIRE(options.reasons().size() == 8);
    CHECK(options.reasons().front().recipe == 0);
    teaching.score = 11;
    options.add(teaching, [](std::uint64_t) { return false; });
    REQUIRE(options.reasons().size() == 8);
    CHECK(options.reasons().front().recipe == 0);
    CHECK(options.reasons().back().recipe == 8);
}
TEST_CASE("fire chain evidence rejects unrelated tending and follows actual carried descendants") {
    // Labelled evidence fixture: it tests the observer, never autonomous success rates.
    FireFixture fixture;
    auto& w = fixture.camp.world();
    const auto original = fixture.hearth;
    fixture.fire().origin = original;
    const auto other = fixture.add(fire_entry("base:dry_stick"), 1000, fixture.fire().at);
    auto unrelated = fixture.fire();
    unrelated.origin = other;
    w.things().raw().emplace<kd::world::Fire>(w.things().handle(other), unrelated);
    const auto carried = fixture.add(fire_entry("base:dry_stick"), 1000, fixture.fire().at);
    auto child = fixture.fire();
    child.source = original;
    child.ignited_at = 30;
    w.things().raw().emplace<kd::world::Fire>(w.things().handle(carried), child);
    const auto friction = required_entry(fire_catalogue(), "blueprint", "base:ember_drill");
    const auto roast = required_entry(fire_catalogue(), "blueprint", "base:roast_food");
    for (int variant = 0; variant < 5; ++variant) {
        kd::proof::FireRun run;
        kd::world::Result ember;
        ember.recipe = friction;
        ember.kind = 1;
        ember.result = original;
        ember.at = 10;
        run.observe_result(w, ember);
        kd::world::Record tending;
        tending.what = 218;
        tending.a = variant == 0 ? other.value : original.value;
        tending.key.second = variant == 3 ? 5 : 20;
        run.observe_record(w, tending);
        kd::world::Record flame;
        flame.what = 219;
        flame.a = variant == 4 ? other.value : original.value;
        flame.b = 2;
        flame.key.second = 25;
        run.observe_record(w, flame);
        kd::world::Result cooked;
        cooked.recipe = roast;
        cooked.kind = 1;
        cooked.result = fixture.food;
        cooked.at = 3630;
        cooked.heat_sources = {{carried, variant == 2 ? other : original, 20, 30, 3600}};
        run.observe_result(w, cooked);
        run.finish_interval(w);
        CHECK(run.complete == (variant == 1));
        if (run.complete) {
            CHECK(run.completed_origin == original);
            CHECK(run.tended_fire == original);
            CHECK(run.cooked_item == fixture.food);
            CHECK(run.flame_at == 25);
        }
    }
}
TEST_CASE("first heated hour retains factual source through split and reopen") {
    CookingOperations ops;
    auto& w = ops.fires.fixture.camp.world();
    const auto fire = ops.fires.fixture.hearth;
    ops.fires.fixture.fire().origin = fire;
    ops.fires.fixture.fire().tended_at = 0;
    // This fixture starts with an ordinary hot hearth as well. Isolate the labelled source.
    for (const auto h : w.things().raw().view<kd::world::Fire>()) {
        if (w.things().id_of(h) == fire) continue;
        auto& other = w.things().raw().get<kd::world::Fire>(h);
        other.heat = 0;
        other.next = 0;
    }
    ops.request(0, 921);
    ops.request(1800, 926);
    REQUIRE(ops.timer().heat_sources.size() == 1);
    CHECK(ops.timer().heat_sources.front().fire == fire);
    CHECK(ops.timer().heat_sources.front().origin == fire);
    CHECK(ops.timer().heat_sources.front().tended_at == 0);
    CHECK(ops.timer().heat_sources.front().seconds == 1800);
    std::string why;
    auto opened = reopen_fire(w, why);
    INFO(why);
    REQUIRE(opened);
    w.run_to(3601);
    opened->world().run_to(3601);
    CHECK(w.digests().whole == opened->world().digests().whole);
    REQUIRE(ops.timer().heat_sources.size() == 1);
    CHECK(ops.timer().heat_sources.front().seconds == 3600);
}
TEST_CASE("heat ancestry readers reject forged root cycle and duration") {
    for (int fault = 0; fault < 5; ++fault) {
        CookingOperations ops;
        auto& w = ops.fires.fixture.camp.world();
        ops.fires.fixture.fire().origin = ops.fires.fixture.hearth;
        ops.request(0, 921);
        ops.request(100, 921);
        if (fault == 0) ops.fires.fixture.fire().origin = ops.portion;
        if (fault == 1) ops.fires.fixture.fire().source = ops.fires.fixture.hearth;
        if (fault == 2) ops.timer().heat_sources.front().seconds = 3601;
        if (fault == 3) ops.timer().heat_sources.front().origin = ops.portion;
        if (fault == 4) ops.timer().heat_sources.front().tended_at = 101;
        std::string why;
        CHECK_FALSE(reopen_fire(w, why));
        CHECK_FALSE(why.empty());
    }
}
