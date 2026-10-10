// Personal evidence and durable skill records. No global recipe unlocks or mind sentences.
#pragma once
#include <array>
#include "kd/core/pages.hpp"
#include "kd/world/craft.hpp"
namespace kd::world {
struct Familiar {
    friend bool operator==(const Familiar&, const Familiar&) = default;
    static constexpr std::string_view name = "familiar";
    static constexpr std::uint32_t version = 1;
    std::uint32_t kind = 0, material = 0, mask = 0;
    std::array<std::uint8_t, 18> values{}, certainty{};
    std::uint8_t edible = 0, state = 0, edible_source = 0;
    std::array<std::uint8_t, 18> sources{};
    std::array<std::int64_t, 18> learned_at{};
    std::array<ecs::Id, 18> source_people{};
    std::array<std::uint64_t, 18> source_events{};
    std::int64_t at = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.entry({"kind", "seen item kind"}, c.kind, "item");
        v.entry({"material", "seen material"}, c.material, "item");
        v.u32({"mask", "known characteristic mask"}, c.mask);
        for (auto& x : c.values) v.u8({"value", "perceived characteristic"}, x);
        for (auto& x : c.certainty) v.u8({"certainty", "evidence certainty"}, x);
        v.u8({"edible", "believed ready to eat"}, c.edible);
        v.u8({"state", "experienced physical state"}, c.state);
        v.u8({"edible_source", "starting or handling evidence for edible use"}, c.edible_source);
        for (std::size_t i = 0; i < 18; ++i) {
            v.u8({"source", "sight, starting knowledge, handling, outcome, watching or telling"}, c.sources[i]);
            v.i64({"learned_at", "actual evidence second"}, c.learned_at[i]);
            v.id({"source_person", "actual source person, or zero"}, c.source_people[i]);
            v.u64({"source_event", "actual evidence result, or zero"}, c.source_events[i]);
        }
        v.i64({"at", "last evidence second"}, c.at);
    }
};
struct Practice {
    friend bool operator==(const Practice&, const Practice&) = default;
    static constexpr std::string_view name = "practice";
    static constexpr std::uint32_t version = 2;
    std::int64_t level = 0, best = 0, seconds = 0, last_use = -1;
    std::int64_t fraction = 0, seconds_remainder = 0, scale_remainder = 0, decay_at = 0, decay_level = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"level", "thousandths of a level"}, c.level);
        v.i64({"best", "best recorded thousandths"}, c.best);
        v.i64({"seconds", "effective practice seconds"}, c.seconds);
        v.i64({"last_use", "last real use, minus one when unused"}, c.last_use);
        v.i64({"fraction", "effective microseconds toward the next skill thousandth"}, c.fraction);
        v.i64({"seconds_remainder", "fractional effective practice microseconds"}, c.seconds_remainder);
        v.i64({"scale_remainder", "fractional multiplier numerator"}, c.scale_remainder);
        v.i64({"decay_at", "fixed fading anchor second"}, c.decay_at);
        v.i64({"decay_level", "skill at the fixed fading anchor"}, c.decay_level);
    }
};
struct Skill {
    friend bool operator==(const Skill&, const Skill&) = default;
    static constexpr std::string_view name = "skill";
    static constexpr std::uint32_t version = 2;
    std::uint32_t recipe = 0, observation_quarters = 0;
    std::int64_t observation_remainder = 0;
    Practice practice;
    ecs::Id source{};
    std::uint64_t source_event = 0;
    std::uint8_t route = 0, known = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.entry({"recipe", "personally learned recipe"}, c.recipe, "blueprint");
        Practice::visit(v, c.practice);
        v.id({"source", "actual learning source"}, c.source);
        v.u64({"source_event", "evidence event, zero for starting knowledge"}, c.source_event);
        v.u8({"route", "starting, own accident, experiment, hunch, watched or taught"}, c.route);
        v.u8({"known", "knows the recipe; unfinished watching or teaching is not an unlock"}, c.known);
        v.u32({"observation_quarters", "credited observed quarters"}, c.observation_quarters);
        v.i64({"observation_remainder", "millionths of an observation quarter"}, c.observation_remainder);
    }
};
struct PeerBelief {
    friend bool operator==(const PeerBelief&, const PeerBelief&) = default;
    static constexpr std::string_view name = "peer-craft-belief";
    static constexpr std::uint32_t version = 1;
    ecs::Id person{};
    std::uint32_t recipe = 0;
    std::uint8_t knows = 0, route = 0;
    std::int64_t at = 0;
    std::uint64_t event = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"person", "person actually seen or spoken with"}, c.person);
        v.entry({"recipe", "own known recipe being discussed or demonstrated"}, c.recipe, "blueprint");
        v.u8({"knows", "belief about knowledge, zero for absence"}, c.knows);
        v.u8({"route", "demonstration, failed attempt or truthful exchange"}, c.route);
        v.i64({"at", "actual evidence second"}, c.at);
        v.u64({"event", "actual evidence result, or zero for exchange"}, c.event);
    }
};
struct Observation {
    friend bool operator==(const Observation&, const Observation&) = default;
    static constexpr std::string_view name = "work-observation";
    static constexpr std::uint32_t version = 1;
    ecs::Id person{};
    std::uint64_t work = 0, attempt = 0;
    std::int64_t settled = 0, weighted_seconds = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"person", "actual demonstrator"}, c.person);
        v.u64({"work", "demonstrator's unique work number"}, c.work);
        v.u64({"attempt", "actual demonstrated try number"}, c.attempt);
        v.i64({"settled", "exposure already considered through this second"}, c.settled);
        v.i64({"weighted_seconds", "visible seconds, weighted four for deliberate watching"}, c.weighted_seconds);
    }
};
struct Lesson {
    static constexpr std::string_view name = "shared-practice";
    static constexpr std::uint32_t version = 1;
    std::uint64_t id = 0, work = 0, last_try = 0;
    ecs::Id teacher{}, learner{};
    std::uint32_t recipe = 0;
    num::Point meeting{};
    std::uint8_t state = 0;
    std::int64_t offered = 0, begun = 0, settled = 0, end = 0;
    std::int64_t seconds = 0, credited_seconds = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"id", "camp's unique shared session"}, c.id);
        v.id({"teacher", "actual teacher"}, c.teacher);
        v.id({"learner", "actual learner"}, c.learner);
        v.entry({"recipe", "personally known recipe offered by the teacher"}, c.recipe, "blueprint");
        v.point({"meeting", "reachable meeting position"}, c.meeting);
        v.u8({"state", "meeting, practising or paused"}, c.state);
        v.i64({"offered", "actual offer second"}, c.offered);
        v.i64({"begun", "first shared work second"}, c.begun);
        v.i64({"settled", "session progress considered through this second"}, c.settled);
        v.i64({"end", "current shared work deadline"}, c.end);
        v.i64({"seconds", "earned shared practice seconds"}, c.seconds);
        v.i64({"credited_seconds", "elapsed practice already credited exactly once"}, c.credited_seconds);
        v.u64({"work", "learner's actual work number, zero while uncollected"}, c.work);
        v.u64({"last_try", "last demonstrated result credited exactly once"}, c.last_try);
    }
};
struct Lessons {
    static constexpr std::string_view name = "camp-learning";
    static constexpr std::uint32_t version = 1;
    std::uint64_t next = 1;
    std::vector<Lesson> sessions;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"next", "next shared-session identity"}, c.next);
        v.records({"sessions", "sorted live or interrupted shared sessions"}, c.sessions, 64);
    }
};
struct Memory {
    friend bool operator==(const Memory&, const Memory&) = default;
    static constexpr std::string_view name = "handling-memory";
    static constexpr std::uint32_t version = 1;
    std::uint64_t id = 0, event = 0;
    std::int64_t at = 0;
    num::Point place{};
    std::uint8_t action = 0, sign = 0, strength = 0, certainty = 0;
    std::vector<Familiar> inputs;
    ecs::Id result{};
    std::vector<Link> participants;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"id", "personal memory identity"}, c.id);
        v.u64({"event", "real result source, or zero for handling"}, c.event);
        v.i64({"at", "experience second"}, c.at);
        v.point({"place", "experience position"}, c.place);
        v.u8({"action", "actually performed base action"}, c.action);
        v.u8({"sign", "observed result sign"}, c.sign);
        v.u8({"strength", "memory importance"}, c.strength);
        v.u8({"certainty", "evidence certainty"}, c.certainty);
        v.records({"inputs", "perceived input kinds"}, c.inputs, 8);
        v.id({"result", "actual resulting thing, if any"}, c.result);
        v.records({"participants", "actual participants"}, c.participants, 5);
    }
};
struct Hunch {
    friend bool operator==(const Hunch&, const Hunch&) = default;
    static constexpr std::string_view name = "hunch";
    static constexpr std::uint32_t version = 2;
    std::uint64_t id = 0;
    std::uint8_t origin = 0;  // ordinary hint, telling, dream; never player provenance
    std::uint8_t action = 0, result_form = 0, failures = 0;
    std::uint64_t source_memory = 0;
    ecs::Id source{};
    std::int64_t last_use = 0;
    std::vector<Familiar> inputs;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u8({"action", "action to try"}, c.action);
        v.u64({"id", "ordinary dream hunch identity, or zero"}, c.id);
        v.u8({"origin", "ordinary hint, telling or dream"}, c.origin);
        v.u8({"result_form", "guessed physical result"}, c.result_form);
        v.u8({"failures", "failed attempts"}, c.failures);
        v.u64({"source_memory", "actual hint memory"}, c.source_memory);
        v.id({"source", "source person, if any"}, c.source);
        v.i64({"last_use", "last attempt or hint"}, c.last_use);
        v.records({"inputs", "familiar input kinds, never a hidden recipe"}, c.inputs, 2);
    }
};
struct CraftReason {
    friend bool operator==(const CraftReason&, const CraftReason&) = default;
    static constexpr std::string_view name = "craft-reason";
    static constexpr std::uint32_t version = 3;
    std::uint8_t kind = 0, intended = 0, action = 0, need = 0;
    std::uint32_t recipe = kNoRecipe;
    std::int64_t score = 0, benefit = 0, seconds = 0;
    std::vector<Link> inputs;
    std::uint8_t need_met = 100, confidence = 100, unavailable = 0, observed_heat = 0;
    std::int64_t observed_fuel_mg = 0;
    std::array<std::int64_t, 3> parts{};  // need benefit, inclination/plan, effort; sum is score
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u8({"kind", "known use or curiosity"}, c.kind);
        v.u8({"intended", "known recipe candidate"}, c.intended);
        if (c.intended)
            v.entry({"recipe", "known recipe only"}, c.recipe, "blueprint");
        else
            v.u32({"recipe", "no recipe sentinel"}, c.recipe);
        v.u8({"action", "known or experimental action"}, c.action);
        v.u8({"need", "actual motive"}, c.need);
        v.i64({"score", "recorded option score"}, c.score);
        v.i64({"benefit", "expected known benefit"}, c.benefit);
        v.i64({"seconds", "expected effort and travel"}, c.seconds);
        v.records({"inputs", "reachable seen inputs"}, c.inputs, 8);
        v.u8({"need_met", "personal satisfaction at choice"}, c.need_met);
        v.u8({"confidence", "personal estimate certainty"}, c.confidence);
        v.u8({"unavailable", "recorded reason an alternative was unavailable"}, c.unavailable);
        v.u8({"observed_heat", "visible heat band at choice"}, c.observed_heat);
        v.i64({"observed_fuel_mg", "visible hearth fuel at choice"}, c.observed_fuel_mg);
        v.i64({"need_score", "expected need benefit contribution"}, c.parts[0]);
        v.i64({"inclination_score", "inclination and plan contribution"}, c.parts[1]);
        v.i64({"effort_score", "effort contribution"}, c.parts[2]);
    }
};
struct Knowledge {
    friend bool operator==(const Knowledge&, const Knowledge&) = default;
    static constexpr std::string_view name = "knowledge";
    static constexpr std::uint32_t version = 3;
    std::uint32_t performed = 0;
    std::uint64_t next_memory = 1, next_work = 1, hourly_draw = 0;
    std::uint8_t curiosity = 50, kindness = 50, curiosity_need = 60, mood = 60;
    std::int64_t learning_ppm = 1000000, settled = 0;
    std::int64_t curiosity_remainder = 0;
    std::uint64_t session = 0, last_observed_event = 0;
    ecs::Id watching{};
    std::array<Practice, 15> sectors{};
    std::vector<Familiar> familiar;
    std::vector<Skill> skills;
    std::vector<Memory> memories;
    std::vector<Hunch> hunches;
    std::vector<CraftReason> reasons;
    std::uint64_t choice = 0;
    std::vector<PeerBelief> peers;
    std::vector<Observation> observations;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u32({"performed", "actions actually performed"}, c.performed);
        v.u64({"next_memory", "next personal memory number"}, c.next_memory);
        v.u64({"next_work", "next personal work number"}, c.next_work);
        v.u64({"hourly_draw", "last eligible curiosity draw"}, c.hourly_draw);
        v.u8({"curiosity", "founder curiosity"}, c.curiosity);
        v.u8({"kindness", "founder kindness"}, c.kindness);
        v.u8({"curiosity_need", "curiosity satisfaction"}, c.curiosity_need);
        v.u8({"mood", "scoped mood"}, c.mood);
        v.i64({"learning_ppm", "learning multiplier"}, c.learning_ppm);
        v.i64({"settled", "last curiosity settlement"}, c.settled);
        v.i64({"curiosity_remainder", "fractional daily curiosity settlement"}, c.curiosity_remainder);
        v.u64({"session", "own live or paused shared session, or zero"}, c.session);
        v.u64({"last_observed_event", "last demonstrated result already considered"}, c.last_observed_event);
        v.id({"watching", "actual deliberate watching target, or zero"}, c.watching);
        for (auto& x : c.sectors) Practice::visit(v, x);
        v.records({"familiar", "known properties with evidence"}, c.familiar, 128);
        v.records({"skills", "personal recipe skills"}, c.skills, 128);
        v.records({"memories", "recent handling and surprise evidence"}, c.memories, 200);
        v.records({"hunches", "bounded guesses"}, c.hunches, 5);
        v.records({"reasons", "chosen and rejected actual options"}, c.reasons, 3);
        v.u64({"choice", "immutable current choice identity, or zero"}, c.choice);
        v.records({"peers", "own evidence about a peer's craft knowledge"}, c.peers, 256);
        v.records({"observations", "unfinished exposure to actual manufacture"}, c.observations, 64);
    }
};
struct Result {
    static constexpr std::string_view name = "craft-result";
    static constexpr std::uint32_t version = 3;
    std::uint64_t id = 0, choice = 0;
    std::int64_t at = 0;
    num::Point place{};
    ecs::Id actor{}, source{}, result{};
    std::uint32_t recipe = 0;
    std::uint8_t route = 0, noticed = 0, kind = 0;
    std::vector<Link> inputs;
    std::string word;
    std::vector<HeatCredit> heat_sources;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"id", "actual result event identity"}, c.id);
        v.u64({"choice", "choice that led to this result, or zero for unplanned evidence"}, c.choice);
        v.i64({"at", "result second"}, c.at);
        v.point({"place", "result position"}, c.place);
        v.id({"actor", "actual maker or noticer"}, c.actor);
        v.id({"source", "learning source, if any"}, c.source);
        v.id({"result", "physical result"}, c.result);
        v.entry({"recipe", "actual fit, for history only"}, c.recipe, "blueprint");
        v.u8({"route", "actual route"}, c.route);
        v.u8({"noticed", "actually noticed"}, c.noticed);
        v.u8({"kind", "making, discovery, learning, loss, return or failure"}, c.kind);
        v.records({"inputs", "actual input identities"}, c.inputs, 8);
        v.text({"word", "stored coined word"}, c.word, 64);
        v.records({"heat_sources", "actual sources of cooked heat"}, c.heat_sources, 3600, 40);
    }
};
struct Choice {
    static constexpr std::string_view name = "choice";
    static constexpr std::uint32_t version = 1;
    std::uint64_t id = 0;
    std::int64_t at = 0;
    ecs::Id actor{};
    std::vector<CraftReason> reasons;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"id", "immutable choice identity"}, c.id);
        v.i64({"at", "actual choice second"}, c.at);
        v.id({"actor", "person making this choice"}, c.actor);
        v.records({"reasons", "winner and two actual rejected options"}, c.reasons, 3);
    }
};
struct PublicResult {
    std::uint64_t id = 0;
    time::Seconds at = 0;
};
struct CraftHistory {
    static constexpr std::string_view name = "craft-history";
    static constexpr std::uint32_t version = 2;
    std::uint64_t next = 1, next_choice = 1;
    Pages<Result> events;
    Pages<Choice> choices;
    mutable Pages<PublicResult> public_results;
    mutable std::uint64_t public_indexed = 0;
    [[nodiscard]] const Pages<PublicResult>& public_index() const {
        const auto first = std::upper_bound(events.begin(), events.end(), public_indexed,
                                            [](std::uint64_t id, const Result& event) { return id < event.id; });
        for (auto at = first; at != events.end(); ++at) {
            if (at->kind != 0 && at->kind != 5) public_results.push_back({at->id, at->at});
            public_indexed = at->id;
        }
        return public_results;
    }
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"next", "next actual result event"}, c.next);
        // History grows with real play; bound allocation by actual wire bytes, not an elapsed-play ceiling.
        v.records({"events", "kept results and discoveries"}, c.events, UINT32_MAX, 76);
        v.u64({"next_choice", "next immutable choice identity"}, c.next_choice);
        v.records({"choices", "kept fire, warmth and craft decisions"}, c.choices, UINT32_MAX, 204);
    }
};
}  // namespace kd::world
