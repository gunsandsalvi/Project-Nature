// A crowd's world kept in its folder (A3.7): opened from its newest whole snapshot with the journal's later commands
// acting again, its history made again checked as it runs; or, when the folder holds no snapshot, made new from its
// seed and written at once.
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "kd/demo/crowd_world.hpp"
#include "kd/save/keeper.hpp"

namespace kd::demo {

/// A crowd's world as its folder gave it.
struct Kept {
    std::unique_ptr<CrowdWorld> crowd;
    /// Made new, rather than opened from a snapshot.
    bool made = false;
    /// Why it could not be opened, when it could not.
    std::string problem;
    /// Each damaged file set aside or log cut, and why.
    std::vector<std::string> damaged;
    /// The snapshot it was opened from.
    std::string snapshot;
    /// The commands the journal holds, which you gave and will not give again, and those of them acted again,
    /// being later than the snapshot.
    std::uint64_t journaled = 0;
    std::uint64_t replayed = 0;
    /// How far it had got before it closed: its snapshot's frontier, or its last pause mark, command or history, if
    /// later; it catches up to there.
    time::Seconds was_at = 0;
};

/// world.toml for a new crowd's world.
[[nodiscard]] std::string crowd_about(std::uint64_t seed, std::int64_t camps);

/// Implements TIM-05 and PLT-07, see A3.7: a crowd's world from its folder, or a new one from this seed and number of
/// camps (0 for the tuning's), unless the folder's world.toml says otherwise.
[[nodiscard]] Kept keep_crowd(save::Keeper& keeper, const data::Catalogue& catalogue, std::uint64_t seed,
                              std::int64_t camps);

}  // namespace kd::demo
