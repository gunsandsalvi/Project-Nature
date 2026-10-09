#include "kd/demo/learning.hpp"
#include "doctest.h"
#include "kd/chance/chance.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/ecs/component.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/snapshot.hpp"

namespace {
kd::world::Practice reopen(const kd::world::Practice& p) {
    kd::ByteWriter writer;
    kd::ecs::write_component(p, writer);
    const auto bytes = writer.take();
    kd::ByteReader reader(bytes);
    kd::world::Practice out;
    REQUIRE(kd::ecs::read_component(out, reader, {}));
    REQUIRE(reader.finished());
    return out;
}
std::vector<std::byte> encoded(const kd::world::Practice& p) {
    kd::ByteWriter writer;
    kd::ecs::write_component(p, writer);
    return writer.take();
}
}  // namespace

TEST_CASE("skill_curve conserves fractional effort and uses the declared two slopes") {
    kd::world::Practice skill{1000, 1000, 0, 0};
    kd::demo::Learning::practice(skill, 0, 180 * kd::time::kHour, false);
    CHECK(skill.level == 5000);
    CHECK(skill.seconds == 180 * kd::time::kHour);
    kd::demo::Learning::practice(skill, 0, 780 * kd::time::kHour, false);
    CHECK(skill.level == 10000);
    CHECK(skill.best == 10000);
    CHECK(skill.fraction == 0);
    kd::world::Practice successful{1000, 1000, 0, 0};
    kd::demo::Learning::practice(successful, 0, 90 * kd::time::kHour, true);
    CHECK(successful.level == 5000);
    kd::world::Practice whole{1000, 1000, 0, 0}, pieces = whole;
    kd::demo::Learning::practice(whole, 0, 317, true, 917123, 5137777);
    for (int i = 0; i < 317; ++i) {
        kd::demo::Learning::practice(pieces, 0, 1, true, 917123, 5137777);
        if (i % 7 == 0) pieces = reopen(pieces);
    }
    CHECK(encoded(pieces) == encoded(whole));
    CHECK(whole.scale_remainder != 0);
    CHECK(whole.seconds_remainder != 0);
}

TEST_CASE("unused skill fades from a saved fixed anchor and never below half its best") {
    kd::world::Practice direct{8000, 10000, 960 * kd::time::kHour, 0}, split = direct;
    kd::demo::Learning::fade(direct, 10 * kd::time::kYear);
    for (int year = 1; year <= 10; ++year) {
        kd::demo::Learning::fade(split, year * kd::time::kYear);
        split = reopen(split);
    }
    CHECK(direct.level == 5750);
    CHECK(encoded(split) == encoded(direct));
    kd::demo::Learning::fade(direct, 1000 * kd::time::kYear);
    CHECK(direct.level == 5000);
    CHECK(direct.best == 10000);
    kd::demo::Learning::practice(direct, 1000 * kd::time::kYear, 562, false);
    CHECK(direct.level == 5001);
    CHECK(direct.level < direct.best);
}

TEST_CASE("unfinished practice does not unlock a recipe and a known faded recipe remains known") {
    kd::world::Knowledge mind;
    kd::world::Skill unfinished;
    unfinished.recipe = 9;
    unfinished.known = 0;
    kd::demo::Learning::practice(unfinished.practice, 0, kd::time::kDay, true);
    REQUIRE(unfinished.practice.level > 1000);
    mind.skills.push_back(unfinished);
    CHECK_FALSE(kd::demo::Learning::knows(mind, 9));
    mind.skills.back().known = 1;
    kd::demo::Learning::fade(mind.skills.back().practice, 10 * kd::time::kYear);
    CHECK(kd::demo::Learning::knows(mind, 9));
    CHECK_FALSE(kd::demo::Learning::knows(mind, 10));
    CHECK(kd::demo::Learning::taught_multiplier({0, 0, 0, 0}) == 4000000);
    CHECK(kd::demo::Learning::taught_multiplier({5000, 5000, 0, 0}) == 6000000);
    CHECK(kd::demo::Learning::taught_multiplier({10000, 10000, 0, 0}) == 8000000);
}

