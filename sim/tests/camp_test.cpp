#include "kd/world/camp.hpp"
#include "doctest.h"
#include "helpers.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"

TEST_CASE("Camp alpha records survive snapshots and pool order without changing foundation worlds") {
    const auto& catalogue = kd::test::camp_fixture();
    kd::demo::CrowdWorld camp(17, catalogue, 1, true);
    auto& w = camp.world();
    std::size_t people = 0;
    w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
        if (id.family() != kd::ecs::Family::person) return;
        const auto& p = w.beings().raw().get<kd::world::Person>(h);
        CHECK(p.age_years >= 18);
        CHECK(p.age_years <= 45);
        CHECK(p.name_index < 25);
        ++people;
    });
    CHECK(people == 25);
    const auto camp_id = camp.camp_ids().front();
    const auto ch = w.beings().handle(camp_id);
    CHECK(w.beings().raw().get<kd::world::Camp>(ch).water_ml == 100000);
    w.run_to(43200);
    const auto saved = w.save();
    const auto digest = w.digests().whole;
    w.beings().fuzz(91);
    CHECK(w.digests().whole == digest);
    std::string why;
    auto opened = kd::demo::CrowdWorld::open(catalogue, saved, why);
    REQUIRE(opened);
    CHECK(opened->world().digests().whole == digest);
    kd::run::Workers workers(4);
    w.run_to(90000);
    opened->world().run_islands(90000, workers, 600);
    CHECK(opened->world().digests().whole == w.digests().whole);
    // A changed amount participates in the digest and round trip.
    w.beings().raw().get<kd::world::Camp>(ch).water_ml -= 1;
    CHECK(opened->world().digests().whole != w.digests().whole);
    auto changed = kd::demo::CrowdWorld::open(catalogue, w.save(), why);
    REQUIRE(changed);
    CHECK(changed->world().digests().whole == w.digests().whole);
}

TEST_CASE("Camp alpha refuses missing, corrupt and out-of-bounds person records") {
    const auto& catalogue = kd::test::camp_fixture();
    kd::demo::CrowdWorld camp(17, catalogue, 1, true);
    std::string why;
    auto saved = camp.world().save();
    saved.erase(std::remove_if(saved.begin(), saved.end(),
                               [](const auto& chunk) { return chunk.tag == kd::save::tag("CAMP"); }),
                saved.end());
    CHECK_FALSE(kd::demo::CrowdWorld::open(catalogue, saved, why));
    saved = camp.world().save();
    for (auto& chunk : saved)
        if (chunk.tag == kd::save::tag("CAMP")) chunk.data.pop_back();
    CHECK_FALSE(kd::demo::CrowdWorld::open(catalogue, saved, why));
    camp.world().beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
        if (id.family() == kd::ecs::Family::person) camp.world().beings().raw().get<kd::world::Place>(h).at = {0, 0};
    });
    CHECK_FALSE(kd::demo::CrowdWorld::open(catalogue, camp.world().save(), why));
}

TEST_CASE("Camp alpha metadata preserves its scene kind across rename and recovery") {
    kd::demo::About about;
    about.name = "A saved camp";
    about.seed = 17;
    about.camps = 1;
    about.camp_alpha = true;
    const auto read = kd::demo::read_about(kd::demo::about_text(about));
    REQUIRE(read);
    if (!read) return;
    CHECK(read->camp_alpha);
    CHECK(read->name == about.name);
    CHECK_FALSE(kd::demo::read_about("kind = \"invented\"\nseed = 1\ncamps = 1\n"));
}

TEST_CASE("checksummed snapshots with duplicate CAMP chunks are refused") {
    const auto& catalogue = kd::test::camp_fixture();
    kd::demo::CrowdWorld camp(17, catalogue, 1, true);
    for (int fault = 0; fault < 3; ++fault) {
        auto chunks = camp.world().save();
        auto duplicate = *kd::save::find_chunk(chunks, kd::save::tag("CAMP"));
        if (fault == 1) duplicate.version = 2;
        if (fault == 2) duplicate.data.clear();
        chunks.push_back(duplicate);
        std::string why;
        const auto decoded = kd::save::read_snapshot(kd::save::write_snapshot(chunks), why);
        REQUIRE(decoded);
        if (!decoded) return;
        CHECK_FALSE(kd::demo::CrowdWorld::open(catalogue, *decoded, why));
    }
}

TEST_CASE("checksummed Camp snapshots refuse noncanonical person and centre coordinates") {
    const auto& catalogue = kd::test::camp_fixture();
    for (bool centre : {false, true}) {
        for (int fault = 0; fault < 4; ++fault) {
            kd::demo::CrowdWorld camp(17, catalogue, 1, true);
            auto& w = camp.world();
            const auto ch = w.beings().handle(camp.camp_ids().front());
            const auto damage = [&](kd::num::Point p) {
                if (fault < 2)
                    p.x += fault == 0 ? w.torus().width() : -w.torus().width();
                else
                    p.y += fault == 2 ? w.torus().height() : -w.torus().height();
                return p;
            };
            if (centre)
                w.beings().raw().get<kd::world::Place>(ch).at = damage(w.beings().raw().get<kd::world::Place>(ch).at);
            w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
                if (id.family() != kd::ecs::Family::person) return;
                if (centre)
                    w.beings().raw().get<kd::demo::Home>(h).at = w.beings().raw().get<kd::world::Place>(ch).at;
                else {
                    auto& p = w.beings().raw().get<kd::world::Place>(h).at;
                    p = damage(p);
                    auto& activity = w.beings().raw().get<kd::world::Activity>(h);
                    activity.from = activity.to = p;
                }
            });
            std::string why;
            const auto decoded = kd::save::read_snapshot(kd::save::write_snapshot(w.save()), why);
            REQUIRE(decoded);
            if (!decoded) return;
            CHECK_FALSE(kd::demo::CrowdWorld::open(catalogue, *decoded, why));
        }
    }
}
