#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>

#include "doctest.h"
#include "kd/chance/chance.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/proof/fixture.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
#include "kd/save/log.hpp"
#include "kd/save/snapshot.hpp"

namespace {

using kd::save::Bytes;

Bytes bytes_of(const std::string& s) {
    Bytes out;
    for (const char c : s) {
        out.push_back(static_cast<std::byte>(c));
    }
    return out;
}

// Bytes that compress poorly and well, from a seed.
Bytes stuff(std::uint64_t seed, std::size_t n) {
    Bytes out;
    std::uint64_t x = seed;
    for (std::size_t i = 0; i < n; ++i) {
        x = x * 6364136223846793005ULL + 1442695040888963407ULL;
        out.push_back(static_cast<std::byte>(i % 3 == 0 ? x >> 56U : i & 7U));
    }
    return out;
}

std::vector<kd::save::Chunk> chunks() {
    return {{kd::save::tag("WRLD"), 1, true, stuff(1, 100)},
            {kd::save::tag("BEIN"), 3, true, stuff(2, 20'000)},
            {kd::save::tag("NOTE"), 1, false, stuff(3, 0)}};
}

bool same(const std::vector<kd::save::Chunk>& a, const std::vector<kd::save::Chunk>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].tag != b[i].tag || a[i].version != b[i].version || a[i].critical != b[i].critical ||
            a[i].data != b[i].data) {
            return false;
        }
    }
    return true;
}

const kd::data::Catalogue& fixture() {
    static const kd::data::Catalogue catalogue = [] {
        kd::data::Catalogue c;
        const auto files = kd::proof::fixture_files();
        REQUIRE(c.load(files).empty());
        return c;
    }();
    return catalogue;
}

// A crowd's world through a snapshot file and back.
std::unique_ptr<kd::demo::CrowdWorld> reopened(const kd::demo::CrowdWorld& crowd) {
    const Bytes file = kd::save::write_snapshot(crowd.world().save());
    std::string why;
    const auto chunks = kd::save::read_snapshot(file, why);
    REQUIRE(chunks.has_value());
    std::unique_ptr<kd::demo::CrowdWorld> out =
        kd::demo::CrowdWorld::open(fixture(), chunks.value_or(std::vector<kd::save::Chunk>{}), why);
    INFO(why);
    REQUIRE(out != nullptr);
    return out;
}

// The camps' ids of a crowd, in order.
std::vector<kd::ecs::Id> camps_of(const kd::world::World& w) {
    std::vector<kd::ecs::Id> out;
    w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle /*h*/) {
        if (id.family() == kd::ecs::Family::place) {
            out.push_back(id);
        }
    });
    return out;
}

Bytes log_of(std::size_t records) {
    Bytes out;
    for (std::size_t i = 0; i < records; ++i) {
        const Bytes r = kd::save::frame(static_cast<std::uint32_t>(1 + i % 2), 7 + i, stuff(i, 10 + 13 * i));
        out.insert(out.end(), r.begin(), r.end());
    }
    return out;
}

}  // namespace

// checks: PLT-07
TEST_CASE("a file written whole is the old one or the new one after a power cut at any moment") {
    kd::save::FakeFiles before;
    const Bytes old = bytes_of("the old snapshot");
    const Bytes fresh = bytes_of("the new snapshot, longer");
    REQUIRE(before.write_whole("worlds/one/snapshot", old));
    const std::uint64_t start = before.calls();
    {
        kd::save::FakeFiles all = before;
        REQUIRE(all.write_whole("worlds/one/snapshot", fresh));
    }
    std::uint64_t olds = 0;
    std::uint64_t news = 0;
    for (std::uint64_t n = 0; n < 8; ++n) {
        kd::save::FakeFiles f = before;
        f.stop_after(n);
        f.write_whole("worlds/one/snapshot", fresh);
        f.power_cut();
        f.restart();
        const std::optional<Bytes> read = f.read("worlds/one/snapshot");
        REQUIRE(read.has_value());
        const Bytes got = read.value_or(Bytes{});
        CHECK((got == old || got == fresh));
        olds += got == old ? 1 : 0;
        news += got == fresh ? 1 : 0;
    }
    CHECK(olds > 0);
    CHECK(news > 0);
    CHECK(start > 0);
    // with no power cut the new one is there as soon as the write returns, and nothing beside it
    kd::save::FakeFiles f = before;
    REQUIRE(f.write_whole("worlds/one/snapshot", fresh));
    f.power_cut();
    CHECK(f.read("worlds/one/snapshot") == fresh);
    CHECK(f.list("worlds/one") == std::vector<std::string>{"snapshot"});
}

