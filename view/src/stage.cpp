#include "stage.hpp"

namespace kd::view {

void Stage::put(std::uint64_t id, const Copy& copy) {
    const auto it = copies_.find(id);
    if (it != copies_.end() && it->second == copy) {
        return;
    }
    copies_[id] = copy;
    changed_[id] = false;
}

void Stage::drop(std::uint64_t id) {
    if (copies_.erase(id) == 0) {
        return;
    }
    changed_[id] = true;
}

const Copy* Stage::find(std::uint64_t id) const {
    const auto it = copies_.find(id);
    return it == copies_.end() ? nullptr : &it->second;
}

std::vector<Change> Stage::drain() {
    std::vector<Change> out;
    out.reserve(changed_.size());
    for (const auto& [id, gone] : changed_) {
        out.push_back({id, gone});
    }
    changed_.clear();
    return out;
}

}  // namespace kd::view
