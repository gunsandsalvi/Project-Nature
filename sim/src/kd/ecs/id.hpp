// Ids (A3.2): every entity has a 64-bit id that is never reused, from one world counter, its top four bits naming its
// family. Components, events, history and saves hold only these ids; EnTT's own handles live within one step and are
// never stored.
#pragma once

#include <compare>
#include <cstdint>

#include "kd/core/check.hpp"

namespace kd::ecs {

/// What kind of entity an id belongs to, in its top four bits. None is for the world's own owners of events.
enum class Family : std::uint8_t { none = 0, marker = 1, place = 2, person = 3, animal = 4, group = 5, thing = 6 };

/// Implements RES-05, see A3.2: an entity's id, never reused in its world.
struct Id {
    std::uint64_t value = 0;

    [[nodiscard]] constexpr Family family() const { return static_cast<Family>(value >> 60U); }
    friend constexpr auto operator<=>(Id, Id) = default;
};

/// The world's owners of events that are no entity, its layers and your commands: their ids come before every
/// entity's, so they act first within their second (A3.3).
namespace owners {
inline constexpr Id daylight{1};
inline constexpr Id commands{2};
inline constexpr std::uint64_t count = 3;
}  // namespace owners

/// Implements RES-05, see A3.2: hands out ids in order, never the same twice.
class IdMaker {
public:
    explicit IdMaker(std::uint64_t next = 1) : next_(next) {}

    Id make(Family f) {
        KD_CHECK(f != Family::none, "ecs::IdMaker: an entity's id names its family");
        KD_CHECK(next_ < (std::uint64_t{1} << 60U), "ecs::IdMaker: the world has used every id");
        return Id{(static_cast<std::uint64_t>(f) << 60U) | next_++};
    }

    /// The number the next id will carry, which a save keeps.
    [[nodiscard]] std::uint64_t next() const { return next_; }

private:
    std::uint64_t next_;
};

}  // namespace kd::ecs
