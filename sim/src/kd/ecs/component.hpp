// Components (A3.2): each is a plain struct with one descriptor, its stable name, its version and one visit() naming
// its fields in order. The digest walks it now; saves and the details view walk the same description later, so no
// component is ever described twice (PRN-14).
//
//     struct Place {
//         static constexpr std::string_view name = "place";
//         static constexpr std::uint32_t version = 1;
//         num::Point at;
//         template <typename V, typename Self>
//         static void visit(V& v, Self& c) { v.point({"at", "where it is"}, c.at); }
//     };
#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

#include "kd/core/bytes.hpp"
#include "kd/ecs/id.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/torus.hpp"

namespace kd::ecs {

/// A component's field as its descriptor names it.
struct Part {
    std::string_view key;
    /// What it means, in plain words, for the details view and whoever reads a save.
    std::string_view about;
};

/// Implements RES-05, see A3.4: a component's fields into the state's digest, each little-endian, in the
/// descriptor's order.
class PartDigest {
public:
    explicit PartDigest(num::Digest& d) : d_(d) {}

    void id(const Part& /*p*/, const Id& v) { d_.u64(v.value); }
    void entry(const Part& /*p*/, const std::uint32_t& v, std::string_view /*folder*/) { d_.u32(v); }
    void u8(const Part& /*p*/, const std::uint8_t& v) { d_.u8(v); }
    void u32(const Part& /*p*/, const std::uint32_t& v) { d_.u32(v); }
    void u64(const Part& /*p*/, const std::uint64_t& v) { d_.u64(v); }
    void i64(const Part& /*p*/, const std::int64_t& v) { d_.i64(v); }
    void point(const Part& /*p*/, const num::Point& v) {
        d_.u32(static_cast<std::uint32_t>(v.x));
        d_.u32(static_cast<std::uint32_t>(v.y));
    }

private:
    num::Digest& d_;
};

/// A component's digest, with its name and version first, so a component that changes shape changes the digest.
template <typename C>
void digest_component(const C& c, num::Digest& d) {
    d.text(C::name);
    d.u32(C::version);
    PartDigest w(d);
    C::visit(w, c);
}

/// Implements PLT-07, see A3.7: a component's fields into a snapshot, each little-endian, in the descriptor's order;
/// an entry of the catalogue by its number, which the snapshot's lists of names turn back into the entry (A3.6).
class PartWriter {
public:
    explicit PartWriter(ByteWriter& w) : w_(w) {}

    void id(const Part& /*p*/, const Id& v) { w_.u64(v.value); }
    void entry(const Part& /*p*/, const std::uint32_t& v, std::string_view /*folder*/) { w_.u32(v); }
    void u8(const Part& /*p*/, const std::uint8_t& v) { w_.u8(v); }
    void u32(const Part& /*p*/, const std::uint32_t& v) { w_.u32(v); }
    void u64(const Part& /*p*/, const std::uint64_t& v) { w_.u64(v); }
    void i64(const Part& /*p*/, const std::int64_t& v) { w_.i64(v); }
    void point(const Part& /*p*/, const num::Point& v) {
        w_.u32(static_cast<std::uint32_t>(v.x));
        w_.u32(static_cast<std::uint32_t>(v.y));
    }

private:
    ByteWriter& w_;
};

/// From a snapshot's number for an entry of a kind, by its folder, to the entry's number in the catalogue now; nothing
/// if the catalogue no longer has it.
using EntryMap = std::function<std::optional<std::uint32_t>(std::string_view folder, std::uint32_t saved)>;

/// Implements PLT-07, see A3.7: a component's fields from a snapshot, as PartWriter wrote them; any short read or
/// entry the catalogue no longer has leaves the reader failed.
class PartReader {
public:
    PartReader(ByteReader& r, const EntryMap& entries) : r_(r), entries_(entries) {}

    void id(const Part& /*p*/, Id& v) { r_.u64(v.value); }
    void entry(const Part& /*p*/, std::uint32_t& v, std::string_view folder) {
        std::uint32_t saved = 0;
        if (!r_.u32(saved)) {
            return;
        }
        const std::optional<std::uint32_t> now = entries_(folder, saved);
        ok_ = ok_ && now.has_value();
        v = now.value_or(0);
    }
    void u8(const Part& /*p*/, std::uint8_t& v) { r_.u8(v); }
    void u32(const Part& /*p*/, std::uint32_t& v) { r_.u32(v); }
    void u64(const Part& /*p*/, std::uint64_t& v) { r_.u64(v); }
    void i64(const Part& /*p*/, std::int64_t& v) { r_.i64(v); }
    void point(const Part& /*p*/, num::Point& v) {
        std::uint32_t x = 0;
        std::uint32_t y = 0;
        r_.u32(x);
        r_.u32(y);
        v = {static_cast<std::int32_t>(x), static_cast<std::int32_t>(y)};
    }

    /// Whether every field read, and every entry is still in the catalogue.
    [[nodiscard]] bool ok() const { return ok_ && !r_.failed(); }

private:
    ByteReader& r_;
    const EntryMap& entries_;
    bool ok_ = true;
};

/// A component into a snapshot, with its version first, so a reader knows which shape it has (A3.7).
template <typename C>
void write_component(const C& c, ByteWriter& w) {
    w.u32(C::version);
    PartWriter pw(w);
    C::visit(pw, c);
}

/// A component from a snapshot, as write_component() wrote it; false if it is short, of a newer version or of an older
/// one it cannot upgrade from, or names an entry the catalogue no longer has. A component whose shape changed reads its
/// older shapes itself (A3.7, PLT-09):
///
///     static bool upgrade(std::uint32_t version, ByteReader& r, const EntryMap& entries, Place& c);
template <typename C>
bool read_component(C& c, ByteReader& r, const EntryMap& entries) {
    std::uint32_t version = 0;
    if (!r.u32(version) || version == 0 || version > C::version) {
        return false;
    }
    if (version < C::version) {
        if constexpr (requires { C::upgrade(version, r, entries, c); }) {
            return C::upgrade(version, r, entries, c);
        } else {
            return false;
        }
    }
    PartReader pr(r, entries);
    C::visit(pr, c);
    return pr.ok();
}

}  // namespace kd::ecs
