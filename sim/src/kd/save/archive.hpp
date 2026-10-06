// A world in one file (A3.7, PLT-08): the .kindling file holds a world's folder, world.toml, its newest snapshot, its
// journal and its history, each part with its path, length and hash, and an end with a hash of every byte before it.
// It is written and read in pieces, so a world of any size passes through Android's file picker without being held
// whole, and an import is checked part by part as it arrives, then refused whole if anything is wrong.
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "kd/num/digest.hpp"
#include "kd/save/files.hpp"

namespace kd::save {

/// Whether a path is one a world's folder holds: world.toml, the journal, a snapshot or a year of history.
[[nodiscard]] bool archive_part(const std::string& path);

/// Implements PLT-08, see A3.7: a world's folder written out as a .kindling file, piece by piece; the world should be
/// saved and still while it is written.
class ArchiveWriter {
public:
    explicit ArchiveWriter(Files& folder);

    /// The next piece of the file, at most this many bytes, and at least one until the file is whole; empty after.
    [[nodiscard]] Bytes next(std::size_t most);
    /// The parts it holds, in order.
    [[nodiscard]] const std::vector<std::string>& parts() const { return paths_; }

private:
    Files& folder_;
    std::vector<std::string> paths_;
    std::size_t part_ = 0;
    num::Digest whole_;
    // what waits to go out, and how much of it has
    Bytes pending_;
    std::size_t sent_ = 0;
    bool ended_ = false;
};

/// Implements PLT-08, see A3.7: a .kindling file read piece by piece into an empty folder, each part checked as it
/// arrives; anything wrong stops it with words naming the damage, and the folder is the caller's to remove.
class ArchiveReader {
public:
    explicit ArchiveReader(Files& folder);

    /// The next piece of the file; false once anything is wrong, with why().
    bool feed(std::span<const std::byte> piece);
    /// After the last piece: whether the file was whole and every part right.
    bool finish();
    /// What was wrong, in words for the screen.
    [[nodiscard]] const std::string& why() const { return why_; }
    /// The parts read so far.
    [[nodiscard]] std::uint32_t parts() const { return done_; }

private:
    bool fail(std::string why);
    bool parse();
    bool close_part();

    num::Digest whole_;
    num::Digest part_hash_;
    Files& folder_;
    std::uint64_t expected_ = 0;
    std::uint64_t body_left_ = 0;
    Bytes buffer_;
    std::string path_;
    std::string why_;
    std::uint32_t parts_ = 0;
    std::uint32_t done_ = 0;
    bool header_ = false;
    bool ended_ = false;
    bool failed_ = false;
};

}  // namespace kd::save
