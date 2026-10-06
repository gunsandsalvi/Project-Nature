// Old saves brought up to date (A3.7, PLT-09). A part of a snapshot carries its version, and each step from one
// version to the next has an upgrade, so a save from any earlier alpha can be read; a component carries its own
// version, and upgrades itself (kd/ecs/component.hpp). A migration is a change made once to every world saved before
// it, as such a world opens: its name is recorded in the save, so it is never made twice.
#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "kd/save/snapshot.hpp"

namespace kd::world {

class World;

/// The steps that bring each part of an older snapshot of a world up to date, oldest first.
[[nodiscard]] std::span<const save::Upgrade> upgrades();

/// A change made once to every world saved before it, as such a world opens: its name, kept in the save, and what it
/// does to the world at its frontier, outside any event.
struct Migration {
    std::string_view name;
    void (*make)(World& w) = nullptr;
};

/// Every migration, oldest first: none yet.
[[nodiscard]] std::span<const Migration> migrations();

/// Implements PLT-09, see A3.7: each of these migrations a world has not had, made in order, its name added to those
/// it has had; the names of those made now.
std::vector<std::string> migrate(World& w, std::vector<std::string>& had, std::span<const Migration> all);

}  // namespace kd::world
