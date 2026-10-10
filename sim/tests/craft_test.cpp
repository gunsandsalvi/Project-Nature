#include "kd/data/craft.hpp"
#include <set>
#include "doctest.h"
#include "kd/chance/chance.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery_scene.hpp"
#include "kd/demo/kept.hpp"
#include "kd/demo/living.hpp"
#include "kd/proof/fire_cases.hpp"
#include "kd/save/archive.hpp"
#include "kd/save/versions.hpp"
#include "kd/world/craft_store.hpp"
namespace {
std::uint32_t entry(const kd::data::Catalogue& c, std::string_view kind, std::string_view name) {
    const auto found = c.find(kind, name);
    KD_CHECK(found.has_value(), "test catalogue entry exists");
    return *found;
}
const kd::data::Catalogue& catalogue() {
    static const kd::data::Catalogue c = []() {
        kd::data::Catalogue out;
        KD_CHECK(out.load(kd::data::read_folder(KD_REPO "/data")).empty(), "craft catalogue loads");
        return out;
    }();
    return c;
}
const kd::world::Item& recorded_item(const kd::world::World& w, kd::ecs::Id id) {
    if (const auto h = w.things().find(id)) return w.things().raw().get<kd::world::Item>(*h);
    const auto* archived = w.archived_item(id);
    KD_CHECK(archived, "test inspection requires a retained physical identity");
    return archived->item;
}
struct StoredCraft {
    kd::demo::CrowdWorld camp{17, catalogue(), 1, true};
    kd::ecs::Id person{}, thing{}, home{};
    StoredCraft() {
        auto& w = camp.world();
        auto& raw = w.beings().raw();
        home = camp.camp_ids().front();
        w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
            if (raw.all_of<kd::world::Person>(h)) {
                if (person.value == 0) person = id;
                raw.emplace<kd::world::Work>(h);
                raw.emplace<kd::world::Knowledge>(h);
            } else if (raw.all_of<kd::world::Camp>(h)) {
                raw.emplace<kd::world::CraftHistory>(h);
                raw.emplace<kd::world::Lessons>(h);
            }
        });
        const auto h = w.make_thing();
        thing = w.things().id_of(h);
        w.things().raw().emplace<kd::world::Place>(h, raw.get<kd::world::Camp>(w.beings().handle(home)).stone_at);
        auto& item = w.things().raw().emplace<kd::world::Item>(h);
        item.home = home;
        item.kind = entry(catalogue(), "item", "base:flint");
        item.material = item.kind;
        item.mass = 2000000;
        item.length = 120;
    }
};
bool accepted(const kd::world::World& w, std::string& why) {
    const auto chunks = kd::save::read_snapshot(kd::save::write_snapshot(w.save()), why);
    REQUIRE(chunks);
    if (!chunks) return false;
    return bool(kd::demo::CrowdWorld::open(catalogue(), *chunks, why));
}
}  // namespace
TEST_CASE("older_save_refused") {
    // The previous build wrote the previous outer format. A deliberately unreadable body proves the version is checked
    // first.
    kd::ByteWriter old;
    old.u64(0x50414e53444b4e49);
    old.u32(kd::save::kSnapshotVersion - 1);
    old.u32(UINT32_MAX);
    const auto bytes = old.take();
    std::string why;
    CHECK_FALSE(kd::save::read_snapshot(bytes, why));
    CHECK(why == kd::save::kOlderSave);
    kd::save::FakeFiles files;
    REQUIRE(files.write_whole("snapshots/00000000000000025200.kds", bytes));
    const kd::save::Bytes journal{std::byte{255}};
    REQUIRE(files.write_whole("journal.log", journal));
    const auto calls = files.calls();
    kd::save::Keeper keeper(files, "α3.13a");
    const auto kept = kd::demo::keep_crowd(keeper, catalogue(), 17, 1, true);
    keeper.flush();
    CHECK_FALSE(kept.crowd);
    CHECK_FALSE(kept.made);
    CHECK(kept.problem == kd::save::kOlderSave);
    CHECK(kept.damaged.empty());
    CHECK(files.calls() == calls);
    CHECK(files.read("snapshots/00000000000000025200.kds") == bytes);
    CHECK(files.read("journal.log") == journal);
    CHECK(files.list("snapshots").size() == 1);
}
TEST_CASE("current craft extensions reopen exactly and continue across worker counts and pool order") {
    StoredCraft fixture;
    auto& w = fixture.camp.world();
    auto& k = w.beings().raw().get<kd::world::Knowledge>(w.beings().handle(fixture.person));
    k.curiosity = 76;
    k.kindness = 31;
    k.mood = 47;
    k.learning_ppm = 900000;
    k.sectors[3] = {3000, 4000, 600, 0};
    const auto before = w.digests().whole;
    std::string why;
    auto copy = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
    INFO(why);
    REQUIRE(copy);
    CHECK(copy->world().digests().whole == before);
    CHECK(kd::save::write_snapshot(copy->world().save()) == kd::save::write_snapshot(w.save()));
    w.beings().fuzz(91);
    w.things().fuzz(17);
    CHECK(w.digests().whole == before);
    kd::run::Workers workers(4);
    w.run_to(90000);
    copy->world().run_islands(90000, workers, 600);
    CHECK(copy->world().digests().whole == w.digests().whole);
    CHECK(accepted(w, why));
}
TEST_CASE("current craft chunks refuse missing duplicate future truncated and feature mismatches") {
    StoredCraft fixture;
    const auto pristine = fixture.camp.world().save();
    for (const auto tag : {kd::save::tag("CRFT"), kd::save::tag("KNOW"), kd::save::tag("HIST"), kd::save::tag("LIFE"),
                           kd::save::tag("DRMS"), kd::save::tag("LEAR")}) {
        for (int fault = 0; fault < 5; ++fault) {
            auto chunks = pristine;
            std::string why;
            auto it = std::find_if(chunks.begin(), chunks.end(), [&](const auto& c) { return c.tag == tag; });
            REQUIRE(it != chunks.end());
            if (fault == 0) chunks.erase(it);
            if (fault == 1) chunks.push_back(*it);
            if (fault == 2) ++it->version;
            if (fault == 3) it->data.pop_back();
            if (fault == 4) it->critical = false;
            const auto decoded = kd::save::read_snapshot(kd::save::write_snapshot(chunks), why);
            REQUIRE(decoded);
            if (!decoded) continue;
            CHECK_FALSE(kd::demo::CrowdWorld::open(catalogue(), *decoded, why));
            CHECK_FALSE(why.empty());
        }
    }
    for (const auto flags : {0U, 1U, 2U, 16U}) {
        auto chunks = pristine;
        auto& camp =
            *std::find_if(chunks.begin(), chunks.end(), [](const auto& c) { return c.tag == kd::save::tag("CAMP"); });
        camp.data[camp.data.size() - 4] = static_cast<std::byte>(flags);
        std::string why;
        CHECK_FALSE(kd::demo::CrowdWorld::open(catalogue(), chunks, why));
    }
}
TEST_CASE("checksummed craft records refuse orphan ownership impossible progress and duplicate reservations") {
    for (int fault = 0; fault < 8; ++fault) {
        StoredCraft fixture;
        auto& w = fixture.camp.world();
        auto& raw = w.beings().raw();
        auto& things = w.things().raw();
        const auto h = w.beings().handle(fixture.person);
        auto& item = things.get<kd::world::Item>(w.things().handle(fixture.thing));
        auto& work = raw.get<kd::world::Work>(h);
        auto& k = raw.get<kd::world::Knowledge>(h);
        if (fault == 0) item.owner = {123};
        if (fault == 1) item.mass = -1;
        if (fault == 2) item.parents.push_back({fixture.thing});
        if (fault == 3) item.changed_mask = 1U << 18U;
        if (fault == 4) k.curiosity = 101;
        if (fault == 5) work.retained_progress = 1;
        if (fault == 6) raw.remove<kd::world::Work>(h);
        if (fault == 7) raw.remove<kd::world::CraftHistory>(w.beings().handle(fixture.home));
        std::string why;
        CHECK_FALSE(accepted(w, why));
    }
}
TEST_CASE("learning records preserve personal fractions and refuse impossible evidence") {
    StoredCraft fixture;
    auto& w = fixture.camp.world();
    auto& raw = w.beings().raw();
    auto& k = raw.get<kd::world::Knowledge>(w.beings().handle(fixture.person));
    k.curiosity_remainder = 43210;
    k.sectors[0] = {1234, 1234, 17, 0, 123456, 876543, 345678, 0, 1234};
    kd::world::Skill unfinished;
    unfinished.recipe = entry(catalogue(), "blueprint", "base:sharp_flake");
    unfinished.known = 0;
    unfinished.observation_quarters = 3;
    unfinished.observation_remainder = 712345;
    k.skills.push_back(unfinished);
    std::string why;
    auto copy = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
    INFO(why);
    REQUIRE(copy);
    CHECK(kd::save::write_snapshot(copy->world().save()) == kd::save::write_snapshot(w.save()));
    for (int fault = 0; fault < 11; ++fault) {
        const auto previous = k;
        if (fault == 0) k.curiosity_remainder = kd::time::kDay;
        if (fault == 1) k.sectors[0].fraction = 561600000;
        if (fault == 2) k.sectors[0].seconds_remainder = -1;
        if (fault == 3) k.sectors[0].scale_remainder = 1000000;
        if (fault == 4) k.sectors[0].decay_at = 1;
        if (fault == 5) k.sectors[0].decay_level = 1233;
        if (fault == 6) k.skills[0].observation_quarters = 21;
        if (fault == 7) k.skills[0].known = 2;
        if (fault == 8) k.session = 1;
        if (fault == 9) k.watching = fixture.person;
        if (fault == 10) k.last_observed_event = 1;
        INFO(fault);
        CHECK_FALSE(accepted(w, why));
        k = previous;
    }
    raw.get<kd::world::Lessons>(w.beings().handle(fixture.home)).next = 0;
    CHECK_FALSE(accepted(w, why));
}
// These sessions are labelled storage setups. Autonomous offers and work are T3.13b.3.
namespace {
struct StoredLesson : StoredCraft {
    kd::ecs::Id learner{};
    StoredLesson(bool active = false) {
        auto& w = camp.world();
        w.run_to(100);
        auto& raw = w.beings().raw();
        w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
            if (learner.value == 0 && id != person && raw.all_of<kd::world::Person>(h)) learner = id;
        });
        const auto teacher_h = w.beings().handle(person), learner_h = w.beings().handle(learner);
        const auto recipe = entry(catalogue(), "blueprint", "base:sharp_flake");
        auto& teacher = raw.get<kd::world::Knowledge>(teacher_h);
        teacher.kindness = 70;
        teacher.session = 1;
        kd::world::Skill known;
        known.recipe = recipe;
        known.known = 1;
        known.practice = {1000, 1000, 0, -1};
        teacher.skills.push_back(known);
        teacher.peers.push_back({learner, recipe, 0, 3, w.frontier() - 100, 0});
        auto& student = raw.get<kd::world::Knowledge>(learner_h);
        student.session = 1;
        student.next_work = 2;
        known.known = 0;
        known.practice = {};
        known.observation_quarters = 3;
        known.observation_remainder = 712345;
        student.skills.push_back(known);
        auto& work = raw.get<kd::world::Work>(learner_h);
        work.state = active ? 2 : 4;
        work.intended = 1;
        work.route = 5;
        work.recipe = recipe;
        work.number = work.lesson = 1;
        work.start = w.frontier() - 100;
        work.active_start = w.frontier();
        work.try_seconds = 120;
        work.retained_progress = 75;
        work.end = w.frontier() + 1800 - 75;
        work.next_try = active ? w.frontier() + 45 : 0;
        work.inputs.push_back({thing, 1000000, 0, 0, 0, 0});
        kd::world::Lesson lesson;
        lesson.id = lesson.work = 1;
        lesson.teacher = person;
        lesson.learner = learner;
        lesson.recipe = recipe;
        lesson.meeting = raw.get<kd::world::Camp>(w.beings().handle(home)).shelter_at;
        lesson.state = active ? 1 : 2;
        lesson.offered = lesson.begun = w.frontier() - 100;
        lesson.settled = w.frontier();
        lesson.end = active ? work.end : 0;
        lesson.seconds = 75;
        lesson.credited_seconds = 60;
        auto& lessons = raw.get<kd::world::Lessons>(w.beings().handle(home));
        lessons.next = 2;
        lessons.sessions.push_back(lesson);
        teacher.observations.push_back({learner, 1, 1, w.frontier(), 300});
        if (active) {
            for (const auto id : {person, learner}) {
                const auto h = w.beings().handle(id);
                raw.get<kd::world::Place>(h).at = lesson.meeting;
                auto& act = raw.get<kd::world::Activity>(h);
                act.what =
                    static_cast<std::uint8_t>(id == person ? kd::world::LivingAct::teach : kd::world::LivingAct::craft);
                act.from = act.to = lesson.meeting;
                act.start = w.frontier();
                act.end = lesson.end;
                raw.get<kd::world::Life>(h).settled = act.start;
                w.schedule(id, kd::world::kActivitySlot, act.end);
            }
            work.target = lesson.meeting;
            w.schedule(learner, 2, work.next_try);
        }
    }
};
}  // namespace
TEST_CASE("partial_lesson_reopen retains active and paused applied counters and personal evidence") {
    for (const bool active : {false, true}) {
        INFO(active);
        StoredLesson fixture(active);
        auto& w = fixture.camp.world();
        std::string why;
        const auto original = kd::save::write_snapshot(w.save());
        auto decoded = kd::save::read_snapshot(original, why);
        REQUIRE(decoded);
        if (!decoded) continue;
        auto reopened = kd::demo::CrowdWorld::open(catalogue(), *decoded, why);
        INFO(why);
        REQUIRE(reopened);
        CHECK(reopened->world().digests().whole == w.digests().whole);
        CHECK(kd::save::write_snapshot(reopened->world().save()) == original);
        auto twice = kd::demo::CrowdWorld::open(catalogue(), reopened->world().save(), why);
        REQUIRE(twice);
        CHECK(kd::save::write_snapshot(twice->world().save()) == original);
        const auto& session = twice->world()
                                  .beings()
                                  .raw()
                                  .get<kd::world::Lessons>(twice->world().beings().handle(fixture.home))
                                  .sessions.front();
        CHECK(session.seconds == 75);
        CHECK(session.credited_seconds == 60);
        w.beings().fuzz(19);
        w.things().fuzz(91);
        CHECK(kd::save::write_snapshot(w.save()) == original);
    }
}
TEST_CASE("meeting session reopens with matching travel and teacher activity events") {
    StoredLesson fixture(true);
    auto& w = fixture.camp.world();
    auto& raw = w.beings().raw();
    auto& session = raw.get<kd::world::Lessons>(w.beings().handle(fixture.home)).sessions.front();
    session.state = 0;
    session.begun = session.seconds = session.credited_seconds = 0;
    const auto learner = w.beings().handle(fixture.learner);
    auto& work = raw.get<kd::world::Work>(learner);
    work.state = 1;
    work.next_try = work.retained_progress = 0;
    raw.get<kd::world::Activity>(learner).what = static_cast<std::uint8_t>(kd::world::LivingAct::walk);
    raw.get<kd::world::Knowledge>(w.beings().handle(fixture.person)).observations.clear();
    w.cancel(fixture.learner, 2);
    std::string why;
    REQUIRE(accepted(w, why));
    auto copy = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
    REQUIRE(copy);
    CHECK(kd::save::write_snapshot(copy->world().save()) == kd::save::write_snapshot(w.save()));
    raw.get<kd::world::Activity>(learner).what = 0;
    CHECK_FALSE(accepted(w, why));
}
TEST_CASE("shared session validation refuses corrupt links counters activities and queued events") {
    for (int fault = 0; fault < 27; ++fault) {
        StoredLesson fixture(true);
        auto& w = fixture.camp.world();
        auto& raw = w.beings().raw();
        const auto teacher = w.beings().handle(fixture.person), learner = w.beings().handle(fixture.learner);
        auto& lessons = raw.get<kd::world::Lessons>(w.beings().handle(fixture.home));
        auto& session = lessons.sessions.front();
        auto& work = raw.get<kd::world::Work>(learner);
        auto& know = raw.get<kd::world::Knowledge>(teacher);
        if (fault == 0) session.teacher = {123};
        if (fault == 1) session.learner = session.teacher;
        if (fault == 2) session.id = lessons.next;
        if (fault == 3) lessons.sessions.push_back(session);
        if (fault == 4) session.state = 3;
        if (fault == 5) session.recipe = UINT32_MAX;
        if (fault == 6) session.meeting.x = w.torus().width();
        if (fault == 7) session.seconds = 1800;
        if (fault == 8) session.credited_seconds = 76;
        if (fault == 9) session.begun = w.frontier() - 74;
        if (fault == 10) ++session.end;
        if (fault == 11) session.last_try = 1;
        if (fault == 12) know.session = 0;
        if (fault == 13) know.kindness = 59;
        if (fault == 14) know.skills.front().known = 0;
        if (fault == 15) work.lesson = 2;
        if (fault == 16) work.number = 0;
        if (fault == 17) work.route = 0;
        if (fault == 18) work.state = 4;
        if (fault == 19) raw.get<kd::world::Activity>(teacher).what = 0;
        if (fault == 20) {
            const auto separated = w.torus().moved(session.meeting, {201, 0});
            raw.get<kd::world::Place>(teacher).at = separated;
            raw.get<kd::world::Activity>(teacher).from = separated;
            raw.get<kd::world::Activity>(teacher).to = separated;
        }
        if (fault == 21) w.cancel(fixture.learner, 2);
        if (fault == 22) w.schedule(fixture.person, kd::world::kActivitySlot, session.end + 1);
        if (fault == 23) raw.remove<kd::world::Work>(learner);
        if (fault == 24) raw.remove<kd::world::Knowledge>(learner);
        if (fault == 25) session.meeting = w.torus().moved(session.meeting, {6000, 0});
        if (fault == 26) session.settled = w.frontier() + 1;
        INFO(fault);
        std::string why;
        CHECK_FALSE(accepted(w, why));
        CHECK_FALSE(why.empty());
    }
}
TEST_CASE("active session separation is refused by its own validation at the two metre boundary") {
    StoredLesson fixture(true);
    auto& w = fixture.camp.world();
    auto& raw = w.beings().raw();
    const auto teacher = w.beings().handle(fixture.person);
    const auto meeting = raw.get<kd::world::Lessons>(w.beings().handle(fixture.home)).sessions.front().meeting;
    const auto move = [&](std::int64_t centimetres) {
        const auto at = w.torus().moved(meeting, {centimetres, 0});
        raw.get<kd::world::Place>(teacher).at = at;
        raw.get<kd::world::Activity>(teacher).from = at;
        raw.get<kd::world::Activity>(teacher).to = at;
    };
    std::string why;
    move(200);
    REQUIRE(accepted(w, why));
    move(201);
    CHECK_FALSE(accepted(w, why));
    CHECK(why == "shared practice participants are separated");
}
TEST_CASE("taught work cannot reopen without its shared session link") {
    StoredLesson fixture(true);
    auto& w = fixture.camp.world();
    auto& raw = w.beings().raw();
    raw.get<kd::world::Lessons>(w.beings().handle(fixture.home)).sessions.clear();
    for (const auto id : {fixture.person, fixture.learner})
        raw.get<kd::world::Knowledge>(w.beings().handle(id)).session = 0;
    raw.get<kd::world::Activity>(w.beings().handle(fixture.person)).what = 0;
    raw.get<kd::world::Work>(w.beings().handle(fixture.learner)).lesson = 0;
    std::string why;
    CHECK_FALSE(accepted(w, why));
    CHECK(why == "invalid work progress");
}
TEST_CASE("ordinary intended work requires personal knowledge rather than a partial skill record") {
    StoredLesson fixture(true);
    auto& w = fixture.camp.world();
    auto& raw = w.beings().raw();
    raw.get<kd::world::Lessons>(w.beings().handle(fixture.home)).sessions.clear();
    for (const auto id : {fixture.person, fixture.learner})
        raw.get<kd::world::Knowledge>(w.beings().handle(id)).session = 0;
    raw.get<kd::world::Activity>(w.beings().handle(fixture.person)).what = 0;
    auto& work = raw.get<kd::world::Work>(w.beings().handle(fixture.learner));
    work.lesson = 0;
    work.route = 0;
    std::string why;
    CHECK_FALSE(accepted(w, why));
    auto& skill = raw.get<kd::world::Knowledge>(w.beings().handle(fixture.learner)).skills.front();
    skill.known = 1;
    skill.practice = {750, 1000, 0, -1};
    CHECK(accepted(w, why));
}
TEST_CASE("personal learning evidence must refer to the actual recipe") {
    StoredLesson fixture;
    auto& w = fixture.camp.world();
    auto& raw = w.beings().raw();
    auto& skill = raw.get<kd::world::Knowledge>(w.beings().handle(fixture.person)).skills.front();
    kd::world::Result result;
    result.id = 1;
    result.at = 0;
    result.place = raw.get<kd::world::Place>(w.beings().handle(fixture.person)).at;
    result.actor = fixture.person;
    result.recipe = skill.recipe;
    result.inputs.push_back({fixture.thing});
    auto& history = raw.get<kd::world::CraftHistory>(w.beings().handle(fixture.home));
    history.next = 2;
    history.events.push_back(result);
    skill.source = fixture.person;
    skill.source_event = result.id;
    skill.route = 2;
    std::string why;
    REQUIRE(accepted(w, why));
    history.events.writable(0).recipe = entry(catalogue(), "blueprint", "base:butcher");
    CHECK_FALSE(accepted(w, why));
}
TEST_CASE("peer beliefs and unfinished observation have bounded unique actual sources") {
    for (int fault = 0; fault < 14; ++fault) {
        StoredLesson fixture;
        auto& w = fixture.camp.world();
        auto& k = w.beings().raw().get<kd::world::Knowledge>(w.beings().handle(fixture.person));
        if (fault == 0) k.peers.push_back(k.peers.front());
        if (fault == 1) k.peers.front().person = fixture.person;
        if (fault == 2) k.peers.front().recipe = UINT32_MAX;
        if (fault == 3) k.peers.front().route = 1;
        if (fault == 4) k.peers.front().event = 1;
        if (fault == 5) k.peers.front().at = w.frontier() + 1;
        if (fault == 6) k.observations.push_back(k.observations.front());
        if (fault == 7) k.observations.front().work = 2;
        if (fault == 8) k.observations.front().attempt = 0;
        if (fault == 9) k.observations.front().attempt = 2;
        if (fault == 10) k.observations.front().weighted_seconds = 481;
        if (fault == 11) k.observations.front().settled -= 100;
        if (fault == 12) k.observations.front().person = fixture.person;
        if (fault == 13) k.observations.front().weighted_seconds = -1;
        INFO(fault);
        std::string why;
        CHECK_FALSE(accepted(w, why));
    }
    StoredLesson fixture;
    auto& w = fixture.camp.world();
    auto& k = w.beings().raw().get<kd::world::Knowledge>(w.beings().handle(fixture.person));
    const auto recipe = k.peers.front().recipe;
    kd::world::Result failed;
    failed.id = 1;
    failed.at = k.peers.front().at;
    failed.actor = fixture.learner;
    failed.place = w.beings().raw().get<kd::world::Place>(w.beings().handle(fixture.learner)).at;
    failed.recipe = recipe;
    failed.kind = 5;
    failed.inputs.push_back({fixture.thing});
    auto& history = w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(fixture.home));
    history.next = 2;
    history.events.push_back(failed);
    k.peers.front().route = 2;
    k.peers.front().event = failed.id;
    std::string why;
    REQUIRE(accepted(w, why));
    // A later skill change cannot rewrite a holder's older evidence about their peer.
    w.beings().raw().get<kd::world::Knowledge>(w.beings().handle(fixture.learner)).skills.front().known = 1;
    CHECK(accepted(w, why));
    k.peers.front().knows = 1;
    CHECK_FALSE(accepted(w, why));
    k.peers.front().knows = 0;
    history.events.writable(0).kind = 0;
    CHECK_FALSE(accepted(w, why));
}
TEST_CASE("generic_fit and granite_control depend on characteristics and sizes rather than material names") {
    const auto& c = catalogue();
    REQUIRE(kd::data::run_checks(c).empty());
    const auto& b = c.kind<kd::data::Blueprint>()[entry(c, "blueprint", "base:sharp_flake")];
    kd::data::FitInput synthetic{"stone", "lump", {}, 120, 2000000};
    synthetic.values[0] = 4;
    synthetic.values[3] = 4;
    CHECK(kd::data::fits(b.inputs[0], synthetic));
    synthetic.values[3] = 0;
    CHECK_FALSE(kd::data::fits(b.inputs[0], synthetic));
    synthetic.values[3] = 5;
    synthetic.length = 79;
    CHECK_FALSE(kd::data::fits(b.inputs[0], synthetic));
    const auto& granite = c.kind<kd::data::ItemKind>()[entry(c, "item", "base:granite")];
    synthetic = {granite.material_class, granite.form, granite.characteristics, 120, 2000000};
    CHECK_FALSE(kd::data::fits(b.inputs[0], synthetic));
}
TEST_CASE("each craft role has an expected fitting witness and rejects each declared boundary") {
    const auto& c = catalogue();
    const auto& kinds = c.kind<kd::data::Blueprint>();
    for (std::size_t n = 0; n < kinds.size(); ++n) {
        INFO(kinds.name(n));
        const auto& b = kinds[static_cast<std::uint32_t>(n)];
        for (const auto& role : b.inputs) {
            kd::data::FitInput witness{
                role.classes.empty()
                    ? (role.material_class == "any" ? std::string_view("stone") : std::string_view(role.material_class))
                    : std::string_view(role.classes.front()),
                role.form == "any" ? std::string_view("lump") : std::string_view(role.form),
                {},
                std::max<std::int64_t>(1, role.min_length),
                role.min_mass};
            for (const auto& range : role.ranges)
                witness.values[static_cast<std::size_t>(range.characteristic)] = range.minimum;
            CHECK(kd::data::fits(role, witness));
            auto bad = witness;
            bad.mass = role.min_mass - 1;
            CHECK_FALSE(kd::data::fits(role, bad));
            bad = witness;
            bad.length = role.max_length + 1;
            CHECK_FALSE(kd::data::fits(role, bad));
            for (const auto& range : role.ranges) {
                if (range.minimum > 0) {
                    bad = witness;
                    bad.values[static_cast<std::size_t>(range.characteristic)] = range.minimum - 1;
                    CHECK_FALSE(kd::data::fits(role, bad));
                }
                if (range.maximum < 5) {
                    bad = witness;
                    bad.values[static_cast<std::size_t>(range.characteristic)] = range.maximum + 1;
                    CHECK_FALSE(kd::data::fits(role, bad));
                }
            }
        }
    }
}
TEST_CASE("craft entries change rules while preserving the existing world-making fingerprint") {
    auto files = kd::data::read_folder(KD_REPO "/data");
    kd::data::Catalogue after, before;
    REQUIRE(after.load(files).empty());
    std::erase_if(files, [](const auto& f) {
        return f.path.starts_with("base/item/") || f.path.starts_with("base/blueprint/") ||
               f.path == "base/tuning/discovery.toml";
    });
    for (auto& f : files)
        if (f.path == "base/source.toml") f.text.replace(f.text.find("version = 5"), 11, "version = 3");
    REQUIRE(before.load(files).empty());
    CHECK(kd::save::making_digest(before) == kd::save::making_digest(after));
    CHECK(kd::save::rules_digest(before) != kd::save::rules_digest(after));
}

