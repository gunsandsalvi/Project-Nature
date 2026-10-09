// Current-format extension storage. Foundation registry layouts remain unchanged.
#pragma once
#include "kd/world/world.hpp"
namespace kd::world {
[[nodiscard]] std::uint32_t craft_features(const World& w);
void save_craft(const World& w, std::vector<save::Chunk>& out);
[[nodiscard]] bool craft_headers(std::span<const save::Chunk> chunks, std::uint32_t& features, std::string& why);
[[nodiscard]] bool load_craft(World& w, std::span<const save::Chunk> chunks, const ecs::EntryMap& entries,
                              std::uint32_t features, std::string& why);
}  // namespace kd::world