// checks: PLT-07
TEST_CASE("a log keeps what was synced through a power cut and drops what was not") {
    kd::save::FakeFiles f;
    const Bytes one = kd::save::frame(1, 1, bytes_of("first"));
    const Bytes two = kd::save::frame(1, 2, bytes_of("second"));
    REQUIRE(f.append("journal", one));
    REQUIRE(f.sync("journal"));
    REQUIRE(f.append("journal", two));
    f.power_cut();
    const kd::save::LogRead read = kd::save::read_log(f.read("journal").value_or(Bytes{}), 1);
    REQUIRE(read.entries.size() == 1);
    CHECK(read.entries[0].body == bytes_of("first"));
}

// checks: PLT-07
TEST_CASE("the disk's files are written whole read back listed and moved aside") {
    char folder[] = "/tmp/kd-save-test-XXXXXX";
    REQUIRE(::mkdtemp(folder) != nullptr);
    kd::save::DiskFiles d(folder);
    REQUIRE(d.write_whole("a/b/snapshot", bytes_of("whole")));
    REQUIRE(d.append("a/journal", bytes_of("one ")));
    REQUIRE(d.append("a/journal", bytes_of("two")));
    REQUIRE(d.sync("a/journal"));
    CHECK(d.read("a/b/snapshot") == bytes_of("whole"));
    CHECK(d.read("a/journal") == bytes_of("one two"));
    CHECK(d.list("a") == std::vector<std::string>{"journal"});
    REQUIRE(d.cut("a/journal", 3));
    CHECK(d.read("a/journal") == bytes_of("one"));
    REQUIRE(d.set_aside("a/b/snapshot"));
    CHECK(d.list("a/b") == std::vector<std::string>{"snapshot.damaged"});
    CHECK_FALSE(d.read("a/b/snapshot").has_value());
    REQUIRE(d.remove("a/b/snapshot.damaged"));
    REQUIRE(d.remove("a/journal"));
    CHECK(d.list("a/b").empty());
    ::rmdir((std::string(folder) + "/a/b").c_str());
    ::rmdir((std::string(folder) + "/a").c_str());
    ::rmdir(folder);
}

