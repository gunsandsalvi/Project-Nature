// Camp alpha's saved records (BIO-03, MAT-01, MAT-02, RES-21). Initial facts, not generated geography.
#pragma once

#include "kd/world/parts.hpp"

namespace kd::world {

struct Person {
    static constexpr std::string_view name = "person";
    static constexpr std::uint32_t version = 1;
    std::uint32_t name_index = 0;
    std::uint32_t age_years = 25;
    std::uint32_t appearance = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u32({"name_index", "index in the camp's name list"}, c.name_index);
        v.u32({"age_years", "age in years at the scene's start"}, c.age_years);
        v.u32({"appearance", "stable stand-in appearance"}, c.appearance);
    }
};

struct Camp {
    static constexpr std::string_view name = "camp";
    static constexpr std::uint32_t version = 1;
    std::int64_t half_width_cm = 1500;
    std::int64_t half_height_cm = 1800;
    std::int64_t water_ml = 100000;
    std::int64_t food_mg = 25000000;
    std::int64_t stone_mg = 80000000;
    std::int64_t wood_mg = 60000000;
    num::Point water_at, food_at, stone_at, wood_at, shelter_at;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"half_width_cm", "traversable patch half width, cm"}, c.half_width_cm);
        v.i64({"half_height_cm", "traversable patch half height, cm"}, c.half_height_cm);
        v.i64({"water_ml", "initial drinking water, ml"}, c.water_ml);
        v.i64({"food_mg", "initial food plants, mg"}, c.food_mg);
        v.i64({"stone_mg", "initial stone, mg"}, c.stone_mg);
        v.i64({"wood_mg", "initial wood, mg"}, c.wood_mg);
        v.point({"water_at", "water source position, cm"}, c.water_at);
        v.point({"food_at", "food plants position, cm"}, c.food_at);
        v.point({"stone_at", "stone position, cm"}, c.stone_at);
        v.point({"wood_at", "fallen wood position, cm"}, c.wood_at);
        v.point({"shelter_at", "natural shelter position, cm"}, c.shelter_at);
    }
};

inline constexpr std::array<std::string_view, 25> kPersonNames{
    "Ari",  "Bela", "Cora", "Daro", "Ena",  "Fenn", "Gara", "Hali", "Ira",  "Jori", "Kara", "Lio", "Mara",
    "Neri", "Ona",  "Pavi", "Rena", "Sami", "Tala", "Uli",  "Vara", "Weni", "Yara", "Zori", "Ada"};

}  // namespace kd::world
