#include "kd/demo/learning.hpp"
#include "doctest.h"
#include "kd/chance/chance.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/choice.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/ecs/component.hpp"
#include "kd/proof/learning_cases.hpp"
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
    WatchFixture(bool busy = false, std::int64_t distance = 500, kd::time::Seconds begun = 7 * kd::time::kHour,
                 std::uint64_t seed = 17) {
        camp = std::make_unique<kd::demo::CrowdWorld>(seed, watch_catalogue(), 1, true, true);
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
        if (cmd.what == 905) {
            kd::demo::Learning::lost(c, w.beings().handle(maker), recipe);
            return;
        }
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
        work.route = intended ? 0 : 2;
        work.number = mind(maker).next_work++;
        work.start = c.now();
        work.active_start = c.now();
        work.try_seconds = duration;
        work.unit_mass = intended ? 1000000 : 20000;
        work.goal_mass = tries * work.unit_mass;
        work.target = raw.get<kd::world::Place>(h).at;
        work.inputs.push_back({input, tries * work.unit_mass, 0, 0, 0, 1});
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

namespace {
struct TeachingFixture : WatchFixture {
    kd::ecs::Id visible_tool{};
    bool accepted = false;
    explicit TeachingFixture(std::uint64_t seed = 17, kd::time::Seconds begun = 7 * kd::time::kHour)
        : WatchFixture(true, 400, begun, seed) {
        auto& w = camp->world();
        auto& raw = w.beings().raw();
        for (const auto id : {maker, watcher}) {
            auto& life = raw.get<kd::world::Life>(w.beings().handle(id));
            life.carried_food = 0;
            life.meal_item = {};
            life.allocated_water = 0;
            life.portion = 0;
            life.applied = 0;
            life.food = 4000000;
            life.water = 3000;
            life.awake = 0;
            life.settled = w.frontier();
            mind(id).sectors.fill({});
        }
        mind(maker).kindness = 80;
        mind(maker).familiar.clear();
    }
    void command(kd::world::Context& c, const kd::world::Command& cmd) override {
        auto& w = c.world();
        c.touch(home);
        c.touch(maker);
        c.touch(watcher);
        const auto teacher = w.beings().handle(maker), learner = w.beings().handle(watcher);
        if (cmd.what == 910) {
            kd::demo::Discovery::learn(c, teacher, input, kd::demo::Discovery::kSight, 1);
            if (visible_tool.value)
                kd::demo::Discovery::learn(c, teacher, visible_tool, kd::demo::Discovery::kSight, 1);
            CHECK(kd::demo::Learning::exchange(c, teacher, learner, recipe));
            return;
        }
        if (cmd.what == 919) {
            kd::demo::ChoiceSet options;
            auto& life = w.beings().raw().get<kd::world::Life>(teacher);
            life.scores = {0, 0, 0, 0};
            for (std::uint8_t goal = 0; goal < 4; ++goal)
                options.add(kd::demo::Choices::body(life, goal), [](std::uint64_t) { return false; });
            kd::demo::Learning::choose(*const_cast<kd::demo::Living*>(camp->living()), c, teacher, &options);
            CHECK(options.commit(c, teacher));
            accepted = mind(watcher).session != 0;
            return;
        }
        if (cmd.what == 918) {
            kd::demo::ChoiceSet options;
            auto& life = w.beings().raw().get<kd::world::Life>(teacher);
            life.scores = {100000, 0, 0, 0};
            for (std::uint8_t goal = 0; goal < 4; ++goal)
                options.add(kd::demo::Choices::body(life, goal), [](std::uint64_t) { return false; });
            kd::demo::Learning::choose(*const_cast<kd::demo::Living*>(camp->living()), c, teacher, &options);
            CHECK(options.reasons().size() > 4);
            CHECK(w.beings().raw().get<kd::world::Work>(learner).state == 0);
            CHECK_FALSE(options.commit(c, teacher));
            return;
        }
        if (cmd.what == 920) {
            auto& raw = w.beings().raw();
            const auto here = w.torus().moved(raw.get<kd::world::Place>(teacher).at, {1000, 0});
            raw.get<kd::world::Place>(teacher).at = here;
            auto& activity = raw.get<kd::world::Activity>(teacher);
            activity.from = activity.to = here;
            CHECK(kd::demo::Learning::choose(*const_cast<kd::demo::Living*>(camp->living()), c, teacher));
            return;
        }
        if (cmd.what == 912) {
            accepted = kd::demo::Learning::choose(*const_cast<kd::demo::Living*>(camp->living()), c, teacher);
            return;
        }
        if (cmd.what == 911) {
            CHECK(kd::demo::Learning::choose(*const_cast<kd::demo::Living*>(camp->living()), c, teacher));
            return;
        }
        WatchFixture::command(c, cmd);
    }
    void offer() {
        const auto now = camp->world().frontier();
        at(now, 910);
        CHECK_FALSE(kd::demo::Learning::knows(mind(watcher), recipe));
        CHECK(mind(watcher).hunches.size() == 1);
        at(now + 1, 911);
    }
    const kd::world::Lessons& sessions() {
        return camp->world().beings().raw().get<kd::world::Lessons>(camp->world().beings().handle(home));
    }
};
}  // namespace
TEST_CASE("telling gives only a hunch and evidence-backed offers meet by ordinary routes") {
    TeachingFixture f;
    f.offer();
    REQUIRE(f.sessions().sessions.size() == 1);
    CHECK(f.sessions().sessions[0].state == 0);
    const auto& w = f.camp->world();
    CHECK(w.beings().raw().get<kd::world::Place>(w.beings().handle(f.watcher)).at !=
          w.beings().raw().get<kd::world::Place>(w.beings().handle(f.maker)).at);
    f.reopen();
    f.camp->world().run_to(7 * kd::time::kHour + 30);
    REQUIRE(f.sessions().sessions.size() == 1);
    INFO(f.sessions().sessions[0].end);
    INFO(f.camp->world().beings().raw().get<kd::world::Activity>(f.camp->world().beings().handle(f.watcher)).end);
    CHECK(f.sessions().sessions[0].state == 1);
    f.reopen();
}
TEST_CASE("lesson selection skips unavailable teacher tools and can use the learner's visible tool") {
    for (const bool learner_owns : {false, true}) {
        TeachingFixture f;
        auto& w = f.camp->world();
        const auto h = w.make_thing();
        f.visible_tool = w.things().id_of(h);
        const auto holder = learner_owns ? f.watcher : f.maker;
        w.things().raw().emplace<kd::world::Place>(
            h, w.beings().raw().get<kd::world::Place>(w.beings().handle(holder)).at);
        auto& tool = w.things().raw().emplace<kd::world::Item>(h);
        tool.kind = watch_entry("item", "base:flake");
        tool.material = watch_entry("item", "base:flint");
        tool.mass = 20000;
        tool.length = 40;
        tool.home = f.home;
        tool.owner = holder;
        tool.changed_mask = 1U << 1U;
        tool.changed[1] = 5;
        f.offer();
        REQUIRE(f.sessions().sessions.size() == 1);
        const auto& work = w.beings().raw().get<kd::world::Work>(w.beings().handle(f.watcher));
        const auto included = std::any_of(work.inputs.begin(), work.inputs.end(),
                                          [&](const auto& r) { return r.item == f.visible_tool; });
        CHECK(included == learner_owns);
        CHECK(w.things().raw().get<kd::world::Item>(h).owner == holder);
        CHECK(w.things().raw().get<kd::world::Item>(h).mass == 20000);
        f.reopen();
    }
}

TEST_CASE("taught_full_chance grants knowledge on success without a discovery discount") {
    int learned = 0;
    for (std::uint64_t seed = 1; seed <= 100; ++seed) {
        TeachingFixture f(seed);
        f.offer();
        f.camp->world().run_to(7 * kd::time::kHour + 2100);
        if (kd::demo::Learning::knows(f.mind(f.watcher), f.recipe)) {
            ++learned;
            const auto& skill = f.mind(f.watcher).skills.front();
            CHECK(skill.source == f.maker);
            CHECK(skill.route == 5);
            CHECK(skill.practice.level >= 1000);
            const auto peer =
                std::find_if(f.mind(f.maker).peers.begin(), f.mind(f.maker).peers.end(),
                             [&](const auto& p) { return p.person == f.watcher && p.recipe == f.recipe; });
            REQUIRE(peer != f.mind(f.maker).peers.end());
            CHECK(peer->knows == 1);
            CHECK(peer->route == 1);
            CHECK(peer->event == skill.source_event);
        }
        CHECK_FALSE(kd::demo::Learning::knows(f.mind(f.absent), f.recipe));
    }
    // A level-zero learner in this rested, unskilled sector has 40% maker chance. A discount cannot pass this sample.
    CHECK(learned >= 25);
    CHECK(learned <= 60);
}
TEST_CASE("partial shared lesson reopens and resumes gradual work with no duplicate practice") {
    TeachingFixture direct, saved;
    direct.offer();
    saved.offer();
    const auto pause = 7 * kd::time::kHour + 310;
    for (auto* f : {&direct, &saved}) {
        f->camp->world().run_to(pause - 1);
        f->camp->world().schedule(f->watcher, 1, pause);
        f->camp->world().run_to(pause + 1);
        REQUIRE(f->sessions().sessions.size() == 1);
        INFO(f->sessions().sessions[0].begun);
        INFO(f->sessions().sessions[0].seconds);
        INFO(f->sessions().sessions[0].settled);
        CHECK(f->sessions().sessions[0].state == 2);
        CHECK(f->sessions().sessions[0].seconds == f->sessions().sessions[0].credited_seconds);
        const auto& work =
            f->camp->world().beings().raw().get<kd::world::Work>(f->camp->world().beings().handle(f->watcher));
        CHECK(work.state == 4);
        CHECK(work.retained_progress > 0);
        CHECK_FALSE(kd::demo::Learning::knows(f->mind(f->watcher), f->recipe));
    }
    saved.reopen();
    saved.camp->world().beings().fuzz(71);
    saved.camp->world().things().fuzz(13);
    kd::run::Workers workers(4);
    direct.camp->world().run_to(pause + 2200);
    saved.camp->world().run_islands(pause + 2200, workers, 1);
    CHECK(direct.camp->world().digests().whole == saved.camp->world().digests().whole);
    saved.reopen();
}
TEST_CASE("shared attendance intervals match the game-second reference through daylight and dusk") {
    for (const auto begun : {7 * kd::time::kHour, 19 * kd::time::kHour + 30 * kd::time::kMinute}) {
        TeachingFixture fast(17, begun), reference(17, begun);
        reference.camp->world().set_scalar_work(true);
        fast.offer();
        reference.offer();
        const auto start = fast.camp->world().frontier();
        for (const auto elapsed : {0, 30, 900, 1800, 3600}) {
            fast.camp->world().run_to(start + elapsed);
            reference.camp->world().run_to(start + elapsed);
            CHECK(fast.camp->world().digests().whole == reference.camp->world().digests().whole);
            fast.reopen();
            reference.reopen();
            reference.camp->world().set_scalar_work(true);
        }
    }
}
TEST_CASE("a paused lesson meets at the teacher's new position before shared practice resumes") {
    TeachingFixture f;
    f.offer();
    auto& w = f.camp->world();
    const auto pause = 7 * kd::time::kHour + 310;
    w.run_to(pause - 1);
    w.schedule(f.watcher, 1, pause);
    w.run_to(pause + 1);
    REQUIRE(f.sessions().sessions.size() == 1);
    REQUIRE(f.sessions().sessions.front().state == 2);
    f.at(w.frontier(), 920);
    const auto teacher = w.beings().handle(f.maker), learner = w.beings().handle(f.watcher);
    const auto meeting = w.beings().raw().get<kd::world::Place>(teacher).at;
    REQUIRE(f.sessions().sessions.size() == 1);
    CHECK(f.sessions().sessions.front().meeting == meeting);
    CHECK(f.sessions().sessions.front().state == 0);
    CHECK(w.beings().raw().get<kd::world::Place>(learner).at != meeting);
    CHECK(w.beings().raw().get<kd::world::Work>(learner).retained_progress > 0);
    bool attended = false;
    w.beings().each([&](kd::ecs::Id, auto h) {
        const auto* mind = w.beings().raw().try_get<kd::world::Knowledge>(h);
        if (!mind) return;
        for (const auto& seen : mind->observations)
            attended = attended || (seen.person == f.watcher && seen.weighted_seconds > 0);
    });
    CHECK(attended);
    auto& gathering = w.beings().raw().get<kd::world::Work>(learner);
    const auto retained = gathering.retained_progress;
    gathering.retained_progress = 0;
    std::string why;
    const auto invalid = kd::save::read_snapshot(kd::save::write_snapshot(w.save()), why);
    REQUIRE(invalid);
    if (!invalid) return;
    CHECK_FALSE(kd::demo::CrowdWorld::open(watch_catalogue(), *invalid, why));
    CHECK(why == "observation attendance disagrees with work");
    gathering.retained_progress = retained;
    f.reopen();
    auto& resumed = f.camp->world();
    resumed.run_to(resumed.frontier() + 120);
    REQUIRE(f.sessions().sessions.size() == 1);
    CHECK(f.sessions().sessions.front().state == 1);
    CHECK(resumed.beings().raw().get<kd::world::Place>(resumed.beings().handle(f.watcher)).at == meeting);
    f.reopen();
}
TEST_CASE("urgent learners and unfinished plans decline offers and telling never interrupts") {
    TeachingFixture f;
    const auto now = f.camp->world().frontier();
    f.at(now, 910);
    auto& raw = f.camp->world().beings().raw();
    auto& life = raw.get<kd::world::Life>(f.camp->world().beings().handle(f.watcher));
    life.food = 500000;
    // Trigger the teacher's ordinary choice; the urgent learner is not recruited.
    f.camp->world().schedule(f.maker, 0, now + 2);
    f.camp->world().run_to(now + 3);
    CHECK(f.sessions().sessions.empty());
    CHECK_FALSE(kd::demo::Learning::knows(f.mind(f.watcher), f.recipe));
}

TEST_CASE("learning history records actual source and survives labelled last-holder removal") {
    WatchFixture f;
    for (int i = 0; i < 5; ++i) f.use();
    auto& w = f.camp->world();
    auto& history = w.beings().raw().get<kd::world::CraftHistory>(w.beings().handle(f.home));
    const auto learning = std::find_if(history.events.begin(), history.events.end(),
                                       [&](const auto& e) { return e.kind == 2 && e.actor == f.watcher; });
    REQUIRE(learning != history.events.end());
    CHECK(learning->source == f.maker);
    CHECK(learning->route == 4);
    CHECK(learning->at == f.mind(f.watcher).skills.front().practice.last_use);
    REQUIRE_FALSE(learning->inputs.empty());
    const auto source_event = f.mind(f.watcher).skills.front().source_event;
    const auto demonstration =
        std::find_if(history.events.begin(), history.events.end(), [&](const auto& e) { return e.id == source_event; });
    REQUIRE(demonstration != history.events.end());
    CHECK(learning->inputs.front().id == demonstration->inputs.front().id);
    CHECK(learning->result == demonstration->result);
    const auto word = learning->word;
    REQUIRE_FALSE(word.empty());
    // Labelled knowledge-holder removal; this is neither disease nor a discovery gate.
    f.mind(f.maker).skills.clear();
    f.mind(f.watcher).skills.clear();
    f.at(w.frontier(), 905);
    CHECK(history.events.back().kind == 3);
    CHECK(history.events.back().word == word);
    CHECK_FALSE(kd::demo::Learning::knows(f.mind(f.absent), f.recipe));
    const auto count = history.events.size();
    f.at(w.frontier(), 905);
    CHECK(history.events.size() == count);
    // Real unknown-use rolls after the labelled removal; no granted knowledge or altered chance.
    f.intended = false;
    f.duration = 1800;  // a real bare-handed butcher fit takes its full declared duration
    bool returned = false;
    for (int attempt = 0; attempt < 100 && !returned; ++attempt) {
        f.use();
        returned = std::any_of(history.events.begin(), history.events.end(),
                               [&](const auto& e) { return e.kind == 4 && e.recipe == f.recipe; });
    }
    REQUIRE(returned);
    const auto return_event = std::find_if(history.events.begin(), history.events.end(),
                                           [&](const auto& e) { return e.kind == 4 && e.recipe == f.recipe; });
    CHECK(return_event->word == word);
    CHECK(return_event->actor == f.maker);
    CHECK(return_event->result.value != 0);
    f.reopen();
    CHECK_FALSE(kd::demo::Learning::knows(f.mind(f.watcher), f.recipe));
}

TEST_CASE("finite learning scene preserves budgets and continuation after reopen and four workers") {
    kd::demo::CrowdWorld direct(31, watch_catalogue(), 1, true, true);
    kd::demo::CrowdWorld other(31, watch_catalogue(), 1, true, true);
    kd::proof::learning_reserves(direct, false);
    kd::proof::learning_reserves(other, false);
    const auto home = direct.camp_ids().front();
    const auto environment = [&]() -> const kd::world::Habitat& {
        return direct.world().beings().raw().get<kd::world::Habitat>(direct.world().beings().handle(home));
    };
    CHECK(environment().crop_budget_mg == 40000000000LL);
    CHECK(environment().root_water_ml == 40000000);
    CHECK(environment().upstream_ml == 120000000);
    const auto initial_stone = kd::demo::Crafting::total(direct.world(), home, "stone");
    CHECK(initial_stone >= 10000000000LL);
    direct.world().run_to(7 * kd::time::kHour);
    other.world().run_to(7 * kd::time::kHour);
    std::string why;
    const auto chunks = kd::save::read_snapshot(kd::save::write_snapshot(other.world().save()), why);
    REQUIRE(chunks);
    if (!chunks) return;
    auto saved = kd::demo::CrowdWorld::open(watch_catalogue(), *chunks, why);
    INFO(why);
    REQUIRE(saved);
    saved->world().beings().fuzz(313);
    saved->world().things().fuzz(317);
    direct.world().run_to(kd::time::kDay);
    kd::run::Workers workers(4);
    saved->world().run_islands(kd::time::kDay, workers, 1);
    CHECK(direct.world().digests().whole == saved->world().digests().whole);
    CHECK(environment().crop_budget_mg + environment().food_grown == 40000000000LL);
    CHECK(environment().upstream_ml + environment().water_added == 120000000);
    CHECK(kd::demo::Crafting::total(direct.world(), home, "stone") == initial_stone);
}
TEST_CASE("twenty skill-curve calibrations use seeded maker chance on the declared daily calendar") {
    const auto recipe = watch_entry("blueprint", "base:sharp_flake");
    const auto flint = watch_entry("item", "base:flint");
    const auto granite = watch_entry("item", "base:granite");
    int on_time = 0;
    for (std::uint64_t seed = 1; seed <= 20; ++seed) {
        WatchFixture f(false, 500, 7 * kd::time::kHour, seed);
        auto& w = f.camp->world();
        auto& skill = f.mind(f.maker).skills.front();
        skill.recipe = recipe;
        skill.practice = {1000, 1000, 0, 0};
        auto& core = w.things().raw().get<kd::world::Item>(w.things().handle(f.input));
        core.kind = flint;
        core.material = flint;
        core.length = 120;
        const auto hh = w.make_thing();
        auto& hammer = w.things().raw().emplace<kd::world::Item>(hh);
        hammer.kind = granite;
        hammer.material = granite;
        hammer.mass = 2000000;
        hammer.length = 120;
        const std::array roles{f.input, w.things().id_of(hh)};
        std::int64_t reached = -1;
        for (std::int64_t day = 0; day <= 3LL * 60 && reached < 0; ++day) {
            if (day % 7 == 6) continue;
            const kd::chance::Draws draws(seed, kd::chance::name("practice calibration"), f.maker.value, day,
                                          kd::chance::name("physical maker trials"));
            for (std::uint64_t attempt = 0; attempt < 120 && reached < 0; ++attempt) {
                const auto chance = kd::demo::Crafting::success(w, w.beings().handle(f.maker), recipe, roles);
                const bool success = draws.below(attempt, 1000000) < static_cast<std::uint64_t>(chance);
                const auto at = day * kd::time::kDay + static_cast<std::int64_t>(attempt + 1) * 30;
                kd::demo::Learning::practice(skill.practice, at, 30, success, f.mind(f.maker).learning_ppm);
                if (skill.practice.level >= 5000) reached = at;
            }
        }
        MESSAGE("calendar seed " << seed << " skill 5 at day " << double(reached) / kd::time::kDay);
        if (reached >= 90 * kd::time::kDay && reached <= 180 * kd::time::kDay) ++on_time;
        auto unused = reopen(skill.practice);
        kd::demo::Learning::fade(unused, skill.practice.last_use + 10 * kd::time::kYear);
        CHECK(unused.level >= (unused.best + 1) / 2);
        CHECK(unused.level < unused.best);
    }
    // This calibrates the real maker-chance/credit equations, not autonomous practice or population spread.
    CHECK(on_time >= 16);
}

TEST_CASE("brief practice does not round fractional fading into a whole-thousandth loss") {
    kd::world::Practice skill{1000, 1000, 0, 0};
    for (int attempt = 1; attempt <= 10; ++attempt) kd::demo::Learning::practice(skill, attempt * 30LL, 30, false);
    CHECK(skill.level == 1001);
    CHECK(skill.seconds == 300);
}

TEST_CASE("stationary observation batches exactly across dawn and dusk while moving sight stays exact") {
    for (const auto begun :
         {5 * kd::time::kHour + 45 * kd::time::kMinute, 19 * kd::time::kHour + 45 * kd::time::kMinute}) {
        for (const bool moving : {false, true}) {
            WatchFixture f(false, 400, begun);
            f.duration = kd::time::kHour;
            const auto at = f.start();
            auto& w = f.camp->world();
            auto& raw = w.beings().raw();
            auto& observer = raw.get<kd::world::Activity>(w.beings().handle(f.watcher));
            if (moving) {
                observer.start = at;
                observer.end = at + f.duration;
                observer.to = w.torus().moved(observer.from, {700, 0});
            }
            const auto demonstration = raw.get<kd::world::Activity>(w.beings().handle(f.maker));
            const auto until = at + f.duration - 1;
            std::int64_t expected = 0;
            for (auto t = at; t < until; ++t)
                if (kd::demo::Learning::can_watch(w, f.home, observer.at(w.torus(), t), demonstration.at(w.torus(), t),
                                                  t))
                    expected += 4;
            f.at(until, 902);
            REQUIRE(f.mind(f.watcher).observations.size() == 1);
            CHECK(f.mind(f.watcher).observations.front().weighted_seconds == expected);
        }
    }
}

TEST_CASE("item position index follows event mutations births and spent stock without saved state") {
    WatchFixture f;
    struct Mutations : kd::world::System {
        std::string_view name() const override { return "item index mutation fixture"; }
        void handle(kd::world::Context&, const kd::event::Event&) override {}
        void command(kd::world::Context& c, const kd::world::Command& cmd) override {
            auto& w = c.world();
            const kd::ecs::Id id{cmd.a};
            const auto present = [&](kd::ecs::Id target, kd::num::Point at) {
                bool found = false;
                for (const auto& site : std::as_const(w).item_sites())
                    for (const auto& entry : site.items)
                        if (entry.id == target) {
                            CHECK(site.at == at);
                            found = true;
                        }
                return found;
            };
            const auto h = w.things().handle(id);
            const auto here = w.things().raw().get<kd::world::Place>(h).at;
            CHECK(present(id, here));
            auto& item = w.things().raw().get<kd::world::Item>(h);
            item.mass = 0;
            c.item_changed(id);
            CHECK_FALSE(present(id, here));
            item.mass = 1000000;
            item.state = 2;
            const auto moved = w.torus().moved(here, {200, 0});
            w.things().raw().get<kd::world::Place>(h).at = moved;
            c.item_changed(id);
            CHECK(present(id, moved));
            const auto copy = item;
            const auto born = w.make_thing();
            const auto new_id = w.things().id_of(born);
            w.things().raw().emplace<kd::world::Place>(born, here);
            w.things().raw().emplace<kd::world::Item>(born, copy);
            c.item_changed(new_id);
            CHECK(present(new_id, here));
            w.things().end(new_id);
            c.item_changed(new_id);
            CHECK_FALSE(present(new_id, here));
        }
    } mutations;
    auto& w = f.camp->world();
    w.set_command_taker(mutations);
    (void)w.command(w.frontier(), 906, f.input.value, 0);
    w.run_to(w.frontier() + 1);
}
TEST_CASE("night observation stops exactly when actual task-light fuel runs out") {
    WatchFixture f(false, 200, 0);
    auto& w = f.camp->world();
    const auto here = w.beings().raw().get<kd::world::Place>(w.beings().handle(f.maker)).at;
    const auto h = w.make_thing();
    const auto at = w.torus().moved(here, {100, 0});
    w.things().raw().emplace<kd::world::Place>(h, at);
    auto& item = w.things().raw().emplace<kd::world::Item>(h);
    item.home = f.home;
    item.kind = item.material = watch_entry("item", "base:dry_stick");
    item.mass = 250000;
    item.length = 300;
    auto& fire = w.things().raw().emplace<kd::world::Fire>(h);
    fire.hearth = f.home;
    fire.at = at;
    fire.heat = 2;
    fire.fuel_mg = item.mass;
    fire.next = fire.deadline();
    CHECK(fire.next == 900);
    CHECK(kd::demo::Learning::can_watch(w, f.home, here, w.torus().moved(here, {200, 0}), 0));
    f.use();
    REQUIRE(f.mind(f.watcher).skills.size() == 1);
    CHECK(f.mind(f.watcher).skills.front().observation_quarters == 2);
    CHECK(f.mind(f.watcher).skills.front().observation_remainder == 0);
    f.reopen();
}

TEST_CASE("all_m3_phases_reopen preserves full-kit pending work and continuation") {
    kd::data::Catalogue cat;
    REQUIRE(cat.load(kd::data::read_catalogue(KD_REPO "/data")).empty());
    kd::demo::CrowdWorld camp(43, cat, 1, true, true);
    kd::proof::learning_reserves(camp, false);
    auto& world = camp.world();
    world.run_to(3 * kd::time::kDay);
    std::string why;
    const auto decoded = kd::save::read_snapshot(kd::save::write_snapshot(world.save()), why);
    REQUIRE(decoded);
    if (!decoded) return;
    const auto reopened = kd::demo::CrowdWorld::open(cat, *decoded, why);
    REQUIRE_MESSAGE(reopened, why);
    CHECK(reopened->world().digests().whole == world.digests().whole);
    bool pending = false;
    world.beings().each([&](kd::ecs::Id, auto h) {
        const auto* work = world.beings().raw().try_get<kd::world::Work>(h);
        pending = pending || (work && work->state != 0);
    });
    CHECK(pending);
    world.run_to(3 * kd::time::kDay + kd::time::kHour);
    kd::run::Workers pool(4);
    reopened->world().beings().fuzz(15);
    reopened->world().things().fuzz(16);
    reopened->world().run_islands(world.frontier(), pool, 1);
    CHECK(reopened->world().digests().whole == world.digests().whole);
}

TEST_CASE("teachers make the same visible offer while learners decide from their own private needs") {
    for (const bool hungry : {false, true}) {
        TeachingFixture f;
        auto& w = f.camp->world();
        const auto now = w.frontier();
        f.at(now, 910);
        auto& raw = w.beings().raw();
        auto& body = raw.get<kd::world::Life>(w.beings().handle(f.watcher));
        body.food = hungry ? 500000 : 4000000;
        std::vector<kd::world::Record> replies;
        w.keep_history(&replies);
        f.at(w.frontier(), 912);
        w.keep_history(nullptr);
        const auto offered = std::count_if(replies.begin(), replies.end(), [&](const auto& r) {
            return r.what == 211 && r.a == f.maker.value && r.b == f.watcher.value;
        });
        const auto answer = std::count_if(replies.begin(), replies.end(), [&](const auto& r) {
            return r.what == (hungry ? 214 : 213) && r.a == f.watcher.value && r.b == f.maker.value;
        });
        CHECK(offered == 1);
        CHECK(answer == 1);
        CHECK(f.accepted == !hungry);
        CHECK(f.sessions().sessions.empty() == hungry);
    }
}

TEST_CASE("a rejected teaching option neither offers nor reserves the learner's practice") {
    TeachingFixture f;
    auto& w = f.camp->world();
    f.at(w.frontier(), 910);
    std::vector<kd::world::Record> events;
    w.keep_history(&events);
    f.at(w.frontier(), 918);
    w.keep_history(nullptr);
    CHECK(f.sessions().sessions.empty());
    CHECK_FALSE(std::any_of(events.begin(), events.end(), [](const auto& e) { return e.what == 211; }));
    CHECK(w.beings().raw().get<kd::world::Work>(w.beings().handle(f.watcher)).state == 0);
    const auto& reasons = f.mind(f.maker).reasons;
    REQUIRE(reasons.size() == 3);
    CHECK(reasons[0].kind == 2);
    CHECK(reasons[1].kind == 5);
}

TEST_CASE("the same common teaching offer is accepted only when it wins the learner's own comparison") {
    for (const auto curiosity_met : {50, 100}) {
        TeachingFixture f;
        auto& w = f.camp->world();
        f.at(w.frontier(), 910);
        f.mind(f.watcher).curiosity_need = curiosity_met;
        f.mind(f.watcher).settled = w.frontier();
        std::vector<kd::world::Record> events;
        w.keep_history(&events);
        f.at(w.frontier(), 919);
        w.keep_history(nullptr);
        CHECK(f.accepted == (curiosity_met == 50));
        CHECK(std::count_if(events.begin(), events.end(), [&](const auto& e) {
                  return e.what == 211 && e.a == f.maker.value && e.b == f.watcher.value;
              }) == 1);
        CHECK(std::count_if(events.begin(), events.end(), [&](const auto& e) {
                  return e.what == (curiosity_met == 50 ? 213 : 214) && e.a == f.watcher.value && e.b == f.maker.value;
              }) == 1);
        if (f.accepted) {
            const auto& reasons = f.mind(f.watcher).reasons;
            REQUIRE(reasons.size() == 3);
            CHECK(reasons[0].kind == 5);
            CHECK(reasons[0].need_met == 50);
            CHECK(reasons[0].score > reasons[1].score);
            CHECK(reasons[1].score >= reasons[2].score);
            f.reopen();
        } else {
            CHECK(w.beings().raw().get<kd::world::Work>(w.beings().handle(f.watcher)).state == 0);
            CHECK(f.sessions().sessions.empty());
        }
    }
}
