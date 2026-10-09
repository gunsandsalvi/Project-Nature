#include "kd/demo/learning.hpp"
#include <algorithm>
#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"
namespace kd::demo {
bool Learning::knows(const world::Knowledge& knowledge, std::uint32_t recipe) {
    return std::any_of(knowledge.skills.begin(), knowledge.skills.end(),
                       [&](const auto& s) { return s.recipe == recipe && s.known; });
}
std::int64_t Learning::taught_multiplier(const world::Practice& teacher) {
    KD_CHECK(teacher.level >= 0 && teacher.level <= 10000, "Teacher skill lies between zero and ten");
    return 4 * (1000000 + teacher.level * 100);
}
void Learning::fade(world::Practice& skill, time::Seconds now) {
    KD_CHECK(now >= 0 && skill.decay_at <= now && skill.level >= 0 && skill.best >= skill.level,
             "Fading uses an earlier personal practice anchor");
    if (skill.decay_level == 0 && skill.level != 0) {
        skill.decay_level = skill.level;
        skill.decay_at = skill.last_use >= 0 ? skill.last_use : 0;
    }
    const auto floor = (skill.best + 1) / 2;
    const auto span = std::max<std::int64_t>(0, skill.decay_level - floor);
    const auto elapsed = static_cast<double>(now - skill.decay_at) / static_cast<double>(5 * time::kYear);
    skill.level = floor + num::to_int(static_cast<double>(span) * num::exp2(-elapsed), num::Round::down);
}
void Learning::practice(world::Practice& skill, time::Seconds now, time::Seconds seconds, bool success,
                        std::int64_t learning_ppm, std::int64_t multiplier_ppm) {
    KD_CHECK(seconds >= 0 && seconds <= time::kYear && learning_ppm >= 0 && learning_ppm <= 10000000 &&
                 multiplier_ppm >= 0 && multiplier_ppm <= 8000000,
             "Practice uses bounded elapsed time and recorded multipliers");
    if (seconds == 0) return;
    fade(skill, now);
    using Wide = __int128;
    const auto numerator =
        static_cast<Wide>(seconds) * learning_ppm * multiplier_ppm * (success ? 2 : 1) + skill.scale_remainder;
    const auto effective = static_cast<std::int64_t>(numerator / 1000000);
    skill.scale_remainder = static_cast<std::int64_t>(numerator % 1000000);
    const auto accumulated = effective + skill.seconds_remainder;
    skill.seconds += accumulated / 1000000;
    skill.seconds_remainder = accumulated % 1000000;
    auto remaining = effective + skill.fraction;
    // 4,000 thousandths over 180 effective hours, then 5,000 over 780 hours.
    while (skill.level < 10000) {
        const auto unit = skill.level < 5000 ? 162000000 : 561600000;
        if (remaining < unit) break;
        remaining -= unit;
        ++skill.level;
    }
    skill.fraction = skill.level == 10000 ? 0 : remaining;
    skill.best = std::max(skill.best, skill.level);
    skill.last_use = now;
    skill.decay_at = now;
    skill.decay_level = skill.level;
}
}  // namespace kd::demo
