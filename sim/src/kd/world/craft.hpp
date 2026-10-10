// Conserved item instances and pending work; BEIN/THNG keep their foundation layouts.
#pragma once
#include <array>
#include <limits>
#include <vector>
#include "kd/ecs/component.hpp"
namespace kd::world {
inline constexpr std::uint32_t kCraft = 1, kLearning = 2, kFire = 4, kIdeas = 8;
inline constexpr std::uint32_t kNoRecipe = std::numeric_limits<std::uint32_t>::max();
struct Link {
    static constexpr std::string_view name = "link";
    static constexpr std::uint32_t version = 1;
    ecs::Id id{};
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"id", "source entity"}, c.id);
    }
};
struct Item {
    static constexpr std::string_view name = "item";
    static constexpr std::uint32_t version = 1;
    std::uint32_t kind = 0, material = 0, changed_mask = 0;
    std::array<std::uint8_t, 18> changed{};
    ecs::Id owner{}, home{}, maker{};
    std::int64_t mass = 0, length = 0, wear = 0, wear_remainder = 0, made_at = -1;
    std::uint8_t state = 0, quality = 2;
    std::vector<Link> parents;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.entry({"kind", "item catalogue kind"}, c.kind, "item");
        v.entry({"material", "inherited source material"}, c.material, "item");
        v.u32({"changed_mask", "characteristics explicitly set by the actual result"}, c.changed_mask);
        for (auto& value : c.changed) v.u8({"changed", "result characteristic override"}, value);
        v.id({"owner", "holder, or zero for shared stock"}, c.owner);
        v.id({"home", "camp owning shared stock"}, c.home);
        v.id({"maker", "actual maker, or zero for raw stock"}, c.maker);
        v.i64({"mass", "remaining milligrams"}, c.mass);
        v.i64({"length", "millimetres"}, c.length);
        v.i64({"wear", "millionths of a wear step"}, c.wear);
        v.i64({"wear_remainder", "fractional wear numerator"}, c.wear_remainder);
        v.i64({"made_at", "creation second, minus one for raw inputs"}, c.made_at);
        v.u8({"state", "physical state; four is spent"}, c.state);
        v.u8({"quality", "source or maker quality, zero to five"}, c.quality);
        v.records({"parents", "actual input identities"}, c.parents, 8);
    }
};
struct Reservation {
    static constexpr std::string_view name = "reservation";
    static constexpr std::uint32_t version = 1;
    ecs::Id item{};
    std::int64_t mass = 0;
    std::uint8_t role = 0, retained = 0, picked = 0, return_shared = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"item", "reserved item"}, c.item);
        v.i64({"mass", "remaining reserved milligrams"}, c.mass);
        v.u8({"role", "input role index"}, c.role);
        v.u8({"retained", "tool kept after use"}, c.retained);
        v.u8({"picked", "input physically collected"}, c.picked);
        v.u8({"return_shared", "return initially shared tools after work"}, c.return_shared);
    }
};
struct Work {
    static constexpr std::string_view name = "work";
    static constexpr std::uint32_t version = 3;
    std::uint8_t state = 0, action = 0, intended = 0, route = 0, rolled = 0;
    std::uint32_t recipe = kNoRecipe;
    std::uint64_t number = 0, completed_tries = 0, applied_marker = 0;
    std::uint64_t lesson = 0, choice = 0;
    std::int64_t start = 0, end = 0, next_try = 0, retained_progress = 0;
    std::int64_t active_start = 0, try_seconds = 0, unit_mass = 0, goal_mass = 0;
    num::Point target{};
    std::vector<Reservation> inputs;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u8({"state", "idle, collecting, working, eating or paused"}, c.state);
        v.u8({"action", "base action"}, c.action);
        v.u8({"intended", "intentional recipe work, including supervised practice"}, c.intended);
        if (c.intended)
            v.entry({"recipe", "intended recipe, personally known or supplied by the teacher"}, c.recipe, "blueprint");
        else
            v.u32({"recipe", "no intended recipe sentinel"}, c.recipe);
        v.u8({"route", "known use, accident, experiment, hunch or taught practice"}, c.route);
        v.u8({"rolled", "unknown fits already settled"}, c.rolled);
        v.u64({"number", "person's never-reused work number"}, c.number);
        v.u64({"completed_tries", "completed known tries"}, c.completed_tries);
        v.u64({"applied_marker", "last applied try"}, c.applied_marker);
        v.u64({"lesson", "matching shared-practice session, or zero"}, c.lesson);
        v.u64({"choice", "immutable choice behind this plan"}, c.choice);
        v.i64({"start", "original work start"}, c.start);
        v.i64({"active_start", "start of current active interval"}, c.active_start);
        v.i64({"end", "current activity end"}, c.end);
        v.i64({"next_try", "next known try second"}, c.next_try);
        v.i64({"retained_progress", "earned seconds of unfinished gradual work"}, c.retained_progress);
        v.i64({"try_seconds", "current try duration"}, c.try_seconds);
        v.i64({"unit_mass", "worked milligrams per try"}, c.unit_mass);
        v.i64({"goal_mass", "original material goal"}, c.goal_mass);
        v.point({"target", "physical work position"}, c.target);
        v.records({"inputs", "reserved inputs and tools"}, c.inputs, 8);
    }
};
}  // namespace kd::world
