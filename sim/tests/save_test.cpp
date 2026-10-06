#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/chance/chance.hpp"
#include "kd/core/bytes.hpp"
#include "kd/data/folder.hpp"
#include "kd/data/toml.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/proof/fixture.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/archive.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
#include "kd/save/log.hpp"
#include "kd/save/snapshot.hpp"
#include "kd/save/versions.hpp"
#include "kd/world/upgrades.hpp"

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

using kd::test::fixture;

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

namespace {

// A world's folder written out as a .kindling file in pieces of sizes drawn by keyed chance.
Bytes exported(kd::save::Files& folder, std::uint64_t key) {
    kd::save::ArchiveWriter w(folder);
    const kd::chance::Draws sizes(key, kd::chance::name("test"), 0, 0, kd::chance::name("pieces"));
    Bytes out;
    for (std::uint64_t i = 0;; ++i) {
        const Bytes piece = w.next(static_cast<std::size_t>(sizes.between(i, 1, 70'000)));
        if (piece.empty()) {
            break;
        }
        out.insert(out.end(), piece.begin(), piece.end());
    }
    return out;
}

// A .kindling file read into a folder in pieces of sizes drawn by keyed chance: whether it was taken, and why not.
std::pair<bool, std::string> imported(const Bytes& file, kd::save::Files& folder, std::uint64_t key) {
    kd::save::ArchiveReader r(folder);
    const kd::chance::Draws sizes(key, kd::chance::name("test"), 0, 0, kd::chance::name("pieces"));
    std::size_t at = 0;
    for (std::uint64_t i = 0; at < file.size(); ++i) {
        const auto n = std::min<std::size_t>(file.size() - at, static_cast<std::size_t>(sizes.between(i, 1, 5'000)));
        if (!r.feed(std::span(file).subspan(at, n))) {
            return {false, r.why()};
        }
        at += n;
    }
    return {r.finish(), r.why()};
}

}  // namespace

// checks: PLT-08 TIM-05
TEST_CASE("a world exported to one file and imported runs on as the one it came from") {
    Folder made = folder();
    const Bytes file = exported(made.files, 1);
    // the same file however it is cut into pieces
    CHECK(exported(made.files, 2) == file);
    kd::save::FakeFiles copy;
    const auto [taken, why] = imported(file, copy, 3);
    INFO(why);
    REQUIRE(taken);
    CHECK(copy.list("").size() == 2);  // world.toml and journal.log
    CHECK(copy.list("snapshots").size() == 1);
    CHECK(copy.read("journal.log") == made.files.read("journal.log"));
    kd::demo::Kept from;
    kd::demo::Kept to;
    const auto [one, one_mismatches] = reopen(made.files, 2 * made.end, &from);
    const auto [two, two_mismatches] = reopen(copy, 2 * made.end, &to);
    CHECK(one == two);
    CHECK(one_mismatches == 0);
    CHECK(two_mismatches == 0);
    CHECK(to.replayed == from.replayed);
}

// checks: PLT-08
TEST_CASE("a damaged world's file is refused with words naming the damage") {
    Folder made = folder();
    const Bytes file = exported(made.files, 1);
    std::uint64_t taken = 0;
    std::uint64_t unexplained = 0;
    const auto refused = [&](const Bytes& damaged, std::uint64_t key) {
        kd::save::FakeFiles into;
        const auto [ok, why] = imported(damaged, into, key);
        taken += ok ? 1 : 0;
        unexplained += !ok && why.empty() ? 1 : 0;
    };
    for (std::size_t n = 0; n < file.size(); n += 1 + n / 7) {
        refused(Bytes(file.begin(), file.begin() + static_cast<std::ptrdiff_t>(n)), n);
    }
    for (std::size_t bit = 0; bit < file.size() * 8; bit += 1 + bit / 3) {
        Bytes damaged = file;
        damaged[bit / 8] ^= static_cast<std::byte>(1U << (bit % 8));
        refused(damaged, bit);
    }
    Bytes zeros = file;
    std::fill(zeros.begin() + static_cast<std::ptrdiff_t>(zeros.size() / 2), zeros.end(), std::byte{0});
    refused(zeros, 1);
    Bytes longer = file;
    longer.push_back(std::byte{7});
    refused(longer, 2);
    CHECK(taken == 0);
    CHECK(unexplained == 0);
    // a byte changed inside the snapshot is caught as its part ends, and the part is named
    const std::string snapshot = made.newest;
    const auto at = std::search(file.begin(), file.end(), snapshot.begin(), snapshot.end(),
                                [](std::byte b, char c) { return b == static_cast<std::byte>(c); });
    REQUIRE(at != file.end());
    Bytes inside = file;
    inside[static_cast<std::size_t>(at - file.begin()) + snapshot.size() + 16 + 100] ^= std::byte{1};
    {
        kd::save::FakeFiles into;
        const auto [ok, why] = imported(inside, into, 9);
        CHECK_FALSE(ok);
        CHECK(why == "its part " + snapshot + " is damaged");
    }
    // a part that is no part of a world, such as a path out of its folder, is refused before it is written
    kd::save::FakeFiles into;
    kd::ByteWriter w;
    w.u64(0x444c574c444e494b);
    w.u32(1);
    w.u32(1);
    w.text("../outside.toml");
    w.u64(1);
    w.u64(0);
    w.u8(1);
    kd::save::ArchiveReader r(into);
    CHECK_FALSE(r.feed(w.bytes()));
    CHECK(r.why().find("not a part of a world") != std::string::npos);
    CHECK(into.list("").empty());
}

namespace {

// A year of the history as the folder holds it.
kd::save::Year year_held(kd::save::Files& files, std::int64_t year) {
    return kd::save::read_year(files.read(kd::save::year_file(year)).value_or(Bytes{}));
}

// A world of one camp kept for 30 years and a day, the camp called home in the middle of each year: its folder, every
// record it made, and its digest at the end.
struct ThirtyYears {
    kd::save::FakeFiles files;
    std::vector<kd::world::Record> made;
    std::uint64_t digest = 0;
    kd::time::Seconds end = 30 * kd::save::Keeper::kYear + kd::time::kDay;
};

const ThirtyYears& thirty_years() {
    static const ThirtyYears out = [] {
        using kd::save::Keeper;
        ThirtyYears t;
        {
            Keeper keeper(t.files);
            kd::demo::Kept kept = kd::demo::keep_crowd(keeper, fixture(), 3, 1);
            REQUIRE(kept.made);
            kd::world::World& w = kept.crowd->world();
            std::vector<kd::world::Record> records;
            w.keep_history(&records);
            const kd::ecs::Id camp = camps_of(w)[0];
            const auto pass_on = [&] {
                keeper.history(records);
                t.made.insert(t.made.end(), records.begin(), records.end());
                records.clear();
            };
            // the camp called home in the middle of each year, which the history keeps for ever; a snapshot each year
            for (kd::time::Seconds at = Keeper::kYear / 2; at < t.end; at += Keeper::kYear) {
                w.run_to(at);
                pass_on();
                keeper.command(
                    w.command(at, static_cast<std::uint32_t>(kd::demo::Commanded::call_home), camp.value, 0));
                keeper.snapshot(w);
            }
            w.run_to(t.end);
            pass_on();
            keeper.flush();
            t.digest = w.digests().whole;
        }
        return t;
    }();
    return out;
}

bool is_call(const kd::world::Record& r) {
    return r.key.owner == kd::ecs::owners::commands.value && r.what == kd::world::Commands::kActed;
}

}  // namespace

// checks: PLT-10 PRN-15
TEST_CASE("a 30-year world's history keeps every event of its last 25 years and only kept kinds before") {
    const ThirtyYears& thirty = thirty_years();
    kd::save::FakeFiles files = thirty.files;
    const std::vector<kd::world::Record>& made = thirty.made;
    const kd::time::Seconds end = thirty.end;
    const std::uint64_t digest = thirty.digest;
    const auto call = is_call;
    // years 1 to 5 ended more than 25 years before the 31st began, and keep only the calls; the rest keep everything
    std::uint64_t wrongly_flagged = 0;
    for (std::int64_t year = 1; year <= 31; ++year) {
        CAPTURE(year);
        std::vector<kd::world::Record> want;
        for (const kd::world::Record& r : made) {
            if (kd::save::year_of(r.key.second) == year && (year > 5 || call(r))) {
                want.push_back(r);
            }
        }
        const kd::save::Year held = year_held(files, year);
        CHECK(held.thinned == (year <= 5));
        CHECK_FALSE(held.cut);
        std::vector<kd::world::Record> got;
        for (const kd::save::Year::Held& h : held.records) {
            got.push_back(h.record);
            wrongly_flagged += h.kept == call(h.record) ? 0 : 1;
        }
        CHECK(got == want);
        // a call in each of the first 30 years, and a day of the 31st
        const bool thinned = year <= 5;
        CHECK(want.size() >= (thinned ? 1 : year <= 30 ? 1000 : 10));
        CHECK((!thinned || want.size() == 1));
    }
    CHECK(wrongly_flagged == 0);
    CHECK(files.list("history").size() == 31);

    // with both snapshots lost, the world is made again from its seed and journal: the thinned years are left as they
    // are, and every record of the others is made again as it was written
    kd::save::FakeFiles lost = files;
    for (const std::string& name : lost.list("snapshots")) {
        lost.remove("snapshots/" + name);
    }
    kd::demo::Kept kept;
    const auto [again, mismatches] = reopen(lost, end, &kept);
    CHECK(kept.made);
    CHECK(kept.replayed == 30);
    CHECK(again == digest);
    CHECK(mismatches == 0);
    for (std::int64_t year = 1; year <= 31; ++year) {
        CAPTURE(year);
        CHECK(lost.read(kd::save::year_file(year)) == files.read(kd::save::year_file(year)));
    }
}

namespace {

std::string read_text(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream text;
    text << in.rdbuf();
    return text.str();
}

// The game's own catalogue, from the data folder, as the corpus's worlds were saved with it.
const kd::data::Catalogue& game_catalogue() {
    static const kd::data::Catalogue catalogue = [] {
        kd::data::Catalogue c;
        REQUIRE(c.load(kd::data::read_folder(KD_REPO "/data")).empty());
        return c;
    }();
    return catalogue;
}

// A world of the corpus, imported into a folder.
kd::save::FakeFiles corpus_world(const std::string& file) {
    kd::save::FakeFiles folder;
    const std::string text = read_text(KD_REPO "/sim/tests/corpus/" + file);
    REQUIRE(!text.empty());
    const auto [taken, why] = imported(bytes_of(text), folder, 1);
    INFO(why);
    REQUIRE(taken);
    return folder;
}

// A snapshot's chunks, none if it cannot be read.
std::vector<kd::save::Chunk> chunks_of(kd::save::Files& folder, const std::string& path) {
    std::string why;
    return kd::save::read_snapshot(folder.read(path).value_or(Bytes{}), why).value_or(std::vector<kd::save::Chunk>{});
}

// What the newest snapshot of a folder was saved under, or nothing.
std::optional<kd::save::Versions> saved_under(kd::save::Files& folder) {
    const std::vector<std::string> names = folder.list("snapshots");
    REQUIRE(!names.empty());
    const std::vector<kd::save::Chunk> chunks = chunks_of(folder, "snapshots/" + names.back());
    const kd::save::Chunk* c = kd::save::find_chunk(chunks, kd::save::kVersionsTag);
    return c != nullptr ? kd::save::read_versions(*c) : std::nullopt;
}

// Whether every year of a folder's history reads whole, and how many records it holds.
std::pair<bool, std::size_t> history_whole(kd::save::Files& folder) {
    bool whole = true;
    std::size_t records = 0;
    for (const std::string& name : folder.list("history")) {
        const kd::save::Year y = kd::save::read_year(folder.read("history/" + name).value_or(Bytes{}));
        whole = whole && !y.cut;
        records += y.records.size();
    }
    return {whole, records};
}

}  // namespace

// checks: PLT-09 PLT-08
TEST_CASE("each alpha's world in the corpus opens and carries on, or after a big update its history is read") {
    const kd::data::Parsed manifest =
        kd::data::parse_toml(read_text(KD_REPO "/sim/tests/corpus/corpus.toml"), "corpus");
    REQUIRE(manifest.problems.empty());
    const kd::data::Value* worlds = manifest.root.find("world");
    REQUIRE(worlds != nullptr);
    CHECK(worlds->items.size() >= 2);
    for (const kd::data::Value& entry : worlds->items) {
        const std::string file = entry.find("file")->text;
        const std::string update = entry.find("update")->text;
        CAPTURE(file);
        kd::save::FakeFiles folder = corpus_world(file);
        const auto [whole_before, records_before] = history_whole(folder);
        CHECK(whole_before);
        CHECK(records_before > 1000);
        kd::save::Keeper keeper(folder, "test");
        kd::demo::Kept kept = kd::demo::keep_crowd(keeper, game_catalogue(), 1, 0);
        if (update == "big") {
            CHECK(kept.update == kd::save::Update::big);
            CHECK(kept.crowd == nullptr);
            CHECK(history_whole(folder) == std::pair{true, records_before});
            continue;
        }
        REQUIRE(update == "small");
        INFO(kept.problem);
        REQUIRE(kept.crowd != nullptr);
        CHECK(kept.update == kd::save::Update::small);
        CHECK(kept.replayed == 1);
        kd::world::World& w = kept.crowd->world();
        std::vector<kd::world::Record> records;
        w.keep_history(&records);
        // it catches up to where it was, made again under this version's rules, and carries on for a day
        w.run_to(kept.was_at + kd::time::kDay);
        keeper.history(records);
        keeper.snapshot(w);
        keeper.flush();
        CHECK(keeper.mismatches() == 0);
        const auto [whole, after] = history_whole(folder);
        CHECK(whole);
        CHECK(after > records_before);
        CHECK(after == w.history_count());
    }
}

// checks: PLT-09
TEST_CASE("after a small update the previous version's last snapshot is kept until the world has run an hour") {
    kd::save::FakeFiles folder = corpus_world("a14a.kindling");
    const std::vector<std::string> kept_aside{"00000000000005184000.kds", "world.toml"};
    {
        kd::save::Keeper keeper(folder, "test");
        kd::demo::Kept kept = kd::demo::keep_crowd(keeper, game_catalogue(), 1, 0);
        REQUIRE(kept.crowd != nullptr);
        CHECK(kept.update == kd::save::Update::small);
        CHECK(folder.list("previous") == kept_aside);
        CHECK(folder.read("previous/" + kept_aside[0]) == folder.read("snapshots/" + kept_aside[0]));
        keeper.played(kd::save::Keeper::kPreviousKept - 1);
        keeper.snapshot(kept.crowd->world());
        keeper.flush();
        CHECK(folder.list("previous") == kept_aside);
    }
    // opened again by the same version: no update now, and its hour goes on where it was
    kd::save::Keeper keeper(folder, "test");
    kd::demo::Kept kept = kd::demo::keep_crowd(keeper, game_catalogue(), 1, 0);
    REQUIRE(kept.crowd != nullptr);
    CHECK(kept.update == kd::save::Update::none);
    keeper.played(1);
    keeper.snapshot(kept.crowd->world());
    keeper.flush();
    CHECK(folder.list("previous").empty());
    // and the save says which versions the world has run under, and from when
    const std::optional<kd::save::Versions> saved = saved_under(folder);
    REQUIRE(saved.has_value());
    const kd::save::Versions v = saved.value_or(kd::save::Versions{});
    CHECK(v.build == "test");
    REQUIRE(v.eras.size() == 1);
    CHECK(v.eras[0].build == "test");
    CHECK(v.eras[0].from == 5'184'000);
    CHECK(v.played == kd::save::Keeper::kPreviousKept);
    CHECK(v.making == kd::save::making_digest(game_catalogue()));
}

// checks: PLT-09
TEST_CASE("after a big update a world is not run, nothing is written, and its history is still read") {
    kd::save::FakeFiles folder = corpus_world("a14b.kindling");
    // its snapshot as a version that made worlds by other rules would have saved it
    const std::string newest = "snapshots/" + folder.list("snapshots").back();
    std::vector<kd::save::Chunk> chunks = chunks_of(folder, newest);
    REQUIRE(kd::save::find_chunk(chunks, kd::save::kVersionsTag) != nullptr);
    for (kd::save::Chunk& c : chunks) {
        if (c.tag == kd::save::kVersionsTag) {
            kd::save::Versions v = kd::save::read_versions(c).value_or(kd::save::Versions{});
            CHECK(v.build == "α1.4b");
            v.making ^= 1U;
            c = kd::save::versions_chunk(v);
        }
    }
    folder.raw(newest) = kd::save::write_snapshot(chunks);
    const kd::save::FakeFiles before = folder;
    kd::save::Keeper keeper(folder, "test");
    const kd::demo::Kept kept = kd::demo::keep_crowd(keeper, game_catalogue(), 1, 0);
    keeper.flush();
    CHECK(kept.update == kd::save::Update::big);
    CHECK(kept.crowd == nullptr);
    CHECK(!kept.problem.empty());
    for (const std::string part : {"", "snapshots", "history", "previous"}) {
        kd::save::FakeFiles was = before;
        CHECK(folder.list(part) == was.list(part));
        for (const std::string& name : folder.list(part)) {
            std::string path = part;
            path += part.empty() ? "" : "/";
            path += name;
            CHECK(folder.read(path) == was.read(path));
        }
    }
    const auto [whole, records] = history_whole(folder);
    CHECK(whole);
    CHECK(records > 1000);
}

namespace {

int moved = 0;

// A migration for the test: every camp's marker of the first kind moved a centimetre east.
void move_east(kd::world::World& w) {
    ++moved;
    w.beings().each([&](kd::ecs::Id id, kd::world::Beings::Handle h) {
        if (id.family() == kd::ecs::Family::marker) {
            w.beings().raw().get<kd::world::Activity>(h).from.x += 1;
        }
    });
}

}  // namespace

// checks: PLT-09
TEST_CASE("a migration is made once to a world saved before it, and the save records it") {
    const std::array<kd::world::Migration, 1> list{{{"test-move-east", move_east}}};
    kd::save::FakeFiles folder = corpus_world("a14b.kindling");
    std::uint64_t digest = 0;
    {
        kd::save::Keeper keeper(folder, "test");
        kd::demo::Kept kept = kd::demo::keep_crowd(keeper, game_catalogue(), 1, 0, list);
        REQUIRE(kept.crowd != nullptr);
        CHECK(kept.migrated == std::vector<std::string>{"test-move-east"});
        CHECK(moved == 1);
        keeper.snapshot(kept.crowd->world());
        keeper.flush();
        digest = kept.crowd->world().digests().whole;
    }
    CHECK(saved_under(folder).value_or(kd::save::Versions{}).migrations == std::vector<std::string>{"test-move-east"});
    {
        kd::save::Keeper keeper(folder, "test");
        kd::demo::Kept kept = kd::demo::keep_crowd(keeper, game_catalogue(), 1, 0, list);
        REQUIRE(kept.crowd != nullptr);
        CHECK(kept.migrated.empty());
        CHECK(moved == 1);
        CHECK(kept.crowd->world().digests().whole == digest);
    }
    // a world made new is made as this version makes worlds, so it needs none
    kd::save::FakeFiles fresh;
    kd::save::Keeper keeper(fresh, "test");
    const kd::demo::Kept kept = kd::demo::keep_crowd(keeper, game_catalogue(), 1, 1, list);
    CHECK(kept.made);
    CHECK(moved == 1);
    CHECK(keeper.versions().migrations == std::vector<std::string>{"test-move-east"});
}

// checks: PLT-09
TEST_CASE("a part of a snapshot saved by an older version is brought up to date a step at a time") {
    // version 1 held a count in 4 bytes, version 2 in 8, and version 3 added a flag after it
    const std::array<kd::save::Upgrade, 2> steps{{
        {kd::save::tag("TEST"), 1,
         [](Bytes& d) {
             d.resize(8);
             return true;
         }},
        {kd::save::tag("TEST"), 2,
         [](Bytes& d) {
             d.push_back(std::byte{1});
             return d.size() == 9;
         }},
    }};
    kd::ByteWriter w;
    w.u32(41);
    kd::save::Chunk c{kd::save::tag("TEST"), 1, true, w.take()};
    std::string why;
    CHECK(kd::save::upgrade(c, 3, steps, why));
    CHECK(c.version == 3);
    kd::ByteReader r(c.data);
    std::uint64_t count = 0;
    std::uint8_t flag = 0;
    CHECK((r.u64(count) && r.u8(flag) && r.finished()));
    CHECK(count == 41);
    CHECK(flag == 1);
    // newer than this version, or with no step on from its version: refused with words
    kd::save::Chunk newer{kd::save::tag("TEST"), 4, true, {}};
    CHECK_FALSE(kd::save::upgrade(newer, 3, steps, why));
    CHECK(why.find("newer") != std::string::npos);
    kd::save::Chunk other{kd::save::tag("ELSE"), 1, true, {}};
    CHECK_FALSE(kd::save::upgrade(other, 2, steps, why));
    CHECK(why.find("older") != std::string::npos);
    CHECK(kd::world::migrations().empty());
}

// checks: PLT-09 RES-10
TEST_CASE("a world's clock saved before test switches, at its first version, opens with none") {
    kd::demo::CrowdWorld crowd(3, fixture(), 2);
    crowd.world().run_to(kd::time::kDay);
    std::vector<kd::save::Chunk> chunks = crowd.world().save();
    // the clock as version 1 wrote it: the same, less the switches' count at its end
    for (kd::save::Chunk& c : chunks) {
        if (c.tag == kd::save::tag("WRLD")) {
            CHECK(c.version == 2);
            c.version = 1;
            c.data.resize(c.data.size() - 8);
        }
    }
    std::string why;
    const std::unique_ptr<kd::demo::CrowdWorld> again = kd::demo::CrowdWorld::open(fixture(), chunks, why);
    INFO(why);
    REQUIRE(again != nullptr);
    CHECK(again->world().switches().empty());
    CHECK(again->world().digests().whole == crowd.world().digests().whole);
}

namespace {

// A component for the test, whose version 1 held one number and version 2 two.
struct Pair {
    [[maybe_unused]] static constexpr std::string_view name = "pair";  // as every component has, though saves skip it
    static constexpr std::uint32_t version = 2;
    std::uint64_t a = 0;
    std::uint64_t b = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.u64({"a", "the first"}, c.a);
        v.u64({"b", "the second, 7 in a pair saved before it was added"}, c.b);
    }

    static bool upgrade(std::uint32_t version, kd::ByteReader& r, const kd::ecs::EntryMap& /*entries*/, Pair& c) {
        c.b = 7;
        return version == 1 && r.u64(c.a);
    }
};

}  // namespace

// checks: PLT-09
TEST_CASE("a component saved in an older shape is read into its new one by its own upgrade") {
    const kd::ecs::EntryMap entries = [](std::string_view, std::uint32_t n) { return std::optional<std::uint32_t>(n); };
    kd::ByteWriter w;
    w.u32(1);
    w.u64(5);
    Pair p;
    kd::ByteReader r(w.bytes());
    CHECK(kd::ecs::read_component(p, r, entries));
    CHECK(p.a == 5);
    CHECK(p.b == 7);
    // its own shape reads as it is, and a newer or unknown one is refused
    kd::ByteWriter now;
    kd::ecs::write_component(Pair{3, 4}, now);
    kd::ByteReader rn(now.bytes());
    Pair q;
    CHECK(kd::ecs::read_component(q, rn, entries));
    CHECK((q.a == 3 && q.b == 4));
    for (const std::uint32_t version : {0U, 3U}) {
        kd::ByteWriter odd;
        odd.u32(version);
        odd.u64(5);
        odd.u64(6);
        kd::ByteReader ro(odd.bytes());
        Pair x;
        CHECK_FALSE(kd::ecs::read_component(x, ro, entries));
    }
}

// checks: TIM-08
TEST_CASE("a world's name of any text is kept in its world.toml and read back as it was") {
    for (const std::string name :
         {"Crowd", "", "Lory's \"best\" \\ world", "two\nlines\tand a bell \x07", "ünïcødé 世界"}) {
        CAPTURE(name);
        kd::demo::About about;
        about.name = name;
        about.seed = 42;
        about.camps = 7;
        about.test = name.empty();
        about.switches =
            name.empty() ? std::vector<std::string>{"plant_wander", "no_greetings"} : std::vector<std::string>{};
        const std::optional<kd::demo::About> read = kd::demo::read_about(kd::demo::about_text(about));
        REQUIRE(read.has_value());
        const kd::demo::About back = read.value_or(kd::demo::About{});
        CHECK(back.name == name);
        CHECK(back.seed == 42);
        CHECK(back.camps == 7);
        CHECK(back.test == about.test);
        CHECK(back.switches == about.switches);
    }
    // α1.4a's, with no name, reads; another kind of world's, or a damaged one, does not
    CHECK(kd::demo::read_about("kind = \"crowd\"\nseed = 1\ncamps = 0\n").has_value());
    CHECK_FALSE(kd::demo::read_about("kind = \"valley\"\nseed = 1\ncamps = 0\n").has_value());
    CHECK_FALSE(kd::demo::read_about("seed = 1\ncamps = \n").has_value());
}

// checks: PLT-10 PLT-09
TEST_CASE("a world saved by α1.4a, which never thinned its history, is thinned as it opens after the update") {
    const ThirtyYears& thirty = thirty_years();
    kd::save::FakeFiles old = thirty.files;
    // as α1.4a kept it: every year whole and numbered on from the year before, with no calls, which it never recorded,
    // and no versions in its snapshots
    std::uint64_t sequence = 1;
    for (std::int64_t year = 1; year <= 31; ++year) {
        Bytes file;
        for (const kd::world::Record& r : thirty.made) {
            if (kd::save::year_of(r.key.second) == year && !is_call(r)) {
                const Bytes framed = kd::save::record_frame(r, sequence++, false);
                file.insert(file.end(), framed.begin(), framed.end());
            }
        }
        old.raw(kd::save::year_file(year)) = file;
    }
    for (const std::string& name : old.list("snapshots")) {
        std::vector<kd::save::Chunk> chunks = chunks_of(old, "snapshots/" + name);
        REQUIRE(!chunks.empty());
        std::erase_if(chunks, [](const kd::save::Chunk& c) { return c.tag == kd::save::kVersionsTag; });
        old.raw("snapshots/" + name) = kd::save::write_snapshot(chunks);
    }
    // opened by this version: a small update, and the years more than 25 years past thinned to what stays for ever,
    // which in α1.4a's history is nothing; the later years read on, as α1.4a numbered them
    kd::save::Keeper keeper(old, "test");
    const kd::demo::Kept kept = kd::demo::keep_crowd(keeper, fixture(), 3, 1);
    REQUIRE(kept.crowd != nullptr);
    CHECK(kept.update == kd::save::Update::small);
    // and as it catches up, what came after its snapshot is made again under this version
    kd::world::World& w = kept.crowd->world();
    std::vector<kd::world::Record> records;
    w.keep_history(&records);
    w.run_to(kept.was_at);
    keeper.history(records);
    keeper.flush();
    CHECK(kd::save::year_of(kept.was_at) == 31);
    CHECK(keeper.mismatches() == 0);
    for (std::int64_t year = 1; year <= 31; ++year) {
        CAPTURE(year);
        const kd::save::Year held = year_held(old, year);
        CHECK_FALSE(held.cut);
        CHECK(held.thinned == (year <= 5));
        CHECK(held.records.empty() == (year <= 5));
    }
}
