// MND-11 MND-23 RES-16: no quota, chance adjustment or knowledge installation.
#include "kd/demo/motivation.hpp"
#include "kd/data/craft.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/living.hpp"
namespace kd::demo {
namespace {
constexpr auto kUnit = world::Motivation::kUnit;
std::int64_t interval(time::Seconds now) {
    return (now + 18 * time::kHour) / time::kDay;
}
std::int64_t normalise(std::uint64_t amount, std::int64_t count, std::int64_t& remainder, std::int64_t previous) {
    // Carry fractional ratio units when the opportunity denominator changes.
    using Wide = __int128;
    const auto carried = previous ? static_cast<std::int64_t>(Wide(remainder) * count / previous) : 0;
    const auto numerator = std::min<std::uint64_t>(amount, static_cast<std::uint64_t>(count)) * kUnit + carried;
    remainder = static_cast<std::int64_t>(numerator % count);
    return std::min<std::int64_t>(kUnit, static_cast<std::int64_t>(numerator / count));
}
std::uint64_t evidence(const world::World& w, world::Beings::Handle h, const world::CraftReason& reason) {
    const auto& mind = w.beings().raw().get<world::Knowledge>(h);
    num::Digest digest;
    digest.u8(reason.kind);
    digest.u8(reason.action);
    for (const auto& link : reason.inputs) {
        const auto handle = w.things().find(link.id);
        if (!handle) continue;
        auto item = w.things().raw().get<world::Item>(*handle);
        // A consumed input's remembered material does not acquire a new identity or estimate.
        if (item.state == 4) item.state = 0;
        const auto* f = Discovery::familiar(mind, item);
        if (!f) continue;
        digest.u32(f->kind);
        digest.u32(f->material);
        digest.u8(f->state);
        digest.u32(f->mask);
        digest.u8(f->edible);
        for (std::size_t n = 0; n < f->values.size(); ++n) {
            digest.u8(f->values[n]);
            digest.u8(f->certainty[n]);
        }
    }
    return digest.value();
}
std::int64_t progress(const world::World& w, world::Beings::Handle h, const world::CraftReason& reason) {
    // Match FireRules' generic maintenance actions, not its private operation numbers.
    const auto action = reason.kind == 3 ? (reason.action == 3 ? 16 : 0) : reason.action;
    std::int64_t value = 0;
    for (const auto& skill : w.beings().raw().get<world::Knowledge>(h).skills)
        if (skill.known && w.catalogue().kind<data::Blueprint>()[skill.recipe].action == action)
            value += skill.practice.best;
    return value;
}
int channel(const world::CraftReason& reason) {
    if (reason.kind == 5 && reason.confidence) return 1;
    if ((reason.kind == 1 && !reason.intended) || (reason.kind == 3 && reason.confidence < 100)) return 0;
    return -1;
}
}  // namespace
world::MotivationDay& Motivation::day(world::Motivation& state, time::Seconds now) {
    const auto index = interval(now);
    auto& row = state.window[static_cast<std::size_t>(index % 7)];
    if (row.day != index) {
        row = {};
        row.day = index;
    }
    return row;
}
void Motivation::dawn(world::Motivation& state, time::Seconds now) {
    if (now % time::kDay != 6 * time::kHour || state.dawn >= now) return;
    const auto completed = interval(now) - 1;
    const auto& today = state.window[static_cast<std::size_t>(completed % 7)];
    for (std::size_t n = 0; n < 2; ++n) {
        if (today.day != completed || !today.opportunities[n]) continue;
        std::uint64_t opportunities = 0, deficits = 0, relief = 0;
        for (const auto& row : state.window)
            if (row.day <= completed && row.day > completed - 7) {
                opportunities += row.opportunities[n];
                deficits += row.deficits[n];
                relief += row.relief[n];
            }
        const auto denominator = static_cast<std::int64_t>(opportunities * 100);
        const auto d = normalise(deficits, denominator, state.deficit_remainder[n], state.denominator[n]);
        const auto r = normalise(relief, denominator, state.relief_remainder[n], state.denominator[n]);
        state.denominator[n] = denominator;
        const auto difference = d - r + state.update_remainder[n];
        const auto delta = difference / 16;
        state.update_remainder[n] = difference % 16;
        state.pressure[n] = std::clamp(state.pressure[n] + delta, std::int64_t{0}, kUnit);
        if ((state.pressure[n] == 0 && difference < 0) || (state.pressure[n] == kUnit && difference > 0))
            state.update_remainder[n] = 0;
    }
    state.dawn = now;
}
bool Motivation::valid(const world::Motivation& state, time::Seconds now) {
    if (state.dawn < -1 || state.dawn > now || (state.dawn >= 0 && state.dawn % time::kDay != 6 * time::kHour) ||
        state.last_satisfaction < -1 || state.last_satisfaction > 100)
        return false;
    for (std::size_t n = 0; n < 2; ++n) {
        if (state.pressure[n] < 0 || state.pressure[n] > kUnit || state.update_remainder[n] <= -16 ||
            state.update_remainder[n] >= 16 || state.denominator[n] < 0 || state.denominator[n] > 700000000 ||
            state.deficit_remainder[n] < 0 || state.relief_remainder[n] < 0 ||
            state.deficit_remainder[n] >= std::max<std::int64_t>(1, state.denominator[n]) ||
            state.relief_remainder[n] >= std::max<std::int64_t>(1, state.denominator[n]))
            return false;
    }
    for (std::size_t i = 0; i < state.window.size(); ++i) {
        const auto& row = state.window[i];
        if (row.day < -1 || row.day > interval(now) || (row.day >= 0 && row.day % 7 != static_cast<std::int64_t>(i)))
            return false;
        for (std::size_t n = 0; n < 2; ++n)
            if (row.opportunities[n] > 1000000 || row.deficits[n] > row.opportunities[n] * 100 ||
                row.relief[n] > 100000000 || row.requests[n] > 1000000 || row.progress[n] > 1000000 ||
                (row.day == -1 &&
                 (row.opportunities[n] || row.deficits[n] || row.relief[n] || row.requests[n] || row.progress[n])))
                return false;
    }
    for (const auto& action : state.actions)
        if (action.failures > 15 || action.progress < 0 || action.progress > 1280000) return false;
    for (const auto& action : state.fire_actions)
        if (action.failures > 15 || action.progress < 0 || action.progress > 1280000) return false;
    return true;
}
void Motivation::value(const world::Motivation& state, world::CraftReason& reason, std::uint8_t failures,
                       bool enabled) {
    const auto which = channel(reason);
    if (which < 0 || reason.unavailable || reason.parts[1] == 90000) return;
    const auto n = static_cast<std::size_t>(which);
    reason.motivation = static_cast<std::uint8_t>(n + 1);
    reason.motivation_pressure = state.pressure[n];
    const auto factor = 1000000 / (1 + std::min<int>(15, failures));
    reason.learning_progress_ppm = static_cast<std::uint32_t>(factor);
    if (!enabled) return;
    const auto learning =
        std::max<std::int64_t>(0, reason.parts[1]) +
        ((reason.need == 3 || (reason.kind == 1 && !reason.intended)) ? std::max<std::int64_t>(0, reason.parts[0]) : 0);
    const auto valued = learning * (kUnit + state.pressure[n]) / kUnit * factor / 1000000;
    reason.parts[1] += valued - learning;
    reason.score = reason.parts[0] + reason.parts[1] + reason.parts[2];
}
void Motivation::choose(world::Context& c, world::Beings::Handle h, std::vector<world::CraftReason>& reasons) {
    auto& w = c.world();
    auto& mind = w.beings().raw().get<world::Knowledge>(h);
    auto& state = mind.motivation;
    auto& row = day(state, c.now());
    const auto needs = Living::needs(w.beings().raw().get<world::Life>(h));
    const auto satisfaction =
        std::min(mind.curiosity_need, static_cast<std::uint8_t>(*std::min_element(needs.begin(), needs.end())));
    std::array<bool, 2> eligible{};
    for (auto& reason : reasons) {
        const auto which = channel(reason);
        if (which < 0 || reason.unavailable || reason.parts[1] == 90000) continue;
        const auto n = static_cast<std::size_t>(which);
        eligible[n] = true;
        reason.motivation = static_cast<std::uint8_t>(n + 1);
        reason.motivation_pressure = state.pressure[n];
        std::uint64_t opportunities = 0, deficits = 0, relief = 0;
        const auto today = interval(c.now());
        for (const auto& remembered : state.window)
            if (remembered.day >= today - 6 && remembered.day <= today) {
                opportunities += remembered.opportunities[n];
                deficits += remembered.deficits[n];
                relief += remembered.relief[n];
            }
        reason.motivation_opportunities = opportunities;
        reason.motivation_deficit =
            opportunities ? static_cast<std::uint8_t>(std::min<std::uint64_t>(100, deficits / opportunities)) : 0;
        reason.motivation_relief =
            opportunities ? static_cast<std::uint8_t>(std::min<std::uint64_t>(100, relief / opportunities)) : 0;
        std::uint8_t failures = 0;
        if (n == 0) {
            const auto& action = feedback(state, reason);
            if (action.evidence == evidence(w, h, reason) && action.progress >= progress(w, h, reason))
                failures = action.failures;
        }
        value(state, reason, failures, w.motivation_enabled());
    }
    for (std::size_t n = 0; n < 2; ++n)
        if (eligible[n]) {
            ++row.opportunities[n];
            row.deficits[n] +=
                n == 1 ? 100
                       : static_cast<std::uint64_t>(std::clamp<std::int64_t>((80 - satisfaction) * 100 / 80, 0, 100));
            if (n == 0 && state.last_satisfaction >= 0 && satisfaction > state.last_satisfaction)
                row.relief[n] += satisfaction - state.last_satisfaction;
        }
    state.last_satisfaction = satisfaction;
}
world::MotivationAction& Motivation::feedback(world::Motivation& state, const world::CraftReason& reason) {
    if (reason.kind == 3) {
        KD_CHECK(reason.action >= 1 && reason.action <= state.fire_actions.size(), "Valid actual fire operation");
        return state.fire_actions[reason.action - 1];
    }
    KD_CHECK(reason.action < state.actions.size(), "Valid generic action");
    return state.actions[reason.action];
}
void Motivation::trial(world::Context& c, world::Beings::Handle h, const world::CraftReason& reason) {
    auto& w = c.world();
    auto& state = w.beings().raw().get<world::Knowledge>(h).motivation;
    auto& action = feedback(state, reason);
    const auto observed = evidence(w, h, reason);
    const auto learned = progress(w, h, reason);
    const bool changed = observed != action.evidence || learned > action.progress;
    action.failures = changed ? 0 : static_cast<std::uint8_t>(std::min<int>(15, action.failures + 1));
    action.evidence = observed;
    action.progress = std::max(action.progress, learned);
    if (changed) relief(c, h, 0, true);
}
void Motivation::relief(world::Context& c, world::Beings::Handle h, std::size_t channel, bool progress) {
    auto& row = day(c.world().beings().raw().get<world::Knowledge>(h).motivation, c.now());
    row.relief[channel] += 100;
    if (progress) ++row.progress[channel];
}
void Motivation::request(world::Context& c, world::Beings::Handle h) {
    ++day(c.world().beings().raw().get<world::Knowledge>(h).motivation, c.now()).requests[1];
}
}  // namespace kd::demo
