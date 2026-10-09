// The crowd's side of the bridge that touches no Godot (A3.8): the demonstration's world stepped on its runner's
// thread, a snapshot of every walker after each batch in a triple buffer, the greetings in a lossless queue, and the
// counters, so its tests run alone.
#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

#include "kd/demo/crowd_world.hpp"
#include "kd/run/runner.hpp"
#include "kd/save/keeper.hpp"
#include "triple.hpp"

namespace kd::view {

/// One walker as the screen draws it: its id, its kind, and its camp's number in id order.
struct Walker {
    std::uint64_t id = 0;
    std::uint32_t kind = 0;
    std::uint32_t camp = 0;
    std::optional<world::Person> person = std::nullopt;
};

/// Where a walker is along a way at a moment with its fraction of a second, in centimetres east and north of a point:
/// at the whole seconds either side, the world's own place (Activity::at), and between them the screen's straight
/// line, so it moves smoothly and is exactly where the world has it at each whole second.
void place(const num::Torus& torus, const world::Activity& way, double t, num::Point origin, double& east,
           double& north);

/// What a batch leaves for the screen: the walkers in id order at the world's frontier, and each one's ways from the
/// screen's time then to the frontier, oldest first; walker i's are ways[first[i]] to ways[first[i + 1] - 1].
struct Snapshot {
    std::int64_t frontier = -1;
    std::vector<Walker> walkers;
    std::vector<world::Activity> ways;
    std::vector<std::uint32_t> first;
    std::vector<world::Camp> supplies{};
    std::vector<world::Habitat> habitats{};
    std::vector<std::optional<world::Life>> lives{};  // aligned with ways in living camps; empty in marker worlds
    std::vector<std::optional<world::Dream>> dreams{};
    // Living thought updates become visible at their event, even within an unchanged activity.
    std::vector<time::Seconds> changed_at{};

    /// The way walker i was on at a moment: the latest that began by then.
    [[nodiscard]] std::size_t way_index(std::size_t i, double t) const;
    [[nodiscard]] const world::Activity& way_at(std::size_t i, double t) const;
};

/// A greeting worth showing: when, and between whom.
struct Greeting {
    std::int64_t second = 0;
    std::uint64_t from = 0;
    std::uint64_t to = 0;
};

/// Implements WLD-13 and PLT-01, see A3.8 and A3.9: the crowd's world as its runner steps it, and what it hands the
/// screen.
class CrowdStepper final : public run::Steppable {
public:
    /// A batch takes about this much real time at most, so the frontier moves on in small steps at top speed and the
    /// screen glides behind it (A3.9).
    static constexpr double kBatchSeconds = 0.012;
    /// The most game time a batch takes in one step, a game hour.
    static constexpr time::Seconds kStep = 3'600;

    explicit CrowdStepper(demo::CrowdWorld& crowd);

    /// One batch of the world, then its snapshot published; timed, and the thread pinned first if asked.
    time::Seconds advance(time::Seconds frontier, time::Seconds goal) override;
    /// Publish the paused frontier without advancing time; called only on the producer's thread.
    void refresh() {
        fill(snapshots_.back());
        snapshots_.publish();
    }

    /// The screen's side of the triple buffer.
    [[nodiscard]] TripleBuffer<Snapshot>& snapshots() { return snapshots_; }
    /// The greetings since the last call, in the order they happened.
    std::vector<Greeting> drain_greetings();
    /// The screen's game time, set each frame: a way that ended before it is never drawn again, so it is let go.
    void set_screen(double t) { screen_.store(t, std::memory_order_relaxed); }
    /// The camps' places and ids, in id order; they never move.
    [[nodiscard]] const std::vector<num::Point>& camps() const { return camp_places_; }
    [[nodiscard]] const std::vector<ecs::Id>& camp_ids() const { return camp_ids_; }
    /// The keeper the world's history goes to after each batch, if the world is kept in a folder (A3.7).
    void keep(save::Keeper* keeper) { keeper_ = keeper; }
    [[nodiscard]] std::size_t walker_count() const { return ids_.size(); }

    /// Pins the runner's thread to these cores from its next batch, or unpins it when empty (A3.9, for the
    /// benchmark).
    void pin(std::vector<int> cores);

    // the counters, read by the screen at any time
    [[nodiscard]] std::uint64_t events() const { return events_.load(std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t batches() const { return batches_.load(std::memory_order_relaxed); }
    [[nodiscard]] double last_batch_ms() const { return last_batch_ms_.load(std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t greetings() const { return greetings_seen_.load(std::memory_order_relaxed); }
    /// The game seconds the world runs in a real second of work, smoothed over the last batches: what the phone can
    /// do now; 0 before the first batch.
    [[nodiscard]] double capacity() const { return capacity_.load(std::memory_order_relaxed); }

private:
    void fill(Snapshot& s) const;

    demo::CrowdWorld& crowd_;
    std::vector<ecs::Id> ids_;  // the walkers, in id order
    std::vector<Walker> walkers_;
    std::vector<num::Point> camp_places_;
    std::vector<ecs::Id> camp_ids_;
    save::Keeper* keeper_ = nullptr;
    // each walker's ways from the screen's time on, oldest first, and the world's new ones since the last batch
    std::vector<std::vector<world::Activity>> trails_;
    std::vector<std::vector<std::optional<world::Life>>> life_trails_;
    std::vector<std::vector<std::optional<world::Dream>>> dream_trails_;
    std::vector<std::vector<time::Seconds>> change_trails_;
    std::vector<world::Way> ways_;
    TripleBuffer<Snapshot> snapshots_;
    std::vector<world::Record> history_;
    std::mutex greetings_mutex_;
    std::vector<Greeting> greetings_;
    std::mutex pin_mutex_;
    std::vector<int> pin_;
    bool pin_asked_ = false;
    std::atomic<double> screen_{0.0};
    std::atomic<std::uint64_t> events_{0};
    std::atomic<std::uint64_t> batches_{0};
    std::atomic<double> last_batch_ms_{0.0};
    std::atomic<std::uint64_t> greetings_seen_{0};
    std::atomic<double> capacity_{0.0};
};

}  // namespace kd::view