TEST_CASE("current-format saves still refuse changed world-making rules without writes") {
    StoredCraft fixture;
    kd::save::FakeFiles files;
    kd::save::Versions versions;
    versions.build = "α3.13a";
    versions.making = kd::save::making_digest(catalogue()) ^ 1U;
    auto chunks = fixture.camp.world().save();
    chunks.push_back(kd::save::versions_chunk(versions));
    REQUIRE(files.write_whole("snapshots/00000000000000000000.kds", kd::save::write_snapshot(chunks)));
    const auto calls = files.calls();
    kd::save::Keeper keeper(files, "α3.13a");
    const auto kept = kd::demo::keep_crowd(keeper, catalogue(), 17, 1, true);
    keeper.flush();
    CHECK(kept.update == kd::save::Update::big);
    CHECK_FALSE(kept.crowd);
    CHECK(files.calls() == calls);
}
TEST_CASE("paused gradual progress and atomic reservations survive current-format reopening") {
    for (int fault = 0; fault < 5; ++fault) {
        StoredCraft fixture;
        auto& w = fixture.camp.world();
        w.run_to(1000);
        auto& raw = w.beings().raw();
        const auto h = w.beings().handle(fixture.person);
        auto& work = raw.get<kd::world::Work>(h);
        auto& know = raw.get<kd::world::Knowledge>(h);
        work.state = 4;
        work.action = 4;
        work.intended = 1;
        work.recipe = entry(catalogue(), "blueprint", "base:butcher");
        work.number = 1;
        know.next_work = 2;
        kd::world::Skill skill;
        skill.recipe = work.recipe;
        skill.known = 1;
        skill.practice = {3000, 3000, 0, -1};
        know.skills.push_back(skill);
        work.start = 100;
        work.active_start = 100;
        work.end = 700;
        work.try_seconds = 600;
        work.retained_progress = 100;
        work.goal_mass = 1000000;
        work.unit_mass = 1000000;
        work.inputs.push_back({fixture.thing, 1000000, 0, 0, 1});
        auto& item = w.things().raw().get<kd::world::Item>(w.things().handle(fixture.thing));
        item.owner = fixture.person;
        if (fault == 1) work.inputs.push_back(work.inputs.front());
        if (fault == 2) work.inputs.front().mass = 3000000;
        if (fault == 3) work.retained_progress = 601;
        if (fault == 4) {
            w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle other) {
                if (id == fixture.person || !raw.all_of<kd::world::Person>(other)) return;
                auto& duplicate = raw.get<kd::world::Work>(other);
                duplicate = work;
                raw.get<kd::world::Knowledge>(other).next_work = 2;
            });
        }
        std::string why;
        if (fault == 0) {
            CHECK(accepted(w, why));
            auto copy = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
            REQUIRE(copy);
            CHECK(copy->world().digests().whole == w.digests().whole);
            CHECK(copy->world()
                      .beings()
                      .raw()
                      .get<kd::world::Work>(copy->world().beings().handle(fixture.person))
                      .retained_progress == 100);
        } else
            CHECK_FALSE(accepted(w, why));
    }
}

