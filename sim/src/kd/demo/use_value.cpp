// MAT-22 MND-09 MND-23: value comes from personally known uses, not item names or hidden truth.
#include "kd/demo/crafting.hpp"
namespace kd::demo {
std::vector<KnownUse> Crafting::known_uses(const data::Catalogue& catalogue, const world::Knowledge& mind,
                                           const std::array<std::int64_t, 3>& needs, const world::Item& item,
                                           const world::Familiar& familiar, std::int64_t warmth) {
    const auto& kind = catalogue.kind<data::ItemKind>()[item.kind];
    const auto& material = catalogue.kind<data::ItemKind>()[item.material];
    data::FitInput perceived{
        kind.inherit ? std::string_view(material.material_class) : std::string_view(kind.material_class),
        kind.form,
        {},
        item.length,
        item.mass};
    for (std::size_t n = 0; n < perceived.values.size(); ++n)
        if (familiar.mask & (1U << n)) perceived.values[n] = familiar.values[n];
    std::vector<KnownUse> uses;
    uses.reserve(mind.skills.size());
    for (const auto& skill : mind.skills) {
        if (!skill.known) continue;
        const auto& recipe = catalogue.kind<data::Blueprint>()[skill.recipe];
        const bool maintenance = recipe.need == 3 && recipe.heat == 1;
        if ((recipe.need >= 3 && !maintenance) || !recipe.benefit) continue;
        const auto need = static_cast<std::uint8_t>(maintenance ? 4 : recipe.need);
        const auto met = maintenance ? warmth : needs[need];
        const auto benefit = std::min<std::int64_t>(recipe.benefit, 100 - met);
        if (!benefit) continue;
        for (std::size_t n = 0; n < recipe.inputs.size(); ++n) {
            const auto& role = recipe.inputs[n];
            if (std::any_of(role.ranges.begin(), role.ranges.end(),
                            [&](const auto& range) {
                                const auto p = static_cast<std::size_t>(range.characteristic);
                                return !(familiar.mask & (1U << p)) || !familiar.certainty[p];
                            }) ||
                !data::fits(role, perceived))
                continue;
            uses.push_back({skill.recipe, static_cast<std::uint8_t>(n), need, benefit,
                            std::max<std::int64_t>(0, 80 - met) * benefit * 10});
        }
    }
    return uses;
}
}  // namespace kd::demo
