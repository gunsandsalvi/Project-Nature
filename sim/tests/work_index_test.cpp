#include "doctest.h"
#include "kd/chance/chance.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/fire.hpp"
#include "kd/ecs/component.hpp"
#include "kd/run/workers.hpp"
#include "kd/world/motion.hpp"

TEST_CASE("rounded torus proximity intervals equal each scalar second through different motion phases") {
    const kd::num::Torus torus(1001, 997);
    for (std::uint64_t seed = 0; seed < 200; ++seed) {
        const kd::chance::Draws draws(seed, kd::chance::name("motion test"), 702, 0, kd::chance::name("geometry"));
        const auto point = [&](std::uint64_t n) {
            return kd::num::Point{static_cast<std::int32_t>(draws.below(n, 1001)),
                                  static_cast<std::int32_t>(draws.below(n + 1, 997))};
        };
        const kd::world::Activity a{1, 37, 630, point(0), point(2)}, b{1, 97, 1079, point(4), point(6)};
        const auto radius = static_cast<std::int64_t>(draws.below(8, 220));
        const auto spans = kd::world::proximity_spans(a, b, torus, 0, 1200, radius);
        std::size_t cursor = 0;
        for (kd::time::Seconds second = 0; second < 1200; ++second) {
            while (cursor < spans.size() && spans[cursor].end <= second) ++cursor;
            const bool indexed = cursor < spans.size() && spans[cursor].begin <= second;
            const bool scalar = torus.squared_distance(a.at(torus, second), b.at(torus, second)) <= radius * radius;
            CHECK(indexed == scalar);
        }
    }
}

TEST_CASE("co-moving exact radius and stationary empty heat intervals have bounded work at long durations") {
    const kd::num::Torus torus(20000, 10000);
    const kd::world::Activity a{1, 0, 1'000'000'000, {19950, 50}, {4050, 50}},
        b{1, 0, 1'000'000'000, {50, 50}, {4150, 50}};
    const auto inside = kd::world::proximity_spans(a, b, torus, 0, 1'000'000'000, 100);
    REQUIRE(inside.size() == 1);
    CHECK(inside.front().begin == 0);
    CHECK(inside.front().end == 1'000'000'000);
    CHECK(kd::world::proximity_spans(a, b, torus, 0, 1'000'000'000, 99).empty());
}

TEST_CASE("interval thermal integration preserves every scalar water fraction and warmed second") {
    const kd::num::Torus torus(1001, 997);
    for (std::uint64_t seed = 0; seed < 80; ++seed) {
        const kd::chance::Draws draws(seed, kd::chance::name("thermal test"), 802, 0, kd::chance::name("geometry"));
        const auto point = [&](std::uint64_t n) {
            return kd::num::Point{static_cast<std::int32_t>(draws.below(n, 1001)),
                                  static_cast<std::int32_t>(draws.below(n + 1, 997))};
        };
        const kd::world::Activity activity{1, 0, 1800, point(0), point(2)};
        std::vector<kd::demo::FireRules::HeatField> fires;
        for (std::uint64_t n = 0; n < seed % 4; ++n)
            fires.push_back({kd::ecs::Id{(static_cast<std::uint64_t>(kd::ecs::Family::thing) << 60U) | (n + 1)},
                             {1, 97, 1500, point(10 + n * 4), point(12 + n * 4)}});
        kd::world::Thermal initial;
        initial.water_remainder = static_cast<std::int64_t>(draws.below(8, 8'640'000'000ULL));
        initial.warming_progress = 101;
        initial.water_due_ml = 17;
        const auto ambient = seed % 2 ? 24000 : 18000;
        const auto scalar =
            kd::demo::FireRules::sample_thermal(initial, activity, torus, 1701, 2111, ambient, fires, true);
        const auto indexed = kd::demo::FireRules::sample_thermal(initial, activity, torus, 1701, 2111, ambient, fires);
        kd::ByteWriter left, right;
        kd::ecs::write_component(scalar, left);
        kd::ecs::write_component(indexed, right);
        CHECK(left.take() == right.take());
    }
}

TEST_CASE("indexed and scalar live work preserve whole states through deadlines workers and reopen") {
    kd::data::Catalogue catalogue;
    REQUIRE(catalogue.load(kd::data::read_catalogue(KD_REPO "/data")).empty());
    // Ordinary regression seeds, separate from every declared acceptance range.
    for (const std::uint64_t seed : {8801, 8802}) {
        const bool cold = seed % 2 == 0;
        kd::demo::CrowdWorld indexed(seed, catalogue, 1, true, true, cold),
            scalar(seed, catalogue, 1, true, true, cold);
        scalar.world().set_scalar_work(true);
        kd::run::Workers workers(4);
        for (const auto at : std::array<kd::time::Seconds, 8>{1, 180, 181, 3601, 9001, kd::time::kDay,
                                                              2 * kd::time::kDay, 3 * kd::time::kDay}) {
            indexed.world().run_islands(at, workers, 301);
            scalar.world().run_to(at);
            CHECK(indexed.world().digests().whole == scalar.world().digests().whole);
            std::string why;
            const auto copy = kd::demo::CrowdWorld::open(catalogue, indexed.world().save(), why);
            REQUIRE_MESSAGE(copy, why);
            if (!copy) return;
            CHECK(copy->world().digests().whole == scalar.world().digests().whole);
        }
    }
}
