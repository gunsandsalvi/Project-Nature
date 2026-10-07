// The catalogue (A3.6): every kind of entry the game knows, and the entries its sources hold, loaded from TOML files
// handed to it as text, checked, numbered by sorted name and fingerprinted. The simulation reads files only as bytes
// handed to it (A3.1), so the same catalogue loads in the cloud's tools and on the phone.
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "kd/chance/chance.hpp"
#include "kd/core/check.hpp"
#include "kd/data/loader.hpp"
#include "kd/data/schema.hpp"
#include "kd/data/toml.hpp"
#include "kd/data/walkers.hpp"

namespace kd::data {

/// A file for the catalogue: its path under the data folder, "demo/marker/walker.toml", and its text.
struct SourceFile {
    std::string path;
    std::string text;
};

/// The version of the code that makes worlds, raised by hand whenever that code changes, so a world made by an
/// older one is known for what it is (A3.6, PLT-09); the golden worlds' test guards it once worlds are made (M3).
inline constexpr std::int64_t kWorldMakingVersion = 1;

/// A source of entries (A3.6), from its source.toml: its id, the same as its folder's; its version; what it holds;
/// the sources it needs, loaded before it; and once loaded, its digests of what affects the rules, the world and the
/// look. Implements MAT-14, see A3.6.
struct Source {
    std::string id;
    std::int64_t version = 0;
    std::string about;
    std::vector<std::string> needs;
    std::array<std::uint64_t, 3> digests{};  // rules, world, look

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        v.text({"id", "the source's name, the same as its folder's"}, s.id);
        v.whole({"version", "raised with every change to the source"}, s.version, {1, INT64_MAX});
        v.text({"about", "what the source holds, in a few words", Affects::look}, s.about);
        v.names({"requires", "the sources it needs, loaded before it", Affects::rules, false}, s.needs);
    }
};

/// One entry: its canonical name, "demo:walker", the file it came from, its values, and the stable hash of its name
/// that keys its chance and breaks its ties, which no other entry's arrival can move, as its number can (A3.6).
template <typename T>
struct Entry {
    std::string name;
    std::string file;
    T value;
    chance::Name key;
    std::vector<Mark> marks;
};

/// Where a kind's entries live in a source: a file each in its folder, `<folder>/<name>.toml`; one entry, the file
/// `<folder>.toml`; or a folder each holding its record, `<folder>/<name>/record.toml`, as the art lane's textures are
/// laid out (A5.4), a name such as "meadow/middle/v2" taking a part a folder.
enum class Layout : std::uint8_t { files, single, records };

/// What every kind does, whatever its fields.
class KindBase {
public:
    KindBase(std::string folder, std::string about, Layout layout)
        : folder_(std::move(folder)), about_(std::move(about)), layout_(layout) {}
    virtual ~KindBase() = default;
    KindBase(const KindBase&) = delete;
    KindBase& operator=(const KindBase&) = delete;

    /// Where its files live in a source: "marker", or for a tuning file, "tuning/time".
    [[nodiscard]] const std::string& folder() const { return folder_; }
    [[nodiscard]] const std::string& about() const { return about_; }
    /// One entry, the file <source>/<folder>.toml, rather than a folder of them.
    [[nodiscard]] bool single() const { return layout_ == Layout::single; }
    [[nodiscard]] Layout layout() const { return layout_; }

    virtual void add(std::string name, const std::string& file, const Value& table, std::vector<Problem>& problems) = 0;
    virtual void sort() = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual const std::string& name(std::size_t i) const = 0;
    [[nodiscard]] virtual const std::string& file(std::size_t i) const = 0;
    /// The stable hash of the entry's name, for chance and tie-breaks; its number is only for arrays.
    /// Implements TIM-16 and MAT-14, see A3.6.
    [[nodiscard]] virtual chance::Name key(std::size_t i) const = 0;
    /// The number of the entry with this canonical name.
    [[nodiscard]] virtual std::optional<std::uint32_t> find(std::string_view canonical) const = 0;
    virtual void resolve(const Lookup& lookup, std::vector<Problem>& problems) = 0;
    [[nodiscard]] virtual EntryDigests digests(std::size_t i) const = 0;
    /// A problem at one of the entry's fields, where it was written, or at the entry's start if it is not written.
    [[nodiscard]] virtual Problem at(std::size_t i, std::string_view key, std::string what) const = 0;
    /// One field of the entry, for MAT-05's orders.
    [[nodiscard]] virtual Picked pick(std::size_t i, std::string_view key) const = 0;
    /// The entry's values, field by field, for the screen (A3.8).
    [[nodiscard]] virtual std::vector<FieldValue> values(std::size_t i) const = 0;
    [[nodiscard]] virtual std::string display(std::size_t i) const = 0;
    [[nodiscard]] virtual std::string schema() const = 0;
    [[nodiscard]] virtual const void* type() const = 0;

private:
    std::string folder_;
    std::string about_;
    Layout layout_;
};

/// A kind of entry, T being a struct with the one visit() that describes its fields (kd/data/schema.hpp).
/// Implements MAT-13 and MAT-17, see A3.6.
template <typename T>
class Kind final : public KindBase {
public:
    using KindBase::KindBase;

    void add(std::string name, const std::string& file, const Value& table, std::vector<Problem>& problems) override {
        const chance::Name key = chance::name(name);
        Entry<T> e{std::move(name), file, T{}, key, {}};
        Loader loader(table, file, problems);
        T::visit(loader, e.value);
        loader.finish();
        e.marks = loader.marks();
        entries_.push_back(std::move(e));
    }

