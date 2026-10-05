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
#include <string_view>

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

}  // namespace kd::ecs
