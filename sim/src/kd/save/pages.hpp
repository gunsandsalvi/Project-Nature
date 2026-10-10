// Immutable snapshot constituents (A19.2). The keeper writes and syncs each page
// before publishing its manifest. Opening verifies every referenced constituent.
#pragma once
#include "kd/save/snapshot.hpp"
namespace kd::save {
[[nodiscard]] bool publish_pages(Files& files, std::vector<Chunk>& chunks);
[[nodiscard]] bool resolve_pages(Files& files, std::vector<Chunk>& chunks, std::string& why);
[[nodiscard]] std::optional<std::vector<std::string>> page_paths(std::span<const Chunk> chunks);
}  // namespace kd::save
