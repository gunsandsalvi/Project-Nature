// The loader (A3.6): reads one entry's table into its kind's struct through the kind's visit(), checking each
// field's type, unit and range, refusing keys no field names, and naming every problem by file, line and column.
#pragma once

#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/schema.hpp"
#include "kd/data/toml.hpp"
#include "kd/data/units.hpp"
#include "kd/num/probability.hpp"
#include "kd/time/duration.hpp"

namespace kd::data {

/// Implements MAT-13 and MAT-17, see A3.6: one entry's values, read and checked.
class Loader {
public:
    Loader(const Value& table, const std::string& file, std::vector<Problem>& problems)
        : table_(table), file_(file), problems_(problems) {}

    void whole(const Field& f, std::int64_t& out, Range range);
    void truth(const Field& f, bool& out);
    void text(const Field& f, std::string& out);
    void choice(const Field& f, std::string& out, std::initializer_list<std::string_view> options);
    void quantity(const Field& f, std::int64_t& out, Measure measure, Range range);
    void chance(const Field& f, num::Probability& out);
    /// A table { life = "3 month", game = "15 d" }, held to TIM-18's rule.
    void duration(const Field& f, time::Duration& out);
    void link(const Field& f, Ref& out, std::string_view kind);
    void links(const Field& f, std::vector<Ref>& out, std::string_view kind);
    /// A list of plain names, such as the sources a source requires.
    void names(const Field& f, std::vector<std::string>& out);

    /// After the visit: each key no field named is a problem.
    void finish();

private:
    const Value* take(const Field& f, Value::Kind kind);
    void problem(const Value& at, std::string what);
    void problem(const Value& at, const Field& f, std::string what);
    void in_range(const Value& at, const Field& f, std::int64_t value, Range range, Measure* measure);

    const Value& table_;
    const std::string& file_;
    std::vector<Problem>& problems_;
    std::vector<std::string_view> named_;
};

}  // namespace kd::data
