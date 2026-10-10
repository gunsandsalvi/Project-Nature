#include "doctest.h"
#include "kd/core/pages.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/choice.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/fire.hpp"
#include "kd/demo/living.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/archive.hpp"
#include "kd/save/keeper.hpp"
#include "kd/save/log.hpp"
#include "kd/save/pages.hpp"

namespace {
const kd::data::Catalogue& storage_catalogue() {
    static const auto cat = [] {
        kd::data::Catalogue out;
        KD_CHECK(out.load(kd::data::read_catalogue(KD_REPO "/data")).empty(), "storage fixture catalogue");
        return out;
    }();
    return cat;
}
struct StorageCamp {
    kd::demo::CrowdWorld camp{333, storage_catalogue(), 1, true, true};
    kd::world::World& w = camp.world();
    kd::ecs::Id home = camp.camp_ids().front();
    StorageCamp() { w.retain_records(0); }
    kd::ecs::Id spent() {
        const auto h = w.make_thing();
        w.things().raw().emplace<kd::world::Place>(
            h, w.beings().raw().get<kd::world::Camp>(w.beings().handle(home)).stone_at);
        auto& item = w.things().raw().emplace<kd::world::Item>(h);
        item.home = home;
        item.kind = item.material = storage_catalogue().find("item", "base:flint").value_or(UINT32_MAX);
        item.length = 100;
        item.state = 4;
        return w.things().id_of(h);
    }
};
}  // namespace

TEST_CASE("immutable pages share sealed facts and sparse serial publications preserve old readers") {
    kd::Pages<kd::world::Choice> records;
    for (std::uint64_t n = 1; n <= 1025; ++n) records.push_back({n * 3, 0, {}, {}});
    const auto before = records;
    REQUIRE(records.pages().size() == 2);
    CHECK(before.pages()[0] == records.pages()[0]);
    records.push_back({4000, 0, {}, {}});
    CHECK(before.size() == 1025);
    CHECK(records.size() == 1026);
    REQUIRE(records.find(3069));
    CHECK(records.find(3069)->id == 3069);
    CHECK_FALSE(records.find(3070));
    records.retain([](const auto& record) { return record.id > 1536; });
    CHECK(records.pages()[0] == before.pages()[1]);
    CHECK(before.find(3));
    CHECK_FALSE(records.find(3));
    kd::Pages<kd::time::Seconds> trail;
    for (kd::time::Seconds n = 0; n < 1537; ++n) trail.push_back(n);
    const auto old_trail = trail;
    trail.drop_prefix(513);
    CHECK(trail.size() == 1024);
    CHECK(trail.front() == 513);
    CHECK(trail.back() == 1536);
    CHECK(trail.pages()[1] == old_trail.pages()[2]);
    CHECK(old_trail.front() == 0);
    trail.drop_prefix(1023);
    CHECK(trail.size() == 1);
    CHECK(trail.front() == 1536);
    trail.push_back(1537);
    CHECK(trail.back() == 1537);
    trail.drop_prefix(2);
    CHECK(trail.empty());
    kd::RecordIndex index;
    index.add(100, 5);
    const auto published = index;
    index.add(101, 6);
    CHECK(published.get(100) == 5);
    CHECK(published.get(101) == kd::RecordIndex::kMissing);
    CHECK(index.get(101) == 6);
    CHECK(index.get(UINT64_MAX) == kd::RecordIndex::kMissing);
    index.add(UINT64_MAX, 7);
    CHECK(index.get(UINT64_MAX) == 7);
    CHECK(published.get(UINT64_MAX) == kd::RecordIndex::kMissing);
}

