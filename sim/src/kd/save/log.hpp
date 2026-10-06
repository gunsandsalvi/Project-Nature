// Logs (A3.7): the command journal and the history, each a run of records framed by their length, type, sequence
// number and checksum, as LevelDB frames its log (research 18). Reading stops at the first record that is short,
// damaged or out of sequence, and the file is cut there, so a write the app was killed in the middle of is dropped
// whole.
#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "kd/save/files.hpp"

namespace kd::save {

/// One record of a log: what it is, its number in the log, and its body.
struct Entry {
    std::uint32_t type = 0;
    std::uint64_t sequence = 0;
    Bytes body;
};

/// The bytes before a record's body: its length, type, sequence number and checksum.
inline constexpr std::size_t kFrame = 24;

/// Implements PLT-07, see A3.7: a record framed, its checksum over the length, type, sequence number and body.
[[nodiscard]] Bytes frame(std::uint32_t type, std::uint64_t sequence, std::span<const std::byte> body);

/// What reading a log found: its good records, and how many bytes they take.
struct LogRead {
    std::vector<Entry> entries;
    std::uint64_t good = 0;
    /// Whether anything after the good records was refused, so the file must be cut at good.
    bool cut = false;
};

/// Implements PLT-07, see A3.7: a log's records in order, the first numbered first and each next one more, up to the
/// first that is short, damaged or out of sequence.
[[nodiscard]] LogRead read_log(std::span<const std::byte> bytes, std::uint64_t first);

}  // namespace kd::save
