// Saved fire, physical exposure and felt comfort (MAT-18, MAT-19, BIO-11).
#pragma once
#include "kd/ecs/component.hpp"
#include "kd/world/craft.hpp"
#include "kd/world/parts.hpp"
namespace kd::world {
struct Fire {
    static constexpr std::string_view name = "fire";
    static constexpr std::uint32_t version = 1;
    ecs::Id hearth{}, owner{};
    num::Point at{};
    std::uint8_t heat = 0, ring = 1, unblown_checked = 1;
    std::int64_t fuel_mg = 0, ash_mg = 0, burn_remainder = 0, settled_at = 0;
    std::int64_t embers_until = 0, banked_until = 0, air_until = 0, next = 0;
    std::int64_t damp_remainder = 0, evaporated_mg = 0;
    [[nodiscard]] std::int64_t deadline() const {
        std::int64_t due = 0;
        const auto take = [&](std::int64_t at) {
            if (at > settled_at) due = due ? std::min(due, at) : at;
        };
        const auto burn = heat >= 3 ? 5000000 : heat == 2 ? 1000000 : 0;
        if (burn && fuel_mg) take(settled_at + (fuel_mg * time::kHour - burn_remainder + burn - 1) / burn);
        if (heat) take(air_until);
        if (heat == 1) take(std::max(embers_until, banked_until));
        return due;
    }
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"hearth", "camp containing the hearth"}, c.hearth);
        v.id({"owner", "carrier, or zero for an open hearth"}, c.owner);
        v.point({"at", "actual hearth position"}, c.at);
        v.u8({"heat", "physical heat level zero to five"}, c.heat);
        v.u8({"ring", "stone ring contains ground spread"}, c.ring);
        v.u8({"unblown_checked", "unblown ember death chance already settled"}, c.unblown_checked);
        v.i64({"fuel_mg", "unburned conserved fuel"}, c.fuel_mg);
        v.i64({"ash_mg", "burned material remaining as ash"}, c.ash_mg);
        v.i64({"burn_remainder", "hourly burn numerator remainder"}, c.burn_remainder);
        v.i64({"settled_at", "last fuel settlement"}, c.settled_at);
        v.i64({"embers_until", "ordinary embers expire at this second"}, c.embers_until);
        v.i64({"banked_until", "banked embers expire at this second"}, c.banked_until);
        v.i64({"air_until", "tending air or heating transition"}, c.air_until);
        v.i64({"next", "next physical transition or zero"}, c.next);
        v.i64({"damp_remainder", "wet mass toward the next damping kilogram"}, c.damp_remainder);
        v.i64({"evaporated_mg", "conserved wet input leaving as vapour"}, c.evaporated_mg);
    }
};
struct HeatTimer {
    static constexpr std::string_view name = "heat_timer";
    static constexpr std::uint32_t version = 2;
    ecs::Id item{}, maker{}, chance_source{};
    std::uint64_t placement_choice = 0;
    std::uint8_t exposure_heat = 0;
    std::vector<Link> notices;
    std::uint8_t target_state = 1, low = 2, high = 3, completed = 0, tried = 0, intended = 0;
    std::int64_t elapsed = 0, hot_elapsed = 0, settled_at = 0, next = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"chance_source", "original physical portion keeps its first chance across splitting"}, c.chance_source);
        v.u8({"exposure_heat", "actual heat during the retained interval"}, c.exposure_heat);
        v.records({"notices", "people whose actual sight already tried noticing"}, c.notices, 128);
        v.id({"item", "physical food identity"}, c.item);
        v.id({"maker", "actual placer, or zero for unmeant exposure"}, c.maker);
        v.u64({"placement_choice", "actual placing choice retained through thermal changes"}, c.placement_choice);
        v.u8({"intended", "person actually intended cooking"}, c.intended);
        v.u8({"target_state", "cooked state"}, c.target_state);
        v.u8({"low", "minimum cooking heat"}, c.low);
        v.u8({"high", "maximum cooking heat"}, c.high);
        v.u8({"completed", "burn exposure is complete"}, c.completed);
        v.u8({"tried", "first-hour maker chance already used"}, c.tried);
        v.i64({"elapsed", "retained total heat exposure seconds"}, c.elapsed);
        v.i64({"hot_elapsed", "retained heat-four exposure seconds"}, c.hot_elapsed);
        v.i64({"settled_at", "last exposure settlement"}, c.settled_at);
        v.i64({"next", "next exposure deadline or zero while away"}, c.next);
    }
};
struct Thermal {
    static constexpr std::string_view name = "thermal";
    static constexpr std::uint32_t version = 2;
    std::int64_t felt_milli_c = 24000, warmth = 100, settled_at = 0, warming_progress = 0;
    std::int64_t water_remainder = 0, water_used_ml = 0, water_due_ml = 0;
    ecs::Id warm_fire{};
    std::uint64_t warm_choice = 0, tending_choice = 0;
    num::Point warm_at{};
    std::int64_t warm_blocked_until = 0;
    std::uint8_t warm_phase = 0;
    ecs::Id tending_fire{}, tending_input{};
    std::int64_t tending_mass = 0, tending_started = 0;
    std::uint8_t tending = 0, tending_phase = 0, tending_shared = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"warm_fire", "remembered or visible warming source"}, c.warm_fire);
        v.u64({"warm_choice", "saved warming choice"}, c.warm_choice);
        v.u64({"tending_choice", "saved tending choice"}, c.tending_choice);
        v.point({"warm_at", "chosen experienced warming position"}, c.warm_at);
        v.i64({"warm_blocked_until", "cold arrival prevents repeated guesses"}, c.warm_blocked_until);
        v.u8({"warm_phase", "none, walking or warming"}, c.warm_phase);
        v.i64({"water_due_ml", "integrated extra depletion pending bodily settlement"}, c.water_due_ml);
        v.id({"tending_fire", "actual tending target"}, c.tending_fire);
        v.id({"tending_input", "reserved finite tending input"}, c.tending_input);
        v.i64({"tending_mass", "reserved tending mass"}, c.tending_mass);
        v.i64({"tending_started", "tending choice second"}, c.tending_started);
        v.u8({"tending", "none, feed, blow, bank or carry"}, c.tending);
        v.u8({"tending_shared", "reserved portion was shared"}, c.tending_shared);
        v.u8({"tending_phase", "none, walking to input, walking to hearth or working"}, c.tending_phase);
        v.i64({"felt_milli_c", "last experienced temperature"}, c.felt_milli_c);
        v.i64({"warmth", "felt comfort zero to one hundred"}, c.warmth);
        v.i64({"settled_at", "last thermal rate settlement"}, c.settled_at);
        v.i64({"warming_progress", "saved warmed seconds"}, c.warming_progress);
        v.i64({"water_remainder", "fractional extra water rate"}, c.water_remainder);
        v.i64({"water_used_ml", "total extra water used by heat"}, c.water_used_ml);
    }
};
struct Ambient {
    static constexpr std::string_view name = "ambient";
    static constexpr std::uint32_t version = 1;
    std::int64_t milli_c = 24000, next = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"milli_c", "mild camp ambient temperature"}, c.milli_c);
        v.i64({"next", "next day or night change"}, c.next);
    }
};
}  // namespace kd::world
