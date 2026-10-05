// The catalogue (A3.6): every kind of entry the game knows, and the entries its sources hold, loaded from TOML files
// handed to it as text, checked, numbered by sorted name and fingerprinted. The simulation reads files only as bytes
// handed to it (A3.1), so the same catalogue loads in the cloud's tools and on the phone.
#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

/// One entry: its canonical name, "demo:walker", the file it came from, and its values.
template <typename T>
struct Entry {
    std::string name;
    std::string file;
    T value;
};

/// What every kind does, whatever its fields.
class KindBase {
public:
    KindBase(std::string folder, std::string about, bool single)
        : folder_(std::move(folder)), about_(std::move(about)), single_(single) {}
    virtual ~KindBase() = default;
    KindBase(const KindBase&) = delete;
    KindBase& operator=(const KindBase&) = delete;

    /// Where its files live in a source: "marker", or for a tuning file, "tuning/time".
    [[nodiscard]] const std::string& folder() const { return folder_; }
    [[nodiscard]] const std::string& about() const { return about_; }
    /// One entry, the file <source>/<folder>.toml, rather than a folder of them.
    [[nodiscard]] bool single() const { return single_; }

    virtual void add(std::string name, const std::string& file, const Value& table, std::vector<Problem>& problems) = 0;
    virtual void sort() = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual const std::string& name(std::size_t i) const = 0;
    [[nodiscard]] virtual const std::string& file(std::size_t i) const = 0;
    /// The number of the entry with this canonical name.
    [[nodiscard]] virtual std::optional<std::uint32_t> find(std::string_view canonical) const = 0;
    virtual void resolve(const Lookup& lookup, std::vector<Problem>& problems) = 0;
    [[nodiscard]] virtual EntryDigests digests(std::size_t i) const = 0;
    [[nodiscard]] virtual std::string display(std::size_t i) const = 0;
    [[nodiscard]] virtual std::string schema() const = 0;
    [[nodiscard]] virtual const void* type() const = 0;

private:
    std::string folder_;
    std::string about_;
    bool single_;
};

/// A kind of entry, T being a struct with the one visit() that describes its fields (kd/data/schema.hpp).
/// Implements MAT-13 and MAT-17, see A3.6.
template <typename T>
class Kind final : public KindBase {
public:
    using KindBase::KindBase;

    void add(std::string name, const std::string& file, const Value& table, std::vector<Problem>& problems) override {
        Entry<T> e{std::move(name), file, T{}};
        Loader loader(table, file, problems);
        T::visit(loader, e.value);
        loader.finish();
        entries_.push_back(std::move(e));
    }

    void sort() override {
        std::stable_sort(entries_.begin(), entries_.end(),
                         [](const Entry<T>& a, const Entry<T>& b) { return a.name < b.name; });
    }

    [[nodiscard]] std::size_t size() const override { return entries_.size(); }
    [[nodiscard]] const std::string& name(std::size_t i) const override { return entries_[i].name; }
    [[nodiscard]] const std::string& file(std::size_t i) const override { return entries_[i].file; }

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
    void add_kind(std::string folder, std::string about, bool single = false) {
        kinds_.push_back(std::make_unique<Kind<T>>(std::move(folder), std::move(about), single));
        std::stable_sort(kinds_.begin(), kinds_.end(),
                         [](const auto& a, const auto& b) { return a->folder() < b->folder(); });
    }

    /// Loads the files of the sources, in any order: every problem found, empty when all is well.
    std::vector<Problem> load(std::span<const SourceFile> files);

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
    std::vector<std::unique_ptr<KindBase>> kinds_;
};

}  // namespace kd::data
