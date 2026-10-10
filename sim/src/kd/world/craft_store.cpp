#include "kd/world/craft_store.hpp"
#include <map>
#include <set>
#include "kd/data/craft.hpp"
#include "kd/demo/living.hpp"
namespace kd::world {
std::uint32_t craft_features(const World& w) {
    return w.beings().raw().view<CraftHistory>().empty() && w.beings().raw().view<Knowledge>().empty() &&
                   w.beings().raw().view<Work>().empty()
               ? 0U
               : kCraft | kLearning | kIdeas | (w.beings().raw().view<Ambient>().empty() ? 0U : kFire);
}
void save_craft(const World& w, std::vector<save::Chunk>& out) {
    if (craft_features(w) == 0) return;
    const auto& beings = w.beings();
    const auto& things = w.things();
    ByteWriter craft, know, history, learning;
    craft.u64(things.raw().view<Item>().size() + beings.raw().view<Work>().size());
    // Families sort people before things in canonical IDs.
    beings.each([&](ecs::Id id, Beings::Handle h) {
        if (const auto* x = beings.raw().try_get<Work>(h)) {
            craft.u64(id.value);
            craft.u8(2);
            ecs::write_component(*x, craft);
        }
    });
    things.each([&](ecs::Id id, Things::Handle h) {
        if (const auto* x = things.raw().try_get<Item>(h)) {
            craft.u64(id.value);
            craft.u8(1);
            ecs::write_component(*x, craft);
        }
    });
    know.u64(beings.raw().view<Knowledge>().size());
    history.u64(beings.raw().view<CraftHistory>().size());
    learning.u64(beings.raw().view<Lessons>().size());
    beings.each([&](ecs::Id id, Beings::Handle h) {
        if (const auto* x = beings.raw().try_get<Knowledge>(h)) {
            know.u64(id.value);
            ecs::write_component(*x, know);
        }
        if (const auto* x = beings.raw().try_get<CraftHistory>(h)) {
            history.u64(id.value);
            ecs::write_component(*x, history);
        }
        if (const auto* x = beings.raw().try_get<Lessons>(h)) {
            learning.u64(id.value);
            ecs::write_component(*x, learning);
        }
    });
    out.push_back({save::tag("CRFT"), 2, true, craft.take()});
    out.push_back({save::tag("KNOW"), 2, true, know.take()});
    out.push_back({save::tag("HIST"), 2, true, history.take()});
    // LEARN1 retains the foundation's four-letter wire tags.
    out.push_back({save::tag("LEAR"), 1, true, learning.take()});
}
bool craft_headers(std::span<const save::Chunk> chunks, std::uint32_t& features, std::string& why) {
    const auto fail = [&](std::string text) {
        why = std::move(text);
        return false;
    };
    const auto* camp = save::find_chunk(chunks, save::tag("CAMP"));
    if (camp) {
        if (camp->version != 4 || !camp->critical || camp->data.size() < 4) return fail("unsupported Camp format");
        ByteReader mask(std::span(camp->data).last(4));
        if (!mask.u32(features) || (features != 0 && features != (kCraft | kLearning | kIdeas) &&
                                    features != (kCraft | kLearning | kFire | kIdeas)))
            return fail("unsupported camp features");
    }
    for (const auto tag : {save::tag("LIFE"), save::tag("DRMS"), save::tag("CRFT"), save::tag("KNOW"),
                           save::tag("HIST"), save::tag("LEAR")}) {
        const auto* c = save::find_chunk(chunks, tag);
        const auto count = std::count_if(chunks.begin(), chunks.end(), [&](const auto& x) { return x.tag == tag; });
        if (count > 1) return fail("duplicate camp extension");
        const bool craft = tag == save::tag("CRFT") || tag == save::tag("KNOW") || tag == save::tag("HIST") ||
                           tag == save::tag("LEAR");
        if ((craft && ((features != 0) != bool(c))) || (features != 0 && !c))
            return fail("required craft extension is missing or mismatched");
        if (c && tag == save::tag("DRMS") && c->version != 3)
            return fail("Unsupported dream format. Start a new camp.");
        if (c && (!c->critical || c->version != (tag == save::tag("LEAR") ? 1U : tag == save::tag("DRMS") ? 3U : 2U)))
            return fail("unsupported camp extension version");
    }
    return true;
}
namespace {
bool point(const World& w, num::Point p) {
    return p.x >= 0 && p.x < w.torus().width() && p.y >= 0 && p.y < w.torus().height();
}
bool person(const World& w, ecs::Id id, bool optional = false) {
    if (id.value == 0) return optional;
    const auto h = w.beings().find(id);
    return h && w.beings().raw().all_of<Person>(*h);
}
bool item(const World& w, ecs::Id id, bool optional = false) {
    if (id.value == 0) return optional;
    const auto h = w.things().find(id);
    return h && w.things().raw().all_of<Item>(*h);
}
bool reason_valid(const World& w, const CraftReason& reason) {
    if ((reason.kind == 2 && reason.need > 3) ||
        (reason.kind == 3 &&
         (reason.action < 1 || reason.action > 4 || (reason.need != 4 && reason.need != 3) || reason.intended)) ||
        (reason.kind == 4 && (reason.action != 11 || reason.need != 4 || reason.intended)) || reason.kind > 5 ||
        reason.intended > 1 || reason.action > 20 || reason.need > 4 ||
        (!reason.intended && reason.recipe != kNoRecipe) || reason.score < -1000000 || reason.score > 1000000 ||
        reason.benefit < 0 || reason.benefit > 100 || reason.seconds < 0 || reason.seconds > time::kDay * 2 ||
        reason.need_met > 100 || reason.confidence > 100 || reason.unavailable > 5 || reason.observed_heat > 5 ||
        reason.observed_fuel_mg < 0 || reason.observed_fuel_mg > 1000000000)
        return false;
    return std::all_of(reason.inputs.begin(), reason.inputs.end(),
                       [&](const auto& input) { return item(w, input.id); });
}
bool practice(const Practice& p, time::Seconds now) {
    return p.level >= 0 && p.level <= 10000 && p.best >= p.level && p.best <= 10000 && p.level >= (p.best + 1) / 2 &&
           p.seconds >= 0 && p.seconds <= 60 * time::kDay * 1000 && p.last_use >= -1 && p.last_use <= now &&
           p.fraction >= 0 && p.fraction < 561600000 && (p.level != 10000 || p.fraction == 0) &&
           p.seconds_remainder >= 0 && p.seconds_remainder < 1000000 && p.scale_remainder >= 0 &&
           p.scale_remainder < 1000000 && p.decay_at >= 0 && p.decay_at <= now && p.decay_level >= 0 &&
           p.decay_level <= p.best && (p.decay_level == 0 || p.decay_level >= p.level);
}
bool familiar(const Familiar& f, time::Seconds now) {
    if (f.mask >= (1U << 18U) || f.edible > 1 || f.state > 4 || f.edible_source > 6 || f.at < 0 || f.at > now)
        return false;
    for (std::size_t i = 0; i < 18; ++i)
        if (f.values[i] > 5 || f.certainty[i] > 100 || f.sources[i] > 6 || f.learned_at[i] < 0 ||
            f.learned_at[i] > now ||
            ((f.mask & (1U << i)) == 0 &&
             (f.values[i] != 0 || f.certainty[i] != 0 || f.sources[i] != 0 || f.learned_at[i] != 0 ||
              f.source_people[i].value != 0 || f.source_events[i] != 0)))
            return false;
    return true;
}
}  // namespace
bool load_craft(World& w, std::span<const save::Chunk> chunks, const ecs::EntryMap& entries, std::uint32_t features,
                std::string& why) {
    const auto fail = [&](std::string text) {
        why = std::move(text);
        return false;
    };
    if (features == 0) return true;
    auto& raw = w.beings().raw();
    auto& things = w.things().raw();
    {
        ByteReader r(save::find_chunk(chunks, save::tag("CRFT"))->data);
        std::uint64_t count = 0;
        ecs::Id last{};
        const auto beings_count = w.beings().size();
        const auto things_count = w.things().size();
        if (!r.u64(count) || count > beings_count + things_count) return fail("invalid craft count");
        for (std::uint64_t n = 0; n < count; ++n) {
            ecs::Id id{};
            std::uint8_t kind = 0;
            if (!r.u64(id.value) || !(last < id) || !r.u8(kind)) return fail("invalid craft identity order");
            last = id;
            if (kind == 1) {
                const auto h = w.things().find(id);
                Item value;
                if (!h || id.family() != ecs::Family::thing || !ecs::read_component(value, r, entries))
                    return fail("invalid item record");
                if (value.changed_mask >= (1U << 18U)) return fail("invalid result characteristic mask");
                for (std::size_t c = 0; c < 18; ++c) {
                    if (value.changed[c] > 5 || ((value.changed_mask & (1U << c)) == 0 && value.changed[c] != 0))
                        return fail("invalid result characteristic value");
                }
                const auto home = w.beings().find(value.home);
                if (!home || !raw.all_of<Camp>(*home) || !person(w, value.owner, true) ||
                    !person(w, value.maker, true) || value.mass < 0 || value.mass > 1000000000 || value.length < 1 ||
                    value.length > 100000 || value.state > 4 || (value.mass == 0) != (value.state == 4) ||
                    value.quality > 5 || value.wear < 0 || value.wear > 5000000 || value.wear_remainder < 0 ||
                    value.wear_remainder >= 4000000 || value.made_at < -1 || value.made_at > w.frontier() ||
                    (value.made_at == -1 && (value.maker.value != 0 || !value.parents.empty())) ||
                    !raw.all_of<Place>(*home) || !things.all_of<Place>(*h) || !point(w, things.get<Place>(*h).at))
                    return fail("invalid item quantity or ownership");
                things.emplace<Item>(*h, std::move(value));
            } else if (kind == 2) {
                const auto h = w.beings().find(id);
                Work value;
                if (!h || !raw.all_of<Person>(*h) || !ecs::read_component(value, r, entries))
                    return fail("invalid work record");
                if (value.state > 4 || value.action > 20 || value.intended > 1 || value.route > 5 || value.rolled > 1 ||
                    ((value.route == 5) != (value.lesson != 0)) || (!value.intended && value.recipe != kNoRecipe) ||
                    value.start < 0 || value.start > w.frontier() || value.active_start < value.start ||
                    value.active_start > w.frontier() || value.end < 0 || value.next_try < 0 ||
                    value.next_try > value.end || value.retained_progress < 0 || value.try_seconds < 0 ||
                    value.try_seconds > 3600 || value.retained_progress > value.try_seconds || value.unit_mass < 0 ||
                    value.unit_mass > 1000000000 || value.goal_mass < 0 || value.goal_mass > 1000000000 ||
                    value.applied_marker > value.completed_tries || !point(w, value.target))
                    return fail("invalid work progress");
                if (value.state == 0 &&
                    (!value.inputs.empty() || value.number != 0 || value.intended || value.recipe != kNoRecipe ||
                     value.completed_tries != 0 || value.applied_marker != 0 || value.rolled ||
                     value.retained_progress != 0 || value.start != 0 || value.active_start != 0 || value.end != 0 ||
                     value.next_try != 0 || value.try_seconds != 0 || value.unit_mass != 0 || value.goal_mass != 0 ||
                     value.lesson != 0))
                    return fail("idle work contains pending progress");
                raw.emplace<Work>(*h, std::move(value));
            } else
                return fail("invalid craft record kind");
        }
        if (!r.finished()) return fail("trailing craft records");
    }
    {
        ByteReader r(save::find_chunk(chunks, save::tag("KNOW"))->data);
        std::uint64_t count = 0;
        ecs::Id last{};
        if (!r.u64(count) || count != raw.view<Person>().size()) return fail("invalid knowledge count");
        for (std::uint64_t n = 0; n < count; ++n) {
            ecs::Id id{};
            Knowledge value;
            if (!r.u64(id.value) || !(last < id) || !person(w, id) || !ecs::read_component(value, r, entries))
                return fail("invalid knowledge record");
            last = id;
            if (value.performed >= (1U << 21U) || value.next_work == 0 || value.next_memory == 0 ||
                value.curiosity > 100 || value.kindness > 100 || value.curiosity_need > 100 || value.mood > 100 ||
                value.learning_ppm < 0 || value.learning_ppm > 10000000 || value.settled < 0 ||
                value.settled > w.frontier() ||
                value.hourly_draw > static_cast<std::uint64_t>(w.frontier() / 3600 + 1) ||
                value.curiosity_remainder < 0 || value.curiosity_remainder >= time::kDay ||
                !person(w, value.watching, true) || value.watching == id)
                return fail("invalid knowledge quantities");
            for (const auto& p : value.sectors)
                if (!practice(p, w.frontier())) return fail("invalid sector practice");
            std::set<std::tuple<std::uint32_t, std::uint32_t, std::uint8_t>> seen;
            for (const auto& f : value.familiar)
                if (!familiar(f, w.frontier()) || !seen.insert({f.kind, f.material, f.state}).second)
                    return fail("invalid familiar evidence");
            std::set<std::uint32_t> skills;
            for (const auto& s : value.skills)
                if (!skills.insert(s.recipe).second || !practice(s.practice, w.frontier()) ||
                    s.observation_quarters > 20 || s.observation_remainder < 0 || s.observation_remainder >= 1000000 ||
                    s.route > 5 || s.known > 1 || !person(w, s.source, true))
                    return fail("invalid skill evidence");
            std::set<std::uint64_t> memories;
            for (const auto& m : value.memories) {
                if (m.id == 0 || m.id >= value.next_memory || !memories.insert(m.id).second || m.at < 0 ||
                    m.at > w.frontier() || m.action > 20 || m.sign > 18 || m.strength > 100 || m.certainty > 100 ||
                    !point(w, m.place) || !item(w, m.result, true))
                    return fail("invalid handling memory");
                for (const auto& f : m.inputs)
                    if (!familiar(f, m.at)) return fail("invalid memory input");
                for (const auto& p : m.participants)
                    if (!person(w, p.id)) return fail("invalid memory participant");
            }
            std::set<std::uint64_t> dream_hunches;
            for (const auto& h : value.hunches) {
                if (h.origin > 2 ||
                    (h.origin == 2 && (h.source.value != 0 || h.id == 0 || h.id != h.source_memory ||
                                       !dream_hunches.insert(h.id).second)) ||
                    (h.origin != 2 && h.id != 0) || h.action > 20 || h.result_form > 14 || h.failures >= 10 ||
                    h.last_use < 0 || h.last_use > w.frontier() || !person(w, h.source, true) ||
                    (h.source_memory == 0 || h.source_memory >= value.next_memory))
                    return fail("invalid hunch source");
                for (const auto& f : h.inputs)
                    if (!familiar(f, w.frontier())) return fail("invalid hunch input");
            }
            for (const auto& reason : value.reasons)
                if (!reason_valid(w, reason)) return fail("invalid craft choice");
            std::pair<ecs::Id, std::uint32_t> previous_peer{};
            for (const auto& peer : value.peers) {
                const auto key = std::pair{peer.person, peer.recipe};
                if (!(previous_peer < key) || !person(w, peer.person) || peer.person == id || peer.knows > 1 ||
                    peer.route < 1 || peer.route > 3 || peer.at < 0 || peer.at > w.frontier())
                    return fail("invalid peer knowledge evidence");
                previous_peer = key;
            }
            ecs::Id previous_observer{};
            for (const auto& seen : value.observations) {
                if (!(previous_observer < seen.person) || !person(w, seen.person) || seen.person == id ||
                    seen.work == 0 || seen.attempt == 0 || seen.settled < 0 || seen.settled > w.frontier() ||
                    seen.weighted_seconds < 0 || seen.weighted_seconds > 4 * time::kHour)
                    return fail("invalid partial work observation");
                previous_observer = seen.person;
            }
            raw.emplace<Knowledge>(w.beings().handle(id), std::move(value));
        }
        if (!r.finished()) return fail("trailing knowledge records");
    }
    {
        ByteReader r(save::find_chunk(chunks, save::tag("HIST"))->data);
        std::uint64_t count = 0;
        ecs::Id last{};
        if (!r.u64(count) || count != raw.view<Camp>().size()) return fail("invalid craft history count");
        for (std::uint64_t n = 0; n < count; ++n) {
            ecs::Id id{};
            CraftHistory value;
            const auto h = [&]() {
                if (!r.u64(id.value)) return std::optional<Beings::Handle>{};
                return w.beings().find(id);
            }();
            if (!h || !(last < id) || !raw.all_of<Camp>(*h) || !ecs::read_component(value, r, entries) ||
                value.next == 0)
                return fail("invalid craft history record");
            last = id;
            std::uint64_t previous = 0;
            for (const auto& e : value.events) {
                if (e.id <= previous || e.id >= value.next || e.at < 0 || e.at > w.frontier() || !point(w, e.place) ||
                    !person(w, e.actor) || !person(w, e.source, true) || !item(w, e.result, true) || e.route > 5 ||
                    e.kind > 5 || e.noticed > 1 || e.inputs.empty() || (e.noticed && e.word.empty()))
                    return fail("invalid craft result history");
                previous = e.id;
                for (const auto& input : e.inputs)
                    if (!item(w, input.id)) return fail("orphan history input");
            }
            std::uint64_t last_choice = 0;
            for (const auto& choice : value.choices) {
                if (choice.id != last_choice + 1 || choice.id >= value.next_choice || choice.at < 0 ||
                    choice.at > w.frontier() || !person(w, choice.actor) ||
                    raw.get<demo::Home>(w.beings().handle(choice.actor)).camp != id || choice.reasons.size() != 3 ||
                    !std::all_of(choice.reasons.begin(), choice.reasons.end(),
                                 [&](const auto& reason) { return reason_valid(w, reason); }))
                    return fail("invalid kept choice");
                last_choice = choice.id;
            }
            if (value.next_choice != value.choices.size() + 1) return fail("invalid next choice identity");
            for (const auto& event : value.events)
                if (event.choice) {
                    if (event.choice > value.choices.size()) return fail("result choice identity disagrees");
                    const auto& choice = value.choices[event.choice - 1];
                    if ((choice.actor != event.actor && choice.actor != event.source) || choice.at > event.at)
                        return fail("result choice identity disagrees");
                }
            raw.emplace<CraftHistory>(*h, std::move(value));
        }
        if (!r.finished()) return fail("trailing craft history records");
    }
    // Cross references below may only dereference complete extension owners.
    bool complete = true;
    w.beings().each([&](ecs::Id, Beings::Handle h) {
        if (raw.all_of<Person>(h) && !raw.all_of<Knowledge, Work, Life, demo::Home, Activity>(h)) complete = false;
        if (raw.all_of<Camp>(h) && !raw.any_of<CraftHistory, Knowledge, Work>(h)) complete = false;
    });
    if (!complete) return fail("missing personal craft state or camp history");
    {
        ByteReader r(save::find_chunk(chunks, save::tag("LEAR"))->data);
        std::uint64_t count = 0;
        ecs::Id last{};
        if (!r.u64(count) || count != raw.view<Camp>().size()) return fail("invalid learning camp count");
        for (std::uint64_t n = 0; n < count; ++n) {
            ecs::Id id{};
            Lessons value;
            if (!r.u64(id.value) || !(last < id)) return fail("invalid learning camp order");
            const auto h = w.beings().find(id);
            if (!h || !raw.all_of<Camp>(*h) || !ecs::read_component(value, r, entries) || value.next == 0)
                return fail("invalid learning camp record");
            last = id;
            std::uint64_t previous = 0;
            std::set<ecs::Id> participants;
            for (const auto& session : value.sessions) {
                if (session.id <= previous || session.id >= value.next || !person(w, session.teacher) ||
                    !person(w, session.learner) || session.teacher == session.learner ||
                    !participants.insert(session.teacher).second || !participants.insert(session.learner).second ||
                    !point(w, session.meeting) || session.state > 2 || session.offered < 0 ||
                    session.offered > w.frontier() || session.begun < 0 || session.begun > w.frontier() ||
                    (session.state != 0 && session.work != 0 && session.begun < session.offered) ||
                    session.settled < session.offered || session.settled > w.frontier() || session.end < 0 ||
                    session.seconds < 0 || session.begun > session.settled ||
                    session.seconds > session.settled - session.begun || session.seconds > 1800 ||
                    session.credited_seconds < 0 || session.credited_seconds > session.seconds ||
                    (session.state == 0 && (session.seconds != 0 || session.begun != 0)) ||
                    (session.state == 1 &&
                     (session.end < w.frontier() || session.end != session.settled + 1800 - session.seconds)))
                    return fail("invalid shared practice progress or participants");
                previous = session.id;
                const auto centre = raw.get<Place>(*h).at;
                const auto offset = w.torus().offset(centre, session.meeting);
                const auto& patch = raw.get<Camp>(*h);
                if (std::abs(offset.dx) > patch.half_width_cm || std::abs(offset.dy) > patch.half_height_cm ||
                    demo::Living::route(w, id, centre, session.meeting).empty())
                    return fail("shared practice meeting is outside reachable camp");
                const auto teacher = w.beings().handle(session.teacher);
                const auto learner = w.beings().handle(session.learner);
                if (!raw.all_of<demo::Home, Knowledge, Work, Activity>(teacher) ||
                    !raw.all_of<demo::Home, Knowledge, Work, Activity>(learner))
                    return fail("shared practice participant is missing required state");
                if (raw.get<demo::Home>(teacher).camp != id || raw.get<demo::Home>(learner).camp != id ||
                    raw.get<Knowledge>(teacher).session != session.id ||
                    raw.get<Knowledge>(learner).session != session.id || raw.get<Knowledge>(teacher).kindness < 60 ||
                    std::none_of(raw.get<Knowledge>(teacher).skills.begin(), raw.get<Knowledge>(teacher).skills.end(),
                                 [&](const auto& s) { return s.recipe == session.recipe && s.known; }))
                    return fail("shared practice disagrees with personal knowledge");
                const auto& work = raw.get<Work>(learner);
                if ((session.work == 0 && (session.state != 2 || work.lesson != 0)) ||
                    (session.work != 0 && (work.number != session.work || work.lesson != session.id || !work.intended ||
                                           work.recipe != session.recipe || work.route != 5)) ||
                    (session.state == 2 && work.lesson != 0 && work.state != 4))
                    return fail("shared practice disagrees with learner work");
                if (session.state < 2 &&
                    (raw.get<Activity>(teacher).what != static_cast<std::uint8_t>(LivingAct::teach) ||
                     raw.get<Activity>(teacher).end != session.end ||
                     (session.state == 1 &&
                      (work.state != 2 || work.end != session.end ||
                       raw.get<Activity>(learner).what != static_cast<std::uint8_t>(LivingAct::craft))) ||
                     (session.state == 0 &&
                      (work.state != 1 ||
                       (raw.get<Activity>(learner).what != static_cast<std::uint8_t>(LivingAct::walk) &&
                        raw.get<Activity>(learner).what != static_cast<std::uint8_t>(LivingAct::carry))))))
                    return fail("shared practice disagrees with activity events");
                if (session.state == 1 &&
                    (w.torus().squared_distance(raw.get<Place>(teacher).at, raw.get<Place>(learner).at) > 200LL * 200 ||
                     !demo::Living::visible(w, id, raw.get<Place>(teacher).at, raw.get<Place>(learner).at)))
                    return fail("shared practice participants are separated");
                if (session.last_try != 0) {
                    const auto& history = raw.get<CraftHistory>(*h);
                    if (std::none_of(history.events.begin(), history.events.end(), [&](const auto& e) {
                            return e.id == session.last_try && e.actor == session.learner &&
                                   e.recipe == session.recipe && e.route == 5 && e.at >= session.begun &&
                                   e.at <= session.settled;
                        }))
                        return fail("shared practice has no actual credited try");
                }
            }
            raw.emplace<Lessons>(*h, std::move(value));
        }
        if (!r.finished()) return fail("trailing learning records");
    }
    bool valid = true;
    std::map<ecs::Id, std::int64_t> reserved;
    std::set<ecs::Id> tools;
    w.things().each([&](ecs::Id id, Things::Handle h) {
        const auto* x = things.try_get<Item>(h);
        if (!x) {
            valid = false;
            return;
        }
        std::set<ecs::Id> parents;
        for (const auto& p : x->parents)
            if (!(p.id < id) || !item(w, p.id) || !parents.insert(p.id).second) valid = false;
        const auto home = w.beings().handle(x->home);
        const auto& camp = raw.get<Camp>(home);
        const auto offset = w.torus().offset(raw.get<Place>(home).at, things.get<Place>(h).at);
        if (std::abs(offset.dx) > camp.half_width_cm || std::abs(offset.dy) > camp.half_height_cm) valid = false;
    });
    const auto events = w.queue().live_in_order([&](const auto& e) { return w.live(e); });
    w.beings().each([&](ecs::Id id, Beings::Handle h) {
        if (raw.all_of<Camp>(h) && !raw.all_of<CraftHistory, Lessons>(h)) valid = false;
        if (!raw.all_of<Person>(h)) return;
        if (!raw.all_of<Work, Knowledge>(h)) {
            valid = false;
            return;
        }
        const auto& work = raw.get<Work>(h);
        const auto& life = raw.get<Life>(h);
        const auto& act = raw.get<Activity>(h);
        const auto& schedule = raw.get<Schedule>(h);
        const auto& know = raw.get<Knowledge>(h);
        if (work.intended && work.lesson == 0 &&
            std::none_of(know.skills.begin(), know.skills.end(),
                         [&](const auto& s) { return s.recipe == work.recipe && s.known; }))
            valid = false;
        if (work.number >= know.next_work ||
            (work.state != 0 && (work.number == 0 || work.inputs.empty() || work.try_seconds == 0)))
            valid = false;
        if ((act.what == static_cast<std::uint8_t>(LivingAct::craft)) != (work.state == 2) || work.state == 3 ||
            (work.state == 2 && (work.end != act.end || work.active_start != act.start || work.end < w.frontier())) ||
            (schedule.expected[2] != 0 &&
             (work.state != 2 || !work.intended || work.next_try < w.frontier() || work.next_try >= work.end)))
            valid = false;
        if (work.state == 2 && work.intended && work.next_try < work.end && schedule.expected[2] == 0) valid = false;
        for (const auto& e : events)
            if (e.key.owner == id.value && e.slot == 2 && e.key.second != work.next_try) valid = false;
        std::set<ecs::Id> inputs;
        std::set<std::uint8_t> roles;
        for (const auto& r : work.inputs) {
            if (!item(w, r.item) || !inputs.insert(r.item).second || !roles.insert(r.role).second || r.role >= 8 ||
                r.retained > 1 || r.picked > 1 || r.return_shared > 1 || r.mass <= 0 || r.mass > 1000000000) {
                valid = false;
                continue;
            }
            const auto& x = things.get<Item>(w.things().handle(r.item));
            if (x.home != raw.get<demo::Home>(h).camp || (x.owner.value != 0 && x.owner != id) || r.mass > x.mass)
                valid = false;
            if (r.retained) {
                if (!tools.insert(r.item).second) valid = false;
            } else
                reserved[r.item] += r.mass;
        }
        if (life.meal_item.value != 0) {
            if (!item(w, life.meal_item) || life.carried_food <= 0 || inputs.contains(life.meal_item) ||
                (act.what != 5 && act.what != 1 && act.what != 7) || work.state == 1 || work.state == 2) {
                valid = false;
            } else {
                const auto& meal = things.get<Item>(w.things().handle(life.meal_item));
                if (meal.home != raw.get<demo::Home>(h).camp || (meal.owner.value != 0 && meal.owner != id))
                    valid = false;
                reserved[life.meal_item] += life.carried_food;
            }
        }
        const auto camp = raw.get<demo::Home>(h).camp;
        const auto& history = raw.get<CraftHistory>(w.beings().handle(camp));
        const auto choice_exists = [&](std::uint64_t n) {
            return n == 0 || std::any_of(history.choices.begin(), history.choices.end(),
                                         [&](const auto& choice) { return choice.id == n && choice.actor == id; });
        };
        if (!choice_exists(work.choice) || !choice_exists(know.choice) || (!know.choice && !know.reasons.empty()))
            valid = false;
        if (know.choice) {
            const auto chosen = std::find_if(history.choices.begin(), history.choices.end(),
                                             [&](const auto& choice) { return choice.id == know.choice; });
            if (chosen != history.choices.end()) {
                ByteWriter current, kept;
                for (const auto& reason : know.reasons) ecs::write_component(reason, current);
                for (const auto& reason : chosen->reasons) ecs::write_component(reason, kept);
                const auto current_bytes = current.take();
                const auto kept_bytes = kept.take();
                if (current_bytes != kept_bytes) valid = false;
            }
        }
        const auto event_exists = [&](std::uint64_t n) {
            return n == 0 ||
                   std::any_of(history.events.begin(), history.events.end(), [&](const auto& e) { return e.id == n; });
        };
        for (const auto& s : know.skills) {
            if (!event_exists(s.source_event)) valid = false;
            if (s.source_event != 0 && std::none_of(history.events.begin(), history.events.end(), [&](const auto& e) {
                    return e.id == s.source_event && e.recipe == s.recipe;
                }))
                valid = false;
        }
        const auto& sessions = raw.get<Lessons>(w.beings().handle(camp)).sessions;
        const auto session = std::find_if(sessions.begin(), sessions.end(), [&](const auto& x) {
            return x.id == know.session && (x.teacher == id || x.learner == id);
        });
        if ((know.session != 0 && session == sessions.end()) ||
            (work.lesson != 0 && (session == sessions.end() || session->learner != id || work.lesson != session->id)) ||
            (act.what == static_cast<std::uint8_t>(LivingAct::teach) &&
             (session == sessions.end() || session->teacher != id || session->state == 2)) ||
            ((act.what == static_cast<std::uint8_t>(LivingAct::watch_craft)) != (know.watching.value != 0)) ||
            !event_exists(know.last_observed_event))
            valid = false;
        for (const auto& peer : know.peers) {
            if (!event_exists(peer.event) || (peer.route == 3 && peer.event != 0) ||
                (peer.route != 3 && peer.event == 0))
                valid = false;
            // The evidence record checks the route, never another person's private skill table.
            if ((peer.route == 1 && !peer.knows) || (peer.route == 2 && peer.knows)) valid = false;
            if (peer.event != 0 && std::none_of(history.events.begin(), history.events.end(), [&](const auto& e) {
                    return e.id == peer.event && e.actor == peer.person && e.recipe == peer.recipe && e.at == peer.at &&
                           (peer.route == 2 ? e.kind == 5 : e.kind == 0 || e.kind == 1);
                }))
                valid = false;
        }
        for (const auto& observation : know.observations) {
            const auto other = w.beings().handle(observation.person);
            if (raw.get<demo::Home>(other).camp != camp || observation.work >= raw.get<Knowledge>(other).next_work)
                valid = false;
            if (observation.weighted_seconds != 0) {
                const auto* source_work = raw.try_get<Work>(other);
                if (!source_work) {
                    valid = false;
                    continue;
                }
                const auto& source = *source_work;
                if (source.number != observation.work || (source.state != 2 && source.state != 4) ||
                    observation.attempt != source.completed_tries + 1 || observation.settled < source.start ||
                    observation.weighted_seconds > 4 * (observation.settled - source.start) ||
                    observation.weighted_seconds > 4 * source.try_seconds)
                    valid = false;
            }
        }
        const auto evidence_valid = [&](const Familiar& f) {
            for (std::size_t i = 0; i < 18; ++i)
                if (!person(w, f.source_people[i], true) || !event_exists(f.source_events[i])) valid = false;
        };
        for (const auto& f : know.familiar) evidence_valid(f);
        for (const auto& m : know.memories) {
            if (!event_exists(m.event)) valid = false;
            for (const auto& f : m.inputs) evidence_valid(f);
        }
        for (const auto& hunch : know.hunches)
            for (const auto& f : hunch.inputs) evidence_valid(f);
    });
    for (const auto& [id, mass] : reserved)
        if (mass > things.get<Item>(w.things().handle(id)).mass || tools.contains(id)) valid = false;
    if (!valid) return fail("invalid craft links, reservations or queued work");
    return true;
}
}  // namespace kd::world
