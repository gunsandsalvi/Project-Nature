#include "kd/save/snapshot.hpp"

#include <zstd.h>

#include "kd/core/bytes.hpp"
#include "kd/core/check.hpp"
#include "kd/num/digest.hpp"

namespace kd::save {

namespace {

// The first and last bytes of every snapshot.
constexpr std::uint64_t kMagic = 0x50414e53444b4e49;  // "INKDSNAP", little-endian
constexpr std::uint64_t kEnd = 0x444e4550414e5344;    // "DSNAPEND"
// zstd's fastest level, about 430 MB/s on a phone (research 18).
constexpr int kLevel = 1;
// No chunk is larger, so a damaged length is refused before anything is made that large.
constexpr std::uint64_t kLargest = std::uint64_t{1} << 32U;

std::uint64_t hash_of(std::span<const std::byte> b) {
    num::Digest d;
    d.bytes(b);
    return d.value();
}

}  // namespace

Bytes write_snapshot(std::span<const Chunk> chunks) {
    ByteWriter w;
    w.u64(kMagic);
    w.u32(kSnapshotVersion);
    w.u32(static_cast<std::uint32_t>(chunks.size()));
    Bytes out = w.take();
    for (const Chunk& c : chunks) {
        Bytes packed(ZSTD_compressBound(c.data.size()));
        const std::size_t n = ZSTD_compress(packed.data(), packed.size(), c.data.data(), c.data.size(), kLevel);
        KD_CHECK(ZSTD_isError(n) == 0U, "save::write_snapshot: zstd could not compress a chunk");
        packed.resize(n);
        const std::uint64_t hash = hash_of(c.data);
        ByteWriter h;
        h.u32(c.tag);
        h.u32(c.version);
        h.u8(c.critical ? 1 : 0);
        h.u64(c.data.size());
        h.u64(hash);
        h.u64(packed.size());
        const Bytes head = h.take();
        out.insert(out.end(), head.begin(), head.end());
        out.insert(out.end(), packed.begin(), packed.end());
    }
    // the end holds a hash of every byte before it, so damage anywhere is refused, even where zstd would not notice
    ByteWriter t;
    t.u64(hash_of(out));
    t.u64(kEnd);
    const Bytes tail = t.take();
    out.insert(out.end(), tail.begin(), tail.end());
    return out;
}

std::optional<std::vector<Chunk>> read_snapshot(std::span<const std::byte> bytes, std::string& why) {
    ByteReader r(bytes);
    std::uint64_t magic = 0;
    std::uint32_t version = 0;
    std::uint32_t count = 0;
    if (!r.u64(magic) || magic != kMagic) {
        why = "it is not a snapshot";
        return std::nullopt;
    }
    if (!r.u32(version) || version != kSnapshotVersion) {
        why = "its format is version " + std::to_string(version) + ", not " + std::to_string(kSnapshotVersion);
        return std::nullopt;
    }
    if (!r.u32(count)) {
        why = "it is cut short in its header";
        return std::nullopt;
    }
    std::vector<Chunk> out;
    std::uint64_t at = 16;
    for (std::uint32_t i = 0; i < count; ++i) {
        ByteReader h(bytes.subspan(std::min<std::uint64_t>(at, bytes.size())));
        Chunk c;
        std::uint8_t critical = 0;
        std::uint64_t raw = 0;
        std::uint64_t hash = 0;
        std::uint64_t stored = 0;
        if (!h.u32(c.tag) || !h.u32(c.version) || !h.u8(critical) || !h.u64(raw) || !h.u64(hash) || !h.u64(stored)) {
            why = "it is cut short in chunk " + std::to_string(i + 1);
            return std::nullopt;
        }
        at += 4 + 4 + 1 + 8 + 8 + 8;
        if (critical > 1 || raw > kLargest || stored > bytes.size() - at) {
            why = "chunk " + std::to_string(i + 1) + " is damaged or cut short";
            return std::nullopt;
        }
        c.critical = critical == 1;
        c.data.resize(raw);
        const std::size_t n = ZSTD_decompress(c.data.data(), c.data.size(), bytes.data() + at, stored);
        if (ZSTD_isError(n) != 0U || n != raw || hash_of(c.data) != hash) {
            why = "chunk " + std::to_string(i + 1) + " is damaged";
            return std::nullopt;
        }
        at += stored;
        out.push_back(std::move(c));
    }
    ByteReader t(bytes.subspan(std::min<std::uint64_t>(at, bytes.size())));
    std::uint64_t sum = 0;
    std::uint64_t end = 0;
    if (!t.u64(sum) || !t.u64(end) || end != kEnd || !t.finished()) {
        why = "its end is missing or damaged";
        return std::nullopt;
    }
    if (sum != hash_of(bytes.first(at))) {
        why = "its bytes do not match its end";
        return std::nullopt;
    }
    return out;
}

}  // namespace kd::save