TEST_CASE("current craft snapshots retain extensions through corruption recovery and journal replay") {
    for (int fault = 0; fault < 5; ++fault) {
        StoredCraft fixture;
        auto& w = fixture.camp.world();
        kd::save::FakeFiles files;
        std::vector<kd::world::Record> records;
        w.keep_history(&records);
        {
            kd::save::Keeper keeper(files, "α3.13a");
            keeper.begin({}, catalogue());
            kd::demo::About about;
            about.seed = 17;
            about.camps = 1;
            about.camp_alpha = true;
            keeper.about(kd::demo::about_text(about));
            keeper.snapshot(w);
            w.run_to(8);
            keeper.history(records);
            records.clear();
            keeper.snapshot(w);
            REQUIRE(keeper.command(w.command(10, kd::demo::Living::kPlaceDream, fixture.person.value, 2)));
            w.run_to(20);
            keeper.history(records);
            keeper.pause_mark(20);
            keeper.flush();
        }
        const auto digest = w.digests().whole;
        const auto names = files.list("snapshots");
        REQUIRE(names.size() == 2);
        files.power_cut();
        auto& damaged = files.raw("snapshots/" + names.back());
        if (fault == 0) damaged[damaged.size() / 2] ^= std::byte{1};
        if (fault == 1) damaged.resize(9);
        if (fault == 2) damaged.resize(11);
        if (fault == 3) damaged.resize(14);
        if (fault == 4)
            for (std::size_t i = 8; i < 12; ++i) damaged[i] = std::byte{0};
        kd::save::Keeper keeper(files, "α3.13a");
        auto opened = kd::demo::keep_crowd(keeper, catalogue(), 17, 1, true);
        INFO(opened.problem);
        REQUIRE(opened.crowd);
        CHECK(opened.replayed == 1);
        CHECK(opened.was_at == 20);
        CHECK_FALSE(opened.damaged.empty());
        records.clear();
        opened.crowd->world().keep_history(&records);
        opened.crowd->world().run_to(opened.was_at);
        keeper.history(records);
        keeper.flush();
        CHECK(keeper.mismatches() == 0);
        CHECK(opened.crowd->world().digests().whole == digest);
        CHECK(opened.crowd->world().things().raw().all_of<kd::world::Item>(
            opened.crowd->world().things().handle(fixture.thing)));
    }
}

