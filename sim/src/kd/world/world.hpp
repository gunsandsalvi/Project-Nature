// The world (A3.1, A3.3): its seed, its sources of data, its two registries of entities, its one queue of events and
// its clock. It runs one event at a time in key order toward a goal, and that one-thread run is the reference every
// faster way must match (TIM-16, TIM-17). Its rules live in systems, each handling the events of the owners it is
// given: one system for each family of entity, and one for each of the world's own owners, its layers.
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/demo/parts.hpp"
#include "kd/ecs/registry.hpp"
#include "kd/event/queue.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/torus.hpp"
#include "kd/run/runner.hpp"
#include "kd/world/parts.hpp"

namespace kd::world {

/// The registries and their components, in the order of their names (A3.2): a new component is its header, its
/// descriptor, its rules and one line here. Beings (people, animals, places, groups and the demonstration's markers)
/// have 32-bit handles; things have 64-bit ones.
using Beings = ecs::Registry<entt::entity, Activity, demo::Home, ecs::Ident, demo::MarkerKind, Place, Schedule>;
using Things = ecs::Registry<ecs::Handle64, ecs::Ident, Place>;

class World;

/// A system of the world's rules: it handles the events of the owners it is given, and keeps any state of its own
/// beyond the components in its digest.
class System {
public:
    virtual ~System() = default;
    [[nodiscard]] virtual std::string_view name() const = 0;
    virtual void handle(World& w, const event::Event& e) = 0;
    virtual void digest(num::Digest& /*d*/) const {}
};

/// A digest of each part of the world's state and of the whole, so a difference narrows to a part and a moment.
struct Digests {
    std::uint64_t clock = 0;
    std::uint64_t queue = 0;
    std::uint64_t beings = 0;
    std::uint64_t things = 0;
    std::uint64_t systems = 0;
    std::uint64_t whole = 0;
};

/// Implements TIM-16, TIM-17 and RES-05, see A3.3 and A3.4: a world run one event at a time.
class World final : public run::Steppable {
public:
    /// The world's surface, 2,000 by 1,000 km in whole centimetres (A3.4, WLD-01).
    static constexpr num::Torus kTorus{200'000'000, 100'000'000};

    World(std::uint64_t seed, const data::Catalogue& catalogue);

    [[nodiscard]] std::uint64_t seed() const { return seed_; }
    [[nodiscard]] const data::Catalogue& catalogue() const { return catalogue_; }
    [[nodiscard]] const num::Torus& torus() const { return torus_; }
    /// The second of the event being run; between runs, the frontier.
    [[nodiscard]] time::Seconds now() const { return now_; }
    /// How far the world has got: every event before it is done, none at or after it.
    [[nodiscard]] time::Seconds frontier() const { return frontier_; }

    [[nodiscard]] Beings& beings() { return beings_; }
    [[nodiscard]] const Beings& beings() const { return beings_; }
    [[nodiscard]] Things& things() { return things_; }
    [[nodiscard]] const Things& things() const { return things_; }

    /// A new being of a family, with its id and its schedule.
    Beings::Handle make_being(ecs::Family f);
    /// Ends a being: its events never run, and its id is never used again.
    void end_being(ecs::Id id);

    /// The system for a family of beings, or for one of the world's own owners (ecs::owners).
    void set_system(ecs::Family f, System& s);
    void set_layer(ecs::Id owner, System& s);

    /// Schedules the owner's slot to wake at a second: the owner's next sequence number, now the one that slot
    /// waits for, so any event it waited for before dies. A handler may schedule only keys after its own, and for
    /// another owner only from the next second on (A3.3).
    void schedule(ecs::Id owner, std::uint32_t slot, time::Seconds at);
    /// The owner's slot waits for nothing: its event dies and never runs.
    void cancel(ecs::Id owner, std::uint32_t slot);

    /// Runs every event before the goal, one at a time in key order; the frontier is then the goal.
    void run_to(time::Seconds goal);
    /// A batch for the runner (A3.9): up to a game day, with the order fuzzer's scramble first when it is on.
    time::Seconds advance(time::Seconds frontier, time::Seconds goal) override;
    /// Turns the order fuzzer on with a key, or off (A3.2): every pool scrambled before each batch.
    void set_fuzz(std::optional<std::uint64_t> key) { fuzz_ = key; }

    /// Each part's digest and the whole's, at the frontier. Implements RES-05.
    [[nodiscard]] Digests digests() const;
    [[nodiscard]] std::uint64_t events_run() const { return events_; }
    [[nodiscard]] const event::Queue& queue() const { return queue_; }

private:
    [[nodiscard]] Schedule* schedule_of(ecs::Id owner);
    [[nodiscard]] const Schedule* schedule_of(ecs::Id owner) const;
    [[nodiscard]] bool live(const event::Event& e) const;
    [[nodiscard]] System& system_of(ecs::Id owner);
    void died();

    std::uint64_t seed_;
    const data::Catalogue& catalogue_;
    num::Torus torus_ = kTorus;
    ecs::IdMaker ids_;
    Beings beings_;
    Things things_;
    event::Queue queue_;
    std::array<Schedule, ecs::owners::count> owners_{};
    std::array<System*, 16> by_family_{};
    std::array<System*, ecs::owners::count> by_owner_{};
    time::Seconds frontier_ = 0;
    time::Seconds now_ = 0;
    event::Key current_{};
    bool in_event_ = false;
    std::optional<std::uint64_t> fuzz_;
    std::uint64_t batches_ = 0;
    std::uint64_t events_ = 0;
};

}  // namespace kd::world
