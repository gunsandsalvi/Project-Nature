#include "kd/data/catalogue.hpp"

#include "kd/num/digest.hpp"

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

std::string joined(std::string_view a, char between, std::string_view b) {
    std::string out(a);
    out += between;
    out += b;
    return out;
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

void Catalogue::load_sources(std::span<const SourceFile* const> files, std::vector<Problem>& problems) {
    std::vector<Source> found;
    std::vector<std::string> files_of;
    for (const SourceFile* f : files) {
        const std::size_t slash = f->path.find('/');
        if (slash == std::string::npos || f->path.substr(slash + 1) != "source.toml") {
            continue;
        }
        Parsed parsed = parse_toml(f->text, f->path);
        if (!parsed.problems.empty()) {
            problems.insert(problems.end(), parsed.problems.begin(), parsed.problems.end());
            continue;
        }
        Source s;
        Loader loader(parsed.root, f->path, problems);
        Source::visit(loader, s);
        loader.finish();
        const std::string folder = f->path.substr(0, slash);
        if (s.id != folder || !valid_name(s.id)) {
            problems.push_back({f->path, 1, 1,
                                "id: \"" + s.id + "\" must be the source's folder's name, \"" + folder +
                                    "\", in lower case letters, digits and _"});
            continue;
        }
        found.push_back(std::move(s));
        files_of.push_back(f->path);
    }
    // each after those it requires, and otherwise by id
    std::vector<bool> placed(found.size(), false);
    sources_.clear();
    for (bool progress = true; progress;) {
        progress = false;
        std::size_t next = found.size();
        for (std::size_t i = 0; i < found.size(); ++i) {
            if (placed[i]) {
                continue;
            }
            const bool ready = std::all_of(found[i].needs.begin(), found[i].needs.end(), [&](const auto& r) {
                return std::any_of(sources_.begin(), sources_.end(), [&](const Source& s) { return s.id == r; });
            });
            if (ready && (next == found.size() || found[i].id < found[next].id)) {
                next = i;
            }
        }
        if (next != found.size()) {
            placed[next] = true;
            sources_.push_back(found[next]);
            progress = true;
        }
    }
    for (std::size_t i = 0; i < found.size(); ++i) {
        if (placed[i]) {
            continue;
        }
        std::string missing;
        for (const std::string& r : found[i].needs) {
            const bool known = std::any_of(found.begin(), found.end(), [&](const Source& s) { return s.id == r; });
            missing += (missing.empty() ? "" : ", ") + r + (known ? " (which cannot load first)" : " (not found)");
        }
        problems.push_back({files_of[i], 1, 1, "requires: the source cannot load after " + missing});
    }
}

void Catalogue::load_renames(const SourceFile& file, const std::string& source, std::vector<Problem>& problems) {
    Parsed parsed = parse_toml(file.text, file.path);
    if (!parsed.problems.empty()) {
        problems.insert(problems.end(), parsed.problems.begin(), parsed.problems.end());
        return;
    }
    for (const Value& kind : parsed.root.items) {
        if (kind.kind != Value::Kind::table || kind_in(kind.key) == nullptr) {
            problems.push_back({file.path, kind.line, kind.column,
                                "\"" + kind.key + "\" is not a kind's folder; renames are listed under each kind's"});
            continue;
        }
        for (const Value& r : kind.items) {
            if (r.kind != Value::Kind::text || !valid_name(r.key) || !valid_name(r.text)) {
                problems.push_back({file.path, r.line, r.column,
                                    "a rename is old_name = \"new_name\", both in lower case letters, digits and _"});
                continue;
            }
            renames_.push_back({kind.key, joined(source, ':', r.key), joined(source, ':', r.text)});
        }
    }
}

std::string_view Catalogue::renamed(std::string_view folder, std::string_view name) const {
    for (const Rename& r : renames_) {
        if (r.folder == folder && r.from == name) {
            return r.to;
        }
    }
    return name;
}

std::optional<std::uint32_t> Catalogue::find(std::string_view folder, std::string_view name) const {
    const KindBase* k = kind_in(folder);
    if (k == nullptr) {
        return std::nullopt;
    }
    return k->find(renamed(folder, name));
}

std::vector<std::string> Catalogue::names(std::string_view folder) const {
    std::vector<std::string> out;
    if (const KindBase* k = kind_in(folder)) {
        for (std::size_t i = 0; i < k->size(); ++i) {
            out.push_back(k->name(i));
        }
    }
    return out;
}

void Catalogue::fingerprint_sources() {
    for (Source& s : sources_) {
        std::array<num::Digest, 3> digests;
        for (num::Digest& d : digests) {
            d.text(s.id);
        }
        const std::string prefix = s.id + ":";
        for (const auto& k : kinds_) {
            for (std::size_t i = 0; i < k->size(); ++i) {
                if (!k->name(i).starts_with(prefix)) {
                    continue;
                }
                const EntryDigests e = k->digests(i);
                for (std::size_t c = 0; c < digests.size(); ++c) {
                    // every entry counts in the rules, since any entry can change what happens; only entries with
                    // fields that make the land or the look count in those, so a new pot is no new world (PLT-09)
                    if (c != static_cast<std::size_t>(Affects::rules) && e.fields[c] == 0) {
                        continue;
                    }
                    digests[c].text(k->folder());
                    digests[c].text(k->name(i));
                    digests[c].u64(e.by_affects[c]);
                }
            }
        }
        for (std::size_t c = 0; c < digests.size(); ++c) {
            s.digests[c] = digests[c].value();
        }
    }
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
    load_sources(sorted, problems);
    renames_.clear();
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
        if (std::none_of(sources_.begin(), sources_.end(), [&](const Source& s) { return s.id == source; })) {
            problem(path, "the source \"" + source + "\" has no source.toml that loads, so its files are not read");
            continue;
        }
        if (rest == "renames") {
            load_renames(*f, source, problems);
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
        std::string name = joined(source, ':', entry);
        if (!valid_name(entry)) {
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
        kind->add(std::move(name), path, parsed.root, problems);
    }
    for (const auto& k : kinds_) {
        k->sort();
    }
    // a rename leads from a name no entry has to one an entry has, so an old save always finds its entry
    for (const Rename& r : renames_) {
        const KindBase* k = kind_in(r.folder);
        const std::string file = r.from.substr(0, r.from.find(':')) + "/renames.toml";
        if (k->find(r.from)) {
            problem(file, "\"" + r.from + "\" is renamed to \"" + r.to + "\" but is still an entry of its own");
        }
        if (!k->find(r.to)) {
            problem(file, "\"" + r.from + "\" is renamed to \"" + r.to + "\", which is no entry");
        }
    }
    // a bare name in a link is the base source's (A3.6)
    const Lookup lookup = [this](std::string_view folder, std::string_view written, std::string_view /*source*/,
                                 std::string& canonical, std::uint32_t& index) {
        const KindBase* k = kind_in(folder);
        if (k == nullptr) {
            return false;
        }
        const std::string full =
            written.find(':') == std::string_view::npos ? joined("base", ':', written) : std::string(written);
        canonical = std::string(renamed(folder, full));
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
    fingerprint_sources();
    return problems;
}

}  // namespace kd::data