TEST_CASE("fresh Discovery scene materialises aggregate stocks exactly once and records finite additions") {
    kd::demo::CrowdWorld camp{91, catalogue(), 1, true, true};
    auto& w = camp.world();
    const auto home = camp.camp_ids().front();
    const auto& facts = w.beings().raw().get<kd::world::Camp>(w.beings().handle(home));
    CHECK(facts.stone_mg == 0);
    CHECK(facts.wood_mg == 0);
    CHECK(facts.food_mg == 25000000);
    CHECK(kd::demo::Crafting::total(w, home, "stone") == 90000000);
    CHECK(kd::demo::Crafting::total(w, home, "wood") == 60000000);
    std::int64_t total = 0;
    std::array<std::int64_t, 3> stone{};
    w.things().each([&](kd::ecs::Id, kd::world::Things::Handle h) {
        const auto& item = w.things().raw().get<kd::world::Item>(h);
        total += item.mass;
        CHECK(item.mass >= 0);
        CHECK(item.owner.value == 0);
        CHECK(item.made_at == (w.things().raw().all_of<kd::world::Fire>(h) ? 0 : -1));
        if (!w.things().raw().all_of<kd::world::Fire>(h)) CHECK(item.parents.empty());
        if (item.kind == entry(catalogue(), "item", "base:flint")) stone[0] += item.mass;
        if (item.kind == entry(catalogue(), "item", "base:chert")) stone[1] += item.mass;
        if (item.kind == entry(catalogue(), "item", "base:granite")) stone[2] += item.mass;
    });
    CHECK(total == 450000000);
    CHECK(stone == std::array<std::int64_t, 3>{20000000, 20000000, 40000000});
    std::string why;
    REQUIRE(accepted(w, why));
    auto copy = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
    REQUIRE(copy);
    CHECK(copy->world().digests().whole == w.digests().whole);
    CHECK(copy->world().things().size() == w.things().size());
    const auto before = w.digests().whole;
    w.beings().fuzz(881);
    w.things().fuzz(93);
    CHECK(w.digests().whole == before);
}
TEST_CASE("scene trims its final item and aggregate shares conserve indivisible milligrams") {
    // A fresh unsaved scene setup, not an opened older camp.
    kd::demo::CrowdWorld camp{17, catalogue(), 1, true};
    auto& w = camp.world();
    const auto home = camp.camp_ids().front();
    auto& facts = w.beings().raw().get<kd::world::Camp>(w.beings().handle(home));
    facts.stone_mg = 80000003;
    facts.wood_mg = 60000009;
    kd::demo::Crafting::initialise(w);
    CHECK(kd::demo::Crafting::total(w, home, "stone") == 90000003);
    CHECK(kd::demo::Crafting::total(w, home, "wood") == 60000009);
    std::int64_t total = 0;
    w.things().each(
        [&](kd::ecs::Id, kd::world::Things::Handle h) { total += w.things().raw().get<kd::world::Item>(h).mass; });
    CHECK(total == 450000012);
    std::string why;
    CHECK(accepted(w, why));
}
TEST_CASE("mass_and_reservation excludes spent portions and tools from competing work") {
    kd::demo::CrowdWorld camp{17, catalogue(), 1, true, true};
    auto& w = camp.world();
    std::vector<kd::ecs::Id> people, items;
    w.beings().each([&](kd::ecs::Id id, auto) {
        if (id.family() == kd::ecs::Family::person) people.push_back(id);
    });
    w.things().each([&](kd::ecs::Id id, auto) { items.push_back(id); });
    REQUIRE(people.size() >= 2);
    REQUIRE(items.size() >= 2);
    auto& work = w.beings().raw().get<kd::world::Work>(w.beings().handle(people[0]));
    const auto mass = w.things().raw().get<kd::world::Item>(w.things().handle(items[0])).mass;
    work.inputs = {{items[0], 123456, 0, 0, 0}, {items[1], 1, 1, 1, 0}};
    CHECK(kd::demo::Crafting::available(w, items[0], people[1]) == mass - 123456);
    CHECK(kd::demo::Crafting::available(w, items[0], people[0]) == mass);
    CHECK_FALSE(kd::demo::Crafting::tool_free(w, items[0], people[1]));
    CHECK_FALSE(kd::demo::Crafting::tool_free(w, items[1], people[1]));
    CHECK(kd::demo::Crafting::available(w, items[1], people[1]) == 0);
    work.inputs.clear();
    CHECK(kd::demo::Crafting::available(w, items[0], people[1]) == mass);
    CHECK(kd::demo::Crafting::tool_free(w, items[1], people[1]));
}
TEST_CASE("quality changes a made edge but never raw characteristics food or water") {
    kd::world::Item item;
    item.kind = entry(catalogue(), "item", "base:flint");
    item.material = item.kind;
    item.mass = 2000000;
    item.length = 120;
    item.quality = 0;
    const auto raw = kd::demo::Crafting::characteristics(catalogue(), item);
    item.quality = 5;
    CHECK(kd::demo::Crafting::characteristics(catalogue(), item) == raw);
    item.kind = entry(catalogue(), "item", "base:flake");
    item.made_at = 0;
    item.changed_mask = (1U << 1U) | (1U << 2U);
    item.changed[1] = 4;
    item.changed[2] = 1;
    const auto fine = kd::demo::Crafting::characteristics(catalogue(), item);
    CHECK(fine[1] == 5);
    CHECK(fine[2] == 1);
    item.quality = 0;
    CHECK(kd::demo::Crafting::characteristics(catalogue(), item)[1] == 3);
    item.wear = 2000000;
    CHECK(kd::demo::Crafting::characteristics(catalogue(), item)[1] == 1);
    item.kind = entry(catalogue(), "item", "base:meat");
    item.material = item.kind;
    item.changed_mask = 0;
    item.changed.fill(0);
    item.wear = 0;
    const auto poor_food = kd::demo::Crafting::characteristics(catalogue(), item);
    item.quality = 5;
    CHECK(kd::demo::Crafting::characteristics(catalogue(), item)[8] == poor_food[8]);
    CHECK(kd::demo::Crafting::characteristics(catalogue(), item)[9] == poor_food[9]);
}
TEST_CASE("fresh founders have personal starting skills and independently keyed varied traits") {
    kd::demo::CrowdWorld camp{17, catalogue(), 1, true, true};
    const auto& w = camp.world();
    std::set<std::pair<std::uint8_t, std::uint8_t>> traits;
    w.beings().each([&](kd::ecs::Id, kd::world::Beings::Handle h) {
        const auto* know = w.beings().raw().try_get<kd::world::Knowledge>(h);
        if (!know) return;
        CHECK(know->skills.size() == 5);
        CHECK(know->memories.empty());
        CHECK(know->performed == 0);
        CHECK(know->curiosity >= 20);
        CHECK(know->curiosity <= 80);
        CHECK(know->kindness >= 20);
        CHECK(know->kindness <= 80);
        traits.insert({know->curiosity, know->kindness});
        for (const auto& skill : know->skills) {
            CHECK(skill.practice.level == 3000);
            CHECK(skill.source_event == 0);
            CHECK(skill.source.value == 0);
            CHECK(catalogue().kind<kd::data::Blueprint>()[skill.recipe].starting);
        }
        const auto age = w.beings().raw().get<kd::world::Person>(h).age_years;
        const auto bonus = static_cast<std::int64_t>(age > 20 ? (age - 20) / 15 : 0) * 1000;
        CHECK(know->sectors[5].level == 3000 + bonus);
        CHECK(know->sectors[4].level == 2000 + bonus);
        CHECK(know->sectors[2].level == 1000 + bonus);
    });
    CHECK(traits.size() > 20);
}

