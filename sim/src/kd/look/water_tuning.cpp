#include "kd/look/water_tuning.hpp"

#include <array>
#include <string_view>
#include <utility>

#include "kd/look/light_tuning.hpp"

namespace kd::look {

void check_water(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const data::Kind<WaterTuning>& waters = cat.kind<WaterTuning>();
    for (std::uint32_t i = 0; i < waters.size(); ++i) {
        const WaterTuning& w = waters[i];
        const std::array<std::pair<std::string_view, std::string_view>, 3> colours{
            {{"deep_colour", w.deep_colour}, {"glint_colour", w.glint_colour}, {"shore_colour", w.shore_colour}}};
        for (const auto& [key, value] : colours) {
            if (!is_colour_text(value)) {
                problems.push_back(
                    waters.at(i, key, "\"" + std::string(value) + "\" is not a colour written #rrggbb in lower case"));
            }
        }
    }
}

}  // namespace kd::look
