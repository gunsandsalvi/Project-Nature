// Ordinary dream thoughts contain no sender. Only the separate private ledger records your acts (GOD-06).
#pragma once
#include <array>
#include <vector>
#include "kd/world/camp.hpp"
#include "kd/world/craft.hpp"
#include "kd/world/parts.hpp"
namespace kd::world {
struct IdeaInput {
    static constexpr std::string_view name = "idea-input";
    static constexpr std::uint32_t version = 1;
    std::uint32_t kind = 0, material = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.entry({"kind", "handled input kind"}, c.kind, "item");
        v.entry({"material", "handled input material"}, c.material, "item");
    }
};
struct IdeaFields {
    std::uint8_t kind = 0, action = 0, desired_property = 0;  // place or idea
    std::uint64_t memory = 0, hunch_id = 0;
    std::uint32_t recipe = kNoRecipe;  // internal fit, never personal recipe knowledge
    std::int64_t first_attempt_at = -1;
    std::vector<IdeaInput> inputs;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u8({"kind", "place or idea"}, c.kind);
        v.u64({"memory", "stable performed-action memory"}, c.memory);
        v.u8({"action", "remembered performed action"}, c.action);
        v.u8({"desired_property", "already experienced benefit"}, c.desired_property);
        if (c.recipe != kNoRecipe)
            v.entry({"recipe", "internal compatible route, not knowledge"}, c.recipe, "blueprint");
        else
            v.u32({"recipe", "no compatible route sentinel"}, c.recipe);
        v.u64({"hunch_id", "ordinary hunch identity, no sender"}, c.hunch_id);
        v.i64({"first_attempt_at", "first actual matching action, or none"}, c.first_attempt_at);
        v.records({"inputs", "remembered input kinds"}, c.inputs, 2);
    }
};
struct Dream : IdeaFields {
    static constexpr std::string_view name = "dream";
    static constexpr std::uint32_t version = 2;
    std::int64_t night = -2, at = -1, until = -1, subject = -1;
    num::Point place{};
    std::int64_t decision_pull = 0, decision_subject = -1, visit_at = -1;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.i64({"night", "last ordinary sleep dream night"}, c.night);
        v.i64({"at", "remembered dream second"}, c.at);
        v.i64({"until", "ordinary thought expiry"}, c.until);
        v.i64({"subject", "remembered site, or none"}, c.subject);
        v.point({"place", "remembered site position"}, c.place);
        v.i64({"decision_pull", "dream contribution to current choice"}, c.decision_pull);
        v.i64({"decision_subject", "site remembered at the current choice"}, c.decision_subject);
        v.i64({"visit_at", "last actual visit to dreamed site"}, c.visit_at);
        IdeaFields::visit(v, c);
    }
};
struct DreamAct : IdeaFields {
    static constexpr std::string_view name = "private_dream_act";
    static constexpr std::uint32_t version = 2;
    std::uint64_t number = 0, person = 0;
    std::int64_t requested = 0, received = 0, executed = -1, until = -1, subject = 0;
    num::Point place{};
    std::uint8_t status = 1, reason = 0;  // pending, delivered, cancelled; gone or nightly cap
    std::int64_t decision_at = -1, choice = -1, pull = 0, visited_at = -1;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"number", "private request number"}, c.number);
        v.u64({"person", "target identity"}, c.person);
        v.i64({"requested", "displayed request second"}, c.requested);
        v.i64({"received", "command execution second"}, c.received);
        v.i64({"executed", "sleep dream second, or pending"}, c.executed);
        v.i64({"until", "delivered influence expiry"}, c.until);
        v.i64({"subject", "chosen known site"}, c.subject);
        v.point({"place", "site when requested"}, c.place);
        v.u8({"status", "pending, delivered or cancelled"}, c.status);
        v.u8({"reason", "cancellation reason"}, c.reason);
        v.i64({"decision_at", "observed subsequent choice second"}, c.decision_at);
        v.i64({"choice", "observed choice, or visit"}, c.choice);
        v.i64({"pull", "ordinary dream contribution at first waking choice"}, c.pull);
        v.i64({"visited_at", "observed actual arrival second"}, c.visited_at);
        IdeaFields::visit(v, c);
    }
};
struct Dreams {
    struct EndedPerson {
        ecs::Id id{};
        Person person{};
        std::int64_t ended_at = 0;
        std::uint64_t next_memory = 0;  // zero for a camp without the knowledge system
    };
    std::int64_t night = -2;
    std::array<std::uint64_t, 3> sent{};
    std::vector<DreamAct> acts{};      // private, persisted in full; only pending work is capped
    std::vector<EndedPerson> ended{};  // typed identities, in ID order; home is this ledger's camp
};
}  // namespace kd::world
