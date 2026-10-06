// Snapshots (A3.7): a world's whole state in one file, a header, then chunks, each with a tag, a version, whether a
// reader must know it, its lengths and a hash, compressed by zstd at level 1 and hashed before compression, since
// zstd's output is the same only within one version; then an end with a hash of every byte before it.
// Reading verifies every hash, so a damaged snapshot is refused whole and never half-loaded.
#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "kd/save/files.hpp"

namespace kd::save {

/// A chunk's tag from its four letters, such as tag("QUEU").
[[nodiscard]] constexpr std::uint32_t tag(const char (&letters)[5]) {
    return static_cast<std::uint32_t>(static_cast<unsigned char>(letters[0])) |
           static_cast<std::uint32_t>(static_cast<unsigned char>(letters[1])) << 8U |
           static_cast<std::uint32_t>(static_cast<unsigned char>(letters[2])) << 16U |
           static_cast<std::uint32_t>(static_cast<unsigned char>(letters[3])) << 24U;
}

/// One part of a world's state as a snapshot keeps it.
struct Chunk {
    std::uint32_t tag = 0;
    std::uint32_t version = 1;
    /// Whether a reader that does not know it must refuse the snapshot, rather than skip it.
    bool critical = true;
    Bytes data;
};

/// The snapshot format's own version, in its header.
inline constexpr std::uint32_t kSnapshotVersion = 1;

/// Implements TIM-05 and PLT-07, see A3.7: the snapshot file of these chunks, each compressed.
[[nodiscard]] Bytes write_snapshot(std::span<const Chunk> chunks);

/// Implements PLT-07, see A3.7: a snapshot's chunks with every hash verified, or nothing, with why, if any part of it
/// is short, damaged or of an unknown format.
[[nodiscard]] std::optional<std::vector<Chunk>> read_snapshot(std::span<const std::byte> bytes, std::string& why);

/// A chunk among a snapshot's by its tag, or nothing.
[[nodiscard]] const Chunk* find_chunk(std::span<const Chunk> chunks, std::uint32_t tag);

/// One step of a chunk's upgrade (A3.7, PLT-09): its bytes as one version wrote them, rewritten as the next writes
/// them; false if they are not what that version wrote.
struct Upgrade {
    std::uint32_t tag = 0;
    std::uint32_t from = 0;
    bool (*apply)(Bytes& data) = nullptr;
};

/// Implements PLT-09, see A3.7: a chunk brought up to the version this one writes, a step at a time; false, with why,
/// if it is newer than that, or no step leads on from its version.
bool upgrade(Chunk& c, std::uint32_t now, std::span<const Upgrade> steps, std::string& why);

}  // namespace kd::save