TEST_CASE("spent archival waits for active references and never archives positive stock or ash") {
    StorageCamp fixture;
    auto& w = fixture.w;
    const auto id = fixture.spent();
    const auto person = *w.beings().raw().view<kd::world::Work>().begin();
    auto& work = w.beings().raw().get<kd::world::Work>(person);
    work.inputs.push_back({id});
    const auto positive = fixture.spent();
    auto& stock = w.things().raw().get<kd::world::Item>(w.things().handle(positive));
    stock.mass = 300;
    stock.state = 2;
    const auto stock_kind = stock.kind;
    w.retain_records(0);
    CHECK(w.things().find(id));
    CHECK(w.things().find(positive));
    CHECK_FALSE(w.archived_item(id));
    work.inputs.clear();
    const auto digest = w.digests().whole;
    w.retain_records(0);
    CHECK_FALSE(w.things().find(id));
    REQUIRE(w.archived_item(id));
    CHECK(w.archived_item(id)->item.mass == 0);
    CHECK(w.archived_item(id)->item.kind == stock_kind);
    CHECK(w.things().find(positive));
    CHECK(w.digests().whole == digest);
    const auto newer = fixture.spent();
    CHECK(id < newer);
}

TEST_CASE("checksummed spent pages cannot resurrect quantity duplicate identities or hide active heat") {
    StorageCamp fixture;
    const auto id = fixture.spent();
    fixture.w.retain_records(0);
    std::string why;
    REQUIRE(kd::demo::CrowdWorld::open(storage_catalogue(), fixture.w.save(), why));
    for (int fault = 0; fault < 6; ++fault) {
        auto chunks = fixture.w.save();
        std::vector<kd::world::ArchivedItem> forged(fixture.w.item_archive().begin(), fixture.w.item_archive().end());
        auto& record = forged.back();
        if (fault == 0) record.item.mass = 1;
        if (fault == 1) record.id = forged.front().id;
        if (fault == 2) record.id = fixture.home;
        if (fault == 3) record.archived_at = 1;
        if (fault == 4) record.item.parents.push_back({id});
        if (fault == 5) {
            auto timer = std::make_shared<kd::world::HeatTimer>();
            timer->item = timer->chance_source = id;
            timer->next = 1;
            record.timer = std::move(timer);
        }
        for (auto& chunk : chunks)
            if (chunk.tag == kd::save::tag("ARPG")) {
                kd::ByteWriter wire;
                kd::ecs::PartWriter writer(wire);
                writer.records({"items", "forged archived fixture"}, forged, 512, 120);
                chunk.data = wire.take();
                chunk.immutable.reset();
            }
        // Recompute outer checksums: refusal must come from semantic validation.
        const auto decoded = kd::save::read_snapshot(kd::save::write_snapshot(chunks), why);
        REQUIRE(decoded);
        if (!decoded) return;
        CHECK_FALSE(kd::demo::CrowdWorld::open(storage_catalogue(), *decoded, why));
    }
}

TEST_CASE("two day diagnostic expiry preserves sparse linked and current reasons through reopening") {
    StorageCamp fixture;
    auto& w = fixture.w;
    auto& raw = w.beings().raw();
    const auto ph = *raw.view<kd::world::Knowledge>().begin();
    const auto person = w.beings().id_of(ph);
    auto& mind = raw.get<kd::world::Knowledge>(ph);
    auto& history = raw.get<kd::world::CraftHistory>(w.beings().handle(fixture.home));
    kd::world::CraftReason body;
    body.kind = 2;
    const std::vector<kd::world::CraftReason> reasons(3, body);
    for (const auto id : {1U, 2U, 17U}) history.choices.push_back({id, 0, person, reasons});
    history.next_choice = 18;
    mind.choice = 17;
    mind.reasons = reasons;
    kd::ecs::Id input{};
    w.things().each([&](auto id, auto h) {
        if (!input.value && w.things().raw().all_of<kd::world::Item>(h)) input = id;
    });
    kd::world::Result result;
    result.id = history.next++;
    result.choice = 1;
    result.actor = person;
    result.place = raw.get<kd::world::Place>(ph).at;
    result.recipe = storage_catalogue().find("blueprint", "base:butcher").value_or(UINT32_MAX);
    result.inputs.push_back({input});
    history.events.push_back(result);
    kd::world::Memory remembered;
    remembered.id = mind.next_memory++;
    remembered.event = result.id;
    remembered.place = result.place;
    remembered.participants.push_back({person});
    mind.memories.push_back(std::move(remembered));
    // Isolated storage fixture, not a simulation outcome: advance only the expiry query.
    w.retain_records(3 * kd::time::kDay);
    CHECK(history.choices.find(1));
    CHECK_FALSE(history.choices.find(2));
    CHECK(history.choices.find(17));
    CHECK(history.next_choice == 18);
    std::string why;
    const auto chunks = kd::save::read_snapshot(kd::save::write_snapshot(w.save()), why);
    REQUIRE_MESSAGE(chunks, why);
    if (!chunks) return;
    const auto reopened = kd::demo::CrowdWorld::open(storage_catalogue(), *chunks, why);
    REQUIRE_MESSAGE(reopened, why);
    if (!reopened) return;
    CHECK(reopened->world().digests().whole == w.digests().whole);
}