TEST_CASE("no_global_unlock and last_holder_removed leave only personal skill holders") {
    kd::world::Knowledge holder, other;
    kd::world::Skill skill;
    skill.recipe = 17;
    skill.known = 1;
    skill.practice = {1000, 1000, 0, -1};
    holder.skills.push_back(skill);
    CHECK(kd::demo::Learning::knows(holder, 17));
    CHECK_FALSE(kd::demo::Learning::knows(other, 17));
    other.performed = 1;
    CHECK_FALSE(kd::demo::Learning::knows(other, 17));
    // Labelled holder removal, not a disease or death implementation.
    holder.skills.clear();
    CHECK_FALSE(kd::demo::Learning::knows(holder, 17));
    CHECK_FALSE(kd::demo::Learning::knows(other, 17));
}
TEST_CASE("sector and recipe practice share effort scaling and saved fading") {
    kd::world::Knowledge mind;
    kd::world::Skill skill;
    skill.recipe = 17;
    skill.known = 1;
    skill.practice = {1000, 1000, 0, 0};
    mind.skills.push_back(skill);
    mind.sectors[0] = skill.practice;
    int first_five = -1;
    for (int day = 0; day < 180; ++day) {
        if (day % 7 == 6) continue;
        const auto now = day * kd::time::kDay;
        kd::demo::Learning::practice(mind.skills.front().practice, now, kd::time::kHour, true, 917123);
        kd::demo::Learning::practice(mind.sectors[0], now, kd::time::kHour, true, 917123);
        if (first_five == -1 && mind.sectors[0].level >= 5000) first_five = day;
    }
    CHECK(first_five >= 90);
    CHECK(first_five < 180);
    CHECK(encoded(mind.sectors[0]) == encoded(mind.skills.front().practice));
    CHECK(mind.sectors[0].level >= 5000);
    auto saved = reopen(mind.sectors[0]);
    kd::demo::Learning::fade(saved, saved.last_use + 10 * kd::time::kYear);
    CHECK(saved.level >= (saved.best + 1) / 2);
}

TEST_CASE("zero elapsed practice neither credits effort nor records a fresh use") {
    kd::world::Practice skill{8000, 10000, 0, 0};
    const auto before = encoded(skill);
    kd::demo::Learning::practice(skill, 10 * kd::time::kYear, 0, true);
    CHECK(encoded(skill) == before);
    kd::demo::Learning::fade(skill, 10 * kd::time::kYear);
    CHECK(skill.level == 5750);
    CHECK(skill.last_use == 0);
}

