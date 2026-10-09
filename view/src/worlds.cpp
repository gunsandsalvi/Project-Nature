#include "worlds.hpp"

#include <sys/stat.h>
#include <sys/statvfs.h>

#include <godot_cpp/core/class_db.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <system_error>
#include <vector>

#include "kd/demo/kept.hpp"
#include "kd/save/snapshot.hpp"
#include "kd/save/versions.hpp"
#include "world.hpp"

namespace kd::view {

namespace {

namespace fs = std::filesystem;

// The file under the root naming the world the Crowd page opens, and the folder an import is read into until it is
// whole, hidden from the list since its name is no world's id.
const std::string kCurrent = "current";
const std::string kImporting = ".importing";

godot::String text_of(const std::string& s) {
    return godot::String::utf8(s.c_str());
}

std::string utf8(const godot::String& s) {
    return s.utf8().get_data();
}

std::optional<std::string> read_text(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::nullopt;
    }
    std::stringstream text;
    text << in.rdbuf();
    return text.str();
}

// A world's id is its folder's name: letters, digits and dashes, never a path out of the root.
bool good_id(const std::string& id) {
    return !id.empty() && id.size() <= 64 && std::all_of(id.begin(), id.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
    });
}

// The entries directly in a folder, or under it, read without exceptions, which the app is built without.
template <typename Iterator>
std::vector<fs::directory_entry> entries(const fs::path& folder) {
    std::vector<fs::directory_entry> out;
    std::error_code error;
    for (Iterator it(folder, error); !error && it != Iterator(); it.increment(error)) {
        out.push_back(*it);
    }
    return out;
}

// The bytes a file takes, or the files under a folder.
std::int64_t size_of(const fs::path& path) {
    std::error_code error;
    if (fs::is_regular_file(path, error)) {
        const std::uintmax_t n = fs::file_size(path, error);
        return error ? 0 : static_cast<std::int64_t>(n);
    }
    std::int64_t total = 0;
    for (const fs::directory_entry& item : entries<fs::recursive_directory_iterator>(path)) {
        if (item.is_regular_file(error)) {
            total += size_of(item.path());
        }
    }
    return total;
}

}  // namespace

void KdWorlds::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("set_root", "root"), &KdWorlds::set_root);
    ClassDB::bind_method(D_METHOD("list"), &KdWorlds::list);
    ClassDB::bind_method(D_METHOD("make", "name", "seed", "camps"), &KdWorlds::make);
    ClassDB::bind_method(D_METHOD("make_discovery", "name", "seed"), &KdWorlds::make_discovery);
    ClassDB::bind_method(D_METHOD("make_camp", "name", "seed"), &KdWorlds::make_camp);
    ClassDB::bind_method(D_METHOD("rename", "id", "name"), &KdWorlds::rename);
    ClassDB::bind_method(D_METHOD("remove", "id"), &KdWorlds::remove);
    ClassDB::bind_method(D_METHOD("current"), &KdWorlds::current);
    ClassDB::bind_method(D_METHOD("set_current", "id"), &KdWorlds::set_current);
    ClassDB::bind_method(D_METHOD("free_space"), &KdWorlds::free_space);
    ClassDB::bind_method(D_METHOD("export_begin", "id"), &KdWorlds::export_begin);
    ClassDB::bind_method(D_METHOD("export_next", "most"), &KdWorlds::export_next);
    ClassDB::bind_method(D_METHOD("import_begin"), &KdWorlds::import_begin);
    ClassDB::bind_method(D_METHOD("import_feed", "piece"), &KdWorlds::import_feed);
    ClassDB::bind_method(D_METHOD("import_finish"), &KdWorlds::import_finish);
}

void KdWorlds::set_root(const godot::String& root) {
    root_ = utf8(root);
    std::error_code error;
    fs::create_directories(root_, error);
}

std::string KdWorlds::folder(const godot::String& id) const {
    return (fs::path(root_) / utf8(id)).string();
}

