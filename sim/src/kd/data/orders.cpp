#include "kd/data/orders.hpp"

#include "kd/data/catalogue.hpp"
#include "kd/data/loader.hpp"

namespace kd::data {

namespace {

// A field's value as the display shows it, such as "1400 mm/s".
std::string shown(const KindBase& k, std::size_t i, std::string_view field) {
    const std::string text = k.display(i);
    const std::string start = std::string(field) + " = ";
    std::size_t at = text.find(start);
    while (at != std::string::npos && at != 0 && text[at - 1] != '\n') {
        at = text.find(start, at + 1);
    }
    if (at == std::string::npos) {
        return {};
    }
    const std::size_t from = at + start.size();
    return text.substr(from, text.find('\n', from) - from);
}

// A problem at one of the order's fields, where it was written.
Problem at(const std::vector<Mark>& marks, const std::string& file, std::string_view key, const std::string& what) {
    for (const Mark& m : marks) {
        if (m.key == key) {
            return {file, m.line, m.column, std::string(key) + ": " + what};
        }
    }
    return {file, 1, 1, what};
}

void check_one(const Catalogue& cat, const std::string& file, const Order& o, const std::vector<Mark>& marks,
               std::vector<Problem>& problems) {
    const KindBase* k = cat.kind_in(o.kind);
    if (k == nullptr) {
        problems.push_back(at(marks, file, "kind", "\"" + o.kind + "\" is no kind's folder"));
        return;
    }
    if (o.least_to_most.size() < 2) {
        problems.push_back(at(marks, file, "least_to_most", "an order names two entries or more"));
        return;
    }
    std::vector<std::uint32_t> entries;
    for (const Ref& r : o.least_to_most) {
        // a bare name is the base source's, as in a link (A3.6)
        const std::string full = r.name.find(':') == std::string::npos ? "base:" + r.name : r.name;
        const std::optional<std::uint32_t> i = cat.find(o.kind, full);
        if (!i) {
            problems.push_back(
                {file, r.line, r.column, "least_to_most: \"" + r.name + "\" names no entry of kind " + o.kind});
            return;
        }
        entries.push_back(*i);
    }
    const Picked first = k->pick(entries[0], o.field);
    if (!first.found) {
        problems.push_back(at(marks, file, "field", "\"" + o.field + "\" is not a field of kind " + o.kind));
        return;
    }
    if (!first.orderable) {
        problems.push_back(at(marks, file, "field",
                              "\"" + o.field +
                                  "\" cannot be put in order: only whole numbers, quantities, chances and durations "
                                  "can"));
        return;
    }
    for (std::size_t j = 1; j < entries.size(); ++j) {
        const std::uint32_t less = entries[j - 1];
        const std::uint32_t more = entries[j];
        if (k->pick(less, o.field).order < k->pick(more, o.field).order) {
            continue;
        }
        const Ref& r = o.least_to_most[j];
        std::string what = "least_to_most: " + k->name(more) + "'s " + o.field + ", " + shown(*k, more, o.field);
        what += ", is not more than " + k->name(less) + "'s, " + shown(*k, less, o.field);
        what += ", against the order of things (MAT-05): " + o.why;
        problems.push_back({file, r.line, r.column, std::move(what)});
    }
}

}  // namespace

void check_orders(const Catalogue& cat, std::vector<Problem>& problems) {
    for (const SourceFile& f : cat.check_files()) {
        if (!f.path.ends_with("/checks/orders.toml")) {
            continue;
        }
        Parsed parsed = parse_toml(f.text, f.path);
        if (!parsed.problems.empty()) {
            problems.insert(problems.end(), parsed.problems.begin(), parsed.problems.end());
            continue;
        }
        for (const Value& v : parsed.root.items) {
            if (v.key != "order" || v.kind != Value::Kind::array) {
                problems.push_back({f.path, v.line, v.column, "an orders file holds only [[order]] tables"});
                continue;
            }
            for (const Value& table : v.items) {
                if (table.kind != Value::Kind::table) {
                    problems.push_back({f.path, table.line, table.column, "each order is an [[order]] table"});
                    continue;
                }
                Order o;
                const std::size_t before = problems.size();
                Loader loader(table, f.path, problems);
                Order::visit(loader, o);
                loader.finish();
                if (problems.size() == before) {
                    check_one(cat, f.path, o, loader.marks(), problems);
                }
            }
        }
    }
}

}  // namespace kd::data
