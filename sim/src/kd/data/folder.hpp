// A data folder on the disk (A3.6): every TOML file under it, by its path within it, for the cloud's tool and tests;
// on the phone, the view hands the catalogue its files through Godot instead.
#pragma once

#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"

namespace kd::data {

/// Every .toml file under a folder, with its path relative to it, such as "demo/marker/walker.toml"; in no order, as
/// the catalogue loads files in path order whatever order they come in.
[[nodiscard]] std::vector<SourceFile> read_folder(const std::string& folder);

}  // namespace kd::data
