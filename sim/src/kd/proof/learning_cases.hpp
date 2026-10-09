// Finite sharp-stone scenes. Initial reserves are a labelled fixture, never runtime replenishment.
#pragma once
#include <functional>
#include "kd/demo/crowd_world.hpp"
namespace kd::proof {
struct LearningRun {
    std::uint64_t seed = 0, work_started = 0, fitting_tries = 0, flakes = 0;
    std::int64_t first = -1, ended = 0, holders = 0, adults = 0;
    std::uint8_t route = 0;
    std::int64_t crop_left = 0, root_water_left = 0, upstream_left = 0, stone_left = 0;
    std::string digest;
};
// 40 tonnes seasonal fruit, 40,000 litres root water, 120,000 litres upstream,
// and 200 additional 50-kg, 300-mm cores (75 flint, 75 chert, 50 granite).
// Same starting minds/traits/choices; the control changes only physical flaking.
void learning_reserves(demo::CrowdWorld& camp, bool non_flaking);
using LearningProgress = std::function<void(const world::World&, time::Seconds)>;
[[nodiscard]] LearningRun sharp_stone(const data::Catalogue& catalogue, std::uint64_t seed, bool non_flaking,
                                      std::int64_t workers = 1, const LearningProgress& progress = {});
}  // namespace kd::proof