godot::Array KdWorlds::list() const {
    std::vector<godot::Dictionary> worlds;
    std::error_code error;
    for (const fs::directory_entry& item : entries<fs::directory_iterator>(root_)) {
        const std::string id = item.path().filename().string();
        if (!item.is_directory(error) || !good_id(id)) {
            continue;
        }
        godot::Dictionary w;
        w["id"] = text_of(id);
        const std::optional<std::string> about_text = read_text(item.path() / "world.toml");
        const std::optional<demo::About> about = about_text ? demo::read_about(*about_text) : std::nullopt;
        w["name"] = text_of(about ? about->name : std::string());
        w["discovery"] = about && about->discovery;
        w["kind"] = about && about->camp_alpha ? "camp_alpha" : "crowd";
        w["seed"] = about ? static_cast<int64_t>(about->seed) : int64_t{0};
        // a test's world, marked as one with the switches it ran with (RES-10, PLT-05)
        w["test"] = about && about->test;
        godot::PackedStringArray switches;
        for (const std::string& s : about ? about->switches : std::vector<std::string>{}) {
            switches.append(text_of(s));
        }
        w["switches"] = switches;
        // the newest snapshot: its moment, when it was saved, and the version that saved it
        std::vector<std::string> snapshots;
        for (const fs::directory_entry& s : entries<fs::directory_iterator>(item.path() / "snapshots")) {
            const std::string name = s.path().filename().string();
            if (name.size() > 4 && name.compare(name.size() - 4, 4, ".kds") == 0) {
                snapshots.push_back(name);
            }
        }
        std::stable_sort(snapshots.begin(), snapshots.end());
        w["moment"] = int64_t{-1};
        if (!snapshots.empty()) {
            const fs::path newest = item.path() / "snapshots" / snapshots.back();
            const time::Seconds at = std::strtoll(snapshots.back().c_str(), nullptr, 10);
            w["moment"] = at;
            w["moment_text"] = KdWorld::moment_text(at);
            struct stat st {};
            w["saved"] = stat(newest.c_str(), &st) == 0 ? static_cast<int64_t>(st.st_mtime) : int64_t{0};
            std::string why;
            const std::optional<std::string> bytes = read_text(newest);
            if (bytes) {
                const save::Bytes raw(reinterpret_cast<const std::byte*>(bytes->data()),
                                      reinterpret_cast<const std::byte*>(bytes->data()) + bytes->size());
                if (const auto chunks = save::read_snapshot(raw, why)) {
                    const save::Chunk* v = save::find_chunk(*chunks, save::kVersionsTag);
                    const std::optional<save::Versions> versions = v ? save::read_versions(*v) : std::nullopt;
                    // only α1.4a saved snapshots before they recorded their versions
                    w["saved_by"] = text_of(versions ? versions->build : std::string("α1.4a"));
                }
            }
        }
        w["snapshots"] = size_of(item.path() / "snapshots");
        w["journal"] = size_of(item.path() / "journal.log");
        w["history"] = size_of(item.path() / "history");
        w["previous"] = size_of(item.path() / "previous");
        w["size"] = size_of(item.path());
        worlds.push_back(w);
    }
    std::stable_sort(worlds.begin(), worlds.end(), [](const godot::Dictionary& a, const godot::Dictionary& b) {
        return godot::String(a["name"]).naturalnocasecmp_to(godot::String(b["name"])) < 0;
    });
    godot::Array out;
    for (const godot::Dictionary& w : worlds) {
        out.append(w);
    }
    return out;
}

std::string KdWorlds::new_id() const {
    // one more than the highest world-N there is
    std::int64_t highest = 0;
    for (const fs::directory_entry& item : entries<fs::directory_iterator>(root_)) {
        const std::string id = item.path().filename().string();
        if (id.rfind("world-", 0) == 0) {
            highest = std::max<std::int64_t>(highest, std::strtoll(id.c_str() + 6, nullptr, 10));
        }
    }
    return "world-" + std::to_string(highest + 1);
}

godot::String KdWorlds::make(const godot::String& name, int64_t seed, int64_t camps) {
    return make_saved(name, seed, camps, false);
}

godot::String KdWorlds::make_discovery(const godot::String& name, int64_t seed) {
    return make_saved(name, seed, 1, true, true);
}

godot::String KdWorlds::make_camp(const godot::String& name, int64_t seed) {
    return make_saved(name, seed, 1, true);
}

godot::String KdWorlds::make_saved(const godot::String& name, int64_t seed, int64_t camps, bool camp_alpha,
                                   bool discovery) {
    const std::string id = new_id();
    std::error_code error;
    fs::create_directories(fs::path(root_) / id, error);
    if (error) {
        return {};
    }
    demo::About about;
    about.name = utf8(name);
    about.seed = static_cast<std::uint64_t>(std::max<int64_t>(seed, 0));
    about.camps = std::max<int64_t>(camps, 0);
    about.camp_alpha = camp_alpha;
    about.discovery = discovery;
    const std::string text = demo::about_text(about);
    save::DiskFiles files((fs::path(root_) / id).string());
    if (!files.write_whole("world.toml", save::Bytes(reinterpret_cast<const std::byte*>(text.data()),
                                                     reinterpret_cast<const std::byte*>(text.data()) + text.size()))) {
        return {};
    }
    return text_of(id);
}

bool KdWorlds::rename(const godot::String& id, const godot::String& name) {
    if (!good_id(utf8(id))) {
        return false;
    }
    const std::optional<std::string> was = read_text(fs::path(folder(id)) / "world.toml");
    std::optional<demo::About> about = was ? demo::read_about(*was) : std::nullopt;
    if (!about) {
        return false;
    }
    about->name = utf8(name);
    const std::string text = demo::about_text(*about);
    save::DiskFiles files(folder(id));
    return files.write_whole("world.toml", save::Bytes(reinterpret_cast<const std::byte*>(text.data()),
                                                       reinterpret_cast<const std::byte*>(text.data()) + text.size()));
}

