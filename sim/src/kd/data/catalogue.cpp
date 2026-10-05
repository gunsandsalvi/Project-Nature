#include "kd/data/catalogue.hpp"

#include <set>

namespace kd::data {

namespace {

// A source's or an entry's name: lower case letters, digits and _, starting with a letter (A3.6).
bool valid_name(std::string_view name) {
    if (name.empty() || name.front() < 'a' || name.front() > 'z') {
        return false;
    }
    return std::all_of(name.begin(), name.end(),
                       [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}

}  // namespace

const KindBase* Catalogue::kind_in(std::string_view folder) const {
    for (const auto& k : kinds_) {
        if (k->folder() == folder) {
            return k.get();
        }
    }
    return nullptr;
}

std::vector<Problem> Catalogue::load(std::span<const SourceFile> files) {
    std::vector<Problem> problems;
    const auto problem = [&](const std::string& file, std::string what) {
        problems.push_back({file, 1, 1, std::move(what)});
    };
    // in the order of their paths, so a catalogue never depends on the order its files were handed over
    std::vector<const SourceFile*> sorted;
    sorted.reserve(files.size());
    for (const SourceFile& f : files) {
        sorted.push_back(&f);
    }
    std::stable_sort(sorted.begin(), sorted.end(), [](const auto* a, const auto* b) { return a->path < b->path; });
    std::string folders;
    for (const auto& k : kinds_) {
        folders += (folders.empty() ? "" : ", ") + k->folder() + (k->single() ? ".toml" : "/");
    }
    for (const SourceFile* f : sorted) {
        const std::string& path = f->path;
        const std::size_t slash = path.find('/');
        if (!path.ends_with(".toml") || slash == std::string::npos) {
            problem(path, "a catalogue file is a .toml file in a source's folder, such as base/marker/walker.toml");
            continue;
        }
        const std::string source = path.substr(0, slash);
        const std::string rest = path.substr(slash + 1, path.size() - slash - 1 - 5);
        if (rest == "source" || rest.starts_with("checks/")) {
            continue;
        }
        KindBase* kind = nullptr;
        std::string entry;
        for (const auto& k : kinds_) {
            if (k->single() && k->folder() == rest) {
                kind = k.get();
                entry = rest.substr(rest.rfind('/') + 1);
            }
        }
        if (kind == nullptr) {
            const std::size_t last = rest.rfind('/');
            for (const auto& k : kinds_) {
                if (last != std::string::npos && !k->single() && k->folder() == rest.substr(0, last)) {
                    kind = k.get();
                    entry = rest.substr(last + 1);
                }
            }
        }
        if (kind == nullptr) {
            problem(path, "no kind of entry lives here; a source's kinds are " + folders);
            continue;
        }
        std::string name = source;
        name += ':';
        name += entry;
        if (!valid_name(source) || !valid_name(entry)) {
            std::string what = "a name is in lower case letters, digits and _, starting with a letter: \"";
            what += name;
            what += "\" is not";
            problem(path, std::move(what));
            continue;
        }
        Parsed parsed = parse_toml(f->text, path);
        if (!parsed.problems.empty()) {
            problems.insert(problems.end(), parsed.problems.begin(), parsed.problems.end());
            continue;
        }
        kind->add(name, path, parsed.root, problems);
    }
    for (const auto& k : kinds_) {
        k->sort();
    }
    // a bare name is the base source's (A3.6)
    const Lookup lookup = [this](std::string_view folder, std::string_view written, std::string_view /*source*/,
                                 std::string& canonical, std::uint32_t& index) {
        const KindBase* k = kind_in(folder);
        if (k == nullptr) {
            return false;
        }
        canonical = written.find(':') == std::string_view::npos ? "base:" + std::string(written) : std::string(written);
        const std::optional<std::uint32_t> found = k->find(canonical);
        if (!found) {
            return false;
        }
        index = *found;
        return true;
    };
    for (const auto& k : kinds_) {
        k->resolve(lookup, problems);
    }
    return problems;
}

}  // namespace kd::data
