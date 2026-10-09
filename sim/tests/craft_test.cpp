#include "kd/data/craft.hpp"
#include "doctest.h"
#include "kd/data/checks.hpp"
#include "kd/data/folder.hpp"
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
    const auto kept = kd::demo::keep_crowd(keeper, catalogue(), 17, 1, {}, true);
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
    std::erase_if(
        files, [](const auto& f) { return f.path.starts_with("base/item/") || f.path.starts_with("base/blueprint/"); });
    for (auto& f : files)
        if (f.path == "base/source.toml") f.text.replace(f.text.find("version = 4"), 11, "version = 3");
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
    const auto kept = kd::demo::keep_crowd(keeper, catalogue(), 17, 1, {}, true);
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
        auto opened = kd::demo::keep_crowd(keeper, catalogue(), 17, 1, {}, true);
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
