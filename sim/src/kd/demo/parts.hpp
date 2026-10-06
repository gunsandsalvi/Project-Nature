// The demonstration's own components (MAT-16): a marker's home camp and its kind.
#pragma once

#include <cstdint>
#include <string_view>

#include "kd/ecs/id.hpp"
#include "kd/num/torus.hpp"

namespace kd::demo {

/// Implements MAT-16, see A3.2: the camp a marker belongs to, and where that camp is.
struct Home {
    static constexpr std::string_view name = "home";
    static constexpr std::uint32_t version = 1;
    ecs::Id camp;
    num::Point at;

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"camp", "the camp's id"}, c.camp);
        v.point({"at", "where the camp is"}, c.at);
    }
};

/// Implements MAT-16, see A3.2: a marker's kind, its entry's number among the catalogue's markers, which a save keeps
/// as the entry's name (A3.7).
struct MarkerKind {
    static constexpr std::string_view name = "marker";
    static constexpr std::uint32_t version = 1;
    std::uint32_t kind = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.entry({"kind", "its kind, an entry of the catalogue's markers"}, c.kind, "marker");
    }
};

}  // namespace kd::demo
