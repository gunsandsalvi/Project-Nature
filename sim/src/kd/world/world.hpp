// The world (A3.1, A3.3): its seed, its sources of data, its two registries of entities, its one queue of events and
// its clock. It runs one event at a time in key order toward a goal, and that one-thread run is the reference every
// faster way must match (TIM-16, TIM-17). Its rules live in systems, each handling the events of the owners it is
// given: one system for each family of entity, and one for each of the world's own owners, its layers.
//
// The faster way is islands (A3.3): game time is cut into windows on a fixed grid, and at each window's start the
// owners that could touch each other within it join one island; islands run side by side, each in key order, and
// their new events merge by key, so the result is the one-thread run's for any window, thread count or goal.
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "kd/core/bytes.hpp"
#include "kd/data/catalogue.hpp"
#include "kd/demo/parts.hpp"
#include "kd/ecs/registry.hpp"
#include "kd/event/queue.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/torus.hpp"
#include "kd/run/runner.hpp"
#include "kd/run/workers.hpp"
#include "kd/save/snapshot.hpp"
#include "kd/world/camp.hpp"
#include "kd/world/dream.hpp"
#include "kd/world/knowledge.hpp"
#include "kd/world/life.hpp"
#include "kd/world/parts.hpp"

namespace kd::world {

/// The registries and their components, in the order of their names (A3.2): a new component is its header, its
/// descriptor, its rules and one line here. Beings (people, animals, places, groups and the demonstration's markers)
/// have 32-bit handles; things have 64-bit ones.
using Beings = ecs::Registry<entt::entity, Activity, demo::Home, ecs::Ident, demo::MarkerKind, Place, Schedule>;
using Things = ecs::Registry<ecs::Handle64, ecs::Ident, Place>;

class World;
class Context;

/// The test switches (RES-10, PRN-12): each turns one thing off, so a scene finds what a result depends on, or plants
/// a fault for the checks to find (RES-12). They are compiled only into the cloud's test builds; the game on the
/// phone has none, though it reads a test world's list to show it.
enum class Switch : std::uint8_t {
    no_greetings,    // the markers never greet
    plant_wander,    // one marker walks far beyond its camp's land
    plant_insomnia,  // one marker never sleeps
    plant_chatter,   // every greeting is recorded twice
    plant_crash,     // the run stops dead halfway
    plant_leak,      // the run takes memory it never gives back
    plant_bad_save,  // the run's last save is damaged
};

/// Each switch's name, as scenes and saves write it, in the order of the switches.
inline constexpr std::array<std::string_view, 7> kSwitchNames{
    "no_greetings", "plant_wander", "plant_insomnia", "plant_chatter", "plant_crash", "plant_leak", "plant_bad_save"};

/// A switch by its name, if there is one.
[[nodiscard]] std::optional<Switch> switch_named(std::string_view name);

/// Whether this build has test switches: the cloud's test builds only.
#ifdef KD_TEST_SWITCHES
inline constexpr bool kSwitches = true;
#else
inline constexpr bool kSwitches = false;
#endif

/// One of your acts (A3.8): its number among the world's commands, the game second it acts at, what it is as the
/// system that takes it numbers it, and whom it is about.
struct Command {
    std::uint64_t number = 0;
    time::Seconds at = 0;
    std::uint32_t what = 0;
    std::uint64_t a = 0;
    std::uint64_t b = 0;
};

/// Where an owner can be within a window: a circle it cannot leave (A3.3), in whole centimetres.
struct Bound {
    ecs::Id id;
    num::Point centre;
    std::int64_t radius = 0;
};

/// A system of the world's rules: it handles the events of the owners it is given, keeps any state of its own beyond
/// the components in its digest, and for islands says where its owners can be within a window.
class System {
public:
    virtual ~System() = default;
    // The bounded craft slice allocates global entity identities at events. Such windows
    // use the reference order until births can be committed safely across independent islands.
    [[nodiscard]] virtual bool serial_windows(const World& /*w*/) const { return false; }
    [[nodiscard]] virtual std::string_view name() const = 0;
    virtual void handle(Context& c, const event::Event& e) = 0;
    virtual void digest(num::Digest& /*d*/) const {}

