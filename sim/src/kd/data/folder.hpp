// A data folder on the disk (A3.6): every TOML file under it, by its path within it, for the cloud's tool and tests;
// on the phone, the view hands the catalogue its files through Godot instead.
#pragma once

#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"

namespace kd::data {

/// Every .toml file under a folder but its scenes, with its path relative to it, such as "demo/marker/walker.toml"; in
/// no order, as the catalogue loads files in path order whatever order they come in.
[[nodiscard]] std::vector<SourceFile> read_folder(const std::string& folder);

/// Every .toml file directly in a folder within the data folder, such as "scenes/look", with its path from the data
/// folder, such as "scenes/look/c1.toml"; in no order, and none if the folder is not there.
[[nodiscard]] std::vector<SourceFile> read_files_in(const std::string& folder, const std::string& within);

/// The art lane's source in a folder that holds art/ (A5.4, A6.1): art/source.toml, each texture's record under
/// art/textures/, such as "art/textures/meadow/middle/record.toml", and each model's recipe under art/models/, such as
/// "art/models/hide_tent_cone/record.toml", by their paths from that folder; nothing else under art/ is the
/// catalogue's, and none if art/source.toml is not there.
[[nodiscard]] std::vector<SourceFile> read_art(const std::string& top);

/// The catalogue's files for a data folder: every .toml file under it but its scenes, and in the repository, where
/// the art lane's source lies beside the data folder rather than in it as on the phone, that source's too.
[[nodiscard]] std::vector<SourceFile> read_catalogue(const std::string& folder);

}  // namespace kd::data
