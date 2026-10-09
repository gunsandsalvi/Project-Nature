// Data-defined direct need affordances and renewal inputs; no blueprint names in choosing (A3.6, A11).
#pragma once
#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
#include "kd/time/duration.hpp"
namespace kd::demo {
struct LivingRules {
    std::int64_t food_day = 0, water_day = 0, speed = 0, loaded_speed = 0;
    std::int64_t spring_hour = 0, fruit_hour = 0, fruit_water = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        using data::Affects;
        using data::Measure;
        v.quantity({"food_day", "raw fruit daily adult food, mg", Affects::rules}, c.food_day, Measure::mass,
                   {1, 10000000});
        v.quantity({"water_day", "daily adult water, ml", Affects::rules}, c.water_day, Measure::volume, {1, 10000});
        v.quantity({"speed", "unloaded flat walking speed", Affects::rules}, c.speed, Measure::speed, {100, 3000});
        v.quantity({"loaded_speed", "walking with gathered fruit", Affects::rules}, c.loaded_speed, Measure::speed,
                   {100, 3000});
        v.quantity({"spring_hour", "hourly upstream transfer limit", Affects::rules}, c.spring_hour, Measure::volume,
                   {0, 100000});
        v.quantity({"fruit_hour", "hourly stand fruit growth bound", Affects::rules}, c.fruit_hour, Measure::mass,
                   {0, 10000000});
        v.quantity({"fruit_water", "root water used per kg new fruit", Affects::rules}, c.fruit_water, Measure::volume,
                   {1, 10000});
    }
};
struct NeedUse {
    std::int64_t need = 0, food_mg = 0, water_ml = 0, rest_seconds = 0, water_per_kg = 0, benefit = 0, area = 0;
    time::Duration use{}, gather{};
    [[nodiscard]] std::int64_t amount() const { return need == 0 ? food_mg : need == 1 ? water_ml : rest_seconds; }
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        using data::Affects;
        v.whole({"need", "0 food, 1 water, 2 rest", Affects::rules}, c.need, {0, 2});
        v.quantity({"food", "edible amount gathered and eaten", Affects::rules}, c.food_mg, data::Measure::mass,
                   {0, 4000000});
        v.quantity({"water", "amount reserved and drunk at the source", Affects::rules}, c.water_ml,
                   data::Measure::volume, {0, 3000});
        v.quantity({"restored", "awake seconds restored by the sleep", Affects::rules}, c.rest_seconds,
                   data::Measure::game_time, {0, 129600});
        v.quantity({"water_per_kg", "water in a kg of this edible supply", Affects::rules}, c.water_per_kg,
                   data::Measure::volume, {0, 3000});
        v.whole({"benefit", "expected satisfaction points", Affects::rules}, c.benefit, {1, 100});
        v.quantity({"area", "interaction area half width around the remembered supply", Affects::rules}, c.area,
                   data::Measure::length, {100, 5000});
        v.duration({"use", "direct body's use time", Affects::rules}, c.use);
        v.duration({"gather", "gather time, if needed", Affects::rules}, c.gather);
    }
};
}  // namespace kd::demo