TEST_CASE("ordinary Discovery camp work remains conserved and reopens at every event phase") {
    kd::demo::CrowdWorld camp{83, catalogue(), 1, true, true};
    auto& w = camp.world();
    const auto home = camp.camp_ids().front();
    std::vector<kd::world::Record> records;
    w.keep_history(&records);
    std::string why;
    for (const auto at : {1, 30, 60, 120, 600, 1800, 3600, 7200, 18000, 25200, 30000, 42000, 60000, 90000}) {
        w.run_to(at);
        INFO(at);
        INFO(why);
        CHECK(accepted(w, why));
        auto copy = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
        REQUIRE(copy);
        if (!copy) return;
        CHECK(copy->world().digests().whole == w.digests().whole);
        std::int64_t mass = 0;
        w.things().each([&](kd::ecs::Id, auto h) { mass += w.things().raw().get<kd::world::Item>(h).mass; });
        std::int64_t eaten = 0;
        for (const auto& r : records)
            if (r.what == 202) eaten += static_cast<std::int64_t>(r.b);
        CHECK(mass + eaten == 450000000);
        CHECK(w.beings().raw().get<kd::world::Camp>(w.beings().handle(home)).stone_mg == 0);
    }
}
TEST_CASE("Discovery work keeps the same history and state across step sizes pools and configured workers") {
    kd::demo::CrowdWorld camp{103, catalogue(), 1, true, true};
    auto& w = camp.world();
    std::string why;
    auto copy = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
    REQUIRE(copy);
    w.beings().fuzz(81);
    w.things().fuzz(83);
    kd::run::Workers workers(4);
    for (kd::time::Seconds at = 300; at <= 90000; at += 300) w.run_to(at);
    copy->world().run_islands(90000, workers, 600);
    CHECK(copy->world().digests().whole == w.digests().whole);
    // Page boundaries and archival publication seconds may differ; facts and continuation must not.
    auto reopened = kd::demo::CrowdWorld::open(catalogue(), w.save(), why);
    REQUIRE_MESSAGE(reopened, why);
    if (!reopened) return;
    reopened->world().run_to(100000);
    copy->world().run_to(100000);
    CHECK(reopened->world().digests().whole == copy->world().digests().whole);
    const auto valid = accepted(w, why);
    INFO(why);
    CHECK(valid);
}
TEST_CASE("edge_improves_work through the declared generic affordance rather than faster berry gathering") {
    const auto& b = catalogue().kind<kd::data::Blueprint>()[entry(catalogue(), "blueprint", "base:butcher")];
    CHECK(kd::demo::Crafting::duration(b, 0, false) == 1800);
    CHECK(kd::demo::Crafting::duration(b, 3, true) == 600);
    CHECK(kd::demo::Crafting::duration(b, 4, true) == 540);
    CHECK(kd::demo::Crafting::duration(b, 5, true) == 480);
    CHECK(b.bare_yield == 500000);
    CHECK(b.yield == 900000);
    kd::data::FitInput synthetic{"metal", "blade", {}, 50, 20000};
    synthetic.values[1] = 3;
    CHECK(kd::data::fits(b.inputs[1], synthetic));
    synthetic.values[1] = 2;
    CHECK_FALSE(kd::data::fits(b.inputs[1], synthetic));
}
TEST_CASE("finite_food_sampling preserves nutrition and water fractions while leaving legacy berries identical") {
    kd::demo::CrowdWorld camp{17, catalogue(), 1, true, true};
    kd::demo::Living living(camp.world());
    kd::world::Life life;
    life.food = 0;
    life.water = 0;
    life.carried_food = 1000001;
    life.portion = 1000001;
    life.meal_item = {123};  // the pure body sampler does not access the item registry
    life.food_factor_ppm = 1500000;
    life.water_ml_per_kg = 600;
    const kd::world::Activity a{5, 0, 600, {0, 0}, {0, 0}};
    const auto once = living.sample(life, a, 600);
    auto split = living.sample(life, a, 137);
    split.applied = life.portion * 137 / 600;
    split = living.sample(split, a, 600);
    CHECK(once.food == split.food);
    CHECK(once.water == split.water);
    CHECK(once.nutrient_remainder == split.nutrient_remainder);
    CHECK(once.food_water_remainder == split.food_water_remainder);
    CHECK(once.nutrient_remainder == 500000);
    life.meal_item = {};
    life.food_factor_ppm = 1000000;
    life.water_ml_per_kg = 800;
    const auto berries = living.sample(life, a, 600);
    CHECK(berries.food == once.food - 500000);
    CHECK(berries.water == once.water + 200);
    CHECK(berries.nutrient_remainder == 0);
}

