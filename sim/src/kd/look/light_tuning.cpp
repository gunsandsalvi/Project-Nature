#include "kd/look/light_tuning.hpp"

#include <algorithm>
#include <array>
#include <string_view>

namespace kd::look {

namespace {

bool colour(std::string_view text) {
    return text.size() == 7 && text.front() == '#' && std::all_of(text.begin() + 1, text.end(), [](char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}

}  // namespace

void check_light(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const data::Kind<LightTuning>& lights = cat.kind<LightTuning>();
    for (std::uint32_t i = 0; i < lights.size(); ++i) {
        const LightTuning& l = lights[i];
        const std::array<std::pair<std::string_view, std::string_view>, 6> colours{
            {{"sun_colour", l.sun_colour},
             {"sky_colour", l.sky_colour},
             {"ambient_colour", l.ambient_colour},
             {"haze_colour", l.haze_colour},
             {"haze_sun_colour", l.haze_sun_colour},
             {"bounce_colour", l.bounce_colour}}};
        for (const auto& [key, value] : colours) {
            if (!colour(value)) {
                problems.push_back(
                    lights.at(i, key, "\"" + std::string(value) + "\" is not a colour written #rrggbb in lower case"));
            }
        }
    }
}

}  // namespace kd::look
