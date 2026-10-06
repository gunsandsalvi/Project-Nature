// The foundations' crowd (MAT-16): camps placed by keyed chance, and markers that walk from their camp to places
// chosen by keyed chance or back to the camp, rest there, meet and greet those within reach, and sleep from dusk to
// dawn, every move an activity ending at its event (TIM-17). It stands in for the world's people until they exist,
// and shows the clockwork running, on one core or in islands with the same result.
#pragma once

#include <cstdint>
#include <optional>
#include <span>
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

/// Implements TIM-17, see A3.3: the daylight layer, one event that reschedules itself at each dawn and each dusk.
/// Its hours come from the crowd's tuning until the world's sky sets them (WLD-07).
class Daylight final : public world::System {
public:
    Daylight(world::World& w, time::Seconds dawn, time::Seconds dusk);

    [[nodiscard]] std::string_view name() const override { return "daylight"; }
    void handle(world::Context& c, const event::Event& e) override;
    void digest(num::Digest& d) const override;

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
    /// Places the camps and their markers by keyed chance, each marker resting at its camp for a while first.
    Markers(world::World& w, const Daylight& daylight, const Crowd& crowd, std::int64_t camps);

    [[nodiscard]] std::string_view name() const override { return "markers"; }
    void handle(world::Context& c, const event::Event& e) override;

    [[nodiscard]] std::int64_t reach() const override { return reach_; }
    void bounds(const world::World& w, time::Seconds a, time::Seconds b, std::span<const ecs::Id> owners,
                std::vector<world::Bound>& out) const override;
    void near(const world::World& w, time::Seconds a, time::Seconds b, const world::Bound& bound,
              std::vector<ecs::Id>& out) const override;
    void after_window(world::World& w, std::span<const ecs::Id> moved) override;

private:
    void walk_ended(world::Context& c, world::Beings::Handle h, ecs::Id id);
    void called(world::Context& c, world::Beings::Handle h, ecs::Id id);
    /// The markers that can be greeted at a place and second: within reach, awake, not greeting, not already called.
    [[nodiscard]] std::vector<ecs::Id> greetable(world::Context& c, ecs::Id self, num::Point at, time::Seconds t,
                                                 std::int64_t reach) const;
    /// A marker's activity changed: the grid follows at once, or after the window in islands.
    void moved(world::Context& c, ecs::Id id);
    void regrid(const world::World& w, ecs::Id id);
    [[nodiscard]] std::size_t index_of(ecs::Id id) const;
    /// The earliest second a marker can set off walking, from what it is doing at a window's start.
    [[nodiscard]] time::Seconds next_walk(const world::Activity& a, const Marker& kind, time::Seconds start) const;

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
    /// camps, if given, replaces the tuning's number, for smaller worlds in tests.
    CrowdWorld(std::uint64_t seed, const data::Catalogue& catalogue, std::optional<std::int64_t> camps = std::nullopt);

    [[nodiscard]] world::World& world() { return world_; }
    [[nodiscard]] const world::World& world() const { return world_; }
    [[nodiscard]] const Daylight& daylight() const { return daylight_; }

private:
    const Crowd& crowd_;
    world::World world_;
    Daylight daylight_;
    Markers markers_;
};

}  // namespace kd::demo
