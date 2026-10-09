#include "kd/demo/fire.hpp"
#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
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
    kd::demo::CrowdWorld camp{91, fire_catalogue(), 1, true, true};
    kd::ecs::Id home{}, hearth{}, food{};
    FireFixture() {
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
        f.next = kd::time::kHour;
        food = add(fire_entry("base:roots"), 1000000, here);
        auto& t = w.things().raw().emplace<kd::world::HeatTimer>(w.things().handle(food));
        t.item = food;
        t.elapsed = 600;
        t.next = kd::time::kHour - 600;
        w.schedule(home, 2, f.next);
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
    for (int fault = 0; fault < 5; ++fault) {
        FireFixture f;
        auto& w = f.camp.world();
        if (fault == 0) f.fire().fuel_mg = -1;
        if (fault == 1) f.fire().ash_mg += 1;
        if (fault == 2) f.fire().owner = kd::ecs::Id{999};
        if (fault == 3) w.cancel(f.home, 2);
        if (fault == 4) w.cancel(f.home, 3);
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