TEST_CASE("packed and reference storage retain identical facts events and continuations") {
    StorageCamp fixture;
    for (int n = 0; n < 700; ++n) (void)fixture.spent();
    std::string why;
    auto reference = kd::demo::CrowdWorld::open(storage_catalogue(), fixture.w.save(), why);
    REQUIRE_MESSAGE(reference, why);
    reference->world().set_archive_enabled(false);
    kd::run::Workers workers(4);
    for (int day = 1; day <= 4; ++day) {
        fixture.w.run_to(day * kd::time::kDay);
        reference->world().run_islands(day * kd::time::kDay, workers, 3600);
        CHECK(fixture.w.digests().whole == reference->world().digests().whole);
        CHECK(fixture.w.events_run() == reference->world().events_run());
    }
    CHECK(fixture.w.item_archive().size() >= 700);
    auto reopened = kd::demo::CrowdWorld::open(storage_catalogue(), fixture.w.save(), why);
    REQUIRE_MESSAGE(reopened, why);
    if (!reopened) return;
    CHECK(reopened->world().digests().whole == fixture.w.digests().whole);
    fixture.w.run_to(5 * kd::time::kDay);
    reopened->world().run_to(5 * kd::time::kDay);
    reference->world().run_to(5 * kd::time::kDay);
    CHECK(reopened->world().digests().whole == fixture.w.digests().whole);
    CHECK(reference->world().digests().whole == fixture.w.digests().whole);
}

TEST_CASE("exhausted exposure facts stay identical after archive and later camp refreshes") {
    StorageCamp fixture;
    auto& w = fixture.w;
    const auto id = fixture.spent();
    const auto h = w.things().handle(id);
    auto& item = w.things().raw().get<kd::world::Item>(h);
    item.kind = item.material = storage_catalogue().find("item", "base:roots").value_or(UINT32_MAX);
    auto& timer = w.things().raw().emplace<kd::world::HeatTimer>(h);
    timer.item = timer.chance_source = id;
    std::string why;
    auto reference = kd::demo::CrowdWorld::open(storage_catalogue(), w.save(), why);
    REQUIRE_MESSAGE(reference, why);
    if (!reference) return;
    reference->world().set_archive_enabled(false);
    w.retain_records(0);
    REQUIRE(w.archived_item(id));
    struct Refresh : kd::world::System {
        kd::ecs::Id home{};
        std::string_view name() const override { return "exhausted exposure refresh fixture"; }
        void handle(kd::world::Context&, const kd::event::Event&) override {}
        void command(kd::world::Context& c, const kd::world::Command&) override {
            kd::demo::FireRules::food_refresh(c, home);
        }
    } packed, full;
    packed.home = full.home = fixture.home;
    w.set_command_taker(packed);
    reference->world().set_command_taker(full);
    for (const auto at : {1000, 2000}) {
        (void)w.command(at, 1002, 0, 0);
        (void)reference->world().command(at, 1002, 0, 0);
        w.run_to(at + 1);
        reference->world().run_to(at + 1);
        CHECK(w.digests().whole == reference->world().digests().whole);
        CHECK(w.archived_item(id)->timer->settled_at == 0);
    }
}