    /// How near two of its owners must come to touch, in centimetres; 0 if they never touch each other.
    [[nodiscard]] virtual std::int64_t reach() const { return 0; }
    /// For islands: the bound within the window [a, b) of each of these owners, which have events in it.
    virtual void bounds(const World& /*w*/, time::Seconds /*a*/, time::Seconds /*b*/,
                        std::span<const ecs::Id> /*owners*/, std::vector<Bound>& /*out*/) const {}
    /// For islands: the system's owners with no events in the window [a, b) whose place within it could come within
    /// the reach of a bound, so they join its island.
    virtual void near(const World& /*w*/, time::Seconds /*a*/, time::Seconds /*b*/, const Bound& /*bound*/,
                      std::vector<ecs::Id>& /*out*/) const {}
    /// After a window run in islands: the owners whose place changed in it, in id order, for the system's indexes.
    virtual void after_window(World& /*w*/, std::span<const ecs::Id> /*moved*/) {}

    /// One of your commands, at its second, for the system that takes them (A3.8).
    virtual void command(Context& /*c*/, const Command& /*cmd*/) {}
    /// Whether a record of this kind, as the system numbers them, stays in the history for ever, rather than only for
    /// its last 25 years (PRN-15, PLT-10).
    [[nodiscard]] virtual bool keeps(std::uint32_t /*what*/) const { return false; }

    /// Its own state for a snapshot, beyond the components (A3.7); none by default.
    virtual void save(ByteWriter& /*w*/) const {}
    /// Its own state from a snapshot, as save() wrote it; false if the bytes are not that.
    virtual bool load(ByteReader& r) { return r.finished(); }
    /// After the world is opened from a snapshot: its indexes made again from the components.
    virtual void opened(World& /*w*/) {}
};

/// Implements TIM-05 and PLT-07, see A3.3 and A3.8: the world's own owner of your commands, one event at the second of
/// the earliest pending command, which hands each command due then, in number order, to the system that takes them.
/// The pending commands are part of the world's state, in its digest and its snapshots.
class Commands final : public System {
public:
    explicit Commands(World& w) : w_(w) {}

    /// What it records in the history: each command as it acts, by its number and whom it was about.
    static constexpr std::uint32_t kActed = 1;

    [[nodiscard]] std::string_view name() const override { return "commands"; }
    void handle(Context& c, const event::Event& e) override;
    void digest(num::Digest& d) const override;
    /// Your acts stay in the history for ever.
    [[nodiscard]] bool keeps(std::uint32_t /*what*/) const override { return true; }
    void save(ByteWriter& w) const override;
    bool load(ByteReader& r) override;

private:
    friend class World;
    void wake();

    World& w_;
    System* taker_ = nullptr;
    std::vector<Command> pending_;  // in the order of their seconds, then numbers
    std::uint64_t made_ = 0;
};

/// A happening worth keeping in the world's history (PRN-15): the event it happened at, what it was as its system
/// numbers it, and the owners it was between. In islands the history is merged by key, so it reads the same.
struct Record {
    event::Key key;
    std::uint32_t n = 0;  // its number within its event
    std::uint32_t what = 0;
    std::uint64_t a = 0;
    std::uint64_t b = 0;

