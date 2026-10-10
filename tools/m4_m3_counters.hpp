// Side-channel instrumentation for the archived M3 diagnosis only. Never linked into the app.
#pragma once
#include <array>
#include <chrono>
#include <cstdint>
namespace kd::diagnostic {
enum Counter : std::size_t {
    decisions,
    urgent,
    comfortable,
    craft_calls,
    curious_hours,
    curious_candidates,
    empty_materials,
    known_candidates,
    selected_experiments,
    fire_calls,
    visible_embers,
    fuel_candidates,
    fire_preempted,
    teacher_calls,
    teacher_comfortable,
    input_visits,
    spent_input_visits,
    food_visits,
    count
};
inline constexpr std::array<const char*, count> names{"decisions",
                                                      "urgent_decisions",
                                                      "comfortable_decisions",
                                                      "craft_calls",
                                                      "curious_hours",
                                                      "curious_candidates",
                                                      "empty_materials",
                                                      "known_candidates",
                                                      "selected_experiments",
                                                      "fire_calls",
                                                      "visible_embers",
                                                      "fuel_candidates",
                                                      "fire_preempted",
                                                      "teacher_calls",
                                                      "teacher_comfortable",
                                                      "reachable_item_visits",
                                                      "reachable_spent_visits",
                                                      "thermal_food_visits"};
inline std::array<std::uint64_t, count> counts{};
enum Cpu : std::size_t { choosing, reachable, fire_deadlines, food_refresh, cooking_fit, cpu_count };
inline constexpr std::array<const char*, cpu_count> cpu_names{
    "choosing_inclusive", "reachable_inputs", "fire_deadlines", "food_refresh_inclusive", "cooking_fit"};
inline std::array<std::int64_t, cpu_count> nanoseconds{};
inline void hit(Counter counter) {
    ++counts[counter];
}
struct Time {
    Cpu part;
    std::chrono::steady_clock::time_point start;
    explicit Time(Cpu p) : part(p), start(std::chrono::steady_clock::now()) {}
    ~Time() { nanoseconds[part] += (std::chrono::steady_clock::now() - start).count(); }
};
}  // namespace kd::diagnostic
