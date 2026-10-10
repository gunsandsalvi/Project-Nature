#include "kd/save/keeper.hpp"

#include <algorithm>
#include <cstdio>
#include <utility>

#include <set>
#include "kd/core/bytes.hpp"
#include "kd/num/whole.hpp"
#include "kd/save/log.hpp"
#include "kd/save/pages.hpp"
#include "kd/save/snapshot.hpp"

namespace kd::save {

namespace {

// The types of the journal's records, and of the history's: a record that goes once its year is thinned, the mark
// that begins a thinned year, and a record that stays for ever.
constexpr std::uint32_t kCommand = 1;
constexpr std::uint32_t kPause = 2;
constexpr std::uint32_t kHappened = 1;
constexpr std::uint32_t kThinned = 2;
constexpr std::uint32_t kKept = 3;

const std::string kJournal = "journal.log";
const std::string kAbout = "world.toml";

// A snapshot's file name: its frontier, padded so names sort as frontiers do.
std::string snapshot_file(time::Seconds frontier) {
    char name[64];
    std::snprintf(name, sizeof name, "snapshots/%020lld.kds", static_cast<long long>(frontier));
    return name;
}

bool ends_with(const std::string& s, const std::string& end) {
    return s.size() >= end.size() && s.compare(s.size() - end.size(), end.size(), end) == 0;
}

// The whole number a name begins with, up to its ending, such as a snapshot's frontier or a history file's year.
std::optional<std::int64_t> number_before(const std::string& name, const std::string& end) {
    if (!ends_with(name, end) || name.size() == end.size()) {
        return std::nullopt;
    }
    std::int64_t v = 0;
    for (std::size_t i = 0; i + end.size() < name.size(); ++i) {
        const char c = name[i];
        if (c < '0' || c > '9' || v > (INT64_MAX - 9) / 10) {
            return std::nullopt;
        }
        v = v * 10 + (c - '0');
    }
    return v;
}

Bytes command_body(const world::Command& c) {
    ByteWriter w;
    w.u64(c.number);
    w.i64(c.at);
    w.u32(c.what);
    w.u64(c.a);
    w.u64(c.b);
    return w.take();
}

bool read_record(ByteReader& r, world::Record& rec) {
    return r.i64(rec.key.second) && r.u64(rec.key.owner) && r.u64(rec.key.sequence) && r.u32(rec.n) &&
           r.u32(rec.what) && r.u64(rec.a) && r.u64(rec.b) && r.finished();
}

}  // namespace

Bytes record_frame(const world::Record& r, std::uint64_t sequence, bool kept) {
    ByteWriter w;
    w.i64(r.key.second);
    w.u64(r.key.owner);
    w.u64(r.key.sequence);
    w.u32(r.n);
    w.u32(r.what);
    w.u64(r.a);
    w.u64(r.b);
    return frame(kept ? kKept : kHappened, sequence, w.bytes());
}

std::int64_t year_of(time::Seconds second) {
    return num::floor_div(second, Keeper::kYear) + 1;
}

std::string year_file(std::int64_t year) {
    char name[64];
    std::snprintf(name, sizeof name, "history/%06lld.log", static_cast<long long>(year));
    return name;
}

std::optional<std::int64_t> file_year(const std::string& name) {
    return number_before(name, ".log");
}

std::string history_file(time::Seconds second) {
    return year_file(year_of(second));
}

Year read_year(std::span<const std::byte> bytes) {
    Year out;
    const LogRead log = read_log(bytes, 0);
    std::uint64_t offset = 0;
    for (const Entry& e : log.entries) {
        if (e.type == kThinned && e.sequence == 1 && e.body.empty()) {
            out.thinned = true;
        } else {
            Year::Held h;
            h.offset = offset;
            h.sequence = e.sequence;
            h.kept = e.type == kKept;
            ByteReader r(e.body);
            if ((e.type != kHappened && e.type != kKept) || !read_record(r, h.record)) {
                break;
            }
            out.records.push_back(h);
        }
        offset += kFrame + e.body.size();
    }
    out.good = offset;
    out.cut = log.cut || offset != log.good;
    return out;
}

Keeper::Keeper(Files& files, std::string build) : build_(std::move(build)), io_(files) {}

Keeper::~Keeper() {
    io_.flush();
}

Found Keeper::open() {
    Found found;
    io_.now([&](Files& f) {
        // the newest snapshot that is whole; any newer one is moved aside
        std::vector<std::string> names = f.list("snapshots");
        std::erase_if(names, [](const std::string& n) { return !ends_with(n, ".kds"); });
        std::reverse(names.begin(), names.end());
        for (const std::string& name : names) {
            const std::string path = "snapshots/" + name;
            const std::optional<Bytes> bytes = f.read(path);
            std::string why = "it cannot be read";
            if (bytes) {
                std::optional<std::vector<Chunk>> chunks = read_snapshot(*bytes, why);
                if (chunks && resolve_pages(f, *chunks, why)) {
                    if (const Chunk* v = find_chunk(*chunks, kVersionsTag)) {
                        found.versions = read_versions(*v);
                    }
                    found.snapshot = std::move(chunks);
                    found.snapshot_name = name;
                    break;
                }
            }
            if (why == kOlderSave || why == "This save has an unsupported format. Start a new camp.") {
                found.problem = why;
                return;
            }
            f.set_aside(path);
            std::string note = path;
            note += ": ";
            note += why;
            note += ", moved aside";
            found.damaged.push_back(std::move(note));
        }

        found.about = [&]() -> std::optional<std::string> {
            const std::optional<Bytes> b = f.read(kAbout);
            if (!b) {
                return std::nullopt;
            }
            std::string text;
            for (const std::byte c : *b) {
                text.push_back(static_cast<char>(c));
            }
            return text;
        }();

        if (!found.snapshot && found.about && !found.about->starts_with(metadata_format())) {
            found.problem = std::string(kOlderSave);
            return;
        }

        // the journal, cut at its first bad record
        if (const std::optional<Bytes> bytes = f.read(kJournal)) {
            const LogRead log = read_log(*bytes, 1);
            if (log.cut) {
                f.cut(kJournal, log.good);
                found.damaged.push_back(kJournal + ": cut at its first bad record");
            }
            for (const Entry& e : log.entries) {
                ByteReader r(e.body);
                if (e.type == kCommand) {
                    world::Command c;
                    if (r.u64(c.number) && r.i64(c.at) && r.u32(c.what) && r.u64(c.a) && r.u64(c.b) && r.finished()) {
                        found.commands.push_back(c);
                    }
                } else if (e.type == kPause) {
                    time::Seconds at = 0;
                    if (r.i64(at) && r.finished()) {
                        found.paused_at = at;
                    }
                }
            }
            journal_next_ = log.entries.size() + 1;
        }

        // the history: its newest file says which years are thinned already, since a year is thinned before anything
        // of the year 26 after it is written; the files from the snapshot's year on are read, each cut at its first
        // bad record, and the later ones go, since they could no longer follow on
        std::vector<std::pair<std::int64_t, std::string>> years;
        for (const std::string& name : f.list("history")) {
            if (const std::optional<std::int64_t> y = file_year(name)) {
                years.emplace_back(*y, "history/" + name);
            }
        }
        std::stable_sort(years.begin(), years.end(), [](const auto& x, const auto& y) { return x.first < y.first; });
        if (!years.empty()) {
            year_ = years.back().first;
            thinned_ = std::max<std::int64_t>(0, year_ - kWholeYears - 1);
        }
        opened_at_ = number_before(found.snapshot_name, ".kds").value_or(0);
        const std::int64_t snapshot_year = year_of(opened_at_);
        bool broken = false;
        for (const auto& [year, path] : years) {
            if (year < snapshot_year || year <= thinned_) {
                continue;
            }
            if (broken) {
                f.remove(path);
                continue;
            }
            const Year y = read_year(f.read(path).value_or(Bytes{}));
            if (y.cut) {
                f.cut(path, y.good);
                found.damaged.push_back(path + ": cut at its first bad record");
            }
            if (y.thinned) {
                // thinned, though the newest file does not show it, as when a power cut took the first record of the
                // year that thinned it: settled, and left as it is
                thinned_ = std::max(thinned_, year);
                continue;
            }
            for (const Year::Held& h : y.records) {
                stored_.push_back(h.record);
                where_.push_back({path, h.offset, h.sequence});
            }
            sequences_[path] = y.records.empty() ? 1 : y.records.back().sequence + 1;
            broken = y.cut;
        }
    });
    found.history = stored_;
    return found;
}

Update Keeper::begin(const Found& found, const data::Catalogue& catalogue) {
    if (!found.snapshot) {
        // a world made new begins under this version
        versions_ = Versions{};
        versions_.build = build_;
        versions_.eras.push_back({build_, 0});
        return Update::none;
    }
    if (found.versions && found.versions->making != making_digest(catalogue)) {
        return Update::big;
    }
    if (found.versions && found.versions->build == build_) {
        versions_ = *found.versions;
        played_.store(versions_.played, std::memory_order_relaxed);
        return Update::none;
    }
    return Update::older;
}

void Keeper::about(const std::string& text) {
    Bytes b;
    const auto formatted = text.starts_with(metadata_format()) ? text : std::string(metadata_format()) + text;
    for (const char c : formatted) {
        b.push_back(static_cast<std::byte>(c));
    }
    io_.now([&](Files& f) { f.write_whole(kAbout, b); });
}

std::optional<Bytes> Keeper::read(const std::string& path) {
    std::optional<Bytes> out;
    io_.now([&](Files& f) { out = f.read(path); });
    return out;
}

void Keeper::write(const std::string& path, Bytes bytes) {
    io_.post([path, bytes = std::move(bytes)](Files& f) { f.write_whole(path, bytes); });
}

bool Keeper::command(const world::Command& c) {
    const Bytes record = frame(kCommand, journal_next_++, command_body(c));
    bool safe = false;
    io_.now([&](Files& f) {
        safe = !failed() && f.append(kJournal, record) && f.sync(kJournal);
        if (!safe) {
            fail();
        }
    });
    return safe;
}

void Keeper::pause_mark(time::Seconds frontier) {
    ByteWriter w;
    w.i64(frontier);
    const Bytes record = frame(kPause, journal_next_++, w.bytes());
    io_.now([&](Files& f) {
        if (!failed() && !(f.append(kJournal, record) && f.sync(kJournal))) {
            fail();
        }
    });
}

void Keeper::expect(time::Seconds frontier) {
    // the records the folder holds from the frontier on, which the world will make again in order
    next_ =
        static_cast<std::size_t>(std::partition_point(stored_.begin(), stored_.end(),
                                                      [&](const world::Record& r) { return r.key.second < frontier; }) -
                                 stored_.begin());
}

void Keeper::history(std::span<const world::Record> records) {
    for (const world::Record& r : records) {
        const std::int64_t year = year_of(r.key.second);
        if (year <= thinned_) {
            // a thinned year made again, by a world made again from its seed: its history is settled, and stays
            continue;
        }
        if (year > year_) {
            // a new year: the years now more than 25 years past are thinned first, so a file of this year shows they
            // are
            for (std::int64_t y = thinned_ + 1; y < year - kWholeYears; ++y) {
                thin(y);
            }
            thinned_ = std::max(thinned_, year - kWholeYears - 1);
            year_ = year;
        }
        if (next_ < stored_.size()) {
            if (r == stored_[next_]) {
                ++next_;
                continue;
            }
            // made differently than it was written: counted, and the history from it on replaced
            mismatches_.fetch_add(1, std::memory_order_relaxed);
            cut_history(next_);
        }
        append(r);
    }
}

void Keeper::cut_history(std::size_t index) {
    const Place at = where_[index];
    std::vector<std::string> later;
    for (auto it = sequences_.upper_bound(at.path); it != sequences_.end(); ++it) {
        later.push_back(it->first);
    }
    io_.now([&](Files& f) {
        if (failed()) {
            return;
        }
        if (!f.cut(at.path, at.offset)) {
            fail();
            return;
        }
        for (const std::string& p : later) {
            f.remove(p);
        }
    });
    sequences_.erase(sequences_.upper_bound(at.path), sequences_.end());
    sequences_[at.path] = at.sequence;
    stored_.resize(index);
    where_.resize(index);
}

void Keeper::append(const world::Record& r) {
    const std::string path = history_file(r.key.second);
    auto [it, made] = sequences_.try_emplace(path, 1);
    Bytes record = record_frame(r, it->second++, keeps_ && keeps_(r));
    io_.post([this, path, record = std::move(record)](Files& f) {
        // after a failed write nothing more: a record written past a gap would be cut away with all after it
        if (failed()) {
            return;
        }
        if (!f.append(path, record)) {
            fail();
            return;
        }
        if (unsynced_.empty() || unsynced_.back() != path) {
            unsynced_.push_back(path);
        }
    });
}

void Keeper::thin(std::int64_t year) {
    sequences_.erase(year_file(year));
    io_.post([this, path = year_file(year)](Files& f) {
        if (failed()) {
            return;
        }
        const std::optional<Bytes> bytes = f.read(path);
        if (!bytes) {
            return;
        }
        const Year y = read_year(*bytes);
        if (y.thinned) {
            return;
        }
        Bytes out = frame(kThinned, 1, {});
        std::uint64_t sequence = 2;
        for (const Year::Held& h : y.records) {
            if (h.kept) {
                const Bytes record = record_frame(h.record, sequence++, true);
                out.insert(out.end(), record.begin(), record.end());
            }
        }
        if (!f.write_whole(path, out)) {
            fail();
        }
    });
}

void Keeper::snapshot(const world::World& w) {
    // the state copied here, between events, with what it is saved under; compressed and written on the I/O thread
    std::vector<Chunk> chunks = w.save();
    Versions v = versions_;
    v.making = making_digest(w.catalogue());
    v.rules = rules_digest(w.catalogue());
    v.played = played_.load(std::memory_order_relaxed);
    chunks.push_back(versions_chunk(v));
    const time::Seconds frontier = w.frontier();
    io_.post([this, chunks = std::move(chunks), frontier](Files& f) mutable {
        // never a snapshot after a failed write: the world opens at the one before, and makes again what was lost
        if (failed()) {
            return;
        }
        // the history before the snapshot is safe first, so it can never be lost behind it
        for (const std::string& path : unsynced_) {
            if (!f.sync(path)) {
                fail();
                return;
            }
        }
        unsynced_.clear();
        if (!publish_pages(f, chunks)) {
            fail();
            return;
        }
        const Bytes bytes = write_snapshot(chunks);
        const std::string name = snapshot_file(frontier);
        if (!f.write_whole(name, bytes)) {
            fail();
            return;
        }
        // the newest two kept, the older removed only now the new one is safe
        std::vector<std::string> names = f.list("snapshots");
        std::erase_if(names, [](const std::string& n) { return !ends_with(n, ".kds"); });
        for (std::size_t i = 0; i + 2 < names.size(); ++i) {
            f.remove("snapshots/" + names[i]);
        }
        // Only constituents absent from both recoverable snapshots may expire.
        std::set<std::string> pinned;
        bool complete = true;
        for (const auto& kept : f.list("snapshots")) {
            if (!ends_with(kept, ".kds")) continue;
            const auto data = f.read("snapshots/" + kept);
            std::string why;
            const auto decoded = data ? read_snapshot(*data, why) : std::nullopt;
            const auto refs = decoded ? page_paths(*decoded) : std::nullopt;
            if (!refs) {
                complete = false;
                break;
            }
            pinned.insert(refs->begin(), refs->end());
        }
        if (complete)
            for (const auto& page : f.list("pages"))
                if (ends_with(page, ".kdp") && !pinned.contains("pages/" + page)) f.remove("pages/" + page);
        snapshots_.fetch_add(1, std::memory_order_relaxed);
        last_snapshot_.store(frontier, std::memory_order_relaxed);
        last_bytes_.store(bytes.size(), std::memory_order_relaxed);
    });
}

void Keeper::flush() {
    io_.flush();
}

}  // namespace kd::save