namespace {
// Tests inject a declared work fixture through an ordinary command event. The real Living
// handler performs collection, body settlement, tries, interruption and physical outcomes.
struct WorkFixture final : kd::world::System {
    kd::demo::CrowdWorld camp;
    kd::world::World& w;
    kd::demo::Living living;
    kd::ecs::Id person{}, home{};
    kd::world::Beings::Handle h{};
    std::vector<kd::ecs::Id> inputs;
    std::uint32_t recipe;
    bool intended = true;
    std::int64_t seconds, goal;
    WorkFixture(std::uint64_t seed, std::string_view name, std::int64_t level,
                const std::vector<std::string_view>& materials, bool known_use = true, std::int64_t tries = 1)
        : camp(seed, catalogue(), 1, true, true),
          w(camp.world()),
          living(w),
          home(camp.camp_ids().front()),
          recipe(entry(catalogue(), "blueprint", name)),
          intended(known_use) {
        std::vector<kd::ecs::Id> remove;
        w.beings().each([&](kd::ecs::Id id, auto handle) {
            if (id.family() != kd::ecs::Family::person) return;
            if (person.value == 0) {
                person = id;
                h = handle;
            } else
                remove.push_back(id);
        });
        for (const auto id : remove) {
            for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(id, slot);
            w.beings().end(id);
        }
        const auto& b = catalogue().kind<kd::data::Blueprint>()[recipe];
        auto& know = w.beings().raw().get<kd::world::Knowledge>(h);
        know.skills.clear();
        know.curiosity = 50;
        know.curiosity_need = 100;
        know.hourly_draw = 1;
        know.sectors[static_cast<std::size_t>(b.sector)] = {level * 1000, level * 1000, 0, -1};
        if (intended) {
            kd::world::Skill skill;
            skill.recipe = recipe;
            skill.known = 1;
            skill.practice = {level * 1000, level * 1000, 0, -1};
            know.skills.push_back(skill);
        }
        auto& life = w.beings().raw().get<kd::world::Life>(h);
        life.food = 4000000;
        life.water = 3000;
        life.awake = 0;
        life.known_amount = {0, 0, 1};
        const auto here = w.beings().raw().get<kd::world::Camp>(w.beings().handle(home)).stone_at;
        w.beings().raw().get<kd::world::Place>(h).at = here;
        w.beings().raw().get<kd::world::Activity>(h).from = here;
        w.beings().raw().get<kd::world::Activity>(h).to = here;
        for (const auto material : materials) {
            const bool synthetic_edge = material == "base:flake";
            const auto kind = entry(catalogue(), "item", synthetic_edge ? "base:flint" : material);
            kd::ecs::Id found{};
            w.things().each([&](kd::ecs::Id id, auto th) {
                if (found.value != 0 || w.things().raw().get<kd::world::Item>(th).mass == 0 ||
                    w.things().raw().get<kd::world::Item>(th).kind != kind ||
                    std::find(inputs.begin(), inputs.end(), id) != inputs.end())
                    return;
                found = id;
            });
            KD_CHECK(found.value != 0, "Work trial inputs occur in the recorded scene");
            w.things().raw().get<kd::world::Place>(w.things().handle(found)).at = here;
            inputs.push_back(found);
            if (synthetic_edge) {
                // Labelled trial-only edge fixture; its original stock mass is conserved.
                auto& item = w.things().raw().get<kd::world::Item>(w.things().handle(found));
                item.kind = entry(catalogue(), "item", "base:flake");
                item.made_at = 0;
                item.maker = person;
                item.changed_mask = (1U << 1U) | (1U << 2U);
                item.changed[1] = 5;
                item.changed[2] = 1;
                item.length = 50;
            }
        }
        bool tool = b.inputs.size() > 1 && inputs.size() > 1;
        if (std::none_of(b.inputs.begin(), b.inputs.end(), [](const auto& r) { return r.optional; })) tool = true;
        std::int64_t edge = 0;
        for (std::size_t i = 1; i < inputs.size(); ++i) {
            const auto& item = recorded_item(w, inputs[i]);
            edge = std::max(edge, kd::demo::Crafting::characteristics(catalogue(), item)[1]);
        }
        seconds = kd::demo::Crafting::time_cost(w, h, kd::demo::Crafting::duration(b, edge, tool));
        goal = b.unit_mass * tries;
        w.set_command_taker(*this);
    }
    std::string_view name() const override { return "declared craft trial"; }
    void handle(kd::world::Context&, const kd::event::Event&) override {}
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        if (cmd.what == 901) {
            kd::demo::Crafting::wear(c, inputs.front(), static_cast<std::int64_t>(cmd.b), 25000);
            return;
        }
        auto& raw = w.beings().raw();
        auto& work = raw.get<kd::world::Work>(h);
        work.state = 1;
        work.intended = intended;
        work.recipe = intended ? recipe : kd::world::kNoRecipe;
        work.action = static_cast<std::uint8_t>(catalogue().kind<kd::data::Blueprint>()[recipe].action);
        work.route = intended ? 0 : 2;
        work.number = raw.get<kd::world::Knowledge>(h).next_work++;
        work.start = c.now();
        work.active_start = c.now();
        work.try_seconds = seconds;
        work.unit_mass = catalogue().kind<kd::data::Blueprint>()[recipe].unit_mass;
        work.goal_mass = goal;
        work.target = raw.get<kd::world::Place>(h).at;
        const auto& b = catalogue().kind<kd::data::Blueprint>()[recipe];
        for (std::size_t i = 0; i < inputs.size(); ++i) {
            const auto& item = recorded_item(w, inputs[i]);
            work.inputs.push_back({inputs[i], item.mass, static_cast<std::uint8_t>(i),
                                   static_cast<std::uint8_t>(b.inputs[i].retained), 0, 1});
        }
        (void)kd::demo::Crafting::continue_work(living, c, h);
    }
    void start() {
        (void)w.command(0, 900, person.value, 0);
        w.run_to(1);
    }
    const kd::world::CraftHistory& history() const {
        return w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(home));
    }
};
}  // namespace
TEST_CASE("partial evidence cannot suppress an actual personal discovery or create a duplicate skill") {
    bool noticed = false;
    for (std::uint64_t seed = 1; seed <= 50 && !noticed; ++seed) {
        WorkFixture trial(seed, "base:sharp_flake", 10, {"base:flint", "base:granite"}, false);
        auto& know = trial.w.beings().raw().get<kd::world::Knowledge>(trial.h);
        kd::world::Skill partial;
        partial.recipe = trial.recipe;
        partial.observation_quarters = 3;
        partial.observation_remainder = 712345;
        know.skills.push_back(partial);
        trial.start();
        trial.w.run_to(trial.seconds + 1);
        REQUIRE(know.skills.size() == 1);
        const auto& skill = know.skills.front();
        if (!skill.known) continue;
        noticed = true;
        CHECK(skill.practice.level == 1000);
        CHECK(skill.source == trial.person);
        CHECK(skill.source_event != 0);
        CHECK(skill.route == 2);
        CHECK(skill.observation_quarters == 3);
        CHECK(skill.observation_remainder == 712345);
    }
    CHECK(noticed);
}
TEST_CASE("interrupted_strike has no result and gradual work retains its earned seconds") {
    for (const bool strike : {true, false}) {
        WorkFixture trial(7, strike ? "base:sharp_flake" : "base:butcher", 2,
                          strike ? std::initializer_list<std::string_view>{"base:flint", "base:granite"}
                                 : std::initializer_list<std::string_view>{"base:carcass"});
        trial.start();
        trial.w.beings().raw().get<kd::world::Life>(trial.h).water = 300;
        trial.w.schedule(trial.person, 1, 10);
        trial.w.run_to(11);
        CHECK(trial.history().events.empty());
        const auto& work = trial.w.beings().raw().get<kd::world::Work>(trial.h);
        if (strike) {
            CHECK(work.state == 0);
            CHECK(trial.w.things().raw().get<kd::world::Item>(trial.w.things().handle(trial.inputs[0])).mass ==
                  2000000);
        } else {
            CHECK(work.state == 4);
            CHECK(work.retained_progress == 10);
        }
        std::string why;
        CHECK(accepted(trial.w, why));
    }
}
TEST_CASE("unknown_once_per_activity does not multiply rolls with potential repeated tries or small steps") {
    for (const auto tries : {1, 10}) {
        WorkFixture trial(39, "base:sharp_flake", 0, {"base:flint", "base:granite"}, false, tries);
        trial.start();
        for (kd::time::Seconds at = 2; at <= tries * trial.seconds + 1; ++at) trial.w.run_to(at);
        std::size_t fits = 0;
        for (const auto& event : trial.history().events)
            if (event.recipe == trial.recipe) ++fits;
        CHECK(fits == 1);
        const auto& know = trial.w.beings().raw().get<kd::world::Knowledge>(trial.h);
        CHECK((know.performed & (1U << 2U)) != 0);
        std::string why;
        CHECK(accepted(trial.w, why));
    }
}
TEST_CASE("quality_wear applies toughness and quality once conserves breakage and saves fractional wear") {
    WorkFixture trial(11, "base:sharp_flake", 2, {"base:flint", "base:granite"});
    auto& item = trial.w.things().raw().get<kd::world::Item>(trial.w.things().handle(trial.inputs[0]));
    item.kind = entry(catalogue(), "item", "base:flake");
    item.made_at = 0;
    item.maker = trial.person;
    item.quality = 2;
    item.changed_mask = (1U << 1U) | (1U << 2U);
    item.changed[1] = 5;
    item.changed[2] = 1;
    (void)trial.w.command(0, 901, trial.person.value, 20000000);
    trial.w.run_to(1);
    CHECK(recorded_item(trial.w, trial.inputs[0]).wear == 1000000);  // one usable-meat deer removes one edge step
    CHECK(kd::demo::Crafting::characteristics(catalogue(), recorded_item(trial.w, trial.inputs[0]))[1] == 4);
    std::string why;
    CHECK(accepted(trial.w, why));
    auto& next = trial.w.things().raw().get<kd::world::Item>(trial.w.things().handle(trial.inputs[0]));
    next.quality = 4;
    next.wear = 0;
    next.wear_remainder = 0;
    (void)trial.w.command(2, 901, trial.person.value, 1);
    trial.w.run_to(3);
    CHECK(recorded_item(trial.w, trial.inputs[0]).wear == 0);
    CHECK(recorded_item(trial.w, trial.inputs[0]).wear_remainder == 100000);
    CHECK(accepted(trial.w, why));
    const auto before = kd::demo::Crafting::total(trial.w, trial.home, "stone");
    (void)trial.w.command(4, 901, trial.person.value, 200000000);
    trial.w.run_to(5);
    CHECK(recorded_item(trial.w, trial.inputs[0]).mass == 0);
    CHECK(kd::demo::Crafting::total(trial.w, trial.home, "stone") == before);
    CHECK(accepted(trial.w, why));
}
TEST_CASE("declared 200 low and high skill trials per runnable craft respect chance bounds and conserve mass") {
    // Bounds declared before results: 200 independent seeds, dark first-hour fixtures,
    // low level 2 expected 40% (flake) or 50%; level 10 is capped at 95%.
    struct Case {
        const char* recipe;
        std::vector<std::string_view> inputs;
    };
    const std::array<Case, 6> cases{{{"base:sharp_flake", {"base:flint", "base:granite"}},
                                     {"base:crack_nuts_bones", {"base:nuts", "base:flint", "base:granite"}},
                                     {"base:butcher", {"base:carcass"}},
                                     {"base:leaf_bed", {"base:grass"}},
                                     {"base:split_firewood", {"base:dry_board", "base:flake"}},
                                     {"base:scrape_hide", {"base:raw_hide", "base:flake"}}}};
    for (const auto& test : cases) {
        for (const auto level : {2, 10}) {
            std::size_t successes = 0;
            for (std::uint64_t seed = 0; seed < 200; ++seed) {
                // Owned inputs keep the declared trial valid on every compiler.
                WorkFixture trial(seed, test.recipe, level, test.inputs);
                trial.start();
                trial.w.run_to(trial.seconds + 1);
                for (const auto& e : trial.history().events)
                    if (e.recipe == trial.recipe && e.kind != 5) ++successes;
                std::int64_t mass = 0;
                trial.w.things().each(
                    [&](kd::ecs::Id, auto h) { mass += trial.w.things().raw().get<kd::world::Item>(h).mass; });
                CHECK(mass == 450000000);
                std::string why;
                CHECK(accepted(trial.w, why));
            }
            MESSAGE(std::string(test.recipe), " at level ", level, ": ", successes, "/200 successes");
            INFO(test.recipe, level, successes);
            // Central 99% binomial acceptance, tightened to PRC/RES-24 without changing rules or seeds.
            const bool flaking = std::string_view(test.recipe) == "base:sharp_flake";
            CHECK(successes >= (level == 10 ? 181 : flaking ? 62 : 82));
            CHECK(successes <= (level == 10 ? 197 : flaking ? 98 : 118));
        }
    }
}

