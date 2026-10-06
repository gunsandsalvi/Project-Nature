// The foundations' crowd (MAT-16): camps placed by keyed chance, and markers that walk from their camp to places
// chosen by keyed chance or back to the camp, rest there, meet and greet those within reach, and sleep from dusk to
// dawn, every move an activity ending at its event (TIM-17). It stands in for the world's people until they exist,
// and shows the clockwork running, on one core or in islands with the same result.
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/demo/crowd.hpp"
#include "kd/demo/marker.hpp"
#include "kd/world/world.hpp"

namespace kd::demo {

/// What a marker does, as its activity's number.
enum class Doing : std::uint8_t { rest = 0, walk = 1, sleep = 2, greet = 3 };

/// What the crowd records in the world's history.
enum class Happened : std::uint8_t { greeting = 1 };

/// Your commands to the crowd (A3.8): call a camp home, its id the command's first number.
enum class Commanded : std::uint8_t { call_home = 1 };

/// The slot of a marker's schedule a call home wakes, after its activity's and a greeting's call.
inline constexpr std::uint32_t kHomeSlot = 2;

/// The square of the world the crowd keeps to, centred on the world: its south-west corner and its side, in
/// centimetres.
struct Square {
    num::Point south_west;
    std::int64_t side = 0;
};

/// Implements MAT-16: the camps' square, with room for the wander beyond it and a margin.
[[nodiscard]] Square square_of(const num::Torus& torus, const Crowd& crowd);

/// Implements TIM-17, see A3.3: the daylight layer, one event that reschedules itself at each dawn and each dusk.
/// Its hours come from the crowd's tuning until the world's sky sets them (WLD-07).
class Daylight final : public world::System {
public:
    Daylight(world::World& w, time::Seconds dawn, time::Seconds dusk);
    /// A new world's first change of light; an opened world has its event in its queue already.
    void start(world::World& w);

    [[nodiscard]] std::string_view name() const override { return "daylight"; }
    void handle(world::Context& c, const event::Event& e) override;
    void digest(num::Digest& d) const override;
    void save(ByteWriter& w) const override;
    bool load(ByteReader& r) override;

    [[nodiscard]] bool night() const { return night_; }
    /// Whether a moment falls between dusk and dawn.
    [[nodiscard]] bool night_at(time::Seconds t) const;
    /// The first dawn after a moment.
    [[nodiscard]] time::Seconds next_dawn(time::Seconds t) const;

private:
    [[nodiscard]] time::Seconds next_change(time::Seconds t) const;

    time::Seconds dawn_;
    time::Seconds dusk_;
    bool night_;
};

/// Where markers can be, for meeting and for islands: a grid of cells over the crowd's area, each listing the
/// markers whose current way crosses it. It is an index, never part of the world's state, so its order never matters.
class Grid {
public:
    struct Cells {
        std::int64_t x0 = 0;
        std::int64_t y0 = 0;
        std::int64_t x1 = -1;
        std::int64_t y1 = -1;
    };

    Grid(const num::Torus& torus, num::Point south_west, std::int64_t side, std::int64_t cell);

    /// The cells a way from one place to another crosses, widened by a margin.
    [[nodiscard]] Cells cells(num::Point from, num::Point to, std::int64_t margin) const;
    void clear();
    void put(ecs::Id id, const Cells& c);
    void take(ecs::Id id, const Cells& c);
    /// Every id listed in the cells, each perhaps more than once.
    void collect(const Cells& c, std::vector<ecs::Id>& out) const;

private:
    [[nodiscard]] std::int64_t clamp_cell(std::int64_t v) const;

    const num::Torus& torus_;
    num::Point south_west_;
    std::int64_t cell_;
    std::int64_t count_;  // cells along each side
    std::vector<std::vector<ecs::Id>> lists_;
};

/// Implements TIM-17 and MAT-16, see A3.3: the markers, each walking, resting, greeting and sleeping in turn.
class Markers final : public world::System {
public:
    /// The markers' rules for a world, which take its commands too.
    Markers(world::World& w, const Daylight& daylight, const Crowd& crowd);
    /// A new world's camps and markers, placed by keyed chance, each marker resting at its camp for a while first.
    void populate(world::World& w, const Crowd& crowd, std::int64_t camps);

