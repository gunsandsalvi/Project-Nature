#include "kd/world/upgrades.hpp"

#include <algorithm>

namespace kd::world {

std::span<const save::Upgrade> upgrades() {
    return {};
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