namespace {
const kd::data::Catalogue& watch_catalogue() {
    static const kd::data::Catalogue value = [] {
        kd::data::Catalogue c;
        KD_CHECK(c.load(kd::data::read_folder(KD_REPO "/data")).empty(), "Observation catalogue loads");
        return c;
    }();
    return value;
}
std::uint32_t watch_entry(std::string_view kind, std::string_view name) {
    const auto found = watch_catalogue().find(kind, name);
    KD_CHECK(found.has_value(), "Observation fixture entry exists");
    return *found;
}
struct WatchFixture : kd::world::System {
    std::unique_ptr<kd::demo::CrowdWorld> camp =
        std::make_unique<kd::demo::CrowdWorld>(17, watch_catalogue(), 1, true, true);
    kd::ecs::Id maker{}, watcher{}, absent{}, home{}, input{};
    std::uint32_t recipe = watch_entry("blueprint", "base:butcher");
    std::int64_t duration = 1800, tries = 1;
    bool intended = true;
    WatchFixture(bool busy = false, std::int64_t distance = 500, kd::time::Seconds begun = 7 * kd::time::kHour) {
        auto& w = camp->world();
        auto& raw = w.beings().raw();
        home = camp->camp_ids().front();
        std::vector<kd::ecs::Id> people;
        w.beings().each([&](kd::ecs::Id id, auto h) {
            if (raw.all_of<kd::world::Person>(h)) people.push_back(id);
        });
        REQUIRE(people.size() >= 3);
        maker = people[0];
        watcher = people[1];
        absent = people[2];
        for (std::size_t i = 3; i < people.size(); ++i) {
            for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(people[i], slot);
            w.beings().end(people[i]);
        }
        const auto centre = raw.get<kd::world::Place>(w.beings().handle(home)).at;
        for (const auto person : {maker, watcher, absent}) {
            const auto h = w.beings().handle(person);
            auto& mind = raw.get<kd::world::Knowledge>(h);
            mind.skills.clear();
            mind.hunches.clear();
            mind.curiosity_need = 100;
            raw.get<kd::world::Work>(h) = {};
            const auto offset = person == maker ? 0 : person == watcher ? distance : 1500;
            const auto at = w.torus().moved(centre, {offset, -1000});
            raw.get<kd::world::Place>(h).at = at;
            const auto what =
                person == watcher && !busy ? kd::world::LivingAct::watch_craft : kd::world::LivingAct::watch;
            raw.get<kd::world::Activity>(h) = {static_cast<std::uint8_t>(what), 0, kd::time::kDay, at, at};
            mind.watching = person == watcher && !busy ? maker : kd::ecs::Id{};
            for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(person, slot);
            w.schedule(person, 0, kd::time::kDay);
        }
        kd::world::Skill skill;
        skill.recipe = recipe;
        skill.known = 1;
        skill.practice = {1000, 1000, 0, -1};
        mind(maker).skills.push_back(skill);
        const auto th = w.make_thing();
        input = w.things().id_of(th);
        w.things().raw().emplace<kd::world::Place>(th, raw.get<kd::world::Place>(w.beings().handle(maker)).at);
        auto& item = w.things().raw().emplace<kd::world::Item>(th);
        item.home = home;
        item.kind = watch_entry("item", "base:carcass");
        item.material = item.kind;
        // Labelled mechanics fixture with a finite fifty-kilogram supply, not a discovery pace run.
        item.mass = 50000000;
        item.length = 500;
        w.set_command_taker(*this);
        w.run_to(begun);
    }
    kd::world::Knowledge& mind(kd::ecs::Id id) {
        return camp->world().beings().raw().get<kd::world::Knowledge>(camp->world().beings().handle(id));
    }
    std::string_view name() const override { return "observation trial commands"; }
    void handle(kd::world::Context&, const kd::event::Event&) override {}
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        auto& w = c.world();
        auto& raw = w.beings().raw();
        const auto h = w.beings().handle(maker);
        if (cmd.what == 902) {
            kd::demo::Learning::observe(c, w.beings().handle(watcher));
            return;
        }
        if (cmd.what == 903) {
            kd::demo::Learning::observe(c, w.beings().handle(watcher));
            const auto wh = w.beings().handle(watcher);
            auto& a = raw.get<kd::world::Activity>(wh);
            const auto at = a.at(w.torus(), c.now());
            a = {static_cast<std::uint8_t>(kd::world::LivingAct::rest), c.now(), kd::time::kDay, at, at};
            raw.get<kd::world::Life>(wh).settled = c.now();
            mind(watcher).watching = {};
            return;
        }
        kd::demo::Crafting::settle_meal(c, h, 0, true);
        raw.get<kd::world::Life>(h).settled = c.now();
        auto& work = raw.get<kd::world::Work>(h);
        work = {};
        work.state = 1;
        work.intended = intended;
        work.recipe = intended ? recipe : kd::world::kNoRecipe;
        work.action = 4;
        work.number = mind(maker).next_work++;
        work.start = c.now();
        work.active_start = c.now();
        work.try_seconds = duration;
        work.unit_mass = 1000000;
        work.goal_mass = tries * 1000000;
        work.target = raw.get<kd::world::Place>(h).at;
        work.inputs.push_back({input, tries * 1000000, 0, 0, 0, 1});
        REQUIRE(kd::demo::Crafting::continue_work(*const_cast<kd::demo::Living*>(camp->living()), c, h));
    }
    kd::time::Seconds start() {
        const auto at = camp->world().frontier();
        (void)camp->world().command(at, 901, 0, 0);
        camp->world().run_to(at + 1);
        return at;
    }
    void use() {
        const auto at = start();
        camp->world().run_to(at + duration + 1);
    }
    void at(kd::time::Seconds when, std::uint32_t command) {
        (void)camp->world().command(when, command, 0, 0);
        camp->world().run_to(when + 1);
    }
    void reopen() {
        std::string why;
        const auto chunks = kd::save::read_snapshot(kd::save::write_snapshot(camp->world().save()), why);
        REQUIRE(chunks);
        if (!chunks) return;
        const auto digest = camp->world().digests().whole;
        auto opened = kd::demo::CrowdWorld::open(watch_catalogue(), *chunks, why);
        INFO(why);
        REQUIRE(opened);
        if (!opened) return;
        CHECK(opened->world().digests().whole == digest);
        camp = std::move(opened);
        camp->world().set_command_taker(*this);
    }
};
}  // namespace
TEST_CASE("five_watches teach skill one from actual demonstrated ends and give a first-use hunch") {
    WatchFixture f;
    for (int i = 1; i <= 5; ++i) {
        f.use();
        REQUIRE(f.mind(f.watcher).skills.size() == 1);
        const auto& s = f.mind(f.watcher).skills.front();
        CHECK(s.observation_quarters == i * 4);
        CHECK(s.observation_remainder == 0);
        CHECK(bool(s.known) == (i == 5));
        if (i == 1) {
            REQUIRE(f.mind(f.watcher).hunches.size() == 1);
            CHECK(f.mind(f.watcher).hunches[0].source == f.maker);
            CHECK(f.mind(f.watcher).performed == 0);
        }
    }
    const auto& skill = f.mind(f.watcher).skills.front();
    CHECK(skill.practice.level == 1000);
    CHECK(skill.source == f.maker);
    CHECK(skill.route == 4);
    CHECK(skill.source_event != 0);
    f.reopen();
}
TEST_CASE("busy_quarter requires twenty complete uses rather than five") {
    WatchFixture f(true);
    for (int i = 1; i <= 20; ++i) {
        f.use();
        REQUIRE(f.mind(f.watcher).skills.size() == 1);
        CHECK(f.mind(f.watcher).skills[0].observation_quarters == i);
        CHECK(kd::demo::Learning::knows(f.mind(f.watcher), f.recipe) == (i == 20));
    }
}
TEST_CASE("occluded_observer also rejects distance beyond five metres and unusable light") {
    SUBCASE("beyond the inclusive boundary") {
        WatchFixture f(false, 501);
        f.use();
        CHECK(f.mind(f.watcher).skills.empty());
        CHECK(f.mind(f.watcher).hunches.empty());
    }
    SUBCASE("solid rock blocks sight") {
        WatchFixture f;
        auto& rock = f.camp->world().beings().raw().get<kd::world::Habitat>(f.camp->world().beings().handle(f.home));
        rock.rock_west = 200;
        rock.rock_east = 300;
        rock.rock_south = -1100;
        rock.rock_north = -900;
        f.use();
        CHECK(f.mind(f.watcher).skills.empty());
    }
    SUBCASE("night has no task light in this slice") {
        WatchFixture f;
        auto& w = f.camp->world();
        const auto from = w.beings().raw().get<kd::world::Place>(w.beings().handle(f.maker)).at;
        CHECK_FALSE(kd::demo::Learning::can_watch(w, f.home, from, from, 5 * kd::time::kHour));
        CHECK(kd::demo::Learning::can_watch(w, f.home, from, from, 6 * kd::time::kHour));
        CHECK_FALSE(kd::demo::Learning::can_watch(w, f.home, from, from, 20 * kd::time::kHour));
    }
}
TEST_CASE("partial_lesson_reopen retains partial watching without recrediting elapsed exposure") {
    WatchFixture f;
    // Seven seconds exercises a non-integral millionth-of-quarter, not only halves.
    f.duration = 7;
    const auto begin = f.start();
    f.at(begin + 2, 902);
    REQUIRE(f.mind(f.watcher).observations.size() == 1);
    CHECK(f.mind(f.watcher).observations[0].weighted_seconds == 8);
    f.reopen();
    f.at(begin + 2 + 1, 903);
    f.camp->world().run_to(begin + 8);
    REQUIRE(f.mind(f.watcher).skills.size() == 1);
    CHECK(f.mind(f.watcher).skills[0].observation_quarters == 1);
    CHECK(f.mind(f.watcher).skills[0].observation_remainder == 714285);
    CHECK(f.mind(f.watcher).observations.empty());
    const auto event = f.mind(f.watcher).last_observed_event;
    f.reopen();
    f.camp->world().run_to(begin + 9);
    CHECK(f.mind(f.watcher).skills[0].observation_remainder == 714285);
    CHECK(f.mind(f.watcher).last_observed_event == event);
}
TEST_CASE("no_global_unlock leaves a distant person ignorant when a watcher learns") {
    WatchFixture f;
    for (int i = 0; i < 5; ++i) f.use();
    CHECK(kd::demo::Learning::knows(f.mind(f.watcher), f.recipe));
    CHECK_FALSE(kd::demo::Learning::knows(f.mind(f.absent), f.recipe));
    CHECK(f.mind(f.absent).skills.empty());
    CHECK(f.mind(f.absent).hunches.empty());
    CHECK(f.mind(f.absent).peers.empty());
}

