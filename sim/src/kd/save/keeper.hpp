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
    // A refused format stops opening before metadata, journals, recovery or any writes.
    std::string problem;
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
    // Versioned compaction preserves every replay record from this frontier on.
    time::Seconds compacted_before = -1;
    std::uint64_t next_sequence = 1;
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

    /// The keeper of a world's folder for a version of the app, such as "α1.4b", which each snapshot records.
    explicit Keeper(Files& files, std::string build = "");
    /// Waits for every write so far.
    ~Keeper();
    Keeper(const Keeper&) = delete;
    Keeper& operator=(const Keeper&) = delete;

    /// Reads the folder, first of all: each snapshot that is not whole moved aside, newest first, until one is; the
    /// journal and the history from the snapshot's year on each cut at their first bad record.
    [[nodiscard]] Found open();
    /// Check this build and its world-making fingerprint before running the opened state.
    Update begin(const Found& found, const data::Catalogue& catalogue);
    /// The current build record each snapshot holds.
    [[nodiscard]] Versions& versions() { return versions_; }
    /// Real seconds the world has run under this version, added from any thread, and in all.
    void played(std::int64_t seconds) { played_.fetch_add(seconds, std::memory_order_relaxed); }
    [[nodiscard]] std::int64_t played() const { return played_.load(std::memory_order_relaxed); }
    /// Writes world.toml whole.
    void about(const std::string& text);
    /// A file of the caller's own beside the world's, such as a scene's samples of each day: read now, or written
    /// whole after everything given before it, by the I/O thread, the only one that touches the folder while the
    /// keeper keeps it (A3.7).
    [[nodiscard]] std::optional<Bytes> read(const std::string& path);
    void write(const std::string& path, Bytes bytes);
    /// Which records stay in the history for ever, asked of each as it is written: the world's own rule (PRN-15). With
    /// none, every record goes once its year is more than 25 years past.
    void keep_kinds(std::function<bool(const world::Record&)> keeps) { keeps_ = std::move(keeps); }

    /// A command, written and synced before it acts: false if it could not be, when the world must not act on it.
    bool command(const world::Command& c);
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
    [[nodiscard]] std::uint64_t mismatches() const { return mismatches_.load(std::memory_order_relaxed); }
    /// Whether a write to the folder failed, as when the phone's storage is full or broken. From then on nothing more
    /// is written, so the folder keeps what was safe before it and opens there whole, and the world must stop: what it
    /// does after cannot be kept (PLT-07).
    [[nodiscard]] bool failed() const { return failed_.load(std::memory_order_acquire); }
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
    void flush_history();
    /// On the I/O thread, after a write that did not complete: nothing more is written.
    void fail() { failed_.store(true, std::memory_order_release); }
    /// Cuts the history back to before a stored record: its file cut there, and the later files removed.
    void cut_history(std::size_t index);
    /// Thins a year's file on the I/O thread to the records that stay for ever, unless it is thinned already.
    void thin(std::int64_t year);

    std::string build_;
    Versions versions_;
    std::atomic<std::int64_t> played_{0};
    // The frontier of the opened snapshot.
    time::Seconds opened_at_ = 0;
    std::function<bool(const world::Record&)> keeps_;
    // the stored records from the snapshot's year on, and where each is; the world makes again those from next_ on
    std::vector<world::Record> stored_;
    std::vector<Place> where_;
    std::size_t next_ = 0;
    std::atomic<std::uint64_t> mismatches_{0};
    std::atomic<bool> failed_{false};
    std::uint64_t journal_next_ = 1;
    // the sequence number of the next record of each history file read or written
    std::map<std::string, std::uint64_t> sequences_;
    // Routine frames before these declared floors were already summarized.
    std::map<std::string, time::Seconds> replay_floors_;
    std::string pending_path_;
    Bytes pending_history_;
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
