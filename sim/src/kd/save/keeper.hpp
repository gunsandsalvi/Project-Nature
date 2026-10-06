// A world's folder kept (A3.7): its snapshots, its command journal and its history, through its own I/O thread. Your
// commands are the only input that cannot be made again, so each is written and synced before it acts; everything
// after the newest snapshot is made again by running the world on from it, exactly (TIM-16), and the history made
// again is checked against what was written.
//
//     <world>/world.toml                  its name, seed and size, for making it again if no snapshot is whole
//     <world>/snapshots/<frontier>.kds    the newest two snapshots
//     <world>/journal.log                 your commands, and a mark each time the app left the screen
//     <world>/history/<year>.log          the history, a file for each game year, each numbered from 1; a year more
//                                         than 25 years past is thinned to the records that stay for ever (PLT-10)
//     <world>/previous/                   after an update, the previous version's last snapshot and world.toml, until
//                                         the world has run an hour under the new one (PLT-09)
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "kd/save/files.hpp"
#include "kd/save/versions.hpp"
#include "kd/world/world.hpp"

namespace kd::save {

/// What a world's folder holds as it is opened.
struct Found {
    /// The newest snapshot whose every hash holds, as its chunks, and its file's name; nothing if there is none.
    std::optional<std::vector<Chunk>> snapshot;
    std::string snapshot_name;
    /// Each damaged file moved aside, or log cut, and why.
    std::vector<std::string> damaged;
    /// world.toml, if it is there.
    std::optional<std::string> about;
    /// Every command the journal holds, in order, and the frontier at its last pause mark, if any.
    std::vector<world::Command> commands;
    std::optional<time::Seconds> paused_at;
    /// The history's records from the snapshot's year on, in key order: those the world may make again.
    std::vector<world::Record> history;
    /// What the snapshot was saved under; nothing for a snapshot from before versions were kept, as α1.4a's.
    std::optional<Versions> versions;
};

/// A year of the history as its file holds it (A3.7, PLT-10).
struct Year {
    /// A record as its file holds it: where its frame begins, its number there, and whether it stays for ever, as its
    /// kind said when it was written. A year is numbered from 1, or on from the year before as α1.4a numbered it.
    struct Held {
        world::Record record;
        std::uint64_t offset = 0;
        std::uint64_t sequence = 0;
        bool kept = false;
    };
    /// Its records in key order.
    std::vector<Held> records;
    /// Whether the year is thinned, holding only the records that stay for ever.
    bool thinned = false;
    /// The bytes its good records take, and whether anything after them was refused, so the file is cut there.
    std::uint64_t good = 0;
    bool cut = false;
};

/// Implements TIM-05 and PLT-07, see A3.7: a world's folder, kept through its own I/O thread.
class Keeper {
public:
    /// A game year, the length of each of the history's files.
    static constexpr time::Seconds kYear = 60 * time::kDay;
    /// The years past whose history keeps every event; a year older than these keeps only the records whose kinds
    /// stay for ever, thinned as each year begins (PRN-15, PLT-10).
    static constexpr std::int64_t kWholeYears = 25;

    /// Real seconds a world runs under a new version before the previous version's last snapshot goes (PLT-09).
    static constexpr std::int64_t kPreviousKept = 3600;

    /// The keeper of a world's folder for a version of the app, such as "α1.4b", which each snapshot records.
    explicit Keeper(Files& files, std::string build = "");
    /// Waits for every write so far.
    ~Keeper();
    Keeper(const Keeper&) = delete;
    Keeper& operator=(const Keeper&) = delete;

    /// Reads the folder, first of all: each snapshot that is not whole moved aside, newest first, until one is; the
    /// journal and the history from the snapshot's year on each cut at their first bad record.
    [[nodiscard]] Found open();
    /// After opening, before the world runs: whether a version other than this one saved the folder's snapshot, and
    /// if so whether the update is small or big for it (PLT-09). After a small one the world carries on: the previous
    /// version's snapshot is kept aside until the world has run an hour under this one, the history after it is made
    /// again under this version's rules rather than compared, and the version is added to the world's eras. After a
    /// big one nothing is written, and the world must not run.
    Update begin(const Found& found, const data::Catalogue& catalogue);
    /// What each snapshot records: the world keeps its migrations up to date here as it opens.
    [[nodiscard]] Versions& versions() { return versions_; }
    /// Real seconds the world has run under this version, added from any thread, and in all.
    void played(std::int64_t seconds) { played_.fetch_add(seconds, std::memory_order_relaxed); }
    [[nodiscard]] std::int64_t played() const { return played_.load(std::memory_order_relaxed); }
    /// Writes world.toml whole.
    void about(const std::string& text);
    /// Which records stay in the history for ever, asked of each as it is written: the world's own rule (PRN-15). With
    /// none, every record goes once its year is more than 25 years past.
    void keep_kinds(std::function<bool(const world::Record&)> keeps) { keeps_ = std::move(keeps); }