// checks: PLT-07
TEST_CASE("a log is read up to its first short damaged or out-of-sequence record") {
    const Bytes whole = log_of(6);
    const kd::save::LogRead all = kd::save::read_log(whole, 7);
    REQUIRE(all.entries.size() == 6);
    CHECK(all.good == whole.size());
    CHECK_FALSE(all.cut);
    CHECK(all.entries[5].sequence == 12);
    CHECK(all.entries[5].body == stuff(5, 75));
    // cut short anywhere: exactly the whole records before the cut, and the rest cut
    std::vector<std::uint64_t> ends;
    std::uint64_t end = 0;
    for (const kd::save::Entry& e : all.entries) {
        end += kd::save::kFrame + e.body.size();
        ends.push_back(end);
    }
    std::uint64_t wrong = 0;
    for (std::size_t n = 0; n < whole.size(); ++n) {
        const kd::save::LogRead r = kd::save::read_log(std::span(whole).first(n), 7);
        std::size_t whole_ones = 0;
        while (whole_ones < ends.size() && ends[whole_ones] <= n) {
            ++whole_ones;
        }
        wrong += r.entries.size() == whole_ones ? 0 : 1;
        for (std::size_t i = 0; i < r.entries.size(); ++i) {
            wrong += r.entries[i].body == all.entries[i].body ? 0 : 1;
        }
        wrong += r.good == (whole_ones == 0 ? 0 : ends[whole_ones - 1]) && r.cut == (r.good != n) ? 0 : 1;
    }
    CHECK(wrong == 0);
    // a bit flipped anywhere: the records before it read, it and those after cut
    for (std::size_t bit = 0; bit < whole.size() * 8; ++bit) {
        Bytes damaged = whole;
        damaged[bit / 8] ^= static_cast<std::byte>(1U << (bit % 8));
        const kd::save::LogRead r = kd::save::read_log(damaged, 7);
        wrong += r.cut && r.good <= bit / 8 ? 0 : 1;
        for (std::size_t i = 0; i < r.entries.size(); ++i) {
            wrong += r.entries[i].body == all.entries[i].body ? 0 : 1;
        }
    }
    CHECK(wrong == 0);
    // zeros after it, as a file system may leave after a crash, and a record out of sequence
    Bytes zeros = whole;
    zeros.resize(whole.size() + 4096);
    const kd::save::LogRead z = kd::save::read_log(zeros, 7);
    CHECK(z.entries.size() == 6);
    CHECK(z.good == whole.size());
    CHECK(z.cut);
    Bytes skipped = whole;
    const Bytes later = kd::save::frame(1, 14, bytes_of("one was skipped"));
    skipped.insert(skipped.end(), later.begin(), later.end());
    CHECK(kd::save::read_log(skipped, 7).entries.size() == 6);
    CHECK(kd::save::read_log(whole, 8).entries.empty());
}

