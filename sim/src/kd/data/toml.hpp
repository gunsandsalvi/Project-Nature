// Reading TOML (A3.6): toml++ lives behind this one file, built with no exceptions, and the rest of the simulation
// reads only this small tree, whose every value keeps its line and column. Floats and dates are refused where they
// are written: quantities are text with units, read exactly (kd/data/units.hpp), so the phone's reading can never
// differ from the cloud's.
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace kd::data {

/// Something wrong in a data file, where it is: every reader and loader returns these rather than stopping.
struct Problem {
    std::string file;
    int line = 0;
    int column = 0;
    std::string what;
};

/// "base/marker/walker.toml:3:9: what is wrong".
std::string problem_text(const Problem& p);

/// A TOML value as the simulation reads it: a table, an array, a text, a whole number or a truth value, with the
/// line and column it starts at.
struct Value {
    enum class Kind : std::uint8_t { table, array, text, whole, truth };

    Kind kind = Kind::table;
    int line = 0;
    int column = 0;
    /// The key the value sits under in its table; empty in a list and at the top.
    std::string key;
    std::string text;
    std::int64_t whole = 0;
    bool truth = false;
    /// A list's values in order, or a table's sorted by their keys, so every walk over them goes in one order.
    std::vector<Value> items;

    /// A table's value under a key, or nothing.
    [[nodiscard]] const Value* find(std::string_view wanted) const;
};

/// The name of a kind of value, for messages: "a table", "a text".
std::string_view kind_name(Value::Kind kind);

/// A file read: its tree, and what is wrong with it, empty when nothing is.
struct Parsed {
    Value root;
    std::vector<Problem> problems;
};

/// Reads TOML 1.0 text. A syntax error stops the reading; every float, date and time is a problem of its own.
/// Implements MAT-13, see A3.6.
Parsed parse_toml(std::string_view text, const std::string& file);

}  // namespace kd::data
