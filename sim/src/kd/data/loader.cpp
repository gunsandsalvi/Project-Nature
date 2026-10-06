#include "kd/data/loader.hpp"

#include <algorithm>

namespace kd::data {

namespace {

std::string with_unit(std::int64_t value, const Measure* measure) {
    std::string text = std::to_string(value);
    if (measure != nullptr) {
        const std::string_view unit = units_of(*measure)[0].name;
        text += unit.empty() ? std::string() : " " + std::string(unit);
    }
    return text;
}

}  // namespace

void Loader::problem(const Value& at, std::string what) {
    problems_.push_back({file_, at.line, at.column, std::move(what)});
}

void Loader::problem(const Value& at, const Field& f, std::string what) {
    problem(at, std::string(f.key) + ": " + std::move(what));
}

const Value* Loader::take(const Field& f, Value::Kind kind) {
    named_.push_back(f.key);
    const Value* v = table_.find(f.key);
    if (v == nullptr) {
        if (f.required) {
            problem(table_, "\"" + std::string(f.key) + "\" is missing: " + std::string(f.about));
        }
        return nullptr;
    }
    if (v->kind != kind) {
        problem(*v, f, "must be " + std::string(kind_name(kind)) + ", not " + std::string(kind_name(v->kind)));
        return nullptr;
    }
    marks_.push_back({std::string(f.key), v->line, v->column});
    return v;
}

void Loader::in_range(const Value& at, const Field& f, std::int64_t value, Range range, Measure* measure) {
    if (value < range.lowest || value > range.highest) {
        problem(at, f,
                with_unit(value, measure) + " is out of its range, " + with_unit(range.lowest, measure) + " to " +
                    with_unit(range.highest, measure));
    }
}

void Loader::whole(const Field& f, std::int64_t& out, Range range) {
    if (const Value* v = take(f, Value::Kind::whole)) {
        out = v->whole;
        in_range(*v, f, out, range, nullptr);
    }
}

void Loader::truth(const Field& f, bool& out) {
    if (const Value* v = take(f, Value::Kind::truth)) {
        out = v->truth;
    }
}

void Loader::text(const Field& f, std::string& out) {
    if (const Value* v = take(f, Value::Kind::text)) {
        out = v->text;
    }
}

void Loader::choice(const Field& f, std::string& out, std::initializer_list<std::string_view> options) {
    choice(f, out, std::span<const std::string_view>(options.begin(), options.size()));
}

void Loader::choice(const Field& f, std::string& out, std::span<const std::string_view> options) {
    if (const Value* v = take(f, Value::Kind::text)) {
        if (std::find(options.begin(), options.end(), v->text) == options.end()) {
            std::string list;
            for (std::string_view o : options) {
                list += (list.empty() ? "\"" : ", \"") + std::string(o) + "\"";
            }
            problem(*v, f, "\"" + v->text + "\" is not one of " + list);
            return;
        }
        out = v->text;
    }
}

void Loader::quantity(const Field& f, std::int64_t& out, Measure measure, Range range) {
    if (const Value* v = take(f, Value::Kind::text)) {
        const Amount a = read_quantity(v->text, measure);
        if (!a.error.empty()) {
            problem(*v, f, a.error);
            return;
        }
        out = a.value;
        in_range(*v, f, out, range, &measure);
    }
}

void Loader::chance(const Field& f, num::Probability& out) {
    if (const Value* v = take(f, Value::Kind::text)) {
        const Chance c = read_probability(v->text);
        if (!c.error.empty()) {
            problem(*v, f, c.error);
            return;
        }
        out = c.value;
    }
}

void Loader::duration(const Field& f, time::Duration& out) {
    const Value* v = take(f, Value::Kind::table);
    if (v == nullptr) {
        return;
    }
    const Value* life = v->find("life");
    const Value* game = v->find("game");
    if (life == nullptr || game == nullptr || life->kind != Value::Kind::text || game->kind != Value::Kind::text ||
        v->items.size() != 2) {
        problem(*v, f, "a duration is written with its two lengths, such as { life = \"3 month\", game = \"15 d\" }");
        return;
    }
    const Amount in_life = read_quantity(life->text, Measure::life_time);
    const Amount in_game = read_quantity(game->text, Measure::game_time);
    if (!in_life.error.empty()) {
        problem(*life, f, in_life.error);
    }
    if (!in_game.error.empty()) {
        problem(*game, f, in_game.error);
    }
    if (!in_life.error.empty() || !in_game.error.empty()) {
        return;
    }
    out = {in_life.value, in_game.value};
    if (const time::RuleCheck rule = time::check_rule(out); !rule.kept) {
        problem(*v, f, "breaks the rule of the game year (TIM-18): it " + rule.why);
    }
}

void Loader::link(const Field& f, Ref& out, std::string_view /*kind*/) {
    if (const Value* v = take(f, Value::Kind::text)) {
        out = {v->text, 0, v->line, v->column};
    }
}

void Loader::links(const Field& f, std::vector<Ref>& out, std::string_view /*kind*/) {
    if (const Value* v = take(f, Value::Kind::array)) {
        out.clear();
        for (const Value& item : v->items) {
            if (item.kind != Value::Kind::text) {
                problem(item, f, "each link is the name of an entry, in quotes");
                continue;
            }
            out.push_back({item.text, 0, item.line, item.column});
        }
    }
}

void Loader::names(const Field& f, std::vector<std::string>& out) {
    if (const Value* v = take(f, Value::Kind::array)) {
        out.clear();
        for (const Value& item : v->items) {
            if (item.kind != Value::Kind::text) {
                problem(item, f, "each name is in quotes");
                continue;
            }
            if (std::find(out.begin(), out.end(), item.text) != out.end()) {
                problem(item, f, "\"" + item.text + "\" is listed twice");
                continue;
            }
            out.push_back(item.text);
        }
    }
}

const Value* Loader::table(const Field& f) {
    return take(f, Value::Kind::table);
}

std::vector<const Value*> Loader::tables(const Field& f) {
    std::vector<const Value*> out;
    if (const Value* v = take(f, Value::Kind::array)) {
        for (const Value& item : v->items) {
            if (item.kind != Value::Kind::table) {
                problem(item, f, "each is a table, written [[" + std::string(f.key) + "]]");
                continue;
            }
            out.push_back(&item);
        }
    }
    return out;
}

void Loader::finish() {
    for (const Value& v : table_.items) {
        if (std::find(named_.begin(), named_.end(), v.key) == named_.end()) {
            std::string known;
            for (std::string_view k : named_) {
                known += (known.empty() ? "" : ", ") + std::string(k);
            }
            problem(v, "\"" + v.key + "\" is not a field of this kind, whose fields are " + known);
        }
    }
}

}  // namespace kd::data
