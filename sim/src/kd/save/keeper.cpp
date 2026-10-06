#include "kd/save/keeper.hpp"

#include <algorithm>
#include <cstdio>

#include "kd/core/bytes.hpp"
#include "kd/num/whole.hpp"
#include "kd/save/log.hpp"
#include "kd/save/snapshot.hpp"

namespace kd::save {

namespace {

// The types of the journal's records, and of the history's.
constexpr std::uint32_t kCommand = 1;
constexpr std::uint32_t kPause = 2;
constexpr std::uint32_t kHappened = 1;

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

Bytes command_body(const world::Command& c) {
    ByteWriter w;
    w.u64(c.number);
    w.i64(c.at);
    w.u32(c.what);
    w.u64(c.a);
    w.u64(c.b);
    return w.take();
}

Bytes record_body(const world::Record& r) {
    ByteWriter w;
    w.i64(r.key.second);
    w.u64(r.key.owner);
    w.u64(r.key.sequence);
    w.u32(r.n);
    w.u32(r.what);
    w.u64(r.a);
    w.u64(r.b);
    return w.take();
}

bool same(const world::Record& x, const world::Record& y) {
    return x.key == y.key && x.n == y.n && x.what == y.what && x.a == y.a && x.b == y.b;
}

}  // namespace

std::string history_file(time::Seconds second) {
    char name[64];
    std::snprintf(name, sizeof name, "history/%06lld.log",
                  static_cast<long long>(num::floor_div(second, Keeper::kYear)) + 1);
    return name;
}

std::optional<std::int64_t> about_number(const std::string& about, const std::string& key) {
    std::size_t at = 0;
    while (at < about.size()) {
        const std::size_t end = std::min(about.find('\n', at), about.size());
        const std::string line = about.substr(at, end - at);
        at = end + 1;
        const std::string head = key + " = ";
        if (line.compare(0, head.size(), head) != 0 || line.size() == head.size()) {
            continue;
        }
        std::int64_t v = 0;
        for (std::size_t i = head.size(); i < line.size(); ++i) {
            const char c = line[i];
            if (c < '0' || c > '9' || v > (INT64_MAX - 9) / 10) {
                return std::nullopt;
            }
            v = v * 10 + (c - '0');
        }
        return v;
    }
    return std::nullopt;
}

Keeper::Keeper(Files& files) : io_(files) {}

Keeper::~Keeper() {
    io_.flush();
}

Found Keeper::open() {
    Found found;
    io_.now([&](Files& f) {
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
                if (chunks) {
                    found.snapshot = std::move(chunks);
                    found.snapshot_name = name;
                    break;
                }
            }
            f.set_aside(path);
            std::string note = path;
            note += ": ";
            note += why;
            note += ", moved aside";
            found.damaged.push_back(std::move(note));
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

        // the history, file by file, each record numbered on from the last; a damaged file is cut, and the later ones
        // go, since they could no longer follow on
        std::vector<std::string> files = f.list("history");
        std::erase_if(files, [](const std::string& n) { return !ends_with(n, ".log"); });
        bool broken = false;
        for (const std::string& name : files) {
            const std::string path = "history/" + name;
            if (broken) {
                f.remove(path);
                continue;
            }
            const std::optional<Bytes> bytes = f.read(path);
            const LogRead log = read_log(bytes.value_or(Bytes{}), stored_.size() + 1);
            std::uint64_t offset = 0;
            for (const Entry& e : log.entries) {
                ByteReader r(e.body);
                world::Record rec;
                if (e.type != kHappened || !r.i64(rec.key.second) || !r.u64(rec.key.owner) ||
                    !r.u64(rec.key.sequence) || !r.u32(rec.n) || !r.u32(rec.what) || !r.u64(rec.a) || !r.u64(rec.b) ||
                    !r.finished()) {
                    break;
                }
                stored_.push_back(rec);
                where_.emplace_back(path, offset);
                offset += kFrame + e.body.size();
            }
            if (log.cut || offset != log.good) {
                f.cut(path, offset);
                found.damaged.push_back(path + ": cut at its first bad record");
                broken = true;
            }
        }
    });
    log_next_ = stored_.size() + 1;
    found.history = stored_;
    return found;
}

