// Frozen α2.13b fixtures; changes in tuning add a proof rather than shifting M1 suites.
#include "kd/proof/camp_cases.hpp"
#include "kd/demo/crowd_world.hpp"
namespace kd::proof {
std::vector<data::SourceFile> camp_files() {
    auto files = fixture_files();
    files.push_back(
        {"base/tuning/living.toml",
         R"camp(# Scoped mild adult camp: raw berries, a finite spring and living stand. Not complete ecology/health.
food_day = "4 kg"
water_day = "3 l"
speed = "1.25 m/s"
loaded_speed = "1 m/s"
spring_hour = "5 l"
fruit_hour = "5 kg"
fruit_water = "1 l"
)camp"});
    files.push_back({"base/need_use/food.toml", R"camp(need = 0
food = "1 kg"
water = "0 ml"
restored = "0 s"
water_per_kg = "800 ml"
benefit = 25
use = { life = "20 min", game = "20 min" }
gather = { life = "30 min", game = "30 min" }
area = "2 m"
)camp"});
    files.push_back({"base/need_use/water.toml", R"camp(need = 1
food = "0 mg"
water = "500 ml"
restored = "0 s"
water_per_kg = "0 ml"
benefit = 17
use = { life = "2 min", game = "2 min" }
gather = { life = "1 s", game = "1 s" }
area = "1 m"
)camp"});
    files.push_back({"base/need_use/rest.toml", R"camp(need = 2
food = "0 mg"
water = "0 ml"
restored = "4 h"
water_per_kg = "0 ml"
benefit = 12
use = { life = "2 h", game = "2 h" }
gather = { life = "1 s", game = "1 s" }
area = "2 m"
)camp"});
    return files;
}
std::string camp_life(run::Workers& workers) {
    data::Catalogue catalogue;
    KD_CHECK(catalogue.load(camp_files()).empty(), "camp proof catalogue loads");
    num::Digest digest;
    for (const std::uint64_t seed : {1ULL, 17ULL, 91ULL}) {
        demo::CrowdWorld reference(seed, catalogue, 1, true);
        std::string why;
        auto parallel = demo::CrowdWorld::open(catalogue, reference.world().save(), why);
        KD_CHECK(parallel != nullptr, "camp proof opens initial actions");
        for (time::Seconds day = 1; day <= 3; ++day) {
            reference.world().run_to(day * time::kDay);
            parallel->world().run_islands(day * time::kDay, workers, 600);
            const auto reference_digest = reference.world().digests().whole;
            const auto parallel_digest = parallel->world().digests().whole;
            KD_CHECK(reference_digest == parallel_digest, "camp workers differ");
            digest.u64(reference.world().digests().whole);
            auto reopened = demo::CrowdWorld::open(catalogue, parallel->world().save(), why);
            KD_CHECK(reopened != nullptr, "camp proof resumes needs, actions and memory");
            parallel = std::move(reopened);
        }
    }
    return digest.hex();
}
}  // namespace kd::proof