TEST_CASE("urgent hunger can choose personally known food preparation when remembered berries are gone") {
    for (const auto level : {3000, 750}) {
        WorkFixture trial(91, "base:butcher", 3, {"base:carcass"});
        auto& raw = trial.w.beings().raw();
        raw.get<kd::world::Knowledge>(trial.h).skills.front().practice = {level, level < 1000 ? 1000 : 3000, 0, -1};
        auto& life = raw.get<kd::world::Life>(trial.h);
        auto& facts = raw.get<kd::world::Camp>(trial.w.beings().handle(trial.home));
        facts.food_mg = 0;
        auto& habitat = raw.get<kd::world::Habitat>(trial.w.beings().handle(trial.home));
        habitat.crop_budget_mg = 0;
        life.food = 0;
        life.water = 3000;
        life.awake = 0;
        // Remove ready-to-eat choices only in this labelled starvation regression fixture.
        trial.w.things().each([&](kd::ecs::Id, auto h) {
            auto& item = trial.w.things().raw().get<kd::world::Item>(h);
            if (!catalogue().kind<kd::data::ItemKind>()[item.kind].edible) return;
            item.mass = 0;
            item.state = 4;
        });
        trial.w.run_to(2);
        const auto& work = raw.get<kd::world::Work>(trial.h);
        CHECK(work.state != 0);
        CHECK(work.intended == 1);
        CHECK(work.recipe == trial.recipe);
        std::string why;
        CHECK(accepted(trial.w, why));
    }
}

TEST_CASE("a paused edible work input cannot also be allocated to a finite meal") {
    WorkFixture trial(7, "base:leaf_bed", 2, {"base:roots"});
    auto& raw = trial.w.beings().raw();
    auto& input = trial.w.things().raw().get<kd::world::Item>(trial.w.things().handle(trial.inputs[0]));
    // Declared trial-only fibrous edible plant, compatible with the generic bedding role.
    input.changed_mask = 1U << 13U;
    input.changed[13] = 1;
    const auto here = raw.get<kd::world::Place>(trial.h).at;
    const auto roots = entry(catalogue(), "item", "base:roots");
    trial.w.things().each([&](kd::ecs::Id, auto th) {
        if (trial.w.things().raw().get<kd::world::Item>(th).kind == roots)
            trial.w.things().raw().get<kd::world::Place>(th).at = here;
    });
    trial.start();
    auto& life = raw.get<kd::world::Life>(trial.h);
    life.water = 300;
    trial.w.schedule(trial.person, 1, 10);
    trial.w.run_to(11);
    REQUIRE(raw.get<kd::world::Work>(trial.h).state == 4);
    const auto reserved = input.mass;
    life.water = 3000;
    life.food = 100000;
    trial.w.schedule(trial.person, 1, trial.w.frontier());
    trial.w.run_to(12);
    REQUIRE(life.meal_item.value != 0);
    CHECK(life.meal_item != trial.inputs[0]);
    CHECK(input.mass == reserved);
    CHECK(raw.get<kd::world::Work>(trial.h).inputs[0].item == trial.inputs[0]);
    std::string why;
    const auto valid = accepted(trial.w, why);
    INFO(why);
    CHECK(valid);
}

