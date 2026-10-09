// Rules-only materials and characteristic-constrained blueprints (MAT-03, MAT-04).
#pragma once
#include <array>
#include <initializer_list>
#include <vector>
#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
namespace kd::data {
inline constexpr std::array<std::string_view, 18> kCharacteristics{
    "hardness", "edge",   "toughness", "flaking", "flexibility", "weight",     "burn",       "fuel",       "food",
    "water",    "poison", "medicine",  "warmth",  "fibre",       "stickiness", "plasticity", "waterproof", "pigment"};
inline constexpr std::initializer_list<std::string_view> kClasses{"any",  "stone", "earth", "wood",  "plant",
                                                                  "bone", "hide",  "flesh", "metal", "water"};
inline constexpr std::initializer_list<std::string_view> kForms{"any",   "lump",   "flake",     "blade",     "point",
                                                                "rod",   "pole",   "sheet",     "strand",    "powder",
                                                                "paste", "liquid", "container", "structure", "crumb"};
struct Change {
    std::int64_t characteristic = 0, state = 0, value = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.whole({"characteristic", "characteristic number"}, c.characteristic, {0, 17});
        v.whole({"state", "physical state"}, c.state, {0, 3});
        v.whole({"value", "state's value"}, c.value, {0, 5});
    }
};
struct ItemKind {
    std::string material_class, form, icon;
    std::array<std::int64_t, 18> characteristics{};
    std::int64_t length = 0, mass = 0, primary = 0;
    bool inherit = false, edible = false;
    Ref broken;
    std::vector<Change> state_changes;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.choice({"class", "material class"}, c.material_class, kClasses);
        v.choice({"form", "physical form"}, c.form, kForms);
        for (std::size_t i = 0; i < 18; ++i)
            v.whole({kCharacteristics[i], "base characteristic"}, c.characteristics[i], {0, 5});
        v.quantity({"length", "usual length"}, c.length, Measure::length, {1, 100000});
        v.quantity({"mass", "usual mass"}, c.mass, Measure::mass, {1, 1000000000});
        v.whole({"primary", "characteristic reduced by wear and shifted by made quality"}, c.primary, {0, 17});
        v.truth({"inherit", "inherits the main input material"}, c.inherit);
        v.truth({"edible", "starting known edible state"}, c.edible);
        v.link({"broken", "physical break result"}, c.broken, "item");
        v.text({"icon", "shared stand-in form"}, c.icon);
        v.records({"state_changes", "explicit changed state values"}, c.state_changes);
    }
};
struct CharacteristicRange {
    std::int64_t characteristic = 0, minimum = 0, maximum = 5;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.whole({"characteristic", "characteristic index"}, c.characteristic, {0, 17});
        v.whole({"minimum", "inclusive lower value"}, c.minimum, {0, 5});
        v.whole({"maximum", "inclusive upper value"}, c.maximum, {0, 5});
    }
};
struct InputRole {
    std::string material_class, form;
    std::vector<std::string> classes;
    std::int64_t min_length = 0, max_length = 100000, min_mass = 1, max_mass = 1000000000, wear = 0;
    bool retained = false, optional = false;
    std::vector<CharacteristicRange> ranges;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.choice({"class", "accepted material class"}, c.material_class, kClasses);
        v.names({"classes", "optional class alternatives", Affects::rules, false}, c.classes);
        v.choice({"form", "accepted form"}, c.form, kForms);
        v.quantity({"min_length", "minimum length"}, c.min_length, Measure::length, {0, 100000});
        v.quantity({"max_length", "maximum length"}, c.max_length, Measure::length, {1, 100000});
        v.quantity({"min_mass", "minimum mass"}, c.min_mass, Measure::mass, {1, 1000000000});
        v.quantity({"max_mass", "maximum mass"}, c.max_mass, Measure::mass, {1, 1000000000});
        v.whole({"wear", "wear millionths per kilogram worked"}, c.wear, {0, 5000000});
        v.truth({"retained", "tool retained"}, c.retained);
        v.truth({"optional", "bare-hand fallback allowed"}, c.optional);
        v.records({"ranges", "characteristic bounds"}, c.ranges);
    }
};
struct Blueprint {
    std::int64_t action = 0, sector = 0, difficulty = 1, seconds = 1, bare_seconds = 1, unit_mass = 1, yield = 1000000,
                 bare_yield = 1000000;
    std::int64_t heat = 0, result_length = 1, result_mass = 0, edge_from = -1, toughness = -1, discovery = 1000000,
                 shatter = 0, hint = 0;
    std::int64_t need = 3, benefit = 0;
    bool starting = false;
    Ref result, leftover, failure;
    std::vector<InputRole> inputs;
    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.whole({"action", "base action zero to twenty"}, c.action, {0, 20});
        v.whole({"sector", "practice sector zero to fourteen"}, c.sector, {0, 14});
        v.whole({"difficulty", "difficulty level"}, c.difficulty, {1, 10});
        v.quantity({"seconds", "time per unit with fitted tools"}, c.seconds, Measure::game_time, {1, 3600});
        v.quantity({"bare_seconds", "bare-hand fallback time"}, c.bare_seconds, Measure::game_time, {1, 3600});
        v.quantity({"unit_mass", "input per try"}, c.unit_mass, Measure::mass, {1, 1000000000});
        v.whole({"yield", "result share in millionths"}, c.yield, {0, 1000000});
        v.whole({"bare_yield", "bare fallback yield"}, c.bare_yield, {0, 1000000});
        v.whole({"heat", "required fire level, zero for none"}, c.heat, {0, 5});
        v.quantity({"result_length", "made length"}, c.result_length, Measure::length, {1, 100000});
        v.quantity({"result_mass", "fixed result mass, zero for proportional yield"}, c.result_mass, Measure::mass,
                   {0, 1000000000});
        v.whole({"edge_from", "source characteristic supplying edge, minus one for none"}, c.edge_from, {-1, 17});
        v.whole({"toughness", "override result toughness, minus one for none"}, c.toughness, {-1, 5});
        v.whole({"discovery", "discovery factor in millionths"}, c.discovery, {0, 1000000});
        v.whole({"shatter", "catastrophic failure share in millionths"}, c.shatter, {0, 1000000});
        v.whole({"hint", "observable failure sign"}, c.hint, {0, 18});
        v.whole({"need", "expected known use: food, water, rest or curiosity"}, c.need, {0, 3});
        v.whole({"benefit", "known benefit per kilogram"}, c.benefit, {0, 100});
        v.truth({"starting", "founder knows this craft"}, c.starting);
        v.link({"result", "result item"}, c.result, "item");
        v.link({"leftover", "conserved leftover item"}, c.leftover, "item");
        v.link({"failure", "physical failure result"}, c.failure, "item");
        v.records({"inputs", "characteristic constrained input roles"}, c.inputs);
    }
};
struct FitInput {
    std::string_view material_class, form;
    std::array<std::int64_t, 18> values{};
    std::int64_t length = 0, mass = 0;
};
[[nodiscard]] bool fits(const InputRole& role, const FitInput& input);
void check_craft(const class Catalogue& cat, std::vector<struct Problem>& problems);
}  // namespace kd::data
