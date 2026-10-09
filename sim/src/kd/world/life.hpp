// Saved, whole-unit camp needs, memories and action progress (BIO-09, MND-09, TIM-17).
#pragma once
#include <algorithm>
#include "kd/world/parts.hpp"

namespace kd::world {
enum class LivingAct : std::uint8_t { watch = 0, walk = 1, rest = 2, gather = 4, eat = 5, drink = 6, carry = 7 };
struct Life {
    static constexpr std::string_view name = "life";
    static constexpr std::uint32_t version = 1;
    std::int64_t food = 4000000, water = 3000, awake = 0, settled = 0;
    std::int64_t food_remainder = 0, water_remainder = 0, food_water_remainder = 0;
    std::int64_t carried_food = 0, allocated_water = 0, portion = 0, applied = 0;
    std::int64_t decision_at = 0, notice_at = -3600, memory_at = -1, memory_kind = 0, memory_amount = 0;
    num::Point explore_at{}, use_at{};
    std::uint8_t goal = 3;
    std::uint32_t gathering_skill = 3;
    std::array<std::int64_t, 3> known_amount{0, 0, 1}, seen{-1, -1, 0}, blocked_until{};
    std::array<std::uint8_t, 3> unavailable{};
    std::array<std::uint8_t, 3> source{0, 0, 2};  // none, sight, starting knowledge, own task
    std::array<num::Point, 3> known_at{};
    std::array<std::int64_t, 3> decision_needs{100, 100, 100};
    std::array<std::int64_t, 3> benefit{}, cost_seconds{};
    std::array<std::int64_t, 4> scores{-1000000, -1000000, -1000000, 0};
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"food", "remaining daily food units, mg"}, c.food);
        v.i64({"water", "remaining water units, ml"}, c.water);
        v.i64({"awake", "fatigue in awake seconds"}, c.awake);
        v.i64({"settled", "last bodily settlement second"}, c.settled);
        v.i64({"food_remainder", "food rate remainder"}, c.food_remainder);
        v.i64({"water_remainder", "water rate remainder"}, c.water_remainder);
        v.i64({"food_water_remainder", "berry water unit remainder"}, c.food_water_remainder);
        v.i64({"carried_food", "gathered berries still in hands, mg"}, c.carried_food);
        v.i64({"allocated_water", "water reserved at the drinking site, ml"}, c.allocated_water);
        v.i64({"portion", "action's original portion"}, c.portion);
        v.i64({"applied", "portion already settled"}, c.applied);
        v.i64({"decision_at", "second of actual choice"}, c.decision_at);
        v.i64({"notice_at", "last bounded sight survey"}, c.notice_at);
        v.i64({"memory_at", "last own consumption/rest memory"}, c.memory_at);
        v.i64({"memory_kind", "remembered activity"}, c.memory_kind);
        v.i64({"memory_amount", "remembered earned quantity"}, c.memory_amount);
        v.point({"explore_at", "current nearby ground to inspect"}, c.explore_at);
        v.point({"use_at", "chosen reachable interaction position"}, c.use_at);
        v.u8({"goal", "chosen need, or watching"}, c.goal);
        v.u32({"gathering_skill", "starting gathering skill"}, c.gathering_skill);
        for (std::size_t i = 0; i < 3; ++i) {
            v.i64({"known_amount", "last observed amount"}, c.known_amount[i]);
            v.i64({"seen", "observation second"}, c.seen[i]);
            v.i64({"blocked_until", "retry after failed route"}, c.blocked_until[i]);
            v.u8({"unavailable", "recorded exclusion reason: unknown, empty, blocked, unskilled or retry"},
                 c.unavailable[i]);
            v.u8({"source", "knowledge source"}, c.source[i]);
            v.point({"known_at", "remembered position"}, c.known_at[i]);
            v.i64({"benefit", "expected need benefit at decision"}, c.benefit[i]);
            v.i64({"cost_seconds", "expected travel and work time at decision"}, c.cost_seconds[i]);
            v.i64({"decision_needs", "need satisfaction at choice"}, c.decision_needs[i]);
        }
        for (auto& score : c.scores) v.i64({"scores", "recorded candidate score"}, score);
    }
};
struct Habitat {
    static constexpr std::string_view name = "habitat";
    static constexpr std::uint32_t version = 1;
    std::int64_t rock_west = 200, rock_east = 400, rock_south = -500, rock_north = 900;
    std::int64_t upstream_ml = 30000000, root_water_ml = 4000000, crop_budget_mg = 1500000000;
    std::int64_t water_cap_ml = 100000, food_cap_mg = 25000000;
    std::int64_t renewed_at = 0, water_added = 0, food_grown = 0, food_taken = 0, water_taken = 0, water_spilled = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"rock_west", "solid rock rectangle west offset, cm"}, c.rock_west);
        v.i64({"rock_east", "solid rock east offset, cm"}, c.rock_east);
        v.i64({"rock_south", "solid rock south offset, cm"}, c.rock_south);
        v.i64({"rock_north", "solid rock north offset, cm"}, c.rock_north);
        v.i64({"upstream_ml", "finite upstream spring input, ml"}, c.upstream_ml);
        v.i64({"root_water_ml", "finite growing input, ml"}, c.root_water_ml);
        v.i64({"crop_budget_mg", "remaining seasonal growth bound, mg"}, c.crop_budget_mg);
        v.i64({"water_cap_ml", "maximum standing pool, ml"}, c.water_cap_ml);
        v.i64({"food_cap_mg", "living stand's fruit capacity, mg; zero means absent"}, c.food_cap_mg);
        v.i64({"renewed_at", "last renewal second"}, c.renewed_at);
        v.i64({"water_added", "total upstream water transferred, ml"}, c.water_added);
        v.i64({"food_grown", "total new fruit, mg"}, c.food_grown);
        v.i64({"food_taken", "total fruit gathered, mg"}, c.food_taken);
        v.i64({"water_spilled", "unused reserved water overflowing pool, ml"}, c.water_spilled);
        v.i64({"water_taken", "total water drunk or reserved, ml"}, c.water_taken);
    }
};
// Exact inclusive segment/rectangle collision on centimetre coordinates.
inline bool camp_line_clear(num::Offset a, num::Offset b, const Habitat& rock) {
    const auto cross = [](num::Offset p, num::Offset q, num::Offset r) {
        return (q.dx - p.dx) * (r.dy - p.dy) - (q.dy - p.dy) * (r.dx - p.dx);
    };
    const auto intersects = [&](num::Offset p, num::Offset q, num::Offset r, num::Offset s) {
        if (std::max(p.dx, q.dx) < std::min(r.dx, s.dx) || std::max(r.dx, s.dx) < std::min(p.dx, q.dx) ||
            std::max(p.dy, q.dy) < std::min(r.dy, s.dy) || std::max(r.dy, s.dy) < std::min(p.dy, q.dy))
            return false;
        const auto a1 = cross(p, q, r), a2 = cross(p, q, s), b1 = cross(r, s, p), b2 = cross(r, s, q);
        return ((a1 <= 0 && a2 >= 0) || (a1 >= 0 && a2 <= 0)) && ((b1 <= 0 && b2 >= 0) || (b1 >= 0 && b2 <= 0));
    };
    const auto inside = [&](num::Offset p) {
        return p.dx >= rock.rock_west && p.dx <= rock.rock_east && p.dy >= rock.rock_south && p.dy <= rock.rock_north;
    };
    if (inside(a) || inside(b)) return false;
    const std::array<num::Offset, 4> corners{{{rock.rock_west, rock.rock_south},
                                              {rock.rock_east, rock.rock_south},
                                              {rock.rock_east, rock.rock_north},
                                              {rock.rock_west, rock.rock_north}}};
    for (std::size_t i = 0; i < 4; ++i)
        if (intersects(a, b, corners[i], corners[(i + 1) % 4])) return false;
    return true;
}
}  // namespace kd::world
