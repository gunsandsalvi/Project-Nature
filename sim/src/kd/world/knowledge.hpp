// Personal evidence and durable skill records. No global recipe unlocks or mind sentences.
#pragma once
#include <array>
#include "kd/world/craft.hpp"
namespace kd::world {
struct Familiar {
    static constexpr std::string_view name = "familiar";
    static constexpr std::uint32_t version = 1;
    std::uint32_t kind = 0, material = 0, mask = 0;
    std::array<std::uint8_t, 18> values{}, certainty{};
    std::uint8_t edible = 0;
    std::int64_t at = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.entry({"kind", "seen item kind"}, c.kind, "item");
        v.entry({"material", "seen material"}, c.material, "item");
        v.u32({"mask", "known characteristic mask"}, c.mask);
        for (auto& x : c.values) v.u8({"value", "perceived characteristic"}, x);
        for (auto& x : c.certainty) v.u8({"certainty", "evidence certainty"}, x);
        v.u8({"edible", "believed ready to eat"}, c.edible);
        v.i64({"at", "last evidence second"}, c.at);
    }
};
struct Practice {
    static constexpr std::string_view name = "practice";
    static constexpr std::uint32_t version = 1;
    std::int64_t level = 0, best = 0, seconds = 0, last_use = -1;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"level", "thousandths of a level"}, c.level);
        v.i64({"best", "best recorded thousandths"}, c.best);
        v.i64({"seconds", "effective practice seconds"}, c.seconds);
        v.i64({"last_use", "last real use, minus one when unused"}, c.last_use);
    }
};
struct Skill {
    static constexpr std::string_view name = "skill";
    static constexpr std::uint32_t version = 1;
    std::uint32_t recipe = 0, observation_quarters = 0;
    std::int64_t observation_remainder = 0;
    Practice practice;
    ecs::Id source{};
    std::uint64_t source_event = 0;
    std::uint8_t route = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.entry({"recipe", "personally learned recipe"}, c.recipe, "blueprint");
        Practice::visit(v, c.practice);
        v.id({"source", "actual learning source"}, c.source);
        v.u64({"source_event", "evidence event, zero for starting knowledge"}, c.source_event);
        v.u8({"route", "starting, own accident, experiment, hunch, watched or taught"}, c.route);
        v.u32({"observation_quarters", "credited observed quarters"}, c.observation_quarters);
        v.i64({"observation_remainder", "millionths of an observation quarter"}, c.observation_remainder);
    }
};
struct Memory {
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
    static constexpr std::string_view name = "hunch";
    static constexpr std::uint32_t version = 1;
    std::uint8_t action = 0, result_form = 0, failures = 0;
    std::uint64_t source_memory = 0;
    ecs::Id source{};
    std::int64_t last_use = 0;
    std::vector<Familiar> inputs;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u8({"action", "action to try"}, c.action);
        v.u8({"result_form", "guessed physical result"}, c.result_form);
        v.u8({"failures", "failed attempts"}, c.failures);
        v.u64({"source_memory", "actual hint memory"}, c.source_memory);
        v.id({"source", "source person, if any"}, c.source);
        v.i64({"last_use", "last attempt or hint"}, c.last_use);
        v.records({"inputs", "familiar input kinds, never a hidden recipe"}, c.inputs, 2);
    }
};
struct CraftReason {
    static constexpr std::string_view name = "craft-reason";
    static constexpr std::uint32_t version = 1;
    std::uint8_t kind = 0, intended = 0, action = 0, need = 0;
    std::uint32_t recipe = kNoRecipe;
    std::int64_t score = 0, benefit = 0, seconds = 0;
    std::vector<Link> inputs;
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
    }
};
struct Knowledge {
    static constexpr std::string_view name = "knowledge";
    static constexpr std::uint32_t version = 1;
    std::uint32_t performed = 0;
    std::uint64_t next_memory = 1, next_work = 1, hourly_draw = 0;
    std::uint8_t curiosity = 50, kindness = 50, curiosity_need = 60, mood = 60;
    std::int64_t learning_ppm = 1000000, settled = 0;
    std::array<Practice, 15> sectors{};
    std::vector<Familiar> familiar;
    std::vector<Skill> skills;
    std::vector<Memory> memories;
    std::vector<Hunch> hunches;
    std::vector<CraftReason> reasons;
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
        for (auto& x : c.sectors) Practice::visit(v, x);
        v.records({"familiar", "known properties with evidence"}, c.familiar, 128);
        v.records({"skills", "personal recipe skills"}, c.skills, 128);
        v.records({"memories", "recent handling and surprise evidence"}, c.memories, 200);
        v.records({"hunches", "bounded guesses"}, c.hunches, 5);
        v.records({"reasons", "chosen and rejected actual options"}, c.reasons, 3);
    }
};
struct Result {
    static constexpr std::string_view name = "craft-result";
    static constexpr std::uint32_t version = 1;
    std::uint64_t id = 0;
    std::int64_t at = 0;
    num::Point place{};
    ecs::Id actor{}, source{}, result{};
    std::uint32_t recipe = 0;
    std::uint8_t route = 0, noticed = 0;
    std::vector<Link> inputs;
    std::string word;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"id", "actual result event identity"}, c.id);
        v.i64({"at", "result second"}, c.at);
        v.point({"place", "result position"}, c.place);
        v.id({"actor", "actual maker or noticer"}, c.actor);
        v.id({"source", "learning source, if any"}, c.source);
        v.id({"result", "physical result"}, c.result);
        v.entry({"recipe", "actual fit, for history only"}, c.recipe, "blueprint");
        v.u8({"route", "actual route"}, c.route);
        v.u8({"noticed", "actually noticed"}, c.noticed);
        v.records({"inputs", "actual input identities"}, c.inputs, 8);
        v.text({"word", "stored coined word"}, c.word, 64);
    }
};
struct CraftHistory {
    static constexpr std::string_view name = "craft-history";
    static constexpr std::uint32_t version = 1;
    std::uint64_t next = 1;
    std::vector<Result> events;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"next", "next actual result event"}, c.next);
        v.records({"events", "kept results and discoveries"}, c.events, 100000);
    }
};
}  // namespace kd::world
