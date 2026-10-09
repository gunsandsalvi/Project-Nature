#include "kd/demo/learning.hpp"
#include <algorithm>
#include "kd/data/craft.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/living.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"
namespace kd::demo {
void Learning::settle_mind(world::Context& c, world::Beings::Handle h) {
    auto& mind = c.world().beings().raw().get<world::Knowledge>(h);
    const auto elapsed = c.now() - mind.settled;
    const auto decline = elapsed * 10 + mind.curiosity_remainder;
    mind.curiosity_need =
        static_cast<std::uint8_t>(std::max<std::int64_t>(0, mind.curiosity_need - decline / time::kDay));
    mind.curiosity_remainder = decline % time::kDay;
    mind.settled = c.now();
    for (auto& skill : mind.skills) fade(skill.practice, c.now());
    for (auto& sector : mind.sectors) fade(sector, c.now());
    std::erase_if(mind.hunches, [&](const auto& hint) { return c.now() - hint.last_use >= time::kYear; });
}
bool Learning::can_watch(const world::World& w, ecs::Id camp, num::Point from, num::Point to, time::Seconds at) {
    // The bounded camp has daylight, but no simulated task light yet (fire comes in alpha 3.13c).
    const auto clock = at % time::kDay;
    return clock >= 6 * time::kHour && clock < 20 * time::kHour &&
           w.torus().squared_distance(from, to) <= 500LL * 500 && Living::visible(w, camp, from, to);
}
namespace {
struct ActiveMaker {
    ecs::Id id, camp;
    const world::Work* work;
    const world::Activity* activity;
};
std::vector<ActiveMaker> active_makers(const world::World& w) {
    std::vector<ActiveMaker> out;
    const auto& raw = w.beings().raw();
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (!raw.all_of<world::Knowledge, world::Work, Home>(h)) return;
        const auto& work = raw.get<world::Work>(h);
        if (work.state == 2 && work.intended && work.try_seconds > 0)
            out.push_back({id, raw.get<Home>(h).camp, &work, &raw.get<world::Activity>(h)});
    });
    return out;
}
void observe_active(world::Context& c, world::Beings::Handle observer, std::span<const ActiveMaker> makers) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* mind = raw.try_get<world::Knowledge>(observer);
    if (!mind) return;
    const auto id = w.beings().id_of(observer);
    const auto camp = raw.get<Home>(observer).camp;
    const auto& act = raw.get<world::Activity>(observer);
    for (const auto& maker : makers) {
        const auto person = maker.id;
        if (person == id || maker.camp != camp) continue;
        c.touch(person);
        const auto& work = *maker.work;
        auto found = std::lower_bound(mind->observations.begin(), mind->observations.end(), person,
                                      [](const auto& o, auto target) { return o.person < target; });
        if (found == mind->observations.end() || found->person != person)
            found =
                mind->observations.insert(found, {person, work.number, work.completed_tries + 1, work.active_start, 0});
        if (found->work != work.number || found->attempt != work.completed_tries + 1)
            *found = {person, work.number, work.completed_tries + 1, work.active_start, 0};
        const auto begin = std::max({found->settled, work.active_start, act.start});
        const auto end = std::min({c.now(), work.next_try, act.end});
        const auto& demonstration = *maker.activity;
        const auto weight =
            act.what == static_cast<std::uint8_t>(world::LivingAct::rest)                                      ? 0
            : act.what == static_cast<std::uint8_t>(world::LivingAct::watch_craft) && mind->watching == person ? 4
                                                                                                               : 1;
        if (weight != 0) {
            // Whole game-second intervals, independent of event batching, frames and save boundaries.
            if (act.from == act.to && demonstration.from == demonstration.to) {
                // Stationary work has constant geometry. Count daylight exactly across the interval
                // rather than repeating identical sight and torus tests every game second.
                const auto daylight = [](time::Seconds t) {
                    return t / time::kDay * (14 * time::kHour) +
                           std::clamp(t % time::kDay - 6 * time::kHour, time::Seconds{0}, 14 * time::kHour);
                };
                if (end > begin && w.torus().squared_distance(act.from, demonstration.from) <= 500LL * 500 &&
                    Living::visible(w, camp, act.from, demonstration.from))
                    found->weighted_seconds += weight * (daylight(end) - daylight(begin));
            } else {
                for (auto t = begin; t < end; ++t)
                    if (Learning::can_watch(w, camp, act.at(w.torus(), t), demonstration.at(w.torus(), t), t))
                        found->weighted_seconds += weight;
            }
        }
        found->settled = std::max(found->settled, end);
    }
}
}  // namespace
void Learning::observe(world::Context& c, world::Beings::Handle observer) {
    const auto active = active_makers(c.world());
    observe_active(c, observer, active);
}
void Learning::observe_maker(world::Context& c, world::Beings::Handle maker) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    const auto camp = raw.get<Home>(maker).camp;
    const auto active = active_makers(w);
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (h == maker || !raw.all_of<world::Knowledge, Home>(h) || raw.get<Home>(h).camp != camp) return;
        c.touch(id);
        observe_active(c, h, active);
    });
}
void Learning::forget_work(world::Context& c, world::Beings::Handle maker) {
    auto& w = c.world();
    const auto person = w.beings().id_of(maker);
    const auto camp = w.beings().raw().get<Home>(maker).camp;
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        auto* mind = w.beings().raw().try_get<world::Knowledge>(h);
        if (!mind || w.beings().raw().get<Home>(h).camp != camp) return;
        c.touch(id);
        std::erase_if(mind->observations, [&](const auto& o) { return o.person == person; });
    });
}
void Learning::demonstrated(world::Context& c, world::Beings::Handle maker, std::uint64_t event) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    const auto person = w.beings().id_of(maker);
    const auto camp = raw.get<Home>(maker).camp;
    const auto& work = raw.get<world::Work>(maker);
    const auto& history = raw.get<world::CraftHistory>(w.beings().handle(camp));
    const auto found_result = std::lower_bound(history.events.begin(), history.events.end(), event,
                                               [](const auto& e, auto id) { return e.id < id; });
    KD_CHECK(found_result != history.events.end() && found_result->id == event && found_result->actor == person,
             "Observation cites an actual demonstrated end");
    // Learning appends to HIST1. Keep the evidence stable while other people acquire it.
    world::Result demonstration;
    demonstration = *found_result;
    const auto* result = &demonstration;
    if (!work.intended) return;
    const auto duration = work.try_seconds;
    const auto& blueprint = w.catalogue().kind<data::Blueprint>()[result->recipe];
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        auto* mind = raw.try_get<world::Knowledge>(h);
        if (!mind || id == person || raw.get<Home>(h).camp != camp) return;
        c.touch(id);
        const auto seen = std::find_if(mind->observations.begin(), mind->observations.end(),
                                       [&](const auto& o) { return o.person == person && o.work == work.number; });
        if (seen == mind->observations.end() || event <= mind->last_observed_event) return;
        const auto weighted = seen->weighted_seconds;
        mind->observations.erase(seen);
        if (weighted == 0) return;
        mind->last_observed_event = event;
        auto skill = std::find_if(mind->skills.begin(), mind->skills.end(),
                                  [&](const auto& s) { return s.recipe == result->recipe; });
        if (skill == mind->skills.end()) {
            world::Skill fresh;
            fresh.recipe = result->recipe;
            mind->skills.push_back(fresh);
            skill = std::prev(mind->skills.end());
        }
        const auto credits =
            std::min<std::int64_t>(20000000, skill->observation_quarters * 1000000LL + skill->observation_remainder +
                                                 weighted * 1000000 / duration);
        skill->observation_quarters = static_cast<std::uint32_t>(credits / 1000000);
        skill->observation_remainder = credits % 1000000;
        if (!skill->known && credits >= 20000000) {
            skill->known = 1;
            skill->practice = {1000, 1000, 0, c.now()};
            skill->source = person;
            skill->source_event = event;
            skill->route = 4;
            learned(c, h, *result, person, 4);
        }
        // Retain only properties the observer can see, never the maker's handling knowledge.
        std::vector<world::Familiar> inputs;
        for (const auto& link : result->inputs) {
            Discovery::learn(c, h, link.id, Discovery::kSight, 5, false, event, person);
            const auto& physical = std::as_const(w);
            const auto& item = physical.things().raw().get<world::Item>(physical.things().handle(link.id));
            inputs.push_back(*Discovery::familiar(*mind, item));
        }
        const auto& activity = raw.get<world::Activity>(h);
        const bool sees_result = activity.what != static_cast<std::uint8_t>(world::LivingAct::rest) &&
                                 can_watch(w, camp, activity.at(w.torus(), c.now()), result->place, c.now());
        Discovery::memory(c, h, static_cast<std::uint8_t>(blueprint.action), inputs, 0,
                          sees_result ? result->result : ecs::Id{}, event);
        if (!skill->known) {
            world::Hunch hint;
            hint.action = static_cast<std::uint8_t>(blueprint.action);
            hint.source = person;
            hint.source_memory = mind->next_memory - 1;
            hint.last_use = c.now();
            if (sees_result && result->result.value != 0) {
                const auto& physical = std::as_const(w);
                const auto& item = physical.things().raw().get<world::Item>(physical.things().handle(result->result));
                const auto& form = w.catalogue().kind<data::ItemKind>()[item.kind].form;
                hint.result_form = static_cast<std::uint8_t>(
                    std::distance(data::kForms.begin(), std::find(data::kForms.begin(), data::kForms.end(), form)));
            }
            for (std::size_t i = 0; i < std::min<std::size_t>(2, inputs.size()); ++i) hint.inputs.push_back(inputs[i]);
            auto same = std::find_if(mind->hunches.begin(), mind->hunches.end(), [&](const auto& old) {
                return old.action == hint.action && old.source == person;
            });
            if (same != mind->hunches.end())
                *same = hint;
            else {
                if (mind->hunches.size() == 5) mind->hunches.erase(mind->hunches.begin());
                mind->hunches.push_back(std::move(hint));
            }
        }
        if (work.intended || result->noticed) {
            const auto key = std::pair{person, result->recipe};
            auto peer = std::lower_bound(mind->peers.begin(), mind->peers.end(), key, [](const auto& p, const auto& k) {
                return std::pair{p.person, p.recipe} < k;
            });
            // A failed demonstration is not evidence that its maker lacks the craft.
            if (result->kind != 5) {
                world::PeerBelief belief{person, result->recipe, 1, 1, c.now(), event};
                if (peer != mind->peers.end() && std::pair{peer->person, peer->recipe} == key)
                    *peer = belief;
                else
                    mind->peers.insert(peer, belief);
            }
        }
        c.moved(id);
    });
}
void Learning::learned(world::Context& c, world::Beings::Handle learner, const world::Result& evidence, ecs::Id source,
                       std::uint8_t route) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto& history = raw.get<world::CraftHistory>(w.beings().handle(raw.get<Home>(learner).camp));
    auto entry = evidence;
    entry.id = history.next++;
    entry.at = c.now();
    entry.place = raw.get<world::Place>(learner).at;
    entry.actor = w.beings().id_of(learner);
    entry.source = source;
    entry.route = route;
    entry.kind = 2;
    if (entry.word.empty()) {
        const auto named = std::find_if(history.events.rbegin(), history.events.rend(),
                                        [&](const auto& e) { return e.recipe == entry.recipe && !e.word.empty(); });
        if (named != history.events.rend()) entry.word = named->word;
    }
    history.events.push_back(std::move(entry));
}
void Learning::lost(world::Context& c, world::Beings::Handle last_holder, std::uint32_t recipe) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    const auto home = raw.get<Home>(last_holder).camp;
    bool retained = false;
    w.beings().each([&](ecs::Id, world::Beings::Handle h) {
        if (raw.all_of<world::Knowledge, Home>(h) && raw.get<Home>(h).camp == home &&
            knows(raw.get<world::Knowledge>(h), recipe))
            retained = true;
    });
    if (retained) return;
    auto& history = raw.get<world::CraftHistory>(w.beings().handle(home));
    const auto previous = std::find_if(history.events.rbegin(), history.events.rend(),
                                       [&](const auto& e) { return e.recipe == recipe && !e.word.empty(); });
    if (previous == history.events.rend() || previous->kind == 3) return;
    auto entry = *previous;
    entry.id = history.next++;
    entry.at = c.now();
    entry.place = raw.get<world::Place>(last_holder).at;
    entry.actor = w.beings().id_of(last_holder);
    entry.source = {};
    entry.result = {};
    entry.noticed = 0;
    entry.kind = 3;
    history.events.push_back(std::move(entry));
}
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
    // Keep a sub-thousandth loss until it becomes representable. Rounding downward at every brief
    // practice anchor otherwise loses one whole thousandth per try and can erase all real progress.
    skill.level = floor + num::to_int(static_cast<double>(span) * num::exp2(-elapsed), num::Round::nearest);
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
