#include "kd/demo/choice.hpp"
// Evidence-backed offers and shared attendance on the ordinary activity clock (MND-13, TIM-17).
#include "kd/data/craft.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/living.hpp"
#include "kd/demo/motivation.hpp"
namespace kd::demo {
namespace {
using world::LivingAct;
world::Lessons& lessons(world::World& w, world::Beings::Handle h) {
    return w.beings().raw().get<world::Lessons>(w.beings().handle(w.beings().raw().get<Home>(h).camp));
}
world::Lesson* session(world::World& w, world::Beings::Handle h) {
    const auto own = w.beings().raw().get<world::Knowledge>(h).session;
    auto& list = lessons(w, h).sessions;
    const auto found = std::find_if(list.begin(), list.end(), [&](const auto& s) { return s.id == own; });
    return found == list.end() ? nullptr : &*found;
}
world::Skill& skill(world::Knowledge& mind, std::uint32_t recipe) {
    const auto found =
        std::find_if(mind.skills.begin(), mind.skills.end(), [&](const auto& s) { return s.recipe == recipe; });
    if (found != mind.skills.end()) return *found;
    world::Skill fresh;
    fresh.recipe = recipe;
    mind.skills.push_back(fresh);
    return mind.skills.back();
}
void belief(world::Knowledge& mind, ecs::Id person, std::uint32_t recipe, bool knows, time::Seconds at,
            std::uint64_t event = 0) {
    const auto key = std::pair{person, recipe};
    auto found = std::lower_bound(mind.peers.begin(), mind.peers.end(), key,
                                  [](const auto& p, const auto& k) { return std::pair{p.person, p.recipe} < k; });
    world::PeerBelief value{
        person, recipe, static_cast<std::uint8_t>(knows), static_cast<std::uint8_t>(event == 0 ? 3 : 1), at, event};
    if (found != mind.peers.end() && std::pair{found->person, found->recipe} == key)
        *found = value;
    else if (mind.peers.size() < 256)
        mind.peers.insert(found, value);
}
bool comfortable(const Living& living, const world::World& w, world::Beings::Handle h, time::Seconds now) {
    const auto& raw = w.beings().raw();
    const auto body = living.sample(raw.get<world::Life>(h), raw.get<world::Activity>(h), now);
    const auto needs = Living::needs(body);
    return *std::min_element(needs.begin(), needs.end()) >= 60 && body.carried_food == 0 && body.allocated_water == 0 &&
           body.meal_item.value == 0;
}
bool free_plan(const world::World& w, world::Beings::Handle h, time::Seconds now) {
    const auto& raw = w.beings().raw();
    const auto& a = raw.get<world::Activity>(h);
    // Joining is a learner-chosen timed plan step. Other unfinished plans wait for their own ends.
    return raw.get<world::Work>(h).state == 0 &&
           (a.end <= now || a.what == static_cast<std::uint8_t>(LivingAct::watch) ||
            a.what == static_cast<std::uint8_t>(LivingAct::watch_craft));
}
world::CraftReason invitation(const world::Knowledge& mind, std::uint32_t recipe, std::int64_t seconds) {
    world::CraftReason reason{5, 1, 10, 3, recipe, 0, 10, seconds, {}};
    reason.need_met = mind.curiosity_need;
    // A kind person's wish to help a believed novice is an inclination, not the novice's hidden need.
    reason.parts = {std::max<std::int64_t>(0, 80 - mind.curiosity_need) * reason.benefit * 10,
                    mind.kindness * reason.benefit, -seconds / 60};
    reason.score = reason.parts[0] + reason.parts[1] + reason.parts[2];
    return reason;
}
void credit(world::Context& c, world::Lesson& s, bool success_bonus = false) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto& mind = raw.get<world::Knowledge>(w.beings().handle(s.learner));
    const auto& teacher = raw.get<world::Knowledge>(w.beings().handle(s.teacher));
    const auto& teaching = *std::find_if(teacher.skills.begin(), teacher.skills.end(),
                                         [&](const auto& x) { return x.recipe == s.recipe && x.known; });
    const auto seconds =
        success_bonus ? raw.get<world::Work>(w.beings().handle(s.learner)).try_seconds : s.seconds - s.credited_seconds;
    const auto sector = w.catalogue().kind<data::Blueprint>()[s.recipe].sector;
    Learning::practice(skill(mind, s.recipe).practice, c.now(), seconds, false, mind.learning_ppm,
                       Learning::taught_multiplier(teaching.practice));
    Learning::practice(mind.sectors[static_cast<std::size_t>(sector)], c.now(), seconds, false, mind.learning_ppm,
                       Learning::taught_multiplier(teaching.practice));
    if (!success_bonus) s.credited_seconds = s.seconds;
}
}  // namespace
bool Learning::exchange(world::Context& c, world::Beings::Handle speaker, world::Beings::Handle listener,
                        std::uint32_t recipe) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    const auto home = raw.get<Home>(speaker).camp;
    auto& sender = raw.get<world::Knowledge>(speaker);
    auto& receiver = raw.get<world::Knowledge>(listener);
    const auto person = w.beings().id_of(speaker), other = w.beings().id_of(listener);
    const auto& sa = raw.get<world::Activity>(speaker);
    const auto& la = raw.get<world::Activity>(listener);
    if (person == other || raw.get<Home>(listener).camp != home || !knows(sender, recipe) ||
        la.what == static_cast<std::uint8_t>(LivingAct::rest) ||
        !can_watch(w, home, sa.at(w.torus(), c.now()), la.at(w.torus(), c.now()), c.now()))
        return false;
    c.touch(other);
    // The listener truthfully answers about their own knowledge. Only this exchange may read that answer;
    // autonomous offers below consult the speaker's resulting evidence, never the listener's skill table.
    const bool answer = knows(receiver, recipe);
    belief(sender, other, recipe, answer, c.now());
    if (!answer) Motivation::request(c, speaker);
    belief(receiver, person, recipe, true, c.now());
    if (!answer) {
        const auto& blueprint = w.catalogue().kind<data::Blueprint>()[recipe];
        std::vector<world::Familiar> inputs;
        // Tell only experienced inputs from a real use, not hidden catalogue characteristics.
        for (auto i = sender.memories.rbegin(); i != sender.memories.rend(); ++i) {
            if (i->action != blueprint.action || i->inputs.empty()) continue;
            inputs = i->inputs;
            break;
        }
        const auto memory_id = receiver.next_memory;
        Discovery::memory(c, listener, static_cast<std::uint8_t>(blueprint.action), inputs, 0);
        const auto remembered = std::find_if(receiver.memories.begin(), receiver.memories.end(),
                                             [&](const auto& m) { return m.id == memory_id; });
        if (remembered != receiver.memories.end()) remembered->participants.push_back({person});
        world::Hunch hint;
        hint.action = static_cast<std::uint8_t>(blueprint.action);
        hint.source = person;
        hint.source_memory = receiver.next_memory - 1;
        hint.last_use = c.now();
        for (std::size_t i = 0; i < std::min<std::size_t>(2, inputs.size()); ++i) hint.inputs.push_back(inputs[i]);
        const auto old = std::find_if(receiver.hunches.begin(), receiver.hunches.end(),
                                      [&](const auto& x) { return x.source == person && x.action == hint.action; });
        if (old != receiver.hunches.end())
            *old = std::move(hint);
        else {
            if (receiver.hunches.size() == 5) receiver.hunches.erase(receiver.hunches.begin());
            receiver.hunches.push_back(std::move(hint));
        }
    }
    c.record(210, person.value, other.value);
    c.moved(other);
    return true;
}
bool Learning::reply_to_lesson(Living& living, world::Context& c, world::Beings::Handle listener, ecs::Id proposer,
                               bool resume, bool record) {
    auto& w = c.world();
    const auto& raw = w.beings().raw();
    const auto self = w.beings().id_of(listener);
    const auto& activity = raw.get<world::Activity>(listener);
    const bool own_plan_free =
        resume ? (activity.end <= c.now() || activity.what == static_cast<std::uint8_t>(LivingAct::watch) ||
                  activity.what == static_cast<std::uint8_t>(LivingAct::watch_craft))
               : free_plan(w, listener, c.now());
    const auto& own_mind = raw.get<world::Knowledge>(listener);
    const auto& own_work = raw.get<world::Work>(listener);
    const bool own_work_free =
        own_work.state == 0 || (resume && own_work.state == 4 && own_work.lesson == own_mind.session);
    const bool accepted = comfortable(living, w, listener, c.now()) && own_plan_free && own_work_free &&
                          (resume || own_mind.session == 0);
    // This is the listener's reply, not the speaker reading needs, meals or private reservations.
    if (record) c.record(accepted ? 213 : 214, self.value, proposer.value);
    return accepted;
}
bool Learning::choose(Living& living, world::Context& c, world::Beings::Handle h, ChoiceSet* proposals) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* mind = raw.try_get<world::Knowledge>(h);
    if (!mind || !comfortable(living, w, h, c.now())) return false;
    auto begin_meeting = [&living, &c](world::Lesson& s, std::uint64_t choice = 0) {
        auto& w = c.world();
        auto& raw = w.beings().raw();
        const auto teacher = w.beings().handle(s.teacher), learner = w.beings().handle(s.learner);
        c.touch(s.teacher);
        c.touch(s.learner);
        Learning::observe(c, teacher);
        Learning::observe(c, learner);
        living.settle(c, teacher, raw.get<Home>(teacher).camp, false);
        living.settle(c, learner, raw.get<Home>(learner).camp, false);
        s.state = 0;
        s.begun = 0;
        s.seconds = 0;
        s.credited_seconds = 0;
        s.last_try = 0;
        s.settled = c.now();
        s.end = c.now() + time::kHour;
        auto& work = raw.get<world::Work>(learner);
        work.state = 1;
        work.target = s.meeting;
        raw.get<world::Life>(teacher).portion = 0;
        auto reason = invitation(raw.get<world::Knowledge>(teacher), s.recipe, 1800);
        if (!choice) Choices::keep(c, teacher, reason);
        Choices::restore(w, learner, work.choice);
        living.begin(c, teacher, LivingAct::teach, time::kHour, raw.get<world::Place>(teacher).at);
        const bool moving = Crafting::continue_work(living, c, learner);
        if (!moving) {
            s.state = 2;
            s.work = 0;
            living.begin(c, teacher, LivingAct::watch, 60, raw.get<world::Place>(teacher).at);
            living.begin(c, learner, LivingAct::watch, 60, raw.get<world::Place>(learner).at);
        }
        return true;
    };
    if (auto* s = session(w, h)) {
        if (s->state != 2) return !proposals;
        const auto id = s->id;
        auto resume = [&living, &c, h, id, begin_meeting](std::uint64_t choice) {
            auto& w = c.world();
            auto& raw = w.beings().raw();
            auto* s = session(w, h);
            KD_CHECK(s && s->id == id, "The selected shared plan still exists");
            const auto teacher = w.beings().handle(s->teacher), learner = w.beings().handle(s->learner);
            if (!reply_to_lesson(living, c, teacher, s->learner, true) ||
                !reply_to_lesson(living, c, learner, s->teacher, true)) {
                if (!choice) return false;
                living.begin(c, h, LivingAct::watch, 60, raw.get<world::Place>(h).at);
                return true;
            }
            if (s->work == 0 &&
                Crafting::prepare_lesson(c, teacher, learner, s->recipe, s->id, choice ? &living : nullptr))
                s->work = raw.get<world::Work>(learner).number;
            if (s->work == 0) {
                if (!choice) return false;
                living.begin(c, h, LivingAct::watch, 60, raw.get<world::Place>(h).at);
                return true;
            }
            if (w.beings().id_of(h) == s->learner) raw.get<world::Work>(h).choice = choice;
            return begin_meeting(*s, choice);
        };
        if (!proposals) return resume(0);
        world::CraftReason reason{5, 1, 10, 3, s->recipe, 90000, 10, 1800, {}};
        reason.need_met = mind->curiosity_need;
        reason.parts = {0, 90000, 0};
        proposals->add(reason, std::move(resume));
        return false;
    }
    if (mind->kindness < 60 || raw.get<world::Work>(h).state != 0) return false;
    const auto home = raw.get<Home>(h).camp;
    const auto person = w.beings().id_of(h);
    std::vector<ecs::Id> nearby;
    w.beings().each([&](ecs::Id id, world::Beings::Handle other) {
        if (id == person || !raw.all_of<world::Knowledge, Home>(other) || raw.get<Home>(other).camp != home) return;
        const auto& activity = raw.get<world::Activity>(other);
        // Offer when the other person visibly watches or finishes an activity. Their bodily reply stays private.
        if (activity.end > c.now() && activity.what != static_cast<std::uint8_t>(LivingAct::watch) &&
            activity.what != static_cast<std::uint8_t>(LivingAct::watch_craft))
            return;
        const auto at = activity.at(w.torus(), c.now());
        if (can_watch(w, home, raw.get<world::Place>(h).at, at, c.now())) nearby.push_back(id);
    });
    std::size_t offers = 0;
    for (const auto other : nearby) {
        const auto learner = w.beings().handle(other);
        c.touch(other);
        for (const auto& known : mind->skills) {
            if (!known.known) continue;
            const auto peer = std::find_if(mind->peers.begin(), mind->peers.end(), [&](const auto& p) {
                return p.person == other && p.recipe == known.recipe;
            });
            // Ask once when no evidence exists. Later observed use can correct this belief.
            if (peer == mind->peers.end()) {
                if (!proposals) {
                    (void)exchange(c, h, learner, known.recipe);
                    continue;
                }
                auto reason = invitation(*mind, known.recipe, 60);
                reason.confidence = 0;  // No observed novice yet; enquiry is not an eligible teaching deficit.
                proposals->add(reason, [&living, &c, h, other, recipe = known.recipe](std::uint64_t) {
                    const auto learner = c.world().beings().handle(other);
                    (void)exchange(c, h, learner, recipe);
                    living.begin(c, h, LivingAct::watch, 60, c.world().beings().raw().get<world::Place>(h).at);
                    return true;
                });
                if (++offers == 8) return false;
                continue;
            }
            if (peer->knows) continue;
            const auto recipe = known.recipe;
            if (!Crafting::lesson_reachable(c, h, learner, recipe)) continue;
            auto offer = [&living, &c, h, person, other, recipe, begin_meeting](std::uint64_t choice) {
                auto& w = c.world();
                auto& raw = w.beings().raw();
                const auto learner = w.beings().handle(other);
                c.touch(other);
                c.record(211, person.value, other.value);
                auto& list = lessons(w, h);
                const bool ready = reply_to_lesson(living, c, learner, person, false, !choice);
                const bool accepted =
                    ready && list.sessions.size() < 64 &&
                    Crafting::prepare_lesson(c, h, learner, recipe, list.next, choice ? &living : nullptr);
                if (choice) c.record(accepted ? 213 : 214, other.value, person.value);
                if (!accepted) {
                    if (!choice) return false;
                    living.begin(c, h, LivingAct::watch, 60, raw.get<world::Place>(h).at);
                    return true;
                }
                world::Lesson s;
                s.id = list.next++;
                s.teacher = person;
                s.learner = other;
                s.recipe = recipe;
                s.meeting = raw.get<world::Place>(h).at;
                s.offered = c.now();
                s.work = raw.get<world::Work>(learner).number;
                raw.get<world::Knowledge>(h).session = s.id;
                raw.get<world::Knowledge>(learner).session = s.id;
                list.sessions.push_back(s);
                return begin_meeting(list.sessions.back(), choice);
            };
            if (!proposals) {
                if (offer(0)) return true;
                continue;
            }
            const auto walking = w.torus().distance(raw.get<world::Place>(h).at,
                                                    raw.get<world::Activity>(learner).at(w.torus(), c.now())) *
                                 10 / living.rules().speed;
            auto reason = invitation(*mind, recipe, 1800 + walking);
            proposals->add(reason, std::move(offer));
            if (++offers == 8) return false;
        }
    }
    return false;
}
void Learning::started(Living& living, world::Context& c, world::Beings::Handle learner) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    auto* s = session(w, learner);
    KD_CHECK(s && s->learner == w.beings().id_of(learner), "Shared work has its session");
    const auto teacher = w.beings().handle(s->teacher);
    c.touch(s->teacher);
    living.settle(c, teacher, raw.get<Home>(teacher).camp, false);
    s->state = 1;
    s->begun = c.now();
    s->settled = c.now();
    s->end = c.now() + 1800;
    auto& work = raw.get<world::Work>(learner);
    work.end = s->end;
    work.next_try = std::min(s->end, c.now() + std::max<std::int64_t>(1, work.try_seconds - work.retained_progress));
    c.cancel(s->learner, 2);
    if (work.next_try < work.end) c.schedule(s->learner, 2, work.next_try);
    living.begin(c, teacher, LivingAct::teach, 1800, raw.get<world::Place>(teacher).at);
}
void Learning::worked(world::Context& c, world::Beings::Handle maker, std::uint64_t event, bool success) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    const auto& work = raw.get<world::Work>(maker);
    if (!work.intended) return;
    const auto& history = raw.get<world::CraftHistory>(w.beings().handle(raw.get<Home>(maker).camp));
    const auto result = std::lower_bound(history.events.begin(), history.events.end(), event,
                                         [](const auto& e, auto id) { return e.id < id; });
    if (result == history.events.end() || result->id != event || result->recipe != work.recipe) return;
    auto& mind = raw.get<world::Knowledge>(maker);
    if (work.lesson == 0) {
        practice(skill(mind, work.recipe).practice, c.now(), work.try_seconds, success, mind.learning_ppm);
        const auto sector = w.catalogue().kind<data::Blueprint>()[work.recipe].sector;
        practice(mind.sectors[static_cast<std::size_t>(sector)], c.now(), work.try_seconds, success, mind.learning_ppm);
        return;
    }
    auto* s = session(w, maker);
    KD_CHECK(s && s->state == 1, "Taught result requires shared attendance");
    if (event <= s->last_try) return;
    s->last_try = event;
    if (success) {
        auto& learned = skill(mind, work.recipe);
        if (!learned.known) {
            learned.known = 1;
            learned.practice.level = std::max<std::int64_t>(1000, learned.practice.level);
            learned.practice.best = std::max(learned.practice.best, learned.practice.level);
            learned.practice.decay_level = learned.practice.level;
            learned.source = s->teacher;
            learned.source_event = event;
            learned.route = 5;
            Motivation::relief(c, maker, 0, true);
            Motivation::relief(c, w.beings().handle(s->teacher), 1, true);
            Learning::learned(c, maker, *result, s->teacher, 5);
        }
        belief(raw.get<world::Knowledge>(w.beings().handle(s->teacher)), s->learner, s->recipe, true, c.now(), event);
        credit(c, *s, true);
    }
}
bool Learning::handle(Living& living, world::Context& c, world::Beings::Handle h, bool interrupted, bool try_event) {
    auto& w = c.world();
    auto& raw = w.beings().raw();
    if (!raw.all_of<world::Knowledge>(h)) return false;
    auto* s = session(w, h);
    if (!s || s->state == 2) return false;
    const auto teacher = w.beings().handle(s->teacher), learner = w.beings().handle(s->learner);
    c.touch(s->teacher);
    c.touch(s->learner);
    const auto home = raw.get<Home>(teacher).camp;
    const auto& ta = raw.get<world::Activity>(teacher);
    const auto& la = raw.get<world::Activity>(learner);
    // Each participant can leave for their own urgent needs; the shared coordinator sees only that request.
    const auto requests_stop = [&](world::Beings::Handle self, ecs::Id other) {
        const auto body = living.sample(raw.get<world::Life>(self), raw.get<world::Activity>(self), c.now());
        const auto own_needs = Living::needs(body);
        const bool leave = *std::min_element(own_needs.begin(), own_needs.end()) < 20;
        if (leave) c.record(216, w.beings().id_of(self).value, other.value);
        return leave;
    };
    const bool teacher_leaves = requests_stop(teacher, s->learner);
    const bool learner_leaves = requests_stop(learner, s->teacher);
    bool stop = interrupted || teacher_leaves || learner_leaves;
    if (s->state == 1) {
        // Account each attended second, even if an external call changes attendance between ends.
        for (auto t = s->settled; t < c.now(); ++t) {
            if (w.torus().squared_distance(ta.at(w.torus(), t), la.at(w.torus(), t)) > 200LL * 200 ||
                !can_watch(w, home, ta.at(w.torus(), t), la.at(w.torus(), t), t)) {
                stop = true;
                break;
            }
            ++s->seconds;
        }
        s->settled = c.now();
        credit(c, *s);
    }
    observe_maker(c, learner);
    observe(c, teacher);
    living.settle(c, teacher, home, stop);
    living.settle(c, learner, home, stop);
    if (s->state == 0 && !stop && c.now() < s->end) {
        if (h == learner && Crafting::continue_work(living, c, learner)) {
            if (s->state == 0)
                living.begin(c, teacher, LivingAct::teach, s->end - c.now(), raw.get<world::Place>(teacher).at);
            return true;
        }
        stop = true;
    }
    if (s->state == 0) stop = true;
    auto& work = raw.get<world::Work>(learner);
    Crafting::settle(living, c, learner, stop, try_event && h == learner);
    if (!stop && work.state == 2 && c.now() < s->end) {
        living.begin(c, teacher, LivingAct::teach, s->end - c.now(), raw.get<world::Place>(teacher).at);
        return true;
    }
    const auto teacher_id = s->teacher, learner_id = s->learner;
    const auto id = s->id;
    c.cancel(teacher_id, 2);
    c.cancel(learner_id, 2);
    if (stop || work.state == 4) {
        s->state = 2;
        s->work = work.lesson == id ? work.number : 0;
        if (work.state == 1) work.state = 4;
    } else {
        raw.get<world::Knowledge>(teacher).session = 0;
        raw.get<world::Knowledge>(learner).session = 0;
        std::erase_if(lessons(w, h).sessions, [&](const auto& x) { return x.id == id; });
    }
    // A call gets an elapsed share and a short new choice boundary; urgent bodies choose immediately.
    for (const auto participant : {teacher, learner}) {
        living.begin(c, participant, LivingAct::watch, 60, raw.get<world::Place>(participant).at);
        const auto n = Living::needs(raw.get<world::Life>(participant));
        if (*std::min_element(n.begin(), n.end()) < 20) living.choose(c, participant, home);
    }
    return true;
}
}  // namespace kd::demo
