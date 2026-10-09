#pragma once
#include "kd/world/world.hpp"
namespace kd::world {
void save_fire(const World& w, std::vector<save::Chunk>& out);
[[nodiscard]] bool fire_headers(std::span<const save::Chunk> chunks, std::uint32_t features, std::string& why);
[[nodiscard]] bool load_fire(World& w, std::span<const save::Chunk> chunks, const ecs::EntryMap& entries,
                             std::uint32_t features, std::string& why);
}  // namespace kd::world
