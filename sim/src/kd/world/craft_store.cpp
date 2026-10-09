#include "kd/world/craft_store.hpp"
#include <map>
#include <set>
#include "kd/data/craft.hpp"
namespace kd::world {
std::uint32_t craft_features(const World& w) {
    return w.beings().raw().view<Knowledge>().empty() ? 0U : kCraft;
}
void save_craft(const World& w, std::vector<save::Chunk>& out) {
    if (craft_features(w) == 0) return;
    const auto& beings = w.beings();
    const auto& things = w.things();
    ByteWriter craft, know, history;
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
    beings.each([&](ecs::Id id, Beings::Handle h) {
        if (const auto* x = beings.raw().try_get<Knowledge>(h)) {
            know.u64(id.value);
            ecs::write_component(*x, know);
        }
        if (const auto* x = beings.raw().try_get<CraftHistory>(h)) {
            history.u64(id.value);
            ecs::write_component(*x, history);
        }
    });
    out.push_back({save::tag("CRFT"), 1, true, craft.take()});
    out.push_back({save::tag("KNOW"), 1, true, know.take()});
    out.push_back({save::tag("HIST"), 1, true, history.take()});
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
        if (!mask.u32(features) || (features != 0 && features != kCraft)) return fail("unsupported camp features");
    }
    for (const auto tag :
         {save::tag("LIFE"), save::tag("DRMS"), save::tag("CRFT"), save::tag("KNOW"), save::tag("HIST")}) {
        const auto* c = save::find_chunk(chunks, tag);
        const auto count = std::count_if(chunks.begin(), chunks.end(), [&](const auto& x) { return x.tag == tag; });
        if (count > 1) return fail("duplicate camp extension");
        const bool craft = tag == save::tag("CRFT") || tag == save::tag("KNOW") || tag == save::tag("HIST");
        if ((craft && ((features == kCraft) != bool(c))) || (features == kCraft && !c))
            return fail("required craft extension is missing or mismatched");
        if (c && (!c->critical || c->version != (tag == save::tag("LIFE") ? 2U : 1U)))
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
bool practice(const Practice& p, time::Seconds now) {
    return p.level >= 0 && p.level <= 10000 && p.best >= p.level && p.best <= 10000 && p.seconds >= 0 &&
           p.seconds <= 60 * time::kDay * 1000 && p.last_use >= -1 && p.last_use <= now;
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
                if (value.state > 4 || value.action > 20 || value.intended > 1 || value.route > 3 || value.rolled > 1 ||
                    (!value.intended && value.recipe != kNoRecipe) || value.start < 0 || value.start > w.frontier() ||
                    value.active_start < value.start || value.active_start > w.frontier() || value.end < 0 ||
                    value.next_try < 0 || value.next_try > value.end || value.retained_progress < 0 ||
                    value.try_seconds < 0 || value.try_seconds > 3600 || value.retained_progress > value.try_seconds ||
                    value.unit_mass < 0 || value.unit_mass > 1000000000 || value.goal_mass < 0 ||
                    value.goal_mass > 1000000000 || value.applied_marker > value.completed_tries ||
                    !point(w, value.target))
                    return fail("invalid work progress");
                if (value.state == 0 &&
                    (!value.inputs.empty() || value.number != 0 || value.intended || value.recipe != kNoRecipe ||
                     value.completed_tries != 0 || value.applied_marker != 0 || value.rolled ||
                     value.retained_progress != 0 || value.start != 0 || value.active_start != 0 || value.end != 0 ||
                     value.next_try != 0 || value.try_seconds != 0 || value.unit_mass != 0 || value.goal_mass != 0))
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
                value.settled > w.frontier() || value.hourly_draw > static_cast<std::uint64_t>(w.frontier() / 3600 + 1))
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
                    s.observation_quarters > 4 || s.observation_remainder < 0 || s.observation_remainder >= 1000000 ||
                    s.route > 5 || !person(w, s.source, true))
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
            for (const auto& h : value.hunches) {
                if (h.action > 20 || h.result_form > 14 || h.failures >= 10 || h.last_use < 0 ||
                    h.last_use > w.frontier() || !person(w, h.source, true) ||
                    (h.source_memory == 0 || h.source_memory >= value.next_memory))
                    return fail("invalid hunch source");
                for (const auto& f : h.inputs)
                    if (!familiar(f, w.frontier())) return fail("invalid hunch input");
            }
            for (const auto& reason : value.reasons) {
                if (reason.kind > 1 || reason.intended > 1 || reason.action > 20 || reason.need > 3 ||
                    (!reason.intended && reason.recipe != kNoRecipe) || reason.score < -1000000 ||
                    reason.score > 100000 || reason.benefit < 0 || reason.benefit > 100 || reason.seconds < 0 ||
                    reason.seconds > time::kDay * 2)
                    return fail("invalid craft choice");
                for (const auto& input : reason.inputs)
                    if (!item(w, input.id)) return fail("invalid choice input");
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
            raw.emplace<CraftHistory>(*h, std::move(value));
        }
        if (!r.finished()) return fail("trailing craft history records");
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
        if (raw.all_of<Camp>(h) && !raw.all_of<CraftHistory>(h)) valid = false;
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
        if (work.number >= know.next_work ||
            (work.state != 0 && (work.number == 0 || work.inputs.empty() || work.try_seconds == 0)))
            valid = false;
        if ((act.what >= 8) != (work.state == 2) || work.state == 3 ||
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
        const auto event_exists = [&](std::uint64_t n) {
            return n == 0 ||
                   std::any_of(history.events.begin(), history.events.end(), [&](const auto& e) { return e.id == n; });
        };
        for (const auto& s : know.skills)
            if (!event_exists(s.source_event)) valid = false;
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
