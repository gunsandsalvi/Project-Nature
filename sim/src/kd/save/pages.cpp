#include "kd/save/pages.hpp"
#include <algorithm>
#include "kd/core/bytes.hpp"
#include "kd/num/digest.hpp"

namespace kd::save {
namespace {
constexpr auto kManifest = tag("PMAN");
struct Reference {
    std::uint64_t at = 0, hash = 0, size = 0;
    std::uint32_t tag = 0, version = 0;
    std::uint8_t critical = 0;
};
std::uint64_t checksum(std::span<const std::byte> bytes) {
    num::Digest d;
    d.bytes(bytes);
    return d.value();
}
std::string path(std::uint64_t hash) {
    return "pages/" + num::to_hex(hash) + ".kdp";
}
bool read_manifest(std::span<const Chunk> chunks, std::uint64_t& total, std::vector<Reference>& references) {
    const auto* manifest = find_chunk(chunks, kManifest);
    if (!manifest) {
        total = chunks.size();
        return true;
    }
    if (manifest->version != 1 || !manifest->critical ||
        std::count_if(chunks.begin(), chunks.end(), [](const auto& c) { return c.tag == kManifest; }) != 1)
        return false;
    ByteReader r(manifest->bytes());
    std::uint64_t count = 0;
    if (!r.u64(total) || !r.u64(count) || count > r.remaining() / 33 || total != chunks.size() - 1 + count)
        return false;
    for (std::uint64_t n = 0; n < count; ++n) {
        Reference ref;
        if (!r.u64(ref.at) || !r.u32(ref.tag) || !r.u32(ref.version) || !r.u8(ref.critical) || !r.u64(ref.hash) ||
            !r.u64(ref.size) || ref.at >= total || (!references.empty() && ref.at <= references.back().at) ||
            ref.version != 1 || ref.critical != 1 ||
            (ref.tag != tag("ARPG") && ref.tag != tag("EVPG") && ref.tag != tag("CHPG")) ||
            ref.size > (std::uint64_t{1} << 27U))
            return false;
        references.push_back(ref);
    }
    return r.finished();
}
}  // namespace

std::optional<std::vector<std::string>> page_paths(std::span<const Chunk> chunks) {
    std::uint64_t total = 0;
    std::vector<Reference> references;
    if (!read_manifest(chunks, total, references)) return {};
    std::vector<std::string> paths;
    for (const auto& ref : references) paths.push_back(path(ref.hash));
    return paths;
}

bool publish_pages(Files& files, std::vector<Chunk>& chunks) {
    ByteWriter manifest;
    const auto count = std::count_if(chunks.begin(), chunks.end(), [](const auto& c) { return bool(c.immutable); });
    if (count == 0) return true;
    manifest.u64(chunks.size());
    manifest.u64(static_cast<std::uint64_t>(count));
    std::vector<Chunk> live;
    for (std::size_t n = 0; n < chunks.size(); ++n) {
        auto& chunk = chunks[n];
        if (!chunk.immutable) {
            live.push_back(std::move(chunk));
            continue;
        }
        const auto hash = checksum(chunk.bytes());
        const auto name = path(hash);
        const auto existing = files.read(name);
        std::string why;
        const auto decoded = existing ? read_snapshot(*existing, why) : std::nullopt;
        if (!decoded || decoded->size() != 1 || decoded->front().tag != chunk.tag ||
            decoded->front().version != chunk.version || decoded->front().critical != chunk.critical ||
            checksum(decoded->front().bytes()) != hash) {
            if (!files.write_whole(name, write_snapshot(std::span(&chunk, 1)))) return false;
        }
        manifest.u64(n);
        manifest.u32(chunk.tag);
        manifest.u32(chunk.version);
        manifest.u8(chunk.critical ? 1 : 0);
        manifest.u64(hash);
        manifest.u64(chunk.bytes().size());
    }
    live.push_back({kManifest, 1, true, manifest.take()});
    chunks = std::move(live);
    return true;
}

bool resolve_pages(Files& files, std::vector<Chunk>& chunks, std::string& why) {
    std::uint64_t total = 0;
    std::vector<Reference> references;
    if (!read_manifest(chunks, total, references)) {
        why = "damaged page manifest";
        return false;
    }
    if (!find_chunk(chunks, kManifest)) return true;
    std::vector<Chunk> resolved;
    std::size_t live = 0, page = 0;
    for (std::uint64_t n = 0; n < total; ++n) {
        if (page < references.size() && references[page].at == n) {
            const auto& ref = references[page++];
            const auto bytes = files.read(path(ref.hash));
            auto decoded = bytes ? read_snapshot(*bytes, why) : std::nullopt;
            if (!decoded || decoded->size() != 1 || decoded->front().tag != ref.tag ||
                decoded->front().version != ref.version || !decoded->front().critical ||
                decoded->front().bytes().size() != ref.size || checksum(decoded->front().bytes()) != ref.hash) {
                why = "missing or damaged immutable page";
                return false;
            }
            resolved.push_back(std::move(decoded->front()));
        } else {
            while (live < chunks.size() && chunks[live].tag == kManifest) ++live;
            if (live == chunks.size()) {
                why = "incomplete page manifest";
                return false;
            }
            resolved.push_back(std::move(chunks[live++]));
        }
    }
    chunks = std::move(resolved);
    return true;
}
}  // namespace kd::save
