#include "kd/data/craft.hpp"
#include "kd/data/catalogue.hpp"
namespace kd::data {
bool fits(const InputRole& r, const FitInput& i) {
    if (!r.classes.empty() && std::find(r.classes.begin(), r.classes.end(), i.material_class) == r.classes.end())
        return false;
    if ((r.material_class != "any" && r.material_class != i.material_class) || (r.form != "any" && r.form != i.form) ||
        i.length < r.min_length || i.length > r.max_length || i.mass < r.min_mass || i.mass > r.max_mass)
        return false;
    for (const auto& range : r.ranges) {
        const auto value = i.values[static_cast<std::size_t>(range.characteristic)];
        if (value < range.minimum || value > range.maximum) return false;
    }
    return true;
}
void check_craft(const Catalogue& cat, std::vector<Problem>& problems) {
    const auto& items = cat.kind<ItemKind>();
    const auto& recipes = cat.kind<Blueprint>();
    for (std::size_t n = 0; n < items.size(); ++n) {
        const auto& i = items[static_cast<std::uint32_t>(n)];
        if (i.material_class == "any" || i.form == "any")
            problems.push_back(items.at(n, "class", "an item has an actual class and form"));
        std::array<std::uint32_t, 4> seen{};
        for (const auto& c : i.state_changes) {
            const auto bit = 1U << static_cast<unsigned>(c.characteristic);
            auto& mask = seen[static_cast<std::size_t>(c.state)];
            if ((mask & bit) != 0) problems.push_back(items.at(n, "state_changes", "duplicate state characteristic"));
            mask |= bit;
        }
    }
    for (std::size_t n = 0; n < recipes.size(); ++n) {
        const auto& b = recipes[static_cast<std::uint32_t>(n)];
        if (b.inputs.empty() || b.inputs.size() > 8 || b.inputs[0].retained || b.inputs[0].optional ||
            b.result_mass > b.unit_mass)
            problems.push_back(
                recipes.at(n, "inputs", "a craft needs a consumed main input and conserved result mass"));
        for (const auto& r : b.inputs) {
            for (const auto& cl : r.classes) {
                if (cl == "any" || std::find(kClasses.begin(), kClasses.end(), cl) == kClasses.end())
                    problems.push_back(recipes.at(n, "inputs", "invalid alternative material class"));
            }
            if (r.min_length > r.max_length || r.min_mass > r.max_mass || (r.optional && !r.retained))
                problems.push_back(recipes.at(n, "inputs", "input bounds or optional tool are invalid"));
            std::uint32_t mask = 0;
            for (const auto& c : r.ranges) {
                const auto bit = 1U << static_cast<unsigned>(c.characteristic);
                if (c.minimum > c.maximum || (mask & bit) != 0)
                    problems.push_back(recipes.at(n, "inputs", "invalid or duplicate characteristic range"));
                mask |= bit;
            }
        }
    }
}
}  // namespace kd::data