TEST_CASE("one million synthetic spent records leave no active chooser or thermal iteration entries") {
    StorageCamp fixture;
    auto& w = fixture.w;
    const auto active = w.things().size();
    const auto original_archive = w.item_archive().size();
    // Labelled storage load, never fed to an acceptance observer or outcome gate.
    for (int n = 0; n < 1000000; ++n) (void)fixture.spent();
    w.retain_records(0);
    REQUIRE(w.item_archive().size() == original_archive + 1000000);
    CHECK(w.things().size() == active);
    std::size_t physical_visits = 0, spent_visits = 0;
    w.things().each([&](auto, auto h) {
        ++physical_visits;
        if (w.things().raw().get<kd::world::Item>(h).mass == 0) ++spent_visits;
    });
    CHECK(physical_visits == active);
    CHECK(spent_visits == 0);
    std::size_t chooser_entries = 0;
    for (const auto& site : std::as_const(w).item_sites()) chooser_entries += site.items.size();
    CHECK(chooser_entries <= active);
    StorageCamp baseline;
    struct Probe : kd::world::System {
        kd::demo::CrowdWorld* camp = nullptr;
        kd::world::World::ItemVisits measured{};
        std::string_view name() const override { return "storage visit probe"; }
        void handle(kd::world::Context&, const kd::event::Event&) override {}
        void command(kd::world::Context& c, const kd::world::Command&) override {
            c.world().clear_item_visits();
            kd::demo::FireRules::food_refresh(c, camp->camp_ids().front());
            const auto person = *c.world().beings().raw().view<kd::world::Knowledge>().begin();
            // Isolated cost query: both observers stand beside the same ordinary stock.
            c.world().beings().raw().get<kd::world::Place>(person).at =
                c.world()
                    .beings()
                    .raw()
                    .get<kd::world::Camp>(c.world().beings().handle(camp->camp_ids().front()))
                    .stone_at;
            kd::demo::ChoiceSet proposals;
            (void)kd::demo::Crafting::choose(*const_cast<kd::demo::Living*>(camp->living()), c, person, &proposals);
            measured = c.world().item_visits();
        }
    } ordinary, archived;
    ordinary.camp = &baseline.camp;
    archived.camp = &fixture.camp;
    baseline.w.set_command_taker(ordinary);
    w.set_command_taker(archived);
    (void)baseline.w.command(0, 1001, 0, 0);
    (void)w.command(0, 1001, 0, 0);
    baseline.w.run_to(1);
    w.run_to(1);
    CHECK(archived.measured.reachable == ordinary.measured.reachable);
    CHECK(archived.measured.thermal == ordinary.measured.thermal);
    CHECK(archived.measured.reachable > 0);
    CHECK(archived.measured.thermal == std::as_const(w).physical_items(fixture.home).food.size());
    CHECK(archived.measured.reachable_spent == 0);
    CHECK(archived.measured.thermal_spent == 0);
    const auto first = w.item_archive().front().id, last = w.item_archive().back().id;
    CHECK(w.archived_item(first));
    CHECK(w.archived_item(last));
    CHECK(w.archive_index().get(last.value & ((std::uint64_t{1} << 60U) - 1)) == original_archive + 999999);
}