    friend bool operator==(const Record&, const Record&) = default;
};

/// A way a doer set off on, for the screen (A3.8): the event it began at, the doer, and its activity then. The screen
/// draws a walker along the way it was on at the screen's time, which may be before the world's frontier; in islands
/// the ways are merged by key, so they read the same.
struct Way {
    event::Key key;
    std::uint32_t n = 0;  // its number within its event
    ecs::Id id;
    Activity activity;
    std::optional<Life> life = std::nullopt;
    std::optional<Dream> dream = std::nullopt;
    std::optional<Work> work = std::nullopt;
    std::shared_ptr<const Knowledge> knowledge = {};
};
// Immutable physical state at a real item event, independent of the producer's later frontier.
struct ItemWay {
    event::Key key;
    std::uint32_t n = 0;
    ecs::Id id;
    Place place;
    Item item;
};

/// One island of a window (A3.3): its events, run in key order on one worker, those it makes at or after the
/// window's end, its history and ways, and the owners whose place changed.
struct Island {
    std::uint32_t index = 0;
    time::Seconds end = 0;
    event::Queue queue;
    std::vector<event::Event> outgoing;
    std::vector<ecs::Id> moved;
    std::vector<Record> history;
    std::vector<Way> ways;
    std::uint64_t events = 0;
    std::uint64_t replaced = 0;
};

/// Implements TIM-17 and RES-05, see A3.3: where events run, the whole world one at a time, or one island within a
/// window. A system reads the moment and schedules through it.
class Context {
public:
    [[nodiscard]] World& world() { return w_; }
    [[nodiscard]] const World& world() const { return w_; }
    /// The second of the event being run.
    [[nodiscard]] time::Seconds now() const { return now_; }
    /// Whether this is an island, holding only some owners for one window.
    [[nodiscard]] bool island() const { return island_ != nullptr; }
    /// Whether an owner is one this context may read and touch: in the whole world any, in an island its own.
    [[nodiscard]] bool holds(ecs::Id id) const;
    /// A read of another owner, which must share the island (A3.3): a rule that reads across islands stops the run
    /// rather than quietly changing history.
    void touch(ecs::Id other) const;
    /// The owners of this island whose place changed in the window so far; empty in the whole world.
    [[nodiscard]] std::span<const ecs::Id> moved() const;
    /// An owner's place or activity changed, at the event being run: in an island its system's indexes take it after
    /// the window, and where the world keeps ways for the screen, the owner's new activity joins them.
    void moved(ecs::Id id);
    void item_changed(ecs::Id id);

    /// Schedules the owner's slot to wake at a second: the owner's next sequence number, now the one that slot waits
    /// for, so any event it waited for before dies. A handler may schedule only keys after its own, and for another
    /// owner only from the next second on (A3.3).
    void schedule(ecs::Id owner, std::uint32_t slot, time::Seconds at);
    /// The owner's slot waits for nothing: its event dies and never runs.
    void cancel(ecs::Id owner, std::uint32_t slot);

    /// Adds to the world's history at the event being run.
    void record(std::uint32_t what, std::uint64_t a, std::uint64_t b);

private:
    friend class World;
    Context(World& w, Island* island) : w_(w), island_(island) {}
    void run(const event::Event& e);

    World& w_;
    Island* island_;
    time::Seconds now_ = 0;
    event::Key current_{};
    std::uint32_t records_ = 0;
    std::uint32_t ways_ = 0;
    bool in_event_ = false;
};

/// A digest of each part of the world's state and of the whole, so a difference narrows to a part and a moment.
struct Digests {
    std::uint64_t clock = 0;
    std::uint64_t queue = 0;
    std::uint64_t beings = 0;
    std::uint64_t things = 0;
    std::uint64_t systems = 0;
    std::uint64_t history = 0;
    std::uint64_t whole = 0;
};

/// Implements TIM-16, TIM-17 and RES-05, see A3.3 and A3.4: a world run one event at a time, or in islands with the
/// same result.
class World final : public run::Steppable {
public:
    /// The world's surface, 2,000 by 1,000 km in whole centimetres (A3.4, WLD-01).
    static constexpr num::Torus kTorus{200'000'000, 100'000'000};
    /// The side of the cells islands are joined in, which divides both of the world's sides.
    static constexpr std::int64_t kIslandCell = 25'000;

    World(std::uint64_t seed, const data::Catalogue& catalogue);

    [[nodiscard]] std::uint64_t seed() const { return seed_; }
    [[nodiscard]] const data::Catalogue& catalogue() const { return catalogue_; }
    [[nodiscard]] const num::Torus& torus() const { return torus_; }
    /// How far the world has got: every event before it is done, none at or after it.
    [[nodiscard]] time::Seconds frontier() const { return frontier_; }

