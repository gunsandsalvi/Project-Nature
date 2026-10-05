// The other walkers of a kind's one description (A3.6): the fingerprinter, the schema writer, the display and the
// link resolver. Each takes the same fields the loader does, so a kind is described once and read four more ways.
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/schema.hpp"
#include "kd/data/toml.hpp"
#include "kd/data/units.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/probability.hpp"
#include "kd/time/duration.hpp"

namespace kd::data {

/// An entry's digests: everything in it, and by what its fields affect (A3.6).
struct EntryDigests {
    std::uint64_t all = 0;
    std::array<std::uint64_t, 3> by_affects{};  // rules, world, look
};

/// Implements MAT-14, see A3.6: an entry's digests from its canonical values, field by field in the kind's order,
/// so a change to one entry changes no other's.
class Fingerprinter {
public:
    void whole(const Field& f, const std::int64_t& v, Range /*range*/) { put(f).i64(v); }
    void truth(const Field& f, const bool& v) { put(f).u8(v ? 1 : 0); }
    void text(const Field& f, const std::string& v) { put(f).text(v); }
    void choice(const Field& f, const std::string& v, std::initializer_list<std::string_view> /*options*/) {
        put(f).text(v);
    }
    void quantity(const Field& f, const std::int64_t& v, Measure /*m*/, Range /*range*/) { put(f).i64(v); }
    void chance(const Field& f, const num::Probability& v) {
        num::Digest& d = put(f);
        d.u64(v.threshold());
        d.u8(v.certain() ? 1 : 0);
    }
    /// The game length is what the simulation reads; the length in life only checks it (TIM-18).
    void duration(const Field& f, const time::Duration& v) {
        put(f).i64(v.game);
        all_.i64(v.life);
    }
    void link(const Field& f, const Ref& v, std::string_view /*kind*/) { put(f).text(v.name); }
    void links(const Field& f, const std::vector<Ref>& v, std::string_view /*kind*/) {
        num::Digest& d = put(f);
        d.u64(v.size());
        for (const Ref& r : v) {
            d.text(r.name);
        }
    }
    void names(const Field& f, const std::vector<std::string>& v) {
        num::Digest& d = put(f);
        d.u64(v.size());
        for (const std::string& n : v) {
            d.text(n);
        }
    }

    [[nodiscard]] EntryDigests digests();

private:
    num::Digest& put(const Field& f);

    num::Digest all_;
    std::array<num::Digest, 3> by_affects_;
};

/// Implements MAT-13 and MAT-17, see A3.6: a kind's fields as whoever writes entries needs them, one line each.
class SchemaWriter {
public:
    void whole(const Field& f, const std::int64_t& /*v*/, Range range) { line(f, "a whole number" + span(range, "")); }
    void truth(const Field& f, const bool& /*v*/) { line(f, "true or false"); }
    void text(const Field& f, const std::string& /*v*/) { line(f, "a text"); }
    void choice(const Field& f, const std::string& /*v*/, std::initializer_list<std::string_view> options);
    void quantity(const Field& f, const std::int64_t& /*v*/, Measure m, Range range);
    void chance(const Field& f, const num::Probability& /*v*/) {
        line(f, "a chance, such as \"15%\", \"1 in 100\" or \"0.413\"");
    }
    void duration(const Field& f, const time::Duration& /*v*/) {
        line(f, "a duration with both lengths, such as { life = \"3 month\", game = \"15 d\" }, held to TIM-18");
    }
    void link(const Field& f, const Ref& /*v*/, std::string_view kind) {
        line(f, "the name of a " + std::string(kind));
    }
    void links(const Field& f, const std::vector<Ref>& /*v*/, std::string_view kind) {
        line(f, "a list of names of " + std::string(kind) + " entries");
    }
    void names(const Field& f, const std::vector<std::string>& /*v*/) { line(f, "a list of names, in quotes"); }

    [[nodiscard]] const std::string& text() const { return text_; }

private:
    static std::string span(Range range, std::string_view unit);
    void line(const Field& f, const std::string& what);

    std::string text_;
};

/// An entry's values as the simulation holds them, one "key = value" line each, in base units.
class Display {
public:
    void whole(const Field& f, const std::int64_t& v, Range /*range*/) { line(f, std::to_string(v)); }
    void truth(const Field& f, const bool& v) { line(f, v ? "true" : "false"); }
    void text(const Field& f, const std::string& v) { line(f, "\"" + v + "\""); }
    void choice(const Field& f, const std::string& v, std::initializer_list<std::string_view> /*options*/) {
        line(f, "\"" + v + "\"");
    }
    void quantity(const Field& f, const std::int64_t& v, Measure m, Range /*range*/);
    void chance(const Field& f, const num::Probability& v);
    void duration(const Field& f, const time::Duration& v) {
        line(f, "{ life = " + std::to_string(v.life) + " s, game = " + std::to_string(v.game) + " s }");
    }
    void link(const Field& f, const Ref& v, std::string_view /*kind*/) { line(f, v.name); }
    void links(const Field& f, const std::vector<Ref>& v, std::string_view /*kind*/);
    void names(const Field& f, const std::vector<std::string>& v);

    [[nodiscard]] const std::string& text() const { return text_; }

private:
    void line(const Field& f, const std::string& value) { text_ += std::string(f.key) + " = " + value + "\n"; }

    std::string text_;
};

/// Finds an entry by name for the resolver: the kind's folder, the name as written and the source it was written
/// in, giving the entry's canonical name and number, or an empty name if there is none.
using Lookup = std::function<bool(std::string_view kind, std::string_view written, std::string_view source,
                                  std::string& canonical, std::uint32_t& index)>;

/// Implements MAT-13, see A3.6: turns each link's written name into its entry's canonical name and number, once
/// every entry is loaded; a link to nothing is a problem at the link.
class Resolver {
public:
    Resolver(const Lookup& lookup, std::string source, const std::string& file, std::vector<Problem>& problems)
        : lookup_(lookup), source_(std::move(source)), file_(file), problems_(problems) {}

    // only links are its business
    void whole(const Field& /*f*/, std::int64_t& /*v*/, Range /*range*/) {}
    void truth(const Field& /*f*/, bool& /*v*/) {}
    void text(const Field& /*f*/, std::string& /*v*/) {}
    void choice(const Field& /*f*/, std::string& /*v*/, std::initializer_list<std::string_view> /*options*/) {}
    void quantity(const Field& /*f*/, std::int64_t& /*v*/, Measure /*m*/, Range /*range*/) {}
    void chance(const Field& /*f*/, num::Probability& /*v*/) {}
    void duration(const Field& /*f*/, time::Duration& /*v*/) {}
    void names(const Field& /*f*/, std::vector<std::string>& /*v*/) {}
    void link(const Field& f, Ref& v, std::string_view kind);
    void links(const Field& f, std::vector<Ref>& v, std::string_view kind) {
        for (Ref& r : v) {
            link(f, r, kind);
        }
    }

private:
    const Lookup& lookup_;
    std::string source_;
    const std::string& file_;
    std::vector<Problem>& problems_;
};

}  // namespace kd::data
