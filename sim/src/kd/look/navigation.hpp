// T2.9a.1: look-only navigation anchors, culling reach and release motion (PRE-03, PRE-28, PRE-33).
#pragma once
#include <cstdint>
#include <vector>
#include "kd/data/catalogue.hpp"
#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
namespace kd::look {
struct NavigationTuning {
    std::int64_t minimum_power = 0, maximum_power = 0, person_power = 0, close_camp_power = 0, camp_power = 0;
    std::int64_t valley_power = 0, region_power = 0, tiny_power = 0, group_power = 0;
    std::int64_t maximum_height = 0, overscan_pixels = 0, shadow_reach = 0, settle_ms = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.whole({"minimum_power", "lowest internal raster density power", Affects::look}, s.minimum_power, {-20, -8});
        v.whole({"maximum_power", "highest authored density power", Affects::look}, s.maximum_power, {6, 6});
        v.whole({"person_power", "person time anchor", Affects::look}, s.person_power, {6, 6});
        v.whole({"close_camp_power", "close camp time anchor", Affects::look}, s.close_camp_power, {-20, 6});
        v.whole({"camp_power", "camp time anchor", Affects::look}, s.camp_power, {-20, 6});
        v.whole({"valley_power", "valley time anchor", Affects::look}, s.valley_power, {-20, 6});
        v.whole({"region_power", "region time anchor", Affects::look}, s.region_power, {-20, 6});
        v.whole({"tiny_power", "tiny form threshold", Affects::look}, s.tiny_power, {-20, 0});
        v.whole({"group_power", "group form threshold", Affects::look}, s.group_power, {-20, 0});
        v.whole({"overscan_pixels", "physical window overscan", Affects::look}, s.overscan_pixels, {0, 512});
        v.whole({"settle_ms", "anchored gesture release duration", Affects::look}, s.settle_ms, {1, 1000});
        v.quantity({"maximum_height", "tallest culling body", Affects::look}, s.maximum_height, data::Measure::length,
                   {0, 1000000});
        v.quantity({"shadow_reach", "conservative directional shadow reach", Affects::look}, s.shadow_reach,
                   data::Measure::length, {0, 10000000});
    }
};
/// Implements PRE-03/PRE-28: ordered anchors and forms fit the configured navigation range.
void check_navigation(const data::Catalogue&, std::vector<data::Problem>&);
}  // namespace kd::look