TEST_CASE("page publication survives each power cut and damaged pages fall back before replay") {
    StorageCamp fixture;
    const auto old_digest = fixture.w.digests().whole;
    kd::save::FakeFiles before;
    {
        kd::save::Keeper keeper(before, "storage-test");
        keeper.snapshot(fixture.w);
        keeper.flush();
        REQUIRE_FALSE(keeper.failed());
    }
    for (int n = 0; n < 513; ++n) (void)fixture.spent();
    fixture.w.run_to(1);
    const auto new_digest = fixture.w.digests().whole;
    auto successful = before;
    const auto initial_calls = successful.calls();
    {
        kd::save::Keeper keeper(successful, "storage-test");
        keeper.snapshot(fixture.w);
        keeper.flush();
        REQUIRE_FALSE(keeper.failed());
    }
    const auto calls = successful.calls() - initial_calls;
    REQUIRE(successful.list("pages").size() == 1);
    for (std::uint64_t stop = 0; stop <= calls; ++stop) {
        auto files = before;
        files.stop_after(stop);
        {
            kd::save::Keeper keeper(files, "storage-test");
            keeper.snapshot(fixture.w);
            keeper.flush();
        }
        files.restart();
        files.power_cut();
        kd::save::Keeper keeper(files, "storage-test");
        const auto found = keeper.open();
        REQUIRE(found.snapshot);
        if (!found.snapshot) return;
        std::string why;
        const auto reopened = kd::demo::CrowdWorld::open(storage_catalogue(), *found.snapshot, why);
        REQUIRE_MESSAGE(reopened, why);
        if (!reopened) return;
        const auto digest = reopened->world().digests().whole;
        CHECK((digest == old_digest || digest == new_digest));
    }
    SUBCASE("a damaged constituent rejects the whole new snapshot") {
        auto files = successful;
        files.raw("pages/" + files.list("pages").front()).front() ^= std::byte{1};
        kd::save::Keeper keeper(files, "storage-test");
        const auto found = keeper.open();
        REQUIRE(found.snapshot);
        if (!found.snapshot) return;
        CHECK(found.damaged.size() == 1);
        std::string why;
        const auto reopened = kd::demo::CrowdWorld::open(storage_catalogue(), *found.snapshot, why);
        REQUIRE_MESSAGE(reopened, why);
        if (!reopened) return;
        CHECK(reopened->world().digests().whole == old_digest);
    }
    SUBCASE("export includes exactly the referenced immutable constituents") {
        kd::save::ArchiveWriter writer(successful);
        kd::save::FakeFiles imported;
        kd::save::ArchiveReader reader(imported);
        for (;;) {
            const auto bytes = writer.next(997);
            if (bytes.empty()) break;
            REQUIRE_MESSAGE(reader.feed(bytes), reader.why());
        }
        REQUIRE_MESSAGE(reader.finish(), reader.why());
        kd::save::Keeper keeper(imported, "storage-test");
        const auto found = keeper.open();
        REQUIRE(found.snapshot);
        if (!found.snapshot) return;
        std::string why;
        const auto reopened = kd::demo::CrowdWorld::open(storage_catalogue(), *found.snapshot, why);
        REQUIRE_MESSAGE(reopened, why);
        if (!reopened) return;
        CHECK(reopened->world().digests().whole == new_digest);
    }
}

// checks: PLT-10 MAT-10 PLT-07 RES-05 TIM-17

