// kd_kit: the model kit's own tool (PRE-46, A6.1, A6.4), for the build and the cloud's checks; it needs no Godot.
//
//   kd_kit info <family.kdkit>
//       the family's parts, each with its triangles for each role, its joints and its bounds.
//   kd_kit check <data folder> <models folder>
//       every family file in the models folder, whose name is its family's: each part's texture pixels held to at
//       most 1.5:1 on every triangle (A6.4), and every recipe in the catalogue (the data folder, with the art lane's
//       source beside it) held to its family's parts: the parts and joints it names are there, each role its parts wear
//       has a texture, a span's parts are as long as the span, and each plug's joints meet. Prints each fault and ends
//       with one line; the exit code is 1 on any fault.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/folder.hpp"
#include "kd/look/model.hpp"
#include "kit.hpp"
#include "kit_assemble.hpp"

namespace {

namespace kit = kd::view::kit;

// The stretch every triangle of a texture pixel is held to (A6.4).
constexpr double kStretchLine = 1.5;

std::vector<std::uint8_t> read_file(const std::filesystem::path& path, bool& ok) {
    std::ifstream in(path, std::ios::binary);
    ok = static_cast<bool>(in);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

int info(const std::string& file) {
    bool ok = false;
    const std::vector<std::uint8_t> bytes = read_file(file, ok);
    if (!ok) {
        std::fprintf(stderr, "kd_kit: cannot read %s\n", file.c_str());
        return 1;
    }
    const kit::Read read = kit::read_family(bytes);
    if (!read.problem.empty()) {
        std::fprintf(stderr, "kd_kit: %s: %s\n", file.c_str(), read.problem.c_str());
        return 1;
    }
    std::size_t triangles = 0;
    for (const kit::Part& p : read.family.parts) {
        triangles += p.triangles();
        std::printf("part %s %zu triangles bounds %.3f %.3f %.3f to %.3f %.3f %.3f\n", p.name.c_str(), p.triangles(),
                    static_cast<double>(p.lowest[0]), static_cast<double>(p.lowest[1]),
                    static_cast<double>(p.lowest[2]), static_cast<double>(p.highest[0]),
                    static_cast<double>(p.highest[1]), static_cast<double>(p.highest[2]));
        for (const kit::Section& s : p.sections) {
            std::printf("  role %s %zu vertices %zu triangles\n", s.role.c_str(), s.vertices(), s.triangles());
        }
        for (const kit::Joint& j : p.joints) {
            std::printf("  joint %s at %.3f %.3f %.3f\n", j.name.c_str(), static_cast<double>(j.at[0]),
                        static_cast<double>(j.at[1]), static_cast<double>(j.at[2]));
        }
    }
    std::printf("family %zu parts %zu triangles\n", read.family.parts.size(), triangles);
    return 0;
}

int check(const std::string& data, const std::string& models) {
    int faults = 0;
    const auto say = [&faults](const std::string& what) {
        std::printf("Kit: %s\n", what.c_str());
        ++faults;
    };
    kd::data::Catalogue catalogue;
    for (const kd::data::Problem& p : catalogue.load(kd::data::read_catalogue(data))) {
        say(kd::data::problem_text(p));
    }
    if (faults != 0) {
        return 1;
    }
    // every family file, by its family's name
    std::map<std::string, kit::Family> families;
    std::error_code error;
    std::vector<std::filesystem::path> files;
    for (std::filesystem::directory_iterator it(models, error); !error && it != std::filesystem::directory_iterator();
         it.increment(error)) {
        if (it->is_regular_file(error) && it->path().extension() == ".kdkit") {
            files.push_back(it->path());
        }
    }
    std::sort(files.begin(), files.end());
    std::size_t parts = 0;
    std::size_t triangles = 0;
    double worst = 1.0;
    for (const std::filesystem::path& file : files) {
        bool ok = false;
        const std::vector<std::uint8_t> bytes = read_file(file, ok);
        const kit::Read read = ok ? kit::read_family(bytes) : kit::Read{{}, "cannot be read"};
        const std::string family = file.stem().string();
        if (!read.problem.empty()) {
            say(file.filename().string() + ": " + read.problem);
            continue;
        }
        const kit::StretchReport report = kit::check_stretch(read.family, kStretchLine);
        parts += read.family.parts.size();
        triangles += report.triangles;
        worst = std::max(worst, report.worst);
        // each part with triangles past the line: how many, and its worst
        std::map<std::string, std::pair<std::size_t, kit::Stretched>> over;
        for (const kit::Stretched& s : report.over) {  // worst first
            auto at = over.find(s.part);
            if (at == over.end()) {
                over.emplace(s.part, std::pair<std::size_t, kit::Stretched>{1, s});
            } else {
                ++at->second.first;
            }
        }
        for (const auto& [part, found] : over) {
            char line[200];
            std::snprintf(
                line, sizeof line,
                "%s: the part %s has %zu triangles past %.1f to 1, the worst %.2f to 1 (its role %s, triangle %zu)",
                family.c_str(), part.c_str(), found.first, kStretchLine, found.second.stretch,
                found.second.role.c_str(), found.second.triangle);
            say(line);
        }
        families.emplace(family, read.family);
    }
    // every recipe against its family
    const kd::data::Kind<kd::look::Model>& recipes = catalogue.kind<kd::look::Model>();
    for (std::uint32_t i = 0; i < recipes.size(); ++i) {
        const kd::look::Model& recipe = recipes[i];
        const auto family = families.find(recipe.family);
        if (family == families.end()) {
            say(recipes.name(i) + ": its family \"" + recipe.family + "\" has no " + recipe.family + ".kdkit in " +
                models);
            continue;
        }
        for (const std::string& what : kit::check_model(recipes.name(i), recipe, family->second)) {
            say(what);
        }
    }
    std::printf("Kit: %zu families, %zu parts, %zu triangles, worst stretch %.2f to 1, %u recipes, %s\n",
                families.size(), parts, triangles, worst, static_cast<unsigned>(recipes.size()),
                faults == 0 ? "all fit" : "faults found");
    return faults == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    const std::string command = argc > 1 ? argv[1] : "";
    if (command == "info" && argc == 3) {
        return info(argv[2]);
    }
    if (command == "check" && argc == 4) {
        return check(argv[2], argv[3]);
    }
    std::fprintf(stderr, "usage: kd_kit info <family.kdkit>\n       kd_kit check <data folder> <models folder>\n");
    return 2;
}
