#include "kd/scene/report.hpp"

#include <algorithm>
#include <cstdio>
#include <optional>
#include <vector>

#include "kd/num/digest.hpp"

namespace kd::scene {

namespace {

// A text as a JSON string, its quotes, backslashes and control characters escaped.
std::string quoted(std::string_view text) {
    std::string out = "\"";
    for (const char c : text) {
        const auto u = static_cast<unsigned char>(c);
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (u < 0x20) {
            char escaped[8];
            std::snprintf(escaped, sizeof escaped, "\\u%04x", u);
            out += escaped;
        } else {
            out += c;
        }
    }
    return out + "\"";
}

std::string texts(const std::vector<std::string>& list) {
    std::string out = "[";
    for (std::size_t i = 0; i < list.size(); ++i) {
        out += (i == 0 ? "" : ", ") + quoted(list[i]);
    }
    return out + "]";
}

std::vector<std::string> switch_names(std::span<const world::Switch> switches) {
    std::vector<std::string> out;
    for (const world::Switch s : switches) {
        out.emplace_back(world::kSwitchNames[static_cast<std::size_t>(s)]);
    }
    return out;
}

}  // namespace

std::string rule_words(const Scene& s) {
    std::string out = s.pass.measure;
    if (s.pass.at_least) {
        out += " at least " + std::to_string(*s.pass.at_least);
    }
    if (s.pass.at_least && s.pass.at_most) {
        out += " and";
    }
    if (s.pass.at_most) {
        out += " at most " + std::to_string(*s.pass.at_most);
    }
    return out + ", in at least " + std::to_string(s.pass.in) + " of " + std::to_string(s.runs) + " runs";
}

std::string report_json(const Scene& s, const WorldKind& kind, std::span<const RunResult> runs,
                        const Outcome& outcome) {
    const Verdict& v = outcome.verdict;
    std::size_t oddities = 0;
    for (const RunResult& r : runs) {
        oddities += r.oddities.size();
    }
    std::string out = "{\n";
    out += "  \"scene\": " + quoted(s.name) + ",\n";
    out += "  \"about\": " + quoted(s.about) + ",\n";
    out += "  \"checks\": " + texts(s.checks) + ",\n";
    out += "  \"world\": " + quoted(s.world) + ",\n";
    out += "  \"camps\": " + std::to_string(s.camps) + ",\n";
    out += "  \"seed\": " + std::to_string(s.seed) + ",\n";
    out += "  \"runs\": " + std::to_string(s.runs) + ",\n";
    out += "  \"until\": " + std::to_string(s.until) + ",\n";
    out += "  \"switches\": " + texts(switch_names(s.switches)) + ",\n";
    out += "  \"one_each\": " + std::string(s.one_each ? "true" : "false") + ",\n";
    out += "  \"rule\": " + quoted(rule_words(s)) + ",\n";
    out += "  \"measure\": " + quoted(s.pass.measure) + ",\n";
    out += "  \"at_least\": " + (s.pass.at_least ? std::to_string(*s.pass.at_least) : std::string("null")) + ",\n";
    out += "  \"at_most\": " + (s.pass.at_most ? std::to_string(*s.pass.at_most) : std::string("null")) + ",\n";
    out += "  \"in\": " + std::to_string(s.pass.in) + ",\n";
    out += "  \"passed\": " + std::string(v.passed ? "true" : "false") + ",\n";
    out += "  \"passes\": " + std::to_string(v.passes) + ",\n";
    out += "  \"judged\": " + std::to_string(v.judged) + ",\n";
    out += "  \"needed\": " + std::to_string(v.needed) + ",\n";
    out += "  \"provisional\": " + std::string(v.provisional ? "true" : "false") + ",\n";
    out += "  \"reran\": " + std::string(v.reran ? "true" : "false") + ",\n";
    out += "  \"oddities\": " + std::to_string(oddities) + ",\n";
    out += "  \"seconds\": " + std::to_string(outcome.seconds) + ",\n";
    out += "  \"budget\": " + std::to_string(s.budget) + ",\n";
    out += "  \"over_budget\": " + std::string(outcome.over_budget ? "true" : "false") + ",\n";
    out += "  \"build\": " + quoted(outcome.build) + ",\n";

    // each measure's range over the runs that gave it, and how many of them its expected range held
    out += "  \"ranges\": [";
    bool first = true;
    for (const Named& m : kind.measures) {
        std::vector<std::int64_t> values;
        for (const RunResult& r : runs) {
            if (const std::optional<std::int64_t> x = r.measure(std::string(m.name))) {
                values.push_back(*x);
            }
        }
        if (values.empty()) {
            continue;
        }
        std::stable_sort(values.begin(), values.end());
        std::int64_t inside = static_cast<std::int64_t>(values.size());
        std::string expected = "null";
        for (const Expect& e : s.expects) {
            if (e.measure == m.name) {
                inside = std::count_if(values.begin(), values.end(),
                                       [&](std::int64_t x) { return x >= e.from && x <= e.to; });
                expected = "[" + std::to_string(e.from) + ", " + std::to_string(e.to) + "]";
            }
        }
        out += std::string(first ? "\n" : ",\n") + "    {\"measure\": " + quoted(m.name) +
               ", \"about\": " + quoted(m.about) + ", \"lowest\": " + std::to_string(values.front()) +
               ", \"median\": " + std::to_string(values[values.size() / 2]) +
               ", \"highest\": " + std::to_string(values.back()) + ", \"runs\": " + std::to_string(values.size()) +
               ", \"expected\": " + expected + ", \"inside\": " + std::to_string(inside) + "}";
        first = false;
    }
    out += "\n  ],\n";

    out += "  \"each\": [";
    for (std::size_t i = 0; i < runs.size(); ++i) {
        const RunResult& r = runs[i];
        out += std::string(i == 0 ? "\n" : ",\n") + "    {\"index\": " + std::to_string(r.index) +
               ", \"seed\": " + std::to_string(r.seed) + ", \"days\": " + std::to_string(r.days) +
               ", \"digest\": " + quoted(num::to_hex(r.digest)) + ", \"switches\": " + texts(switch_names(r.switches)) +
               ", \"measures\": {";
        for (std::size_t j = 0; j < r.measures.size(); ++j) {
            out += (j == 0 ? "" : ", ") + quoted(r.measures[j].first) + ": " + std::to_string(r.measures[j].second);
        }
        out += "}, \"oddities\": " + texts(r.oddities) + "}";
    }
    out += "\n  ]\n}\n";
    return out;
}

}  // namespace kd::scene
