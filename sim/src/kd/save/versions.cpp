#include "kd/save/versions.hpp"

#include "kd/core/bytes.hpp"
#include "kd/num/digest.hpp"

namespace kd::save {

std::uint64_t making_digest(const data::Catalogue& c) {
    num::Digest d;
    d.i64(data::kWorldMakingVersion);
    for (const data::Source& s : c.sources()) {
        d.text(s.id);
        d.u64(s.digests[1]);
    }
    return d.value();
}

std::uint64_t rules_digest(const data::Catalogue& c) {
    num::Digest d;
    for (const data::Source& s : c.sources()) {
        d.text(s.id);
        d.i64(s.version);
        d.u64(s.digests[0]);
    }
    return d.value();
}

Chunk versions_chunk(const Versions& v) {
    ByteWriter w;
    w.text(v.build);
    w.u64(v.making);
    w.u64(v.rules);
    w.u64(0);  // Reserved in this build's VERS1 layout; no conversions.
    w.u64(v.eras.size());
    for (const Versions::Era& e : v.eras) {
        w.text(e.build);
        w.i64(e.from);
    }
    w.i64(v.played);
    return {kVersionsTag, 1, false, w.take()};
}

std::optional<Versions> read_versions(const Chunk& c) {
    if (c.tag != kVersionsTag || c.version != 1) {
        return std::nullopt;
    }
    Versions v;
    ByteReader r(c.data);
    std::uint64_t n = 0;
    bool ok = r.text(v.build) && r.u64(v.making) && r.u64(v.rules) && r.u64(n) && n == 0;
    ok = ok && r.u64(n);
    for (std::uint64_t i = 0; ok && i < n; ++i) {
        Versions::Era& e = v.eras.emplace_back();
        ok = r.text(e.build) && r.i64(e.from);
    }
    if (!ok || !r.i64(v.played) || !r.finished()) {
        return std::nullopt;
    }
    return v;
}

}  // namespace kd::save