TEST_CASE("success quality uses only the roles of the actual fitting blueprint") {
    WorkFixture trial(17, "base:sharp_flake", 2, {"base:flint", "base:granite"});
    trial.start();
    for (const auto id : trial.inputs)
        trial.w.things().raw().get<kd::world::Item>(trial.w.things().handle(id)).quality = 5;
    const auto before = kd::demo::Crafting::success(trial.w, trial.h, trial.recipe, trial.inputs);
    auto& work = trial.w.beings().raw().get<kd::world::Work>(trial.h);
    kd::ecs::Id unrelated{};
    trial.w.things().each([&](kd::ecs::Id id, auto h) {
        if (unrelated.value == 0 && std::find(trial.inputs.begin(), trial.inputs.end(), id) == trial.inputs.end()) {
            unrelated = id;
            trial.w.things().raw().get<kd::world::Item>(h).quality = 0;
        }
    });
    REQUIRE(unrelated.value != 0);
    work.inputs.push_back({unrelated, 1, 2, 0, 0, 0});
    CHECK(kd::demo::Crafting::success(trial.w, trial.h, trial.recipe, trial.inputs) == before);
    const std::array all{trial.inputs[0], trial.inputs[1], unrelated};
    CHECK(kd::demo::Crafting::success(trial.w, trial.h, trial.recipe, all) == before - 100000);
}

TEST_CASE("busy surprise is halved once even when the maker is also tired") {
    std::size_t observed = 0;
    for (const std::uint8_t curiosity : {std::uint8_t{0}, std::uint8_t{100}}) {
        for (const bool tired : {false, true}) {
            for (std::uint64_t seed = 1; seed <= 1000; ++seed) {
                WorkFixture trial(seed, "base:crack_nuts_bones", 10, {"base:nuts", "base:flint", "base:granite"});
                if (tired) trial.w.beings().raw().get<kd::world::Life>(trial.h).awake = 119000;
                trial.w.beings().raw().get<kd::world::Knowledge>(trial.h).curiosity = curiosity;
                trial.start();
                trial.w.run_to(trial.seconds + 1);
                for (const auto& e : trial.history().events) {
                    if (e.recipe != entry(catalogue(), "blueprint", "base:sharp_flake") || e.kind == 5) continue;
                    ++observed;
                    REQUIRE(e.route == 1);
                    const kd::chance::Draws draws(trial.w.seed(), kd::chance::name("surprise"), trial.person.value,
                                                  e.at, catalogue().kind<kd::data::Blueprint>().key(e.recipe));
                    CHECK(e.noticed == (draws.below(0, 1000000) < (250000 + curiosity * 5000U) / 2));
                }
            }
        }
    }
    REQUIRE(observed > 0);
}

TEST_CASE("dry_friction_both_routes and wet_never_ignites use material constraints") {
    for (const auto name : {"base:ember_drill", "base:ember_plough"}) {
        const auto& b = catalogue().kind<kd::data::Blueprint>()[entry(catalogue(), "blueprint", name)];
        CHECK(b.seconds == 300);
        CHECK_FALSE(b.starting);
        for (std::size_t role = 0; role < b.inputs.size(); ++role) {
            kd::data::FitInput input{"wood", role ? "sheet" : "rod", {}, 100, 1000};
            input.values[0] = 2;
            input.values[6] = 3;
            CHECK(kd::data::fits(b.inputs[role], input));
            input.values[9] = 1;
            CHECK(kd::data::fits(b.inputs[role], input));
            for (const auto wet : {2, 3, 4, 5}) {
                input.values[9] = wet;
                CHECK_FALSE(kd::data::fits(b.inputs[role], input));
            }
            input.values[9] = 0;
            input.length = 99;
            CHECK_FALSE(kd::data::fits(b.inputs[role], input));
            input.length = 100;
            input.values[6] = 1;
            CHECK_FALSE(kd::data::fits(b.inputs[role], input));
        }
    }
}
TEST_CASE("200 low and high friction trials conserve material and keep smoke without heat") {
    // Before results: dark level zero drill is capped at 5%, plough is 10%, high at 95%.
    // Central 99% binomial bounds use the declared recipe difficulty, not observed outcomes.
    for (const auto recipe : {"base:ember_drill", "base:ember_plough"}) {
        for (const auto level : {0, 10}) {
            std::size_t successes = 0, smoke = 0, failures = 0;
            for (std::uint64_t seed = 0; seed < 200; ++seed) {
                WorkFixture trial(seed, recipe, level, {"base:dry_stick", "base:dry_board"});
                trial.start();
                trial.w.run_to(trial.seconds + 1);
                for (const auto& e : trial.history().events) {
                    if (e.recipe != trial.recipe) continue;
                    if (e.kind != 5)
                        ++successes;
                    else
                        ++failures;
                }
                const auto& know = trial.w.beings().raw().get<kd::world::Knowledge>(trial.h);
                for (const auto& memory : know.memories)
                    if (memory.sign == 13) ++smoke;
                std::int64_t mass = 0;
                trial.w.things().each([&](kd::ecs::Id, auto h) {
                    mass += trial.w.things().raw().get<kd::world::Item>(h).mass;
                    if (trial.w.things().raw().all_of<kd::world::Fire>(h)) {
                        const auto& physical_fire = trial.w.things().raw().get<kd::world::Fire>(h);
                        CHECK((physical_fire.heat == 3 || physical_fire.heat <= 1));
                    }
                });
                CHECK(mass == 450000000);
                std::string why;
                CHECK(accepted(trial.w, why));
            }
            MESSAGE(std::string(recipe), " level ", level, ": ", successes, "/200, smoke ", smoke, "/", failures);
            const bool drill = std::string_view(recipe) == "base:ember_drill";
            CHECK(successes >= (level == 0 ? (drill ? 2 : 9) : 181));
            CHECK(successes <= (level == 0 ? (drill ? 19 : 32) : 197));
            CHECK(smoke > 0);
            CHECK(smoke < failures);
        }
    }
}
TEST_CASE("ordinary drill and grind fits discover friction without an idea dream") {
    for (const auto recipe : {"base:ember_drill", "base:ember_plough"}) {
        std::size_t discovered = 0, hints = 0;
        for (std::uint64_t seed = 0; seed < 200; ++seed) {
            WorkFixture trial(seed, recipe, 10, {"base:dry_stick", "base:dry_board"}, false);
            trial.w.beings().raw().get<kd::world::Knowledge>(trial.h).curiosity = 80;
            trial.start();
            trial.w.run_to(trial.seconds + 1);
            const auto& know = trial.w.beings().raw().get<kd::world::Knowledge>(trial.h);
            for (const auto& skill : know.skills)
                if (skill.recipe == trial.recipe && skill.known) ++discovered;
            for (const auto& memory : know.memories)
                if (memory.sign == 13) ++hints;
            kd::proof::FireRun observed;
            for (const auto& event : trial.history().events) observed.observe_result(trial.w, event);
            const auto made = std::count_if(trial.history().events.begin(), trial.history().events.end(),
                                            [](const auto& event) { return event.kind == 0 || event.kind == 1; });
            CHECK(observed.friction_results == static_cast<std::uint64_t>(made));
            CHECK(trial.w.beings().raw().get<kd::world::Dream>(trial.h).at == -1);
        }
        CHECK(discovered > 0);
        CHECK(hints > 0);
    }
}

TEST_CASE("large kept choice and result histories reopen beyond the former elapsed-play ceiling") {
    StoredCraft fixture;
    auto& w = fixture.camp.world();
    auto& raw = w.beings().raw();
    auto& kept = raw.get<kd::world::CraftHistory>(w.beings().handle(fixture.home));
    kd::world::CraftReason reason;
    reason.kind = 2;
    const std::vector<kd::world::CraftReason> reasons(3, reason);
    constexpr std::uint64_t count = 100001;
    kept.choices.reserve(count);
    kept.events.reserve(count);
    for (std::uint64_t n = 1; n <= count; ++n) {
        kept.choices.push_back({n, 0, fixture.person, reasons});
        kd::world::Result event;
        event.id = n;
        event.choice = n;
        event.actor = fixture.person;
        event.place = raw.get<kd::world::Place>(w.beings().handle(fixture.person)).at;
        event.recipe = entry(catalogue(), "blueprint", "base:butcher");
        event.inputs.push_back({fixture.thing});
        kept.events.push_back(std::move(event));
    }
    kept.next_choice = kept.next = count + 1;
    std::string why;
    REQUIRE_MESSAGE(accepted(w, why), why);
    kept.events.writable(kept.events.size() - 1).choice = count + 1;
    CHECK_FALSE(accepted(w, why));
}
