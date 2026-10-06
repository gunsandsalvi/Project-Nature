// A world's folder kept (A3.7): its snapshots, its command journal and its history, through its own I/O thread. Your
// commands are the only input that cannot be made again, so each is written and synced before it acts; everything
// after the newest snapshot is made again by running the world on from it, exactly (TIM-16), and the history made
// again is checked against what was written.
//
//     <world>/world.toml                  its name, seed and size, for making it again if no snapshot is whole
//     <world>/snapshots/<frontier>.kds    the newest two snapshots
//     <world>/journal.log                 your commands, and a mark each time the app left the screen
//     <world>/history/<year>.log          the history, a file for each game year
#pragma once

#include <atomic>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "kd/save/files.hpp"
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
    /// Every record the history holds, in key order.
    std::vector<world::Record> history;
};

/// Implements TIM-05 and PLT-07, see A3.7: a world's folder, kept through its own I/O thread.
class Keeper {
public:
    /// A game year, the length of each of the history's files.
    static constexpr time::Seconds kYear = 60 * time::kDay;

    explicit Keeper(Files& files);
    /// Waits for every write so far.
    ~Keeper();
    Keeper(const Keeper&) = delete;
    Keeper& operator=(const Keeper&) = delete;

    /// Reads the folder, first of all: each snapshot that is not whole moved aside, newest first, until one is; the
    /// journal and the history each cut at their first bad record.
    [[nodiscard]] Found open();
    /// Writes world.toml whole.
    void about(const std::string& text);

    /// A command, written and synced before it acts.
    void command(const world::Command& c);
    /// A pause mark at the frontier, synced, as the app leaves the screen.
    void pause_mark(time::Seconds frontier);
    /// History records as they happen: written at once, synced with the next snapshot. While the world makes again
    /// those it had written before it closed, each is compared instead; one that differs is counted, and the history
    /// written from it on is replaced.
    void history(std::span<const world::Record> records);
    /// After opening: the world's history holds this many records at its snapshot, so those the folder holds beyond
    /// them will be made again.
    void expect(std::uint64_t count);
    /// A snapshot of a world between events: its state copied now, then compressed and written on the I/O thread,
    /// after the history is synced; the newest two are kept.
    void snapshot(const world::World& w);
    /// Waits until everything given so far is written.
    void flush();

    /// Records made again that differed from those written, which is a bug (PLT-07).
    [[nodiscard]] std::uint64_t mismatches() const { return mismatches_; }
    /// Records still expected to be made again.
    [[nodiscard]] std::uint64_t expecting() const { return expected_.size() - expect_at_; }
    /// The snapshots written since the folder was opened, and the newest one's frontier and size.
    [[nodiscard]] std::uint64_t snapshots() const { return snapshots_.load(std::memory_order_relaxed); }
    [[nodiscard]] time::Seconds last_snapshot() const { return last_snapshot_.load(std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t last_snapshot_bytes() const { return last_bytes_.load(std::memory_order_relaxed); }

private:
    void append(const world::Record& r);
    /// Cuts the history back to before a stored record, its file cut there and later files removed.
    void cut_history(std::uint64_t index);

    // where each stored record of the history is: its file and where its frame begins
    std::vector<std::pair<std::string, std::uint64_t>> where_;
    std::vector<world::Record> stored_;
    // the stored records the world will make again, from the stored record base_ on, and how many it has made
    std::vector<world::Record> expected_;
    std::uint64_t base_ = 0;
    std::size_t expect_at_ = 0;
    std::uint64_t mismatches_ = 0;
    std::uint64_t journal_next_ = 1;
    // the sequence number of the history's next record in its files
    std::uint64_t log_next_ = 1;
    // the history files written to since the last sync
    std::vector<std::string> unsynced_;
    std::atomic<std::uint64_t> snapshots_{0};
    std::atomic<time::Seconds> last_snapshot_{-1};
    std::atomic<std::uint64_t> last_bytes_{0};
    // last, so its thread finishes before what its jobs use is gone
    IoThread io_;
};

/// The history's file for a game second's year.
[[nodiscard]] std::string history_file(time::Seconds second);
/// A whole number from world.toml's "key = 123" lines, if it is there.
[[nodiscard]] std::optional<std::int64_t> about_number(const std::string& about, const std::string& key);

}  // namespace kd::save
