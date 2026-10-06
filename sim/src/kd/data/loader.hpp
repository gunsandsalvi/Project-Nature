// The loader (A3.6): reads one entry's table into its kind's struct through the kind's visit(), checking each
// field's type, unit and range, refusing keys no field names, and naming every problem by file, line and column.
#pragma once

#include <cstdint>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "kd/data/schema.hpp"
#include "kd/data/toml.hpp"
#include "kd/data/units.hpp"
#include "kd/num/probability.hpp"
#include "kd/time/duration.hpp"

namespace kd::data {

/// Where a field was written in its file, kept with the entry so a check on the whole catalogue can name the place.
struct Mark {
    std::string key;
    int line = 0;
    int column = 0;
};

/// Implements MAT-13 and MAT-17, see A3.6: one entry's values, read and checked.
class Loader {
public:
    Loader(const Value& table, const std::string& file, std::vector<Problem>& problems)
        : table_(table), file_(file), problems_(problems) {}

    void whole(const Field& f, std::int64_t& out, Range range);
    void truth(const Field& f, bool& out);
    void text(const Field& f, std::string& out);
    void choice(const Field& f, std::string& out, std::initializer_list<std::string_view> options);
    void choice(const Field& f, std::string& out, std::span<const std::string_view> options);
    void quantity(const Field& f, std::int64_t& out, Measure measure, Range range);
    void chance(const Field& f, num::Probability& out);
    /// A table { life = "3 month", game = "15 d" }, held to TIM-18's rule.
    void duration(const Field& f, time::Duration& out);
    void link(const Field& f, Ref& out, std::string_view kind);
    void links(const Field& f, std::vector<Ref>& out, std::string_view kind);
    /// A list of plain names, such as the sources a source requires.
    void names(const Field& f, std::vector<std::string>& out);
    /// A table of its own, such as a scene's pass rule, for a loader of its own; null when it is missing or wrong.
    const Value* table(const Field& f);
    /// A list of tables, such as a scene's expected ranges, each for a loader of its own; empty when missing.
    std::vector<const Value*> tables(const Field& f);

    /// After the visit: each key no field named is a problem.
    void finish();
    /// A problem found by whoever reads the table, beyond what each field holds, named at a value's place.
    void refuse(const Value& at, std::string what) { problem(at, std::move(what)); }
    /// The table read.
    [[nodiscard]] const Value& table() const { return table_; }

    /// Where each field read was written, in the order of the kind's fields.
    [[nodiscard]] std::vector<Mark> marks() const { return marks_; }

private:
    const Value* take(const Field& f, Value::Kind kind);
    void problem(const Value& at, std::string what);
    void problem(const Value& at, const Field& f, std::string what);
    void in_range(const Value& at, const Field& f, std::int64_t value, Range range, Measure* measure);

    const Value& table_;
    const std::string& file_;
    std::vector<Problem>& problems_;
    std::vector<std::string_view> named_;
    std::vector<Mark> marks_;
};

}  // namespace kd::data
