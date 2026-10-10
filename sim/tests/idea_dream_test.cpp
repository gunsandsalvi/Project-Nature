// Idea record faults and conditional dream scenes; no scripted discovery or guaranteed result.
#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/living.hpp"
#include "kd/save/snapshot.hpp"
namespace {
const kd::data::Catalogue& idea_catalogue() {
    static const auto catalogue = [] {
        kd::data::Catalogue out;
        REQUIRE(out.load(kd::data::read_catalogue(std::string(KD_REPO) + "/data")).empty());
        return out;
    }();
    return catalogue;
}
std::uint32_t idea_entry(std::string_view folder, std::string_view name) {
    const auto found = idea_catalogue().find(folder, name);
    REQUIRE(found);
    return found.value_or(0);
}
struct StoredIdea {
    kd::demo::CrowdWorld camp{73, idea_catalogue(), 1, true, true};
    kd::ecs::Id person{}, home{camp.camp_ids().front()};
    explicit StoredIdea(bool delivered) {
        auto& w = camp.world();
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (!person.value && w.beings().raw().all_of<kd::world::Person>(h)) person = id;
        });
        const auto command = w.command(0, 3, person.value, 1);
        w.run_to(1);
        auto& raw = w.beings().raw();
        const auto ph = w.beings().handle(person);
        // Labelled reader state: memory 1 has been lost after queuing. It must reopen for sleep cancellation.
        raw.get<kd::world::Knowledge>(ph).next_memory = 2;
        kd::world::DreamAct act;
        act.number = command.number;
        act.person = person.value;
        act.kind = 1;
        act.memory = 1;
        act.subject = -1;
        act.action = 11;
        act.desired_property = 12;
        act.recipe = idea_entry("blueprint", "base:ember_drill");
        act.inputs = {{idea_entry("item", "base:dry_stick"), idea_entry("item", "base:dry_stick")},
                      {idea_entry("item", "base:dry_board"), idea_entry("item", "base:dry_board")}};
        act.place = raw.get<kd::world::Place>(ph).at;
        auto& ledger = raw.get<kd::world::Dreams>(w.beings().handle(home));
        if (delivered) {
            act.status = 2;
            act.executed = 0;
            act.until = kd::demo::Living::kDreamLife;
            act.hunch_id = 1;
            ledger.night = kd::demo::Living::night(0);
            ledger.sent[0] = person.value;
            auto& thought = raw.get<kd::world::Dream>(ph);
            static_cast<kd::world::IdeaFields&>(thought) = act;
            thought.night = ledger.night;
            thought.at = 0;
            thought.until = act.until;
            thought.subject = -1;
            thought.place = act.place;
        }
        ledger.acts.push_back(act);
    }
    kd::world::DreamAct& act() {
        auto& w = camp.world();
        return w.beings().raw().get<kd::world::Dreams>(w.beings().handle(home)).acts.front();
    }
};
std::unique_ptr<kd::demo::CrowdWorld> open_idea(const kd::world::World& w, std::string& why) {
    const auto decoded = kd::save::read_snapshot(kd::save::write_snapshot(w.save()), why);
    REQUIRE(decoded);
    if (!decoded) return {};
    return kd::demo::CrowdWorld::open(idea_catalogue(), *decoded, why);
}
}  // namespace
TEST_CASE("DRMS2 preserves pending and delivered idea fields without granting recipe knowledge") {
    for (const bool delivered : {false, true}) {
        StoredIdea fixture(delivered);
        auto& w = fixture.camp.world();
        const auto chunks = w.save();
        const auto* dreams = kd::save::find_chunk(chunks, kd::save::tag("DRMS"));
        REQUIRE(dreams);
        CHECK(dreams->version == 2);
        CHECK(kd::save::kSnapshotVersion == 5);
        std::string why;
        auto copy = open_idea(w, why);
        INFO(why);
        REQUIRE(copy);
        CHECK(copy->world().digests().whole == w.digests().whole);
        CHECK(kd::save::write_snapshot(copy->world().save()) == kd::save::write_snapshot(w.save()));
        const auto& knowledge =
            copy->world().beings().raw().get<kd::world::Knowledge>(copy->world().beings().handle(fixture.person));
        CHECK(knowledge.skills.size() == 5);
        CHECK(std::none_of(knowledge.skills.begin(), knowledge.skills.end(),
                           [&](const auto& s) { return s.known && s.recipe == fixture.act().recipe; }));
    }
}
TEST_CASE("DRMS2 rejects invalid idea kinds references attempt times and duplicated pull") {
    for (int fault = 0; fault < 10; ++fault) {
        StoredIdea fixture(true);
        auto& act = fixture.act();
        auto& w = fixture.camp.world();
        if (fault == 0) act.kind = 2;
        if (fault == 1) act.memory = 0;
        if (fault == 2) act.action = 21;
        if (fault == 3) act.desired_property = 18;
        if (fault == 4) act.recipe = idea_entry("blueprint", "base:ember_plough");
        if (fault == 5) act.inputs.clear();
        if (fault == 6) act.hunch_id = 0;
        if (fault == 7) act.first_attempt_at = 2;
        if (fault == 8) act.memory = 2;
        if (fault == 9) w.beings().raw().get<kd::world::Dream>(w.beings().handle(fixture.person)).decision_pull = 120;
        std::string why;
        INFO(fault);
        CHECK_FALSE(open_idea(w, why));
        CHECK_FALSE(why.empty());
    }
}