void Keeper::about(const std::string& text) {
    Bytes b;
    for (const char c : text) {
        b.push_back(static_cast<std::byte>(c));
    }
    io_.now([&](Files& f) { f.write_whole(kAbout, b); });
}

void Keeper::command(const world::Command& c) {
    const Bytes record = frame(kCommand, journal_next_++, command_body(c));
    io_.now([&](Files& f) {
        f.append(kJournal, record);
        f.sync(kJournal);
    });
}

void Keeper::pause_mark(time::Seconds frontier) {
    ByteWriter w;
    w.i64(frontier);
    const Bytes record = frame(kPause, journal_next_++, w.bytes());
    io_.now([&](Files& f) {
        f.append(kJournal, record);
        f.sync(kJournal);
    });
}

void Keeper::expect(std::uint64_t count) {
    // the records the folder holds beyond the snapshot's, which the world will make again in order
    base_ = std::min<std::uint64_t>(count, stored_.size());
    expected_.assign(stored_.begin() + static_cast<std::ptrdiff_t>(base_), stored_.end());
    expect_at_ = 0;
}

void Keeper::history(std::span<const world::Record> records) {
    for (const world::Record& r : records) {
        if (expect_at_ < expected_.size()) {
            if (same(r, expected_[expect_at_])) {
                ++expect_at_;
                continue;
            }
            // made differently than it was written: counted, and the history from it on replaced
            ++mismatches_;
            cut_history(base_ + expect_at_);
            expected_.clear();
            expect_at_ = 0;
        }
        append(r);
    }
}

void Keeper::cut_history(std::uint64_t index) {
    if (index >= where_.size()) {
        return;
    }
    const auto [path, offset] = where_[index];
    std::vector<std::string> later;
    for (std::size_t i = index; i < where_.size(); ++i) {
        if (where_[i].first != path && (later.empty() || later.back() != where_[i].first)) {
            later.push_back(where_[i].first);
        }
    }
    io_.now([&](Files& f) {
        f.cut(path, offset);
        for (const std::string& p : later) {
            f.remove(p);
        }
    });
    where_.resize(index);
    stored_.resize(index);
    log_next_ = index + 1;
}

void Keeper::append(const world::Record& r) {
    const std::string path = history_file(r.key.second);
    Bytes record = frame(kHappened, log_next_++, record_body(r));
    io_.post([this, path, record = std::move(record)](Files& f) {
        f.append(path, record);
        if (unsynced_.empty() || unsynced_.back() != path) {
            unsynced_.push_back(path);
        }
    });
}

void Keeper::snapshot(const world::World& w) {
    // the state copied here, between events; compressed and written on the I/O thread
    std::vector<Chunk> chunks = w.save();
    const time::Seconds frontier = w.frontier();
    io_.post([this, chunks = std::move(chunks), frontier](Files& f) {
        // the history before the snapshot is safe first, so it can never be lost behind it
        for (const std::string& path : unsynced_) {
            f.sync(path);
        }
        unsynced_.clear();
        const Bytes bytes = write_snapshot(chunks);
        const std::string name = snapshot_file(frontier);
        if (!f.write_whole(name, bytes)) {
            return;
        }
        // the newest two kept, the older removed only now the new one is safe
        std::vector<std::string> names = f.list("snapshots");
        std::erase_if(names, [](const std::string& n) { return !ends_with(n, ".kds"); });
        for (std::size_t i = 0; i + 2 < names.size(); ++i) {
            f.remove("snapshots/" + names[i]);
        }
        snapshots_.fetch_add(1, std::memory_order_relaxed);
        last_snapshot_.store(frontier, std::memory_order_relaxed);
        last_bytes_.store(bytes.size(), std::memory_order_relaxed);
    });
}

void Keeper::flush() {
    io_.flush();
}

}  // namespace kd::save