// checks: PLT-07 TIM-05
TEST_CASE("a snapshot reads back exactly and any damage refuses it whole") {
    const std::vector<kd::save::Chunk> in = chunks();
    const Bytes file = kd::save::write_snapshot(in);
    std::string why;
    const auto out = kd::save::read_snapshot(file, why);
    REQUIRE(out.has_value());
    CHECK(same(out.value_or(std::vector<kd::save::Chunk>{}), in));
    // the large chunk is compressed
    CHECK(file.size() < 20'000);
    std::uint64_t accepted = 0;
    for (std::size_t n = 0; n < file.size(); n += 1 + n / 50) {
        accepted += kd::save::read_snapshot(std::span(file).first(n), why).has_value() ? 1 : 0;
    }
    for (std::size_t bit = 0; bit < file.size() * 8; bit += 7) {
        Bytes damaged = file;
        damaged[bit / 8] ^= static_cast<std::byte>(1U << (bit % 8));
        const auto got = kd::save::read_snapshot(damaged, why);
        accepted += got.has_value() ? 1 : 0;
    }
    Bytes zeros = file;
    std::fill(zeros.begin() + static_cast<std::ptrdiff_t>(zeros.size() / 2), zeros.end(), std::byte{0});
    accepted += kd::save::read_snapshot(zeros, why).has_value() ? 1 : 0;
    Bytes longer = file;
    longer.push_back(std::byte{0});
    accepted += kd::save::read_snapshot(longer, why).has_value() ? 1 : 0;
    CHECK(accepted == 0);
}

// checks: TIM-05 PLT-07 RES-05
TEST_CASE("a world saved at any second and reopened carries on exactly as one never closed") {
    const kd::time::Seconds end = 2 * kd::time::kDay;
    kd::demo::CrowdWorld unbroken(9, fixture(), 16);
    unbroken.world().run_to(end);
    const kd::world::Digests want = unbroken.world().digests();
    const kd::chance::Draws draws(3, kd::chance::name("test"), 0, 0, kd::chance::name("saves"));
    kd::run::Workers workers(3);
    for (std::uint64_t i = 0; i < 6; ++i) {
        const kd::time::Seconds at = draws.between(i, 1, end - 1);
        // the even ones in islands of a quarter hour, so most saves fall inside a window's grid
        const bool islands = i % 2 == 0;
        CAPTURE(at);
        CAPTURE(islands);
        kd::demo::CrowdWorld crowd(9, fixture(), 16);
        if (islands) {
            crowd.world().run_islands(at, workers, 900);
        } else {
            crowd.world().run_to(at);
        }
        const kd::world::Digests before = crowd.world().digests();
        const std::unique_ptr<kd::demo::CrowdWorld> again = reopened(crowd);
        CHECK(again->world().digests().whole == before.whole);
        CHECK(again->world().frontier() == at);
        if (islands) {
            again->world().run_islands(end, workers, 900);
        } else {
            again->world().run_to(end);
        }
        const kd::world::Digests got = again->world().digests();
        CHECK(got.beings == want.beings);
        CHECK(got.queue == want.queue);
        CHECK(got.history == want.history);
        CHECK(got.whole == want.whole);
    }
}

// checks: TIM-05 PLT-07
TEST_CASE("a camp called home sets off home and a command pending at a save acts after it") {
    kd::demo::CrowdWorld crowd(4, fixture(), 6);
    kd::world::World& w = crowd.world();
    w.run_to(9 * kd::time::kHour);
    const kd::ecs::Id camp = camps_of(w)[2];
    const auto call = static_cast<std::uint32_t>(kd::demo::Commanded::call_home);
    const kd::world::Command c = w.command(10 * kd::time::kHour, call, camp.value, 0);
    CHECK(c.number == 1);
    CHECK(w.commands_made() == 1);
    // saved and reopened with the command still pending, it acts all the same
    const std::unique_ptr<kd::demo::CrowdWorld> again = reopened(crowd);
    w.run_to(10 * kd::time::kHour + 2);
    again->world().run_to(10 * kd::time::kHour + 2);
    CHECK(again->world().digests().whole == w.digests().whole);
    std::size_t home = 0;
    std::size_t away = 0;
    w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
        if (id.family() != kd::ecs::Family::marker || w.beings().raw().get<kd::demo::Home>(h).camp != camp) {
            return;
        }
        const auto& a = w.beings().raw().get<kd::world::Activity>(h);
        const kd::num::Point at = w.beings().raw().get<kd::demo::Home>(h).at;
        // each is walking home, resting at home, or greeting someone, which lets the call pass
        const bool going = a.to == at && a.what != static_cast<std::uint8_t>(kd::demo::Doing::sleep);
        const bool greeting = a.what == static_cast<std::uint8_t>(kd::demo::Doing::greet);
        home += going ? 1 : 0;
        away += going || greeting ? 0 : 1;
    });
    CHECK(home > 20);
    CHECK(away == 0);
    // a later command after the reopening, replayed from a journal, gives the same world
    const kd::world::Command later = w.command(w.frontier(), call, camps_of(w)[4].value, 0);
    again->world().replay(later);
    w.run_to(kd::time::kDay);
    again->world().run_to(kd::time::kDay);
    CHECK(again->world().digests().whole == w.digests().whole);
}

