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
