// PLT-10: exact daily counts of actual routine work and intake. These are history
// outputs, never a person's evidence or an input to choices/physical chances.
#pragma once
#include <tuple>
#include "kd/core/pages.hpp"
#include "kd/world/craft.hpp"
namespace kd::world {
struct RoutineDay {
    static constexpr std::string_view name = "routine-day";
    static constexpr std::uint32_t version = 1;
    std::uint64_t id = 0;
    std::int64_t day = 0;
    ecs::Id actor{};
    std::uint8_t type = 0;  // zero: resolved craft, one: finite food, two: gathered food
    std::uint32_t recipe = 0, input_kind = 0, result_kind = 0;
    std::uint64_t tries = 0, successes = 0;
    std::int64_t input_mg = 0, result_mg = 0, eaten_mg = 0;
    [[nodiscard]] auto key() const { return std::tuple{actor, type, recipe, input_kind, result_kind}; }
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"id", "stable daily summary identity, zero while current"}, c.id);
        v.i64({"day", "actual game-day midnight"}, c.day);
        v.id({"actor", "person who actually did the work"}, c.actor);
        v.u8({"type", "resolved crafting, finite food or gathered food"}, c.type);
        if (c.type == 0)
            v.entry({"recipe", "actual resolved recipe"}, c.recipe, "blueprint");
        else
            v.u32({"recipe", "unused for eating"}, c.recipe);
        if (c.type == 2) {
            v.u32({"input_kind", "unused for aggregate gathered food"}, c.input_kind);
            v.u32({"result_kind", "unused for aggregate gathered food"}, c.result_kind);
        } else {
            v.entry({"input_kind", "actual worked material or eaten kind"}, c.input_kind, "item");
            v.entry({"result_kind", "actual result or eaten kind"}, c.result_kind, "item");
        }
        v.u64({"tries", "actual resolved tries"}, c.tries);
        v.u64({"successes", "actual successes"}, c.successes);
        v.i64({"input_mg", "actually processed milligrams"}, c.input_mg);
        v.i64({"result_mg", "created result milligrams"}, c.result_mg);
        v.i64({"eaten_mg", "actually eaten milligrams"}, c.eaten_mg);
    }
};
struct RoutineHistory {
    std::uint64_t next = 1;
    std::int64_t day = -1;
    Pages<RoutineDay> days;
    std::vector<RoutineDay> current;
    void record(RoutineDay row) {
        KD_CHECK(day <= row.day, "routine history follows actual event time");
        if (day != row.day) {
            for (auto& old : current) {
                old.id = next++;
                days.push_back(old);
            }
            current.clear();
            day = row.day;
        }
        const auto at = std::lower_bound(current.begin(), current.end(), row.key(),
                                         [](const auto& old, const auto& key) { return old.key() < key; });
        if (at == current.end() || at->key() != row.key()) {
            current.insert(at, row);
        } else {
            at->tries += row.tries;
            at->successes += row.successes;
            at->input_mg += row.input_mg;
            at->result_mg += row.result_mg;
            at->eaten_mg += row.eaten_mg;
        }
    }
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"next_routine", "next sealed daily summary identity"}, c.next);
        v.i64({"routine_day", "day of current counters, minus one before any"}, c.day);
        v.records({"routine_days", "sealed actual daily summaries"}, c.days, UINT32_MAX, 77);
        v.records({"routine_current", "current actual daily counters"}, c.current, 4096, 77);
    }
};
}  // namespace kd::world
