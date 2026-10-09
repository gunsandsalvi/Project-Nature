#include "kd/save/archive.hpp"
#include "kd/save/snapshot.hpp"

#include <algorithm>
#include <utility>

#include "kd/core/bytes.hpp"

namespace kd::save {

namespace {

constexpr std::uint64_t kMagic = 0x444c574c444e494b;  // "KINDLWLD", little-endian
constexpr std::uint64_t kEnd = 0x444e45444c524f57;    // "WORLDEND"
constexpr std::uint32_t kVersion = kSnapshotVersion;
// No world's file is larger, so a damaged length is refused before anything that large is written.
constexpr std::uint64_t kLargest = std::uint64_t{1} << 34U;
constexpr std::uint64_t kLongestPath = 256;

bool digits(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; });
}

bool starts_ends(const std::string& path, const std::string& folder, const std::string& end) {
    return path.size() > folder.size() + end.size() && path.compare(0, folder.size(), folder) == 0 &&
           path.compare(path.size() - end.size(), end.size(), end) == 0 &&
           digits(path.substr(folder.size(), path.size() - folder.size() - end.size()));
}

bool ends_with(const std::string& s, const std::string& end) {
    return s.size() >= end.size() && s.compare(s.size() - end.size(), end.size(), end) == 0;
}

}  // namespace

bool archive_part(const std::string& path) {
    return path == "world.toml" || path == "journal.log" || starts_ends(path, "snapshots/", ".kds") ||
           starts_ends(path, "history/", ".log");
}

// --- writing

ArchiveWriter::ArchiveWriter(Files& folder) : folder_(folder) {
    // world.toml, the newest snapshot, the journal and every year of the history, in that order
    if (folder_.read("world.toml")) {
        paths_.emplace_back("world.toml");
    }
    std::vector<std::string> snapshots = folder_.list("snapshots");
    std::erase_if(snapshots, [](const std::string& n) { return !ends_with(n, ".kds"); });
    if (!snapshots.empty()) {
        paths_.push_back("snapshots/" + snapshots.back());
    }
    if (folder_.read("journal.log")) {
        paths_.emplace_back("journal.log");
    }
    for (const std::string& name : folder_.list("history")) {
        if (ends_with(name, ".log")) {
            paths_.push_back("history/" + name);
        }
    }
    ByteWriter w;
    w.u64(kMagic);
    w.u32(kVersion);
    w.u32(static_cast<std::uint32_t>(paths_.size()));
    pending_ = w.take();
    whole_.stream(pending_);
}

Bytes ArchiveWriter::next(std::size_t most) {
    while (pending_.size() - sent_ < most && !ended_) {
        // the next part whole, its hash before its bytes, or the end once every part is out
        Bytes more;
        if (part_ < paths_.size()) {
            const std::string& path = paths_[part_++];
            const Bytes body = folder_.read(path).value_or(Bytes{});
            num::Digest d;
            d.stream(body);
            ByteWriter w;
            w.text(path);
            w.u64(body.size());
            w.u64(d.value());
            more = w.take();
            more.insert(more.end(), body.begin(), body.end());
            whole_.stream(more);
        } else {
            ByteWriter w;
            w.u64(whole_.value());
            w.u64(kEnd);
            more = w.take();
            ended_ = true;
        }
        pending_.erase(pending_.begin(), pending_.begin() + static_cast<std::ptrdiff_t>(sent_));
        sent_ = 0;
        pending_.insert(pending_.end(), more.begin(), more.end());
    }
    const std::size_t n = std::min(most, pending_.size() - sent_);
    Bytes out(pending_.begin() + static_cast<std::ptrdiff_t>(sent_),
              pending_.begin() + static_cast<std::ptrdiff_t>(sent_ + n));
    sent_ += n;
    return out;
}

// --- reading

ArchiveReader::ArchiveReader(Files& folder) : folder_(folder) {}

bool ArchiveReader::fail(std::string why) {
    failed_ = true;
    why_ = std::move(why);
    return false;
}