TEST_CASE("retained result links expire identically after page certification and public history thinning") {
    StorageCamp indexed, scalar;
    scalar.w.set_scalar_work(true);
    const auto prepare = [](StorageCamp& fixture) {
        auto& history = fixture.w.beings().raw().get<kd::world::CraftHistory>(fixture.w.beings().handle(fixture.home));
        for (std::uint64_t n = 1; n <= 1025; ++n) {
            const auto choice = history.next_choice++;
            history.choices.push_back({choice, 0, {}, {}});
            kd::world::Result result;
            result.id = history.next++;
            result.choice = choice;
            result.kind = 2;
            history.events.push_back(result);
        }
        history.choices.push_back({history.next_choice++, 0, {}, {}});
    };
    prepare(indexed);
    prepare(scalar);
    for (const auto at : {kd::time::Seconds{0}, 3 * kd::time::kDay, 4 * kd::time::kDay, 26 * kd::time::kYear}) {
        indexed.w.retain_records(at);
        scalar.w.retain_records(at);
        CHECK(indexed.w.digests().whole == scalar.w.digests().whole);
        const auto& history = std::as_const(indexed.w).beings().raw().get<kd::world::CraftHistory>(
            std::as_const(indexed.w).beings().handle(indexed.home));
        CHECK(history.choices.size() == (at == 0 ? 1026 : at < 26 * kd::time::kYear ? 1025 : 1));
    }
    // An external mutable access invalidates derived certification before a
    // linked event can be removed without a simulated event.
    prepare(indexed);
    prepare(scalar);
    indexed.w.retain_records(0);
    scalar.w.retain_records(0);
    for (auto* fixture : {&indexed, &scalar})
        fixture->w.beings()
            .raw()
            .get<kd::world::CraftHistory>(fixture->w.beings().handle(fixture->home))
            .events.clear();
    indexed.w.retain_records(3 * kd::time::kDay);
    scalar.w.retain_records(3 * kd::time::kDay);
    CHECK(indexed.w.digests().whole == scalar.w.digests().whole);
    CHECK(std::as_const(indexed.w)
              .beings()
              .raw()
              .get<kd::world::CraftHistory>(std::as_const(indexed.w).beings().handle(indexed.home))
              .choices.empty());
}

TEST_CASE("daily routine counters equal actual finite intake and resolved craft through batching and reopen") {
    StorageCamp fixture;
    auto& w = fixture.w;
    std::vector<kd::world::Record> trace;
    w.keep_history(&trace);
    std::uint64_t cursor = 0, resolved = 0, eaten = 0;
    for (kd::time::Seconds at = kd::time::kHour; at <= 3 * kd::time::kDay; at += kd::time::kHour) {
        w.run_to(at);
        const auto& history = std::as_const(w).beings().raw().get<kd::world::CraftHistory>(
            std::as_const(w).beings().handle(fixture.home));
        auto first = std::upper_bound(history.events.begin(), history.events.end(), cursor,
                                      [](auto id, const auto& event) { return id < event.id; });
        for (; first != history.events.end(); ++first) {
            cursor = first->id;
            if ((first->kind == 0 || first->kind == 1 || first->kind == 5) &&
                storage_catalogue().kind<kd::data::Blueprint>()[first->recipe].heat == 0)
                ++resolved;
        }
        for (const auto& record : trace)
            if (record.what == 202) eaten += record.b;
        trace.clear();
    }
    const auto& routine = std::as_const(w)
                              .beings()
                              .raw()
                              .get<kd::world::CraftHistory>(std::as_const(w).beings().handle(fixture.home))
                              .routine;
    std::uint64_t tries = 0, intake = 0;
    const auto sum = [&](const auto& rows) {
        for (const auto& row : rows) {
            if (row.type == 0) tries += row.tries;
            if (row.type == 1) intake += static_cast<std::uint64_t>(row.eaten_mg);
        }
    };
    sum(routine.days);
    sum(routine.current);
    CHECK(tries == resolved);
    CHECK(intake == eaten);
    CHECK(tries > 0);
    CHECK(routine.days.size() + routine.current.size() < tries);
    std::string why;
    auto copy = kd::demo::CrowdWorld::open(storage_catalogue(), w.save(), why);
    REQUIRE_MESSAGE(copy, why);
    if (!copy) return;
    CHECK(copy->world().digests().whole == w.digests().whole);
    w.run_to(4 * kd::time::kDay);
    copy->world().run_to(4 * kd::time::kDay);
    CHECK(copy->world().digests().whole == w.digests().whole);
    w.keep_history(nullptr);
}