bool KdWorlds::remove(const godot::String& id) {
    if (!good_id(utf8(id))) {
        return false;
    }
    std::error_code error;
    fs::remove_all(folder(id), error);
    if (utf8(current()) == utf8(id)) {
        fs::remove(fs::path(root_) / kCurrent, error);
    }
    return !error;
}

godot::String KdWorlds::current() const {
    const std::optional<std::string> id = read_text(fs::path(root_) / kCurrent);
    return id && good_id(*id) ? text_of(*id) : godot::String();
}

void KdWorlds::set_current(const godot::String& id) {
    if (!good_id(utf8(id))) {
        return;
    }
    const std::string text = utf8(id);
    save::DiskFiles files(root_);
    files.write_whole(kCurrent, save::Bytes(reinterpret_cast<const std::byte*>(text.data()),
                                            reinterpret_cast<const std::byte*>(text.data()) + text.size()));
}

int64_t KdWorlds::free_space() const {
    struct statvfs s {};
    if (statvfs(root_.c_str(), &s) != 0) {
        return -1;
    }
    return static_cast<int64_t>(s.f_bavail) * static_cast<int64_t>(s.f_frsize);
}

int64_t KdWorlds::export_begin(const godot::String& id) {
    writer_.reset();
    export_files_.reset();
    std::error_code error;
    if (!good_id(utf8(id)) || !fs::is_directory(folder(id), error)) {
        return -1;
    }
    export_files_ = std::make_unique<save::DiskFiles>(folder(id));
    writer_ = std::make_unique<save::ArchiveWriter>(*export_files_);
    std::int64_t total = 0;
    for (const std::string& part : writer_->parts()) {
        total += size_of(fs::path(folder(id)) / part);
    }
    return total;
}

godot::PackedByteArray KdWorlds::export_next(int64_t most) {
    godot::PackedByteArray out;
    if (!writer_) {
        return out;
    }
    const save::Bytes piece = writer_->next(static_cast<std::size_t>(std::max<int64_t>(most, 1)));
    if (piece.empty()) {
        writer_.reset();
        export_files_.reset();
        return out;
    }
    out.resize(static_cast<int64_t>(piece.size()));
    std::copy(piece.begin(), piece.end(), reinterpret_cast<std::byte*>(out.ptrw()));
    return out;
}

bool KdWorlds::import_begin() {
    reader_.reset();
    import_files_.reset();
    // an import cut off before, as by a kill, leaves nothing that counts
    const fs::path where = fs::path(root_) / kImporting;
    std::error_code error;
    fs::remove_all(where, error);
    fs::create_directories(where, error);
    if (error) {
        return false;
    }
    import_files_ = std::make_unique<save::DiskFiles>(where.string());
    reader_ = std::make_unique<save::ArchiveReader>(*import_files_);
    return true;
}

bool KdWorlds::import_feed(const godot::PackedByteArray& piece) {
    if (!reader_) {
        return false;
    }
    const auto* bytes = reinterpret_cast<const std::byte*>(piece.ptr());
    return reader_->feed(std::span<const std::byte>(bytes, static_cast<std::size_t>(piece.size())));
}

godot::Dictionary KdWorlds::import_finish() {
    godot::Dictionary out;
    if (!reader_) {
        out["why"] = godot::String("no file was being imported");
        return out;
    }
    const bool taken = reader_->finish();
    const std::string why = reader_->why();
    reader_.reset();
    import_files_.reset();
    std::error_code error;
    const fs::path importing = fs::path(root_) / kImporting;
    if (!taken) {
        // a refused file leaves nothing behind
        fs::remove_all(importing, error);
        out["why"] = text_of(why);
        return out;
    }
    // whole and right: it becomes a world, under a new id, its name made safe with it
    const std::string id = new_id();
    const fs::path where = fs::path(root_) / id;
    save::DiskFiles root(root_);
    if (!root.rename(kImporting, id)) {
        fs::remove_all(importing, error);
        out["why"] = godot::String("it could not be kept on the phone");
        return out;
    }
    // the copy is told apart from the world it came from by its name
    const std::optional<std::string> was = read_text(where / "world.toml");
    std::optional<demo::About> about = was ? demo::read_about(*was) : std::nullopt;
    if (about) {
        about->name = (about->name.empty() ? std::string("World") : about->name) + " (imported)";
        rename(text_of(id), text_of(about->name));
    }
    out["id"] = text_of(id);
    out["name"] = text_of(about ? about->name : std::string());
    return out;
}

}  // namespace kd::view