bool ArchiveReader::feed(std::span<const std::byte> piece) {
    if (failed_) {
        return false;
    }
    if (ended_) {
        return fail("it goes on after its end");
    }
    buffer_.insert(buffer_.end(), piece.begin(), piece.end());
    return parse();
}

bool ArchiveReader::parse() {
    for (;;) {
        std::size_t used = 0;
        if (!header_) {
            if (buffer_.size() < 12) {
                return true;
            }
            ByteReader r{buffer_};
            std::uint64_t magic = 0;
            std::uint32_t version = 0;
            r.u64(magic);
            r.u32(version);
            if (magic != kMagic) {
                return fail("it is not a world's file");
            }
            if (version != kVersion) {
                return fail(version > 0 && version < kVersion
                                ? std::string(kOlderSave)
                                : "This save has an unsupported format. Start a new camp.");
            }
            if (buffer_.size() < 16) return true;
            r.u32(parts_);
            header_ = true;
            used = 16;
        } else if (body_left_ > 0) {
            // a part's bytes as they come, written and hashed
            const auto n = static_cast<std::size_t>(std::min<std::uint64_t>(body_left_, buffer_.size()));
            if (n == 0) {
                return true;
            }
            const std::span<const std::byte> bytes = std::span(buffer_).first(n);
            if (!folder_.append(path_, bytes)) {
                return fail("its part " + path_ + " cannot be written");
            }
            part_hash_.stream(bytes);
            body_left_ -= n;
            used = n;
            if (body_left_ == 0 && !close_part()) {
                return false;
            }
        } else if (done_ < parts_) {
            // a part's head: its path, length and hash
            ByteReader r(buffer_);
            std::uint64_t path_length = 0;
            if (!r.u64(path_length)) {
                return true;
            }
            if (path_length > kLongestPath) {
                return fail("part " + std::to_string(done_ + 1) + " is damaged");
            }
            if (buffer_.size() < 8 + path_length + 16) {
                return true;
            }
            std::string path;
            std::uint64_t length = 0;
            r = ByteReader(buffer_);
            r.text(path);
            r.u64(length);
            r.u64(expected_);
            if (!archive_part(path) || length > kLargest) {
                return fail("part " + std::to_string(done_ + 1) + " is damaged, or not a part of a world");
            }
            path_ = path;
            body_left_ = length;
            part_hash_.reset();
            used = 8 + path_length + 16;
            if (length == 0) {
                folder_.append(path_, {});
                whole_.stream(std::span(buffer_).first(used));
                buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(used));
                if (!close_part()) {
                    return false;
                }
                continue;
            }
        } else {
            // the end: a hash of every byte before it
            if (buffer_.size() < 16) {
                return true;
            }
            ByteReader r{std::span(buffer_).first(16)};
            std::uint64_t sum = 0;
            std::uint64_t end = 0;
            r.u64(sum);
            r.u64(end);
            if (end != kEnd || sum != whole_.value()) {
                return fail("its end does not match what came before it");
            }
            buffer_.erase(buffer_.begin(), buffer_.begin() + 16);
            ended_ = true;
            if (!buffer_.empty()) {
                return fail("it goes on after its end");
            }
            return true;
        }
        whole_.stream(std::span(buffer_).first(used));
        buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(used));
    }
}

bool ArchiveReader::close_part() {
    ++done_;
    if (part_hash_.value() != expected_) {
        return fail("its part " + path_ + " is damaged");
    }
    // each part made safe as it ends, so a world taken in is whole after a power cut
    if (!folder_.sync(path_)) {
        return fail("its part " + path_ + " cannot be kept");
    }
    return true;
}

bool ArchiveReader::finish() {
    if (failed_) {
        return false;
    }
    if (!ended_) {
        return fail(done_ < parts_ ? "it is cut short in part " + std::to_string(done_ + 1)
                                   : "it is cut short at its end");
    }
    return true;
}

}  // namespace kd::save