    [[nodiscard]] Beings& beings() { return beings_; }
    [[nodiscard]] const Beings& beings() const { return beings_; }
    [[nodiscard]] Things& things() { return things_; }
    [[nodiscard]] const Things& things() const { return things_; }

    /// A new being of a family, with its id and its schedule; never inside an island.
    Beings::Handle make_being(ecs::Family f);
    // A new thing between windows, using the same global identity counter.
    Things::Handle make_thing();
    /// Ends a being: its events never run, and its id is never used again.
    void end_being(ecs::Id id);

    /// The system for a family of beings, or for one of the world's own owners (ecs::owners).
    void set_system(ecs::Family f, System& s);
    void set_layer(ecs::Id owner, System& s);

    /// Schedules and cancels from outside any event, at or after the frontier, as a system does as the world is made
    /// and as your commands do.
    void schedule(ecs::Id owner, std::uint32_t slot, time::Seconds at);
    void cancel(ecs::Id owner, std::uint32_t slot);

    /// Runs every event before the goal, one at a time in key order; the frontier is then the goal.
    void run_to(time::Seconds goal);
    /// Runs every event before the goal in islands, windows of the given length on the workers; the result is
    /// run_to's.
    void run_islands(time::Seconds goal, run::Workers& workers, time::Seconds window);
    /// Islands for the runner's batches: on with workers and a window, or off with none. Up to a window's length
    /// ahead, as at camp speed and below, one worker runs the events in order anyway (A3.3).
    void set_islands(run::Workers* workers, time::Seconds window) {
        workers_ = workers;
        window_ = window;
    }
    /// A batch for the runner (A3.9): up to a game day, with the order fuzzer's scramble first when it is on.
    time::Seconds advance(time::Seconds frontier, time::Seconds goal) override;
    /// Turns the order fuzzer on with a key, or off (A3.2): every pool scrambled before each batch.
    void set_fuzz(std::optional<std::uint64_t> key) { fuzz_ = key; }

    /// The test switches, set before the world runs, in a build that has them (RES-10).
    void set_switches(std::vector<Switch> switches);
    /// The switches the world runs with, which its save records and its digest holds.
    [[nodiscard]] std::span<const Switch> switches() const { return switches_; }
    /// Whether a switch is on; never in the game's own build, which has no switches.
    [[nodiscard]] bool switched(Switch s) const {
        if constexpr (kSwitches) {
            return std::find(switches_.begin(), switches_.end(), s) != switches_.end();
        } else {
            return false;
        }
    }

    /// The system that takes your commands.
    void set_command_taker(System& s) { commands_.taker_ = &s; }
    /// One of your commands, from outside any event, acting at a second at or after the frontier: numbered, and
    /// pending until the world reaches it. Implements TIM-05, see A3.8.
    Command command(time::Seconds at, std::uint32_t what, std::uint64_t a, std::uint64_t b);
    /// A command from the journal again, after the world is reopened from a snapshot older than it (A3.7): it must be
    /// the next by number, and acts at its own second, as it did.
    void replay(const Command& c);
    /// How many commands the world has had.
    [[nodiscard]] std::uint64_t commands_made() const { return commands_.made_; }

    /// The world's state as a snapshot's chunks (A3.7), at an event boundary: its clock, the catalogue's names, both
    /// registries, the live events and each system's own state. Implements PLT-07 and TIM-05.
    [[nodiscard]] std::vector<save::Chunk> save() const;
    /// Fills a world, made from its catalogue with its systems set but nothing yet in it, from a snapshot's chunks;
    /// false, with why, if they are not a world this catalogue and these systems can open. Implements PLT-07 and
    /// TIM-05.
    bool load(std::span<const save::Chunk> chunks, std::string& why);