TEST_CASE("compacted journals allow only declared public prefix gaps and reject missing replay frames") {
    kd::ByteWriter marker;
    marker.i64(100);
    marker.u64(8);
    const auto frame = kd::save::frame(4, 0, marker.bytes());
    const auto person = (std::uint64_t(kd::ecs::Family::person) << 60U) | 1;
    const kd::world::Record first{{50, person, 1}, 0, 200, 1, 1}, replay{{100, person, 2}, 0, 202, 1, 2},
        later{{101, person, 3}, 0, 202, 1, 3};
    const auto make = [&](bool lose_first, bool lose_middle, bool private_prefix) {
        auto bytes = frame;
        const auto add = [&](const auto& record, auto sequence, bool kept) {
            const auto raw = kd::save::record_frame(record, sequence, kept);
            bytes.insert(bytes.end(), raw.begin(), raw.end());
        };
        add(first, 3, !private_prefix);
        if (!lose_first) add(replay, 8, false);
        add(later, lose_middle ? 10 : 9, false);
        return kd::save::read_year(bytes);
    };
    const auto whole = make(false, false, false);
    CHECK_FALSE(whole.cut);
    CHECK(whole.compacted_before == 100);
    CHECK(whole.records.size() == 3);
    CHECK(whole.next_sequence == 10);
    auto prefix = frame;
    const auto public_record = kd::save::record_frame(first, 3, true);
    prefix.insert(prefix.end(), public_record.begin(), public_record.end());
    const auto empty_tail = kd::save::read_year(prefix);
    CHECK_FALSE(empty_tail.cut);
    CHECK(empty_tail.next_sequence == 8);
    const auto appended = kd::save::record_frame(replay, 8, false);
    prefix.insert(prefix.end(), appended.begin(), appended.end());
    CHECK_FALSE(kd::save::read_year(prefix).cut);
    CHECK(make(true, false, false).cut);
    CHECK(make(false, true, false).cut);
    CHECK(make(false, false, true).cut);
    kd::ByteWriter invalid;
    invalid.i64(100);
    invalid.u64(0);
    CHECK(kd::save::read_year(kd::save::frame(4, 0, invalid.bytes())).cut);
}

TEST_CASE("a corrected public prefix lowers the compacted replay floor without hiding a later gap") {
    kd::save::FakeFiles files;
    kd::ByteWriter marker;
    marker.i64(100);
    marker.u64(8);
    auto bytes = kd::save::frame(4, 0, marker.bytes());
    const auto person = (std::uint64_t(kd::ecs::Family::person) << 60U) | 1;
    const kd::world::Record original{{50, person, 1}, 0, 200, 1, 1}, replay{{100, person, 2}, 0, 202, 1, 2};
    for (const auto& frame : {kd::save::record_frame(original, 3, true), kd::save::record_frame(replay, 8, false)})
        bytes.insert(bytes.end(), frame.begin(), frame.end());
    REQUIRE(files.write_whole(kd::save::year_file(1), bytes));
    auto corrected = original;
    corrected.b = 99;
    {
        kd::save::Keeper keeper(files);
        CHECK(keeper.open().problem.empty());
        keeper.keep_kinds([](const auto& record) { return record.what == 200; });
        const std::array records{corrected, replay};
        keeper.history(records);
        keeper.flush();
        CHECK_FALSE(keeper.failed());
        CHECK(keeper.mismatches() == 1);
    }
    const auto journal_bytes = files.read(kd::save::year_file(1));
    REQUIRE(journal_bytes.has_value());
    if (!journal_bytes) return;
    const auto held = kd::save::read_year(*journal_bytes);
    CHECK_FALSE(held.cut);
    CHECK(held.compacted_before == 50);
    REQUIRE(held.records.size() == 2);
    CHECK(held.records[0].record == corrected);
    CHECK(held.records[1].record == replay);
    CHECK(held.records[0].sequence == 3);
    CHECK(held.records[1].sequence == 4);
}
