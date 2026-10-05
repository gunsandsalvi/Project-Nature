// The foundations' crowd (MAT-16): camps placed by keyed chance, and markers that walk from their camp to places
// chosen by keyed chance, rest there, and sleep from dusk to dawn, every move an activity ending at its event
// (TIM-17). It stands in for the world's people until they exist, and shows the clockwork running.
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "kd/data/catalogue.hpp"
#include "kd/demo/crowd.hpp"
#include "kd/demo/marker.hpp"
#include "kd/world/world.hpp"

namespace kd::demo {

/// What a marker does, as its activity's number.
enum class Doing : std::uint8_t { rest = 0, walk = 1, sleep = 2 };

/// Implements TIM-17, see A3.3: the daylight layer, one event that reschedules itself at each dawn and each dusk.
/// Its hours come from the crowd's tuning until the world's sky sets them (WLD-07).
class Daylight final : public world::System {
public:
    Daylight(world::World& w, time::Seconds dawn, time::Seconds dusk);

    [[nodiscard]] std::string_view name() const override { return "daylight"; }
    void handle(world::World& w, const event::Event& e) override;
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

/// Implements TIM-17 and MAT-16, see A3.3: the markers, each walking, resting and sleeping in turn.
class Markers final : public world::System {
public:
    /// Places the camps and their markers by keyed chance, each marker resting at its camp for a while first.
    Markers(world::World& w, const Daylight& daylight, const Crowd& crowd, std::int64_t camps);

    [[nodiscard]] std::string_view name() const override { return "markers"; }
    void handle(world::World& w, const event::Event& e) override;

private:
    const Daylight& daylight_;
    const data::Kind<Marker>& kinds_;
    std::int64_t wander_;  // centimetres
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
