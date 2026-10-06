// The files a world keeps (A3.7): a small interface over a world's folder, the disk's own and a fake for tests that
// can cut the power, and the I/O thread that owns every file and every sync, so neither the simulation nor the screen
// waits on the storage. Writing whole files and keeping logs are written once, on the interface's few primitives, so
// the fake tests the very steps the disk takes.
#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "kd/run/thread.hpp"

namespace kd::save {

using Bytes = std::vector<std::byte>;

/// Implements PLT-07, see A3.7: a world's folder. Paths are relative to it, with '/' between their parts.
class Files {
public:
    virtual ~Files() = default;

    /// The whole of a file, or nothing if there is none.
    [[nodiscard]] virtual std::optional<Bytes> read(const std::string& path) = 0;
    /// The files directly in a folder, by name in order; empty when there is none.
    [[nodiscard]] virtual std::vector<std::string> list(const std::string& folder) = 0;

    /// Writes a file whole and safely: a new file beside it, synced, renamed over the old one, then the folder synced,
    /// so after a crash or a power cut there is either the old file or the new one, whole.
    bool write_whole(const std::string& path, std::span<const std::byte> bytes);
    /// Adds to the end of a file, made if it is missing; nothing is safe until sync().
    bool append(const std::string& path, std::span<const std::byte> bytes);
    /// Makes what was written to a file safe, and its name.
    bool sync(const std::string& path);
    /// Cuts a file to a length, safely, as a log is cut at its first bad record.
    bool cut(const std::string& path, std::uint64_t length);
    /// Moves a damaged file aside, never to be loaded again: it keeps its name with ".damaged" after it.
    bool set_aside(const std::string& path);
    /// Removes a file, safely.
    bool remove(const std::string& path);
    /// Renames a file or a folder within its folder, safely: the folder synced, so the new name lasts.
    bool rename(const std::string& from, const std::string& to);

protected:
    // the primitives, as the system's calls; none is safe from a power cut until its file or folder is synced
    virtual bool put(const std::string& path, std::span<const std::byte> bytes) = 0;
    virtual bool add(const std::string& path, std::span<const std::byte> bytes) = 0;
    virtual bool truncate(const std::string& path, std::uint64_t length) = 0;
    virtual bool move(const std::string& from, const std::string& to) = 0;
    virtual bool erase(const std::string& path) = 0;
    virtual bool sync_file(const std::string& path) = 0;
    virtual bool sync_folder(const std::string& folder) = 0;
    virtual bool make_folder(const std::string& folder) = 0;
};

/// The folder a path is in: "" for the world's own.
[[nodiscard]] std::string folder_of(const std::string& path);

/// Implements PLT-07, see A3.7: a world's folder on the disk, through the system's own calls, since Godot's files
/// never sync (research 18).
class DiskFiles final : public Files {
public:
    explicit DiskFiles(std::string root);

    [[nodiscard]] std::optional<Bytes> read(const std::string& path) override;
    [[nodiscard]] std::vector<std::string> list(const std::string& folder) override;

protected:
    bool put(const std::string& path, std::span<const std::byte> bytes) override;
    bool add(const std::string& path, std::span<const std::byte> bytes) override;
    bool truncate(const std::string& path, std::uint64_t length) override;
    bool move(const std::string& from, const std::string& to) override;
    bool erase(const std::string& path) override;
    bool sync_file(const std::string& path) override;
    bool sync_folder(const std::string& folder) override;
    bool make_folder(const std::string& folder) override;

private:
    [[nodiscard]] std::string full(const std::string& path) const;
    bool write_all(const std::string& path, std::span<const std::byte> bytes, int flags);

    std::string root_;
};

/// Implements PLT-07, see A3.7: files in memory that keep, beside what each file and folder holds now, what a sync
/// made safe, so a test can cut the power at any moment and keep only that, or stop the program between any two of
/// the system's calls.
class FakeFiles final : public Files {
public:
    [[nodiscard]] std::optional<Bytes> read(const std::string& path) override;
    [[nodiscard]] std::vector<std::string> list(const std::string& folder) override;

    /// The power goes: every file holds what was synced, and every folder the names it had when it was synced.
    void power_cut();
    /// The program stops after this many more of the system's calls: later ones do nothing and fail.
    void stop_after(std::uint64_t calls) { calls_left_ = calls; }
    /// Starts again after a stop, as a reopened program does.
    void restart() { calls_left_ = std::nullopt; }
    /// The system's calls made so far.
    [[nodiscard]] std::uint64_t calls() const { return calls_; }
    /// A file's bytes now, to damage in a test; the file must exist.
    [[nodiscard]] Bytes& raw(const std::string& path);

protected:
    bool put(const std::string& path, std::span<const std::byte> bytes) override;
    bool add(const std::string& path, std::span<const std::byte> bytes) override;
    bool truncate(const std::string& path, std::uint64_t length) override;
    bool move(const std::string& from, const std::string& to) override;
    bool erase(const std::string& path) override;
    bool sync_file(const std::string& path) override;
    bool sync_folder(const std::string& folder) override;
    bool make_folder(const std::string& folder) override;

private:
    // a file's contents, which names point to, as a system's inodes
    struct Node {
        Bytes now;
        Bytes safe;
    };

    bool call();

    std::vector<Node> nodes_;
    std::map<std::string, std::size_t> names_;       // path to node, now
    std::map<std::string, std::size_t> safe_names_;  // path to node, as the folders were last synced
    std::uint64_t calls_ = 0;
    std::optional<std::uint64_t> calls_left_;
};

/// Implements PLT-07, see A3.7: the one thread that owns a world's files; jobs run in the order posted.
class IoThread {
public:
    explicit IoThread(Files& files);
    /// Runs every job already posted, then stops.
    ~IoThread();
    IoThread(const IoThread&) = delete;
    IoThread& operator=(const IoThread&) = delete;

    /// A job for the thread, run after every job posted before it.
    void post(std::function<void(Files&)> job);
    /// Waits until every job posted so far has run.
    void flush();
    /// Runs a job on the thread and waits for it, as a command waits to be synced.
    void now(const std::function<void(Files&)>& job);

private:
    void loop();

    Files& files_;
    std::mutex mutex_;
    std::condition_variable wake_;
    std::condition_variable done_;
    std::deque<std::function<void(Files&)>> jobs_;
    std::uint64_t posted_ = 0;
    std::uint64_t ran_ = 0;
    bool stopping_ = false;
    // last, so the thread stops before what it uses is gone
    std::unique_ptr<run::Thread> thread_;
};

}  // namespace kd::save
