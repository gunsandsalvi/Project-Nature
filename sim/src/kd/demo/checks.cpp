#include "kd/demo/checks.hpp"
#include <array>

#include "kd/data/catalogue.hpp"
#include "kd/demo/living_rules.hpp"
#include "kd/demo/marker.hpp"

namespace kd::demo {

void check_living(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const auto& rules = cat.kind<LivingRules>();
    const auto& uses = cat.kind<NeedUse>();
    if (rules.size() == 0 && uses.size() == 0) return;  // accepted foundation fixtures have no living camp
    if (rules.size() != 1 || uses.size() != 3) {
        if (uses.size() > 0)
            problems.push_back(uses.at(0, "need", "a living camp needs one tuning entry and three unique need uses"));
        else if (rules.size() > 0)
            problems.push_back(rules.at(0, "food_day", "a living camp needs its three need uses"));
        return;
    }
    std::array<bool, 3> found{};
    for (std::size_t i = 0; i < uses.size(); ++i) {
        const auto& use = uses[static_cast<std::uint32_t>(i)];
        const auto n = static_cast<std::size_t>(use.need);
        if (found[n]) problems.push_back(uses.at(i, "need", "a body need has two affordances"));
        found[n] = true;
        if (use.use.game < 1 || use.gather.game < 1 || use.use.game > 86400 || use.gather.game > 86400)
            problems.push_back(uses.at(i, "use", "camp activities last from one second to one day"));
        if (use.amount() == 0 || (n != 0 && use.food_mg != 0) || (n != 1 && use.water_ml != 0) ||
            (n != 2 && use.rest_seconds != 0))
            problems.push_back(uses.at(i, "need", "only the chosen body need has a positive amount"));
        if ((n == 1 && use.amount() > 3000) || (n == 2 && use.amount() != 2 * use.use.game))
            problems.push_back(
                uses.at(i, "restored",
                        "portion must fit body capacity and rest restores two awake seconds per sleeping second"));
    }
    if (rules[0].loaded_speed > rules[0].speed)
        problems.push_back(rules.at(0, "loaded_speed", "loaded walking cannot be faster than unloaded walking"));
}
void check_company(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const data::Kind<Marker>& markers = cat.kind<Marker>();
    std::vector<std::uint8_t> named(markers.size(), 0);
    for (std::uint32_t i = 0; i < markers.size(); ++i) {
        for (const data::Ref& r : markers[i].walks_with) {
            named[r.index] = 1;
        }
    }
    for (std::uint32_t i = 0; i < markers.size(); ++i) {
        if (markers[i].walks_with.empty() && named[i] == 0) {
            problems.push_back(markers.at(i, "walks_with",
                                          markers.name(i) +
                                              " keeps company with no one: it names no marker in walks_with and no "
                                              "marker names it, so it would walk the crowd alone"));
        }
    }
}

}  // namespace kd::demo
