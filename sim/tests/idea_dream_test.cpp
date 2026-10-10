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

#include "kd/demo/crafting.hpp"
#include "kd/demo/idea_dreams.hpp"
namespace {
struct RememberedWood {
    kd::demo::CrowdWorld camp;
    kd::ecs::Id person{}, home{};
    std::uint64_t remembered = 0;
    explicit RememberedWood(std::uint64_t seed = 81) : camp(seed, idea_catalogue(), 1, true, true) {
        auto& w = camp.world();
        home = camp.camp_ids().front();
        auto& raw = w.beings().raw();
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (!person.value && raw.all_of<kd::world::Person>(h)) person = id;
        });
        const auto ph = w.beings().handle(person);
        kd::num::Point at{};
        w.things().each([&](kd::ecs::Id, auto h) {
            if (w.things().raw().all_of<kd::world::Fire>(h)) at = w.things().raw().get<kd::world::Place>(h).at;
        });
        at = w.torus().moved(at, {150, 0});
        auto& work = raw.get<kd::world::Work>(ph);
        auto& know = raw.get<kd::world::Knowledge>(ph);
        work.state = 2;
        work.number = know.next_work++;
        work.action = 11;
        work.start = work.active_start = 0;
        work.end = 100;
        work.next_try = work.try_seconds = 300;
        work.unit_mass = work.goal_mass = 20000;
        work.target = at;
        // Conditional scene: real dry inputs, only 100 seconds of twirling, shorter than any fitted result.
        for (const auto name : {"base:dry_stick", "base:dry_board"}) {
            const auto kind = idea_entry("item", name);
            kd::ecs::Id chosen{};
            w.things().each([&](kd::ecs::Id id, auto h) {
                const auto& things = std::as_const(w).things().raw();
                if (!chosen.value && things.all_of<kd::world::Item>(h) && !things.all_of<kd::world::Fire>(h) &&
                    things.get<kd::world::Item>(h).kind == kind && things.get<kd::world::Item>(h).mass > 0)
                    chosen = id;
            });
            REQUIRE(chosen.value != 0);
            const auto ih = w.things().handle(chosen);
            auto& item = w.things().raw().get<kd::world::Item>(ih);
            item.owner = person;
            w.things().raw().get<kd::world::Place>(ih).at = at;
            work.inputs.push_back({chosen, item.mass, static_cast<std::uint8_t>(work.inputs.size()), 1, 1, 0});
        }
        raw.get<kd::world::Place>(ph).at = at;
        raw.get<kd::world::Activity>(ph) = {static_cast<std::uint8_t>(kd::world::LivingAct::craft), 0, 100, at, at};
        for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(person, slot);
        w.schedule(person, kd::world::kActivitySlot, 100);
        w.run_to(101);
        for (const auto& m : know.memories)
            if (m.action == 11 && m.inputs.size() == 2) remembered = m.id;
        REQUIRE(remembered != 0);
    }
    kd::world::Knowledge& know() {
        return camp.world().beings().raw().get<kd::world::Knowledge>(camp.world().beings().handle(person));
    }
    kd::world::Dream& thought() {
        return camp.world().beings().raw().get<kd::world::Dream>(camp.world().beings().handle(person));
    }
    kd::world::Dreams& ledger() {
        return camp.world().beings().raw().get<kd::world::Dreams>(camp.world().beings().handle(home));
    }
    void rest() {
        auto& w = camp.world();
        const auto ph = w.beings().handle(person);
        auto& raw = w.beings().raw();
        const auto at = raw.get<kd::world::Place>(ph).at;
        raw.get<kd::world::Work>(ph) = {};
        auto& life = raw.get<kd::world::Life>(ph);
        life.settled = 101;
        life.portion = life.applied = life.allocated_water = 0;
        raw.get<kd::world::Activity>(ph) = {2, 101, 7201, at, at};
        for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(person, slot);
        w.schedule(person, 0, 7201);
    }
};
}  // namespace
TEST_CASE("idea handled_memory_only pairs real twirling with actual experienced warmth") {
    RememberedWood f;
    auto& w = f.camp.world();
    const auto ph = w.beings().handle(f.person);
    const auto fit = kd::demo::IdeaDreams::fit(w, ph, f.remembered);
    REQUIRE(fit);
    if (!fit) return;
    CHECK(fit->action == 11);
    CHECK(fit->desired_property == 12);
    CHECK(fit->inputs.size() == 2);
    CHECK_FALSE(std::any_of(f.know().skills.begin(), f.know().skills.end(),
                            [&](const auto& skill) { return skill.known && skill.recipe == fit->recipe; }));
    auto& m = *std::find_if(f.know().memories.begin(), f.know().memories.end(),
                            [&](const auto& memory) { return memory.id == f.remembered; });
    m.inputs[0].sources.fill(1);  // bypassed input memory: seen, never handled
    CHECK_FALSE(kd::demo::IdeaDreams::fit(w, ph, f.remembered));
}
TEST_CASE("idea pending_idea_reopen cancels lost memory at sleep without granting knowledge") {
    RememberedWood f;
    auto& w = f.camp.world();
    REQUIRE(kd::demo::IdeaDreams::problem(w, f.person, f.remembered).empty());
    w.command(101, 3, f.person.value, f.remembered);
    w.run_to(102);
    REQUIRE(f.ledger().acts.size() == 1);
    if (f.ledger().acts.empty()) return;
    CHECK(f.ledger().acts.front().status == 1);
    std::string why;
    auto copy = open_idea(w, why);
    INFO(why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK(copy->world().digests().whole == w.digests().whole);
    auto& know = copy->world().beings().raw().get<kd::world::Knowledge>(copy->world().beings().handle(f.person));
    std::erase_if(know.memories, [&](const auto& m) { return m.id == f.remembered; });
    copy->world().run_to(2 * kd::time::kDay);
    const auto& act =
        copy->world().beings().raw().get<kd::world::Dreams>(copy->world().beings().handle(f.home)).acts.front();
    CHECK(act.status == 3);
    CHECK(act.reason == 4);
    CHECK(act.first_attempt_at == -1);
}
TEST_CASE("idea shared_nightly_cap refresh_not_stack and no_recipe_leak") {
    RememberedWood f;
    f.rest();
    auto& w = f.camp.world();
    const auto skills = f.know().skills.size();
    w.command(101, 3, f.person.value, f.remembered);
    w.run_to(102);
    REQUIRE(f.ledger().acts.size() == 1);
    if (f.ledger().acts.empty()) return;
    CHECK(f.ledger().acts.front().status == 2);
    CHECK(f.thought().kind == 1);
    CHECK(f.thought().subject == -1);
    CHECK(f.thought().first_attempt_at == -1);
    CHECK(f.know().skills.size() == skills);
    CHECK_FALSE(kd::demo::Living::dream_problem(w, f.person, 2).empty());
    const auto hints = f.know().hunches.size();
    w.command(102, 3, f.person.value, f.remembered);
    w.run_to(103);
    CHECK(f.ledger().acts.size() == 1);
    CHECK(f.know().hunches.size() == hints);
    std::string why;
    auto copy = open_idea(w, why);
    INFO(why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK(copy->world().digests().whole == w.digests().whole);
}
TEST_CASE("idea urgent_need_wins and actual attempts alone receive attribution") {
    for (const bool urgent : {false, true}) {
        RememberedWood f;
        f.rest();
        auto& w = f.camp.world();
        auto& life = w.beings().raw().get<kd::world::Life>(w.beings().handle(f.person));
        life.food = 4000000;
        life.water = urgent ? 300 : 3000;
        life.awake = 0;
        w.command(101, 3, f.person.value, f.remembered);
        w.run_to(7202);
        CHECK(f.thought().decision_pull == (urgent ? 0 : 60));
        CHECK(f.ledger().acts.front().first_attempt_at == -1);  // collection is not an attempt
        if (urgent) CHECK(f.ledger().acts.front().pull == 0);
        w.run_to(75000);
        if (!urgent) CHECK(f.ledger().acts.front().first_attempt_at >= 0);
        std::string why;
        auto copy = open_idea(w, why);
        INFO(why);
        REQUIRE(copy);
    }
}
namespace {
class OrdinaryIdea final : public kd::world::System {
public:
    [[nodiscard]] std::string_view name() const override { return "idea constructor fixture"; }
    void handle(kd::world::Context&, const kd::event::Event&) override {}
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        const auto h = c.world().beings().handle(kd::ecs::Id{cmd.a});
        const auto fit = kd::demo::IdeaDreams::fit(c.world(), h, cmd.b);
        REQUIRE(fit);
        if (!fit) return;
        kd::demo::IdeaDreams::dream(c, h, *fit);
        c.moved(kd::ecs::Id{cmd.a});
    }
};
}  // namespace
TEST_CASE("idea natural_sent_same_thought uses one constructor and refresh_not_stack") {
    RememberedWood natural, sent;
    natural.rest();
    sent.rest();
    OrdinaryIdea ordinary;
    natural.camp.world().set_command_taker(ordinary);
    natural.camp.world().command(101, 100, natural.person.value, natural.remembered);
    sent.camp.world().command(101, 3, sent.person.value, sent.remembered);
    natural.camp.world().run_to(102);
    sent.camp.world().run_to(102);
    kd::ByteWriter a, b;
    kd::ecs::write_component(natural.thought(), a);
    kd::ecs::write_component(sent.thought(), b);
    CHECK(a.bytes() == b.bytes());
    kd::ByteWriter first, second;
    kd::ecs::write_component(natural.know(), first);
    kd::ecs::write_component(sent.know(), second);
    CHECK(first.bytes() == second.bytes());
    CHECK(natural.ledger().acts.empty());
    CHECK(sent.ledger().acts.size() == 1);
    auto& hint = natural.know().hunches.front();
    hint.failures = 2;
    const auto count = natural.know().hunches.size();
    natural.camp.world().command(102, 100, natural.person.value, natural.remembered);
    natural.camp.world().run_to(103);
    CHECK(natural.know().hunches.size() == count);
    CHECK(natural.know().hunches.front().failures == 2);
    CHECK(natural.thought().decision_pull == 0);
    CHECK(natural.thought().first_attempt_at == -1);
}

#include "kd/proof/idea_cases.hpp"
TEST_CASE("idea missing-input control removes actual boards without erasing a conserved hearth") {
    const auto pair = kd::proof::idea_pair(idea_catalogue(), 201);
    CHECK(pair.removed_mg > 0);
    CHECK(pair.pending_reopen);
    CHECK(pair.delivered_reopen);
    CHECK(pair.final_reopen);
    CHECK(pair.absent_conserved);
    CHECK(pair.missing_attempt_at == -1);
    CHECK(pair.missing_success == 0);
    CHECK(pair.status == 2);
    CHECK(pair.attempt_at >= pair.dream_at);
    CHECK(pair.attempt_at < pair.dream_at + kd::demo::Living::kDreamLife);
}

TEST_CASE("idea paired seed 221 keeps interrupted shared practice from abandoning a teacher's drink") {
    // The former direct Work resume stole the teacher's unfinished drink, leaving 330 ml
    // allocated while walking; the strict reader correctly refused the final snapshot.
    const auto pair = kd::proof::idea_pair(idea_catalogue(), 221);
    CHECK(pair.pending_reopen);
    CHECK(pair.delivered_reopen);
    CHECK(pair.final_reopen);
    CHECK(pair.absent_conserved);
    CHECK(pair.control_try_at == -1);
    CHECK(pair.missing_attempt_at == -1);
    CHECK(pair.missing_success == 0);
}

#include "kd/demo/kept.hpp"
TEST_CASE("idea command_cut_recovery replays one durable request and never a torn request") {
    auto scene = kd::proof::remembered_wood(idea_catalogue(), 201);
    kd::save::FakeFiles saved;
    {
        kd::save::Keeper keeper(saved, "idea-cut-test");
        (void)keeper.begin({}, idea_catalogue());
        keeper.snapshot(scene.camp->world());
        keeper.pause_mark(scene.camp->world().frontier());
        keeper.flush();
        REQUIRE_FALSE(keeper.failed());
    }
    const auto command =
        scene.camp->world().command(101, kd::demo::Living::kIdeaDream, scene.person.value, scene.memory);
    scene.camp->world().run_to(102);
    const auto sent_digest = scene.camp->world().digests().whole;
    auto untouched = kd::proof::remembered_wood(idea_catalogue(), 201);
    untouched.camp->world().run_to(102);
    const auto untouched_digest = untouched.camp->world().digests().whole;
    for (std::uint64_t calls = 0; calls <= 4; ++calls) {
        auto files = saved;
        {
            kd::save::Keeper keeper(files, "idea-cut-test");
            (void)keeper.open();
            files.stop_after(calls);
            (void)keeper.command(command);
        }
        files.restart();
        files.power_cut();
        kd::save::Keeper keeper(files, "idea-cut-test");
        auto kept = kd::demo::keep_crowd(keeper, idea_catalogue(), 201, 1, true);
        INFO(calls);
        INFO(kept.problem);
        REQUIRE(kept.crowd);
        auto& w = kept.crowd->world();
        w.run_to(102);
        const auto& acts = w.beings().raw().get<kd::world::Dreams>(w.beings().handle(scene.home)).acts;
        REQUIRE(acts.size() <= 1);
        CHECK(w.digests().whole == (acts.empty() ? untouched_digest : sent_digest));
        if (!acts.empty()) {
            CHECK(acts.front().memory == scene.memory);
            CHECK(acts.front().status == 1);
            CHECK(acts.front().first_attempt_at == -1);
        }
    }
}