    /// Each part's digest and the whole's, at the frontier. Implements RES-05.
    [[nodiscard]] Digests digests() const;
    [[nodiscard]] std::uint64_t events_run() const { return events_; }
    [[nodiscard]] const event::Queue& queue() const { return queue_; }
    /// Whether an event is still the one its owner's slot waits for.
    [[nodiscard]] bool live(const event::Event& e) const;
    /// How many records the history holds, and its digest, each record in key order.
    [[nodiscard]] std::uint64_t history_count() const { return history_count_; }
    /// Whether a record stays in the history for ever, as the system of its owner says (PRN-15).
    [[nodiscard]] bool keeps(const Record& r) const;
    /// Keeps every record in a list too, for tests and reports; nothing when null.
    void keep_history(std::vector<Record>* list) { history_list_ = list; }
    /// Keeps every way a doer sets off on in a list, in key order, for the screen; nothing when null. Ways are no
    /// part of the world's state, so keeping them never changes it.
    void keep_ways(std::vector<Way>* list) { ways_list_ = list; }
    void keep_item_ways(std::vector<ItemWay>* list) { item_ways_list_ = list; }
    /// The islands of the last window run in islands, and the owners in the largest, for the counters.
    [[nodiscard]] std::uint64_t islands_run() const { return islands_run_; }
    [[nodiscard]] std::uint64_t largest_island() const { return largest_island_; }
    /// Since the world began: windows run in islands, the islands with events in them, the owners those windows
    /// joined, and the events in each window's largest island, for the counters and the reports.
    struct IslandCounts {
        std::uint64_t windows = 0;
        std::uint64_t islands = 0;
        std::uint64_t owners = 0;
        std::uint64_t events = 0;
        std::uint64_t largest_events = 0;
    };
    [[nodiscard]] const IslandCounts& island_counts() const { return counts_; }

private:
    friend class Context;
    friend class Commands;

    [[nodiscard]] Schedule* schedule_of(ecs::Id owner);
    [[nodiscard]] const Schedule* schedule_of(ecs::Id owner) const;
    [[nodiscard]] System& system_of(ecs::Id owner);
    [[nodiscard]] std::optional<std::uint32_t> island_of(ecs::Id id) const;
    [[nodiscard]] time::Seconds next_layer_event() const;
    void died();
    void add_history(const Record& r);
    void run_window(time::Seconds a, time::Seconds b, run::Workers& workers);
    [[nodiscard]] std::size_t form_islands(time::Seconds a, time::Seconds b, std::span<const ecs::Id> active);

    std::uint64_t seed_;
    const data::Catalogue& catalogue_;
    num::Torus torus_ = kTorus;
    ecs::IdMaker ids_;
    Beings beings_;
    Things things_;
    event::Queue queue_;
    std::array<Schedule, ecs::owners::count> owners_{};
    // when each slot of each of the world's own owners is due, while it waits
    std::array<std::array<time::Seconds, Schedule::kSlots>, ecs::owners::count> owner_due_{};
    std::array<System*, 16> by_family_{};
    std::array<System*, ecs::owners::count> by_owner_{};
    Context context_{*this, nullptr};
    time::Seconds frontier_ = 0;
    std::optional<std::uint64_t> fuzz_;
    std::uint64_t batches_ = 0;
    std::uint64_t events_ = 0;
    run::Workers* workers_ = nullptr;
    time::Seconds window_ = 0;
    // the window being run in islands: its owners in id order, and each one's island
    std::vector<ecs::Id> window_ids_;
    std::vector<std::uint32_t> window_island_;
    bool in_islands_ = false;
    std::uint64_t history_count_ = 0;
    std::uint64_t history_hash_ = 0;
    std::vector<Switch> switches_;
    std::vector<Record>* history_list_ = nullptr;
    std::vector<Way>* ways_list_ = nullptr;
    std::vector<ItemWay>* item_ways_list_ = nullptr;
    std::uint64_t islands_run_ = 0;
    std::uint64_t largest_island_ = 0;
    IslandCounts counts_;
    Commands commands_{*this};
};

}  // namespace kd::world