    void sort() override {
        std::stable_sort(entries_.begin(), entries_.end(),
                         [](const Entry<T>& a, const Entry<T>& b) { return a.name < b.name; });
    }

    [[nodiscard]] std::size_t size() const override { return entries_.size(); }
    [[nodiscard]] const std::string& name(std::size_t i) const override { return entries_[i].name; }
    [[nodiscard]] const std::string& file(std::size_t i) const override { return entries_[i].file; }
    [[nodiscard]] chance::Name key(std::size_t i) const override { return entries_[i].key; }

    [[nodiscard]] std::optional<std::uint32_t> find(std::string_view canonical) const override {
        const auto at =
            std::lower_bound(entries_.begin(), entries_.end(), canonical,
                             [](const Entry<T>& e, std::string_view n) { return std::string_view(e.name) < n; });
        if (at == entries_.end() || at->name != canonical) {
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(at - entries_.begin());
    }

    void resolve(const Lookup& lookup, std::vector<Problem>& problems) override {
        for (Entry<T>& e : entries_) {
            Resolver resolver(lookup, std::string(e.name.substr(0, e.name.find(':'))), e.file, problems);
            T::visit(resolver, e.value);
        }
    }

    [[nodiscard]] EntryDigests digests(std::size_t i) const override {
        Fingerprinter f;
        T::visit(f, entries_[i].value);
        return f.digests();
    }

    [[nodiscard]] Problem at(std::size_t i, std::string_view key, std::string what) const override {
        const Entry<T>& e = entries_[i];
        for (const Mark& m : e.marks) {
            if (m.key == key) {
                return {e.file, m.line, m.column, std::string(key) + ": " + std::move(what)};
            }
        }
        return {e.file, 1, 1, std::move(what)};
    }

    [[nodiscard]] Picked pick(std::size_t i, std::string_view key) const override {
        Picker p(key);
        T::visit(p, entries_[i].value);
        return p.picked();
    }

    [[nodiscard]] std::vector<FieldValue> values(std::size_t i) const override {
        Valuer v;
        T::visit(v, entries_[i].value);
        return std::move(v).values();
    }

    [[nodiscard]] std::string display(std::size_t i) const override {
        Display d;
        T::visit(d, entries_[i].value);
        return d.text();
    }

    [[nodiscard]] std::string schema() const override {
        SchemaWriter w;
        const T blank{};
        T::visit(w, blank);
        return w.text();
    }

    [[nodiscard]] const void* type() const override { return tag(); }
    static const void* tag() {
        static const char t = 0;
        return &t;
    }

    [[nodiscard]] const std::vector<Entry<T>>& entries() const { return entries_; }
    [[nodiscard]] const T& operator[](std::uint32_t i) const { return entries_[i].value; }

private:
    std::vector<Entry<T>> entries_;
};

/// Implements MAT-13, MAT-14 and MAT-17, see A3.6: the game's catalogue.
class Catalogue {
public:
    /// A catalogue that knows every kind of entry the game has (kd/data/kinds.cpp), with no entries yet.
    Catalogue();

    template <typename T>
    void add_kind(std::string folder, std::string about, Layout layout = Layout::files) {
        kinds_.push_back(std::make_unique<Kind<T>>(std::move(folder), std::move(about), layout));
        std::stable_sort(kinds_.begin(), kinds_.end(),
                         [](const auto& a, const auto& b) { return a->folder() < b->folder(); });
    }

    /// Loads the files of the sources, in any order: every problem found, empty when all is well.
    std::vector<Problem> load(std::span<const SourceFile> files);

    /// The sources, in the order they load: each after those it requires, and otherwise by id.
    [[nodiscard]] std::span<const Source> sources() const { return sources_; }

    /// An entry's number in its kind by its canonical name, following renames, so a save written with an old name
    /// still finds its entry. Implements MAT-13 and MAT-14, see A3.6.
    [[nodiscard]] std::optional<std::uint32_t> find(std::string_view folder, std::string_view name) const;

    /// A kind's canonical names in their order, the list a save keeps (A3.7).
    [[nodiscard]] std::vector<std::string> names(std::string_view folder) const;

    /// The files under each loaded source's checks/, which only the checks read (MAT-17, MAT-05), in path order.
    [[nodiscard]] std::span<const SourceFile> check_files() const { return check_files_; }

    /// The kinds, in the order of their folders' names.
    [[nodiscard]] std::span<const std::unique_ptr<KindBase>> kinds() const { return kinds_; }
    [[nodiscard]] const KindBase* kind_in(std::string_view folder) const;

    /// A kind by its struct, which must be one the catalogue knows.
    template <typename T>
    [[nodiscard]] const Kind<T>& kind() const {
        for (const auto& k : kinds_) {
            if (k->type() == Kind<T>::tag()) {
                return static_cast<const Kind<T>&>(*k);
            }
        }
        kd::fail(__FILE__, __LINE__, "data::Catalogue: no such kind");
    }

private:
    struct Rename {
        std::string folder;
        std::string from;
        std::string to;
    };

    void load_sources(std::span<const SourceFile* const> files, std::vector<Problem>& problems);
    void load_renames(const SourceFile& file, const std::string& source, std::vector<Problem>& problems);
    [[nodiscard]] std::string_view renamed(std::string_view folder, std::string_view name) const;
    void fingerprint_sources();

    std::vector<std::unique_ptr<KindBase>> kinds_;
    std::vector<Source> sources_;
    std::vector<Rename> renames_;
    std::vector<SourceFile> check_files_;
};

}  // namespace kd::data
