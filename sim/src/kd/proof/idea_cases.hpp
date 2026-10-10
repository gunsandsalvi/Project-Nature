// Conditional remembered-action scenes; their frequency is not a natural discovery claim.
#pragma once
#include "kd/demo/crowd_world.hpp"
namespace kd::proof {
struct IdeaScene {
    std::unique_ptr<demo::CrowdWorld> camp;
    ecs::Id person{}, home{};
    std::uint64_t memory = 0;
};
// Real 100-second twirl beside the starting fire, below the 300-second fitted result.
// No traits, needs, knowledge, rolls or stocks are replenished or changed into success.
[[nodiscard]] IdeaScene remembered_wood(const data::Catalogue& catalogue, std::uint64_t seed);
struct IdeaPair {
    std::uint64_t seed = 0;
    std::int64_t dream_at = -1, attempt_at = -1, control_try_at = -1, missing_attempt_at = -1;
    std::int64_t success = 0, control_success = 0, missing_success = 0, removed_mg = 0;
    std::uint8_t status = 0;
    bool pending_reopen = false, delivered_reopen = false, final_reopen = false, absent_conserved = false;
    std::string sent_digest, control_digest, missing_digest;
};
[[nodiscard]] IdeaPair idea_pair(const data::Catalogue& catalogue, std::uint64_t seed);
}  // namespace kd::proof
