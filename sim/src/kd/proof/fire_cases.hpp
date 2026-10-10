// An autonomous cold-hearth chain: setup changes physical reserves, never minds or actions.
#pragma once
#include "kd/proof/learning_cases.hpp"
namespace kd::proof {
struct FireRun {
    std::uint64_t seed = 0, friction_choices = 0, wood_pairs = 0, friction_results = 0, tended = 0, cooked = 0;
    std::int64_t ember_at = -1, flame_at = -1, tend_at = -1, cooked_at = -1, completed_at = -1, ended = 0;
    bool complete = false, wet_control = false, ordinary_setup = true;
    std::uint32_t reopen_failures = 0;
    std::size_t peak_trace_records = 0;
    std::string digest;
};
// Same 25 adults and finite full-kit food/water reserves as the sharp-stone scene.
// Three game years; cold initial hearth; control wets all starting combustible inputs.
// Complete means a friction ember, an actual tended flame and subsequently cooked food.
[[nodiscard]] FireRun fire_chain(const data::Catalogue& catalogue, std::uint64_t seed, bool wet_control,
                                 time::Seconds duration = 3 * time::kYear, const LearningProgress& progress = {});
}  // namespace kd::proof
