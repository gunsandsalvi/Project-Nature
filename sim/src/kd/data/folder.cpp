#include "kd/data/folder.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace kd::data {

std::vector<SourceFile> read_folder(const std::string& folder) {
    std::vector<SourceFile> files;
    std::error_code error;
    // read without exceptions, which the simulation is built without
    for (std::filesystem::recursive_directory_iterator it(folder, error);
         !error && it != std::filesystem::recursive_directory_iterator(); it.increment(error)) {
        const std::filesystem::directory_entry& item = *it;
        // the scenes are tests' settings, not entries (A17)
        if (item.is_directory(error) && it.depth() == 0 && item.path().filename() == "scenes") {
            it.disable_recursion_pending();
            continue;
        }
        if (item.is_regular_file(error) && item.path().extension() == ".toml") {
            std::ifstream in(item.path(), std::ios::binary);
            std::stringstream text;
            text << in.rdbuf();
            files.push_back({std::filesystem::relative(item.path(), folder).generic_string(), text.str()});
        }
    }
    return files;
}

namespace {

SourceFile read_file(const std::filesystem::path& path, std::string relative) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream text;
    text << in.rdbuf();
    return {std::move(relative), text.str()};
}

}  // namespace

std::vector<SourceFile> read_art(const std::string& top) {
    std::vector<SourceFile> files;
    std::error_code error;
    const std::filesystem::path art = std::filesystem::path(top) / "art";
    if (!std::filesystem::is_regular_file(art / "source.toml", error)) {
        return files;
    }
    files.push_back(read_file(art / "source.toml", "art/source.toml"));
    for (std::filesystem::recursive_directory_iterator it(art / "textures", error);
         !error && it != std::filesystem::recursive_directory_iterator(); it.increment(error)) {
        if (it->is_regular_file(error) && it->path().filename() == "record.toml") {
            files.push_back(read_file(it->path(), std::filesystem::relative(it->path(), top).generic_string()));
        }
    }
    return files;
}

std::vector<SourceFile> read_catalogue(const std::string& folder) {
    std::vector<SourceFile> files = read_folder(folder);
    std::error_code error;
    std::filesystem::path data = std::filesystem::path(folder).lexically_normal();
    if (data.filename().empty()) {
        data = data.parent_path();  // written with a slash at its end
    }
    const std::filesystem::path beside = data.parent_path();
    if (!std::filesystem::exists(data / "art", error)) {
        for (SourceFile& f : read_art(beside.empty() ? std::string(".") : beside.string())) {
            files.push_back(std::move(f));
        }
    }
    return files;
}

std::vector<SourceFile> read_files_in(const std::string& folder, const std::string& within) {
    std::vector<SourceFile> files;
    std::error_code error;
    const std::filesystem::path base = std::filesystem::path(folder) / within;
    for (std::filesystem::directory_iterator it(base, error); !error && it != std::filesystem::directory_iterator();
         it.increment(error)) {
        if (it->is_regular_file(error) && it->path().extension() == ".toml") {
            std::ifstream in(it->path(), std::ios::binary);
            std::stringstream text;
            text << in.rdbuf();
            files.push_back({within + "/" + it->path().filename().generic_string(), text.str()});
        }
    }
    return files;
}

}  // namespace kd::data
