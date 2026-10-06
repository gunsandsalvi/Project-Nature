#include "kd/world/upgrades.hpp"

#include <algorithm>
#include <array>

namespace kd::world {

namespace {

// WRLD 1 to 2 (α1.5a): the test switches a world runs with joined its clock; a world saved before had none.
bool switches_joined(save::Bytes& data) {
    data.insert(data.end(), 8, std::byte{0});
    return true;
}

constexpr std::array<save::Upgrade, 1> kUpgrades{{{save::tag("WRLD"), 1, switches_joined}}};

}  // namespace

std::span<const save::Upgrade> upgrades() {
    return kUpgrades;
}

std::span<const Migration> migrations() {
    return {};
}

std::vector<std::string> migrate(World& w, std::vector<std::string>& had, std::span<const Migration> all) {
    std::vector<std::string> made;
    for (const Migration& m : all) {
        if (std::find(had.begin(), had.end(), m.name) != had.end()) {
            continue;
        }
        m.make(w);
        had.emplace_back(m.name);
        made.emplace_back(m.name);
    }
    return made;
}

}  // namespace kd::world