    /// A command, written and synced before it acts.
    void command(const world::Command& c);
    /// A pause mark at the frontier, synced, as the app leaves the screen.
    void pause_mark(time::Seconds frontier);
    /// History records as they happen: written at once, synced with the next snapshot. While the world makes again
    /// those it had written before it closed, each is compared instead; one that differs is counted, and the history
    /// written from it on is replaced. The first record of each year thins the year now more than 25 years past.
    void history(std::span<const world::Record> records);
    /// After opening: the world's frontier at its snapshot, so the records the folder holds from there on will be
    /// made again.
    void expect(time::Seconds frontier);
    /// A snapshot of a world between events: its state copied now, then compressed and written on the I/O thread,
    /// after the history is synced; the newest two are kept.
    void snapshot(const world::World& w);
    /// Waits until everything given so far is written.
    void flush();

    /// Records made again that differed from those written, which is a bug (PLT-07).
    [[nodiscard]] std::uint64_t mismatches() const { return mismatches_; }
    /// Records still expected to be made again.
    [[nodiscard]] std::uint64_t expecting() const { return stored_.size() - next_; }
    /// The snapshots written since the folder was opened, and the newest one's frontier and size.
    [[nodiscard]] std::uint64_t snapshots() const { return snapshots_.load(std::memory_order_relaxed); }
    [[nodiscard]] time::Seconds last_snapshot() const { return last_snapshot_.load(std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t last_snapshot_bytes() const { return last_bytes_.load(std::memory_order_relaxed); }

private:
    // a stored record's place: its file, where its frame begins, and its sequence number there
    struct Place {
        std::string path;
        std::uint64_t offset = 0;
        std::uint64_t sequence = 1;
    };

    void append(const world::Record& r);
    /// Cuts the history back to before a stored record: its file cut there, and the later files removed.
    void cut_history(std::size_t index);
    /// Thins a year's file on the I/O thread to the records that stay for ever, unless it is thinned already.
    void thin(std::int64_t year);

    std::string build_;
    Versions versions_;
    std::atomic<std::int64_t> played_{0};
    // the frontier of the snapshot opened, whether it was saved by another version, and whether a previous version's
    // snapshot is kept aside
    time::Seconds opened_at_ = 0;
    bool updated_ = false;
    bool previous_ = false;
    std::function<bool(const world::Record&)> keeps_;
    // the stored records from the snapshot's year on, and where each is; the world makes again those from next_ on
    std::vector<world::Record> stored_;
    std::vector<Place> where_;
    std::size_t next_ = 0;
    std::uint64_t mismatches_ = 0;
    std::uint64_t journal_next_ = 1;
    // the sequence number of the next record of each history file read or written
    std::map<std::string, std::uint64_t> sequences_;
    // the year of the newest file of the history, and the newest year thinned, 0 for none
    std::int64_t year_ = 0;
    std::int64_t thinned_ = 0;
    // the history files written to since the last sync
    std::vector<std::string> unsynced_;
    std::atomic<std::uint64_t> snapshots_{0};
    std::atomic<time::Seconds> last_snapshot_{-1};
    std::atomic<std::uint64_t> last_bytes_{0};
    // last, so its thread finishes before what its jobs use is gone
    IoThread io_;
};

/// Implements PLT-10, see A3.7: a year of the history from its file's bytes, up to its first bad record.
[[nodiscard]] Year read_year(std::span<const std::byte> bytes);
/// A record of the history framed for its year's file: its number there, and whether it stays for ever.
[[nodiscard]] Bytes record_frame(const world::Record& r, std::uint64_t sequence, bool kept);
/// A game second's year, the first being 1.
[[nodiscard]] std::int64_t year_of(time::Seconds second);
/// The history's file for a year, "history/000031.log", and a file's year from its name, "000031.log", if it is one.
[[nodiscard]] std::string year_file(std::int64_t year);
[[nodiscard]] std::optional<std::int64_t> file_year(const std::string& name);
/// The history's file for a game second's year.
[[nodiscard]] std::string history_file(time::Seconds second);

}  // namespace kd::save
