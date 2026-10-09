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
            } else if (raw.all_of<kd::world::Camp>(h))
                raw.emplace<kd::world::CraftHistory>(h);
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
    // 31305 wrote outer format 1. A deliberately unreadable body proves the version is checked first.
    kd::ByteWriter old;
    old.u64(0x50414e53444b4e49);
    old.u32(1);
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
                           kd::save::tag("DRMS")}) {
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
    for (const auto flags : {0U, 2U, 3U, 16U}) {
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
        CHECK(item.mass > 0);
        CHECK(item.owner.value == 0);
        CHECK(item.made_at == -1);
        CHECK(item.parents.empty());
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
            CHECK(skill.practice.level >= 3000);
            CHECK(skill.source_event == 0);
            CHECK(skill.source.value == 0);
            CHECK(catalogue().kind<kd::data::Blueprint>()[skill.recipe].starting);
        }
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
    CHECK(kd::save::write_snapshot(copy->world().save()) == kd::save::write_snapshot(w.save()));
    CHECK(accepted(w, why));
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
                if (found.value != 0 || w.things().raw().get<kd::world::Item>(th).kind != kind ||
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
            const auto& item = w.things().raw().get<kd::world::Item>(w.things().handle(inputs[i]));
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
            const auto& item = w.things().raw().get<kd::world::Item>(w.things().handle(inputs[i]));
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
    CHECK(item.wear == 1000000);  // one usable-meat deer removes one edge step at ordinary quality
    CHECK(kd::demo::Crafting::characteristics(catalogue(), item)[1] == 4);
    std::string why;
    CHECK(accepted(trial.w, why));
    item.quality = 4;
    item.wear = 0;
    item.wear_remainder = 0;
    (void)trial.w.command(2, 901, trial.person.value, 1);
    trial.w.run_to(3);
    CHECK(item.wear == 0);
    CHECK(item.wear_remainder == 100000);
    CHECK(accepted(trial.w, why));
    const auto before = kd::demo::Crafting::total(trial.w, trial.home, "stone");
    (void)trial.w.command(4, 901, trial.person.value, 200000000);
    trial.w.run_to(5);
    CHECK(trial.w.things().raw().get<kd::world::Item>(trial.w.things().handle(trial.inputs[0])).mass == 0);
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
    WorkFixture trial(91, "base:butcher", 3, {"base:carcass"});
    auto& raw = trial.w.beings().raw();
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
