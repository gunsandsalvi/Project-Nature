// The winning option and two real alternatives, kept before a timed plan begins (PRN-13).
#pragma once
#include <functional>
#include "kd/world/world.hpp"
namespace kd::demo {
class ChoiceSet {
public:
    using Commit = std::function<bool(std::uint64_t)>;
    static constexpr std::size_t kLimit = 30;
    void add(world::CraftReason reason, Commit commit);
    [[nodiscard]] std::span<const world::CraftReason> reasons() const { return reasons_; }
    bool commit(world::Context& c, world::Beings::Handle h);

private:
    std::vector<world::CraftReason> reasons_;
    std::vector<Commit> commits_;
};
class Choices {
public:
    static std::uint64_t keep(world::Context& c, world::Beings::Handle h, world::CraftReason winner,
                              std::vector<world::CraftReason> alternatives = {}, bool add_body = true);
    [[nodiscard]] static world::CraftReason body(const world::Life& life, std::uint8_t goal);
    static void restore(world::World& w, world::Beings::Handle h, std::uint64_t choice);
};
}  // namespace kd::demo
