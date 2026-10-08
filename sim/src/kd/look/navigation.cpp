#include "kd/look/navigation.hpp"
namespace kd::look {
void check_navigation(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const auto& entries = cat.kind<NavigationTuning>();
    for (std::uint32_t i = 0; i < entries.size(); ++i) {
        const auto& s = entries[i];
        if (!(s.maximum_power >= s.person_power && s.person_power > s.close_camp_power &&
              s.close_camp_power > s.camp_power && s.camp_power > s.valley_power && s.valley_power > s.region_power &&
              s.region_power > s.minimum_power))
            problems.push_back(
                entries.at(i, "person_power", "time anchors must descend strictly inside the navigation range"));
        if (!(s.tiny_power > s.group_power && s.group_power >= s.minimum_power))
            problems.push_back(entries.at(i, "tiny_power", "form thresholds must descend inside the navigation range"));
    }
}
}  // namespace kd::look
