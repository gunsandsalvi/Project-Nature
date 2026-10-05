#include "kd/data/toml.hpp"

#include <algorithm>

// toml++ is used in this file only (A3.6): no exceptions, no writing, no SIMD.
#define TOML_EXCEPTIONS 0
#define TOML_ENABLE_FORMATTERS 0
#define TOML_ENABLE_SIMD 0
#include <toml.hpp>

namespace kd::data {

namespace {

struct Converter {
    const std::string& file;
    std::vector<Problem> problems;

    void problem(const toml::source_region& at, std::string what) {
        problems.push_back({file, static_cast<int>(at.begin.line), static_cast<int>(at.begin.column), std::move(what)});
    }

    Value convert(const toml::node& node) {
        Value v;
        v.line = static_cast<int>(node.source().begin.line);
        v.column = static_cast<int>(node.source().begin.column);
        switch (node.type()) {
            case toml::node_type::table:
                v.kind = Value::Kind::table;
                for (const auto& [key, child] : *node.as_table()) {
                    v.items.push_back(convert(child));
                    v.items.back().key = std::string(key.str());
                }
                std::stable_sort(v.items.begin(), v.items.end(),
                                 [](const Value& a, const Value& b) { return a.key < b.key; });
                break;
            case toml::node_type::array:
                v.kind = Value::Kind::array;
                for (const toml::node& child : *node.as_array()) {
                    v.items.push_back(convert(child));
                }
                break;
            case toml::node_type::string:
                v.kind = Value::Kind::text;
                v.text = node.as_string()->get();
                break;
            case toml::node_type::integer:
                v.kind = Value::Kind::whole;
                v.whole = node.as_integer()->get();
                break;
            case toml::node_type::boolean:
                v.kind = Value::Kind::truth;
                v.truth = node.as_boolean()->get();
                break;
            case toml::node_type::floating_point:
                problem(node.source(),
                        "a bare decimal number, which is never read: write a quantity as text with its unit, such "
                        "as \"3.5 kg\", or a whole number");
                break;
            default:
                problem(node.source(),
                        "a date or a time, which is never read: write a duration as text, such as \"1 h 30 min\"");
                break;
        }
        return v;
    }
};

}  // namespace

std::string problem_text(const Problem& p) {
    return p.file + ":" + std::to_string(p.line) + ":" + std::to_string(p.column) + ": " + p.what;
}

const Value* Value::find(std::string_view wanted) const {
    if (kind != Kind::table) {
        return nullptr;
    }
    const auto at = std::lower_bound(items.begin(), items.end(), wanted,
                                     [](const Value& v, std::string_view k) { return v.key < k; });
    return at != items.end() && at->key == wanted ? &*at : nullptr;
}

std::string_view kind_name(Value::Kind kind) {
    switch (kind) {
        case Value::Kind::table:
            return "a table";
        case Value::Kind::array:
            return "a list";
        case Value::Kind::text:
            return "a text";
        case Value::Kind::whole:
            return "a whole number";
        case Value::Kind::truth:
            return "true or false";
    }
    return "a value";
}

Parsed parse_toml(std::string_view text, const std::string& file) {
    Parsed out;
    toml::parse_result result = toml::parse(text, file);
    if (!result) {
        const toml::parse_error& error = result.error();
        out.problems.push_back({file, static_cast<int>(error.source().begin.line),
                                static_cast<int>(error.source().begin.column),
                                "the TOML is broken here: " + std::string(error.description())});
        return out;
    }
    Converter converter{file, {}};
    out.root = converter.convert(result.table());
    out.problems = std::move(converter.problems);
    return out;
}

}  // namespace kd::data
