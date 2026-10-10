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
    /// Whether another version saved it, and how big an update this one is for it (PLT-09); after a big one it is not
    /// opened, and its history can still be read.
    save::Update update = save::Update::none;
};

/// A crowd's world.toml (A3.7): its name, which the Worlds page shows and you may change, and the seed and number of
/// camps (0 for the tuning's) that make it again if no snapshot is whole; and, for a test's world, that it is one and
/// the switches it runs with, which a world made new takes (PLT-05, RES-10).
struct About {
    std::string name;
    std::uint64_t seed = 1;
    std::int64_t camps = 0;
    bool test = false;
    bool camp_alpha = false;
    bool discovery = false;
    std::vector<std::string> switches;
    bool fire_already_out = false;
};

/// world.toml's text for a crowd's world, and what a world.toml says; nothing if it is not a crowd's.
[[nodiscard]] std::string about_text(const About& a);
[[nodiscard]] std::optional<About> read_about(const std::string& text);

/// Opens current-format saves and replays their journal, or makes a new camp when no snapshot exists.
/// Older formats are refused before recovery writes.
[[nodiscard]] Kept keep_crowd(save::Keeper& keeper, const data::Catalogue& catalogue, std::uint64_t seed,
                              std::int64_t camps, bool camp_alpha = false);

}  // namespace kd::demo