    [[nodiscard]] std::string_view name() const override { return "markers"; }
    void handle(world::Context& c, const event::Event& e) override;
    /// Implements PLT-07, see A3.8: calling a camp home, the demonstration's one command.
    void command(world::Context& c, const world::Command& cmd) override;
    void opened(world::World& w) override;

    [[nodiscard]] std::int64_t reach() const override { return reach_; }
    void bounds(const world::World& w, time::Seconds a, time::Seconds b, std::span<const ecs::Id> owners,
                std::vector<world::Bound>& out) const override;
    void near(const world::World& w, time::Seconds a, time::Seconds b, const world::Bound& bound,
              std::vector<ecs::Id>& out) const override;
    void after_window(world::World& w, std::span<const ecs::Id> moved) override;

private:
    void walk_ended(world::Context& c, world::Beings::Handle h, ecs::Id id);
    void called(world::Context& c, world::Beings::Handle h, ecs::Id id);
    void going_home(world::Context& c, world::Beings::Handle h, ecs::Id id);
    /// A walk from one place to another at the kind's pace, beginning now.
    [[nodiscard]] static world::Activity walk_to(const world::World& w, const Marker& kind, num::Point from,
                                                 num::Point to, time::Seconds now);
    /// The markers that can be greeted at a place and second: within reach, awake, not greeting, not already called.
    [[nodiscard]] std::vector<ecs::Id> greetable(world::Context& c, ecs::Id self, num::Point at, time::Seconds t,
                                                 std::int64_t reach) const;
    /// A marker's activity changed: the world's ways take it, and the grid follows at once, or after the window in
    /// islands.
    void moved(world::Context& c, ecs::Id id);
    void regrid(const world::World& w, ecs::Id id);
    [[nodiscard]] std::size_t index_of(ecs::Id id) const;
    /// The earliest second a marker can set off walking, from what it is doing at a window's start.
    [[nodiscard]] time::Seconds next_walk(const world::Activity& a, const Marker& kind, time::Seconds start) const;
    /// A rest's length, by keyed chance from a second to twice its kind's rest, so its kind's on average and the
    /// crowd never moves in step; and the longest it can be.
    [[nodiscard]] static time::Seconds rest_for(const world::World& w, ecs::Id id, time::Seconds now,
                                                const Marker& kind);
    [[nodiscard]] static time::Seconds longest_rest(const Marker& kind);

    const Daylight& daylight_;
    const data::Kind<Marker>& kinds_;
    std::int64_t wander_;  // centimetres
    num::Probability homeward_;
    time::Seconds greeting_;  // game seconds
    std::int64_t reach_ = 0;  // the farthest any kind reaches, in centimetres
    Grid grid_;
    std::vector<ecs::Id> ids_;  // the markers, in id order
    std::vector<Grid::Cells> cells_;
};

/// A world of the crowd, from the catalogue's demonstration source: the world, its daylight and its markers.
class CrowdWorld {
public:
    /// A new crowd's world; camps, if given, replaces the tuning's number, for smaller worlds in tests.
    CrowdWorld(std::uint64_t seed, const data::Catalogue& catalogue, std::optional<std::int64_t> camps = std::nullopt);
    /// A crowd's world opened from a snapshot's chunks, or nothing, with why. Implements PLT-07, see A3.7.
    [[nodiscard]] static std::unique_ptr<CrowdWorld> open(const data::Catalogue& catalogue,
                                                          std::span<const save::Chunk> chunks, std::string& why);

    [[nodiscard]] world::World& world() { return world_; }
    [[nodiscard]] const world::World& world() const { return world_; }
    [[nodiscard]] const Daylight& daylight() const { return daylight_; }
    [[nodiscard]] Square square() const { return square_of(world_.torus(), crowd_); }

private:
    struct Opening {};
    CrowdWorld(const data::Catalogue& catalogue, Opening opening);

    const Crowd& crowd_;
    world::World world_;
    Daylight daylight_;
    Markers markers_;
};

}  // namespace kd::demo
