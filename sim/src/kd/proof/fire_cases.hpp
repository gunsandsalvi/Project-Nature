// An autonomous cold-hearth chain: setup changes physical reserves, never minds or actions.
#pragma once
#include <map>
#include <set>
#include "kd/proof/learning_cases.hpp"
namespace kd::proof {
struct FireRun {
    std::uint64_t seed = 0, friction_choices = 0, wood_pairs = 0, friction_results = 0, tended = 0, cooked = 0;
    std::int64_t ember_at = -1, flame_at = -1, tend_at = -1, cooked_at = -1, completed_at = -1, ended = 0;
    bool complete = false, wet_control = false, ordinary_setup = true;
    std::uint64_t choice_count = 0, choice_wire_bytes = 0, snapshot_bytes = 0;
    std::uint32_t reopen_failures = 0;
    std::size_t peak_trace_records = 0;
    std::string digest;
    ecs::Id completed_origin{}, cooked_item{}, tended_fire{};
    std::map<ecs::Id, time::Seconds> embers;
    std::set<std::pair<ecs::Id, time::Seconds>> tending, flames;
    std::vector<world::Result> pending_cooking;
    void observe_record(const world::World& w, const world::Record& record);
    void finish_interval(const world::World& w);
    void observe_result(const world::World& w, const world::Result& event);
};
// Same 25 adults and finite full-kit food/water reserves as the sharp-stone scene.
// Three game years; cold initial hearth; control wets all starting combustible inputs.
// Complete means a friction ember, an actual tended flame and subsequently cooked food.
[[nodiscard]] FireRun fire_chain(const data::Catalogue& catalogue, std::uint64_t seed, bool wet_control,
                                 time::Seconds duration = 3 * time::kYear, const LearningProgress& progress = {});
}  // namespace kd::proof