TEST_CASE("observation exposure follows daylight boundaries, movement and sleeping") {
    SUBCASE("a demonstration wholly in darkness teaches nothing") {
        WatchFixture f(false, 500, 5 * kd::time::kHour);
        f.use();
        CHECK(f.mind(f.watcher).skills.empty());
    }
    SUBCASE("only the lit half across dawn or dusk counts") {
        for (const auto boundary : {6 * kd::time::kHour, 20 * kd::time::kHour}) {
            WatchFixture f(false, 500, boundary - 900);
            f.use();
            REQUIRE(f.mind(f.watcher).skills.size() == 1);
            CHECK(f.mind(f.watcher).skills[0].observation_quarters == 2);
            CHECK(f.mind(f.watcher).skills[0].observation_remainder == 0);
        }
    }
    SUBCASE("a passing busy observer earns only the visible elapsed share") {
        WatchFixture f(true, 0);
        f.duration = 7;
        auto& w = f.camp->world();
        const auto h = w.beings().handle(f.watcher);
        auto& act = w.beings().raw().get<kd::world::Activity>(h);
        const auto point = act.from;
        act = {static_cast<std::uint8_t>(kd::world::LivingAct::walk), w.frontier(), w.frontier() + 7, point,
               w.torus().moved(point, {1000, 0})};
        w.beings().raw().get<kd::world::Life>(h).settled = w.frontier();
        w.schedule(f.watcher, 0, act.end);
        f.use();
        REQUIRE(f.mind(f.watcher).skills.size() == 1);
        CHECK(f.mind(f.watcher).skills[0].observation_quarters == 0);
        CHECK(f.mind(f.watcher).skills[0].observation_remainder == 571428);
    }
    SUBCASE("sleeping beside the maker gives no credit") {
        WatchFixture f(true);
        f.camp->world().beings().raw().get<kd::world::Activity>(f.camp->world().beings().handle(f.watcher)).what =
            static_cast<std::uint8_t>(kd::world::LivingAct::rest);
        f.use();
        CHECK(f.mind(f.watcher).skills.empty());
    }
}
TEST_CASE("partial observation continuation matches reopen, four workers and shuffled pools") {
    WatchFixture direct, saved;
    direct.duration = 7;
    saved.duration = 7;
    const auto begin = direct.start();
    CHECK(saved.start() == begin);
    direct.at(begin + 2, 902);
    saved.at(begin + 2, 902);
    saved.reopen();
    direct.at(begin + 3, 903);
    saved.at(begin + 3, 903);
    saved.camp->world().beings().fuzz(13);
    saved.camp->world().things().fuzz(71);
    direct.camp->world().run_to(begin + 8);
    kd::run::Workers workers(4);
    saved.camp->world().run_islands(begin + 8, workers, 1);
    CHECK(direct.camp->world().digests().whole == saved.camp->world().digests().whole);
    CHECK(kd::save::write_snapshot(direct.camp->world().save()) ==
          kd::save::write_snapshot(saved.camp->world().save()));
}

TEST_CASE("partial maker interruption preserves observation across resume and reopen") {
    WatchFixture f;
    f.duration = 7;
    const auto begin = f.start();
    f.camp->world().schedule(f.maker, 1, begin + 3);
    f.camp->world().run_to(begin + 4);
    REQUIRE(f.mind(f.watcher).observations.size() == 1);
    CHECK(f.mind(f.watcher).observations[0].weighted_seconds == 12);
    f.reopen();
    f.camp->world().run_to(begin + 8);
    REQUIRE(f.mind(f.watcher).skills.size() == 1);
    CHECK(f.mind(f.watcher).skills[0].observation_quarters == 4);
    CHECK(f.mind(f.watcher).skills[0].observation_remainder == 0);
}
TEST_CASE("an accidental fit is not an intentional demonstration to observers") {
    WatchFixture f;
    f.duration = 7;
    f.tries = 2;
    f.intended = false;
    f.mind(f.maker).skills.clear();
    const auto begin = f.start();
    f.at(begin + 10, 902);
    CHECK(f.mind(f.watcher).observations.empty());
    CHECK(f.mind(f.watcher).skills.empty());
    f.reopen();
}
