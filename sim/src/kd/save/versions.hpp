// What a snapshot was saved under (A3.7, PLT-09): the app's version, the rules for making worlds and the catalogue's
// rules, the migrations the world has had, the versions it has run under and how long under the last. The keeper
// writes them as a chunk of each snapshot, so a world opened by a new version knows what changed: a change to the
// rules for making worlds is a big update, which the world cannot carry on through; anything else is a small one.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/save/snapshot.hpp"
#include "kd/time/calendar.hpp"

namespace kd::save {

/// An update as a world meets it: none when the version that saved it opens it.
enum class Update : std::uint8_t { none, small, big };

/// Implements PLT-09, see A3.7: what a snapshot was saved under.
struct Versions {
    /// A version the world has run under, from a game second on.
    struct Era {
        std::string build;
        time::Seconds from = 0;
    };

    /// The app's version that saved it, such as "α1.4b".
    std::string build;
    /// The rules for making worlds: the world-making version and each source's world digest.
    std::uint64_t making = 0;
    /// Each source's version and rules digest, for the record.
    std::uint64_t rules = 0;
    /// The migrations the world has had, by name.
    std::vector<std::string> migrations;
    /// The versions the world has run under, oldest first.
    std::vector<Era> eras;
    /// Real seconds the world has run under the last.
    std::int64_t played = 0;
};

/// The chunk's tag.
inline constexpr std::uint32_t kVersionsTag = tag("VERS");

/// The rules for making worlds as a catalogue has them, and each source's version and rules digest.
[[nodiscard]] std::uint64_t making_digest(const data::Catalogue& c);
[[nodiscard]] std::uint64_t rules_digest(const data::Catalogue& c);

/// The chunk that holds them, which a reader that does not know it skips; and them from it, or nothing if it is
/// damaged or of a version this one cannot read.
[[nodiscard]] Chunk versions_chunk(const Versions& v);
[[nodiscard]] std::optional<Versions> read_versions(const Chunk& c);

}  // namespace kd::save