namespace {

// A crowd's folder in fake files: made, run with two snapshots and history written past the newest, which is returned
// with the digest the world had at the end.
struct Folder {
    kd::save::FakeFiles files;
    std::string newest;
    kd::time::Seconds end = 0;
    std::uint64_t digest = 0;
};

Folder folder() {
    Folder out;
    kd::save::Keeper keeper(out.files);
    kd::demo::Kept kept = kd::demo::keep_crowd(keeper, fixture(), 5, 6);
    REQUIRE(kept.made);
    kd::world::World& w = kept.crowd->world();
    std::vector<kd::world::Record> records;
    w.keep_history(&records);
    const auto camps = camps_of(w);
    for (const kd::time::Seconds stop : {kd::time::Seconds{30'000}, kd::time::Seconds{45'000}}) {
        w.run_to(stop);
        keeper.history(records);
        records.clear();
        keeper.snapshot(w);
    }
    keeper.command(
        w.command(w.frontier(), static_cast<std::uint32_t>(kd::demo::Commanded::call_home), camps[1].value, 0));
    out.end = 60'000;
    w.run_to(out.end);
    keeper.history(records);
    keeper.flush();
    out.digest = w.digests().whole;
    out.newest = "snapshots/" + out.files.list("snapshots").back();
    return out;
}

// The folder opened and run on to its end: the world's digest there, and the history's mismatches.
std::pair<std::uint64_t, std::uint64_t> reopen(kd::save::FakeFiles& files, kd::time::Seconds end, kd::demo::Kept* out) {
    kd::save::Keeper keeper(files);
    *out = kd::demo::keep_crowd(keeper, fixture(), 5, 6);
    REQUIRE(out->crowd != nullptr);
    kd::world::World& w = out->crowd->world();
    std::vector<kd::world::Record> records;
    w.keep_history(&records);
    w.run_to(end);
    keeper.history(records);
    keeper.flush();
    return {w.digests().whole, keeper.mismatches()};
}

}  // namespace

// checks: PLT-07 TIM-05
TEST_CASE("a damaged snapshot is set aside and the one before it opens and the world catches up exactly") {
    Folder made = folder();
    const std::string older = made.files.list("snapshots").front();
    for (int damage = 0; damage < 3; ++damage) {
        CAPTURE(damage);
        kd::save::FakeFiles files = made.files;
        Bytes& raw = files.raw(made.newest);
        if (damage == 0) {
            raw.resize(raw.size() / 2);  // cut short
        } else if (damage == 1) {
            raw[raw.size() / 3] ^= std::byte{0x10};  // a flipped bit
        } else {
            std::fill(raw.begin() + static_cast<std::ptrdiff_t>(raw.size() / 4), raw.end(), std::byte{0});  // zeros
        }
        kd::demo::Kept kept;
        const auto [digest, mismatches] = reopen(files, made.end, &kept);
        CHECK(kept.snapshot == older);
        REQUIRE(kept.damaged.size() == 1);
        CHECK(kept.replayed == 1);
        CHECK(digest == made.digest);
        CHECK(mismatches == 0);
        const std::vector<std::string> left = files.list("snapshots");
        CHECK(std::find(left.begin(), left.end(), made.newest.substr(10) + ".damaged") != left.end());
    }
}

// checks: PLT-07 TIM-05
TEST_CASE("after a power cut the folder opens and the world made again matches the history that was written") {
    Folder made = folder();
    // the power goes: the history after the newest snapshot was never synced, the command was
    kd::save::FakeFiles cut = made.files;
    cut.power_cut();
    kd::demo::Kept kept;
    const auto [digest, mismatches] = reopen(cut, made.end, &kept);
    CHECK(kept.snapshot == made.newest.substr(10));
    CHECK(kept.replayed == 1);
    CHECK(digest == made.digest);
    CHECK(mismatches == 0);
    // and the history holds every record of the world, none lost behind a snapshot
    {
        kd::save::Keeper keeper(cut);
        CHECK(keeper.open().history.size() == kept.crowd->world().history_count());
    }
    // with no power cut, the history written after the snapshot is made again and compared, and a journal with a
    // damaged end is cut there
    kd::save::FakeFiles kill = made.files;
    Bytes& journal = kill.raw("journal.log");
    journal.resize(journal.size() + 40, std::byte{0});
    kd::demo::Kept again;
    const auto [digest2, mismatches2] = reopen(kill, made.end, &again);
    CHECK(again.was_at > 45'000);
    CHECK(again.damaged.size() == 1);
    CHECK(digest2 == made.digest);
    CHECK(mismatches2 == 0);
}
