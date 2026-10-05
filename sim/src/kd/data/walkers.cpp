#include "kd/data/walkers.hpp"

#include <utility>

namespace kd::data {

namespace {

constexpr std::array<std::string_view, 3> kAffects = {"rules", "world", "look"};

std::string unit_names(Measure m) {
    std::string out;
    for (const Unit& u : units_of(m)) {
        if (!u.name.empty()) {
            out += (out.empty() ? "" : ", ") + std::string(u.name);
        }
    }
    return out;
}

}  // namespace

num::Digest& Fingerprinter::put(const Field& f) {
    num::Digest& d = by_affects_[static_cast<std::size_t>(f.affects)];
    d.text(f.key);
    return d;
}

EntryDigests Fingerprinter::digests() {
    EntryDigests e;
    num::Digest combined;
    for (std::size_t i = 0; i < by_affects_.size(); ++i) {
        e.by_affects[i] = by_affects_[i].value();
        combined.u64(e.by_affects[i]);
    }
    combined.u64(all_.value());
    e.all = combined.value();
    return e;
}

std::string SchemaWriter::span(Range range, std::string_view unit) {
    if (range.lowest == INT64_MIN && range.highest == INT64_MAX) {
        return {};
    }
    const std::string u = unit.empty() ? std::string() : " " + std::string(unit);
    return ", from " + std::to_string(range.lowest) + u + " to " + std::to_string(range.highest) + u;
}

void SchemaWriter::line(const Field& f, const std::string& what) {
    text_ += "  " + std::string(f.key) + ": " + what + "; " + (f.required ? "required" : "optional") + "; affects " +
             std::string(kAffects[static_cast<std::size_t>(f.affects)]) + ". " + std::string(f.about) + "\n";
}

void SchemaWriter::choice(const Field& f, const std::string& /*v*/, std::initializer_list<std::string_view> options) {
    std::string list;
    for (std::string_view o : options) {
        list += (list.empty() ? "\"" : ", \"") + std::string(o) + "\"";
    }
    line(f, "one of " + list);
}

void SchemaWriter::quantity(const Field& f, const std::int64_t& /*v*/, Measure m, Range range) {
    line(f, std::string(measure_name(m)) + ", written in " + unit_names(m) + span(range, units_of(m)[0].name));
}

void Display::quantity(const Field& f, const std::int64_t& v, Measure m, Range /*range*/) {
    const std::string_view unit = units_of(m)[0].name;
    line(f, std::to_string(v) + (unit.empty() ? "" : " " + std::string(unit)));
}

void Display::chance(const Field& f, const num::Probability& v) {
    if (v.certain()) {
        line(f, "certain");
        return;
    }
    // in whole parts per million, to the nearest, from the exact threshold
    using Wide = unsigned __int128;
    const auto ppm =
        static_cast<std::uint64_t>((static_cast<Wide>(v.threshold()) * 1'000'000 + (Wide{1} << 63U)) >> 64U);
    line(f, std::to_string(ppm) + " ppm (threshold " + std::to_string(v.threshold()) + " of 2^64)");
}

void Display::links(const Field& f, const std::vector<Ref>& v, std::string_view /*kind*/) {
    std::string list;
    for (const Ref& r : v) {
        list += (list.empty() ? "" : ", ") + r.name;
    }
    line(f, "[" + list + "]");
}

void Resolver::link(const Field& f, Ref& v, std::string_view kind) {
    std::string canonical;
    std::uint32_t index = 0;
    if (!lookup_(kind, v.name, source_, canonical, index)) {
        problems_.push_back({file_, v.line, v.column,
                             std::string(f.key) + ": \"" + v.name + "\" names no entry of kind " + std::string(kind)});
        return;
    }
    v.name = std::move(canonical);
    v.index = index;
}

}  // namespace kd::data
