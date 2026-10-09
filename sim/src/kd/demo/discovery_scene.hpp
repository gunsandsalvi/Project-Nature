// Recorded finite inputs for the labelled Discovery camp; not world geography.
#pragma once
#include <vector>
#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
namespace kd::demo {
struct SceneStock {
    data::Ref kind;
    std::int64_t mass = 0, share = 0, aggregate = 0, site = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.link({"kind", "scene's initial material"}, c.kind, "item");
        v.quantity({"mass", "finite addition, milligrams"}, c.mass, data::Measure::mass, {0, 1000000000});
        v.whole({"share", "aggregate share in millionths"}, c.share, {0, 1000000});
        v.whole({"aggregate", "none, existing stone or existing wood"}, c.aggregate, {0, 2});
        v.whole({"site", "food, stone or wood site"}, c.site, {0, 2});
    }
};
struct DiscoveryScene {
    std::vector<SceneStock> stock;
    std::vector<std::string> syllables;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.records({"stock", "recorded scene inputs; no replenishment"}, c.stock);
        v.names({"syllables", "ordinary coined words"}, c.syllables);
    }
};
}  // namespace kd::demo
