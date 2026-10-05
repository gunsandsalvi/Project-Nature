// A small catalogue for the tests: the demonstration's sources as data/ holds them, written out here so a test can
// change one file and see what the catalogue makes of it.
#pragma once

#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"

namespace kd::test {

namespace data = kd::data;

inline constexpr const char* kWalker =
    "speed = \"1.4 m/s\"\n"
    "reach = \"2 m\"\n"
    "greets = \"30%\"\n"
    "rest = { life = \"2 h\", game = \"2 h\" }\n"
    "colour = \"#e8c25a\"\n"
    "gait = \"walk\"\n"
    "walks_with = [\"demo:strider\"]\n";

inline constexpr const char* kStrider =
    "speed = \"2 m/s\"\n"
    "reach = \"3 m\"\n"
    "greets = \"1 in 10\"\n"
    "rest = { life = \"45 min\", game = \"45 min\" }\n"
    "colour = \"#8fd18a\"\n"
    "gait = \"stride\"\n";

inline constexpr const char* kTime =
    "person = \"1 min\"\nclose_camp = \"1 h\"\ncamp = \"8 h\"\nvalley = \"1 season\"\nregion = \"3 year\"\n";

inline constexpr const char* kBase = "id = \"base\"\nversion = 1\nabout = \"the game\"\n";
inline constexpr const char* kDemo =
    "id = \"demo\"\nversion = 3\nabout = \"the demonstration\"\nrequires = [\"base\"]\n";

inline constexpr const char* kCrowd =
    "camps = 400\nper_camp = 25\narea = \"20 km\"\nwander = \"1 km\"\ndawn = \"6 h\"\ndusk = \"20 h\"\n"
    "greeting = { life = \"1 min\", game = \"1 min\" }\n";

inline constexpr const char* kOrders =
    "[[order]]\n"
    "kind = \"marker\"\n"
    "field = \"speed\"\n"
    "least_to_most = [\"demo:walker\", \"demo:strider\"]\n"
    "why = \"a stride covers more ground than a walk\"\n";

inline std::vector<data::SourceFile> good() {
    return {{"demo/marker/walker.toml", kWalker}, {"demo/marker/strider.toml", kStrider},
            {"base/tuning/time.toml", kTime},     {"base/source.toml", kBase},
            {"demo/source.toml", kDemo},          {"demo/tuning/crowd.toml", kCrowd},
            {"demo/checks/orders.toml", kOrders}};
}

// The problems a catalogue finds when one file of the good ones is replaced or added.
inline std::vector<std::string> problems_with(const std::string& path, const std::string& text) {
    std::vector<data::SourceFile> files;
    for (data::SourceFile& f : good()) {
        if (f.path != path) {
            files.push_back(f);
        }
    }
    files.push_back({path, text});
    data::Catalogue cat;
    std::vector<std::string> out;
    for (const data::Problem& p : cat.load(files)) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

// The problems the checks find (MAT-17) when one file of the good ones is replaced or added, once it loads.
inline std::vector<std::string> checked_with(const std::string& path, const std::string& text) {
    std::vector<data::SourceFile> files;
    for (data::SourceFile& f : good()) {
        if (f.path != path) {
            files.push_back(f);
        }
    }
    files.push_back({path, text});
    data::Catalogue cat;
    std::vector<std::string> out;
    std::vector<data::Problem> found = cat.load(files);
    if (found.empty()) {
        found = data::run_checks(cat);
    }
    out.reserve(found.size());
    for (const data::Problem& p : found) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

// The walker with one line replaced, or added when the key is new.
inline std::string walker_with(const std::string& key, const std::string& line) {
    std::string text;
    bool replaced = false;
    for (std::size_t at = 0, end = 0; at < std::string(kWalker).size(); at = end + 1) {
        const std::string all(kWalker);
        end = all.find('\n', at);
        const std::string this_line = all.substr(at, end - at);
        if (this_line.rfind(key + " =", 0) == 0) {
            if (!line.empty()) {
                text += line + "\n";
            }
            replaced = true;
        } else {
            text += this_line + "\n";
        }
    }
    return replaced ? text : text + line + "\n";
}

}  // namespace kd::test
