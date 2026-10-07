// The light of the moment the look shows (A4.3, PRE-30): base/tuning/light.toml, where the sun stands and the colours
// of its light, the sky's fill, the haze and the light bounced up from the ground. The light function reads them
// through the view's globals (game/look/afternoon.gd); they are stand-ins until α2.3b tunes the light against the
// target card, and only the screen reads them, so they count in the look, never in the world's rules.
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"

namespace kd::look {

/// Implements PRE-30, see A4.3: the late afternoon's light.
struct LightTuning {
    std::int64_t sun_height = 0;  // degrees above the horizon
    std::int64_t sun_turn = 0;    // degrees clockwise from north, where the sun stands
    std::string sun_colour;
    std::int64_t sun_energy = 0;  // parts per million of Godot's light at 1
    std::string sky_colour;
    std::string ambient_colour;
    std::int64_t ambient_energy = 0;  // parts per million
    std::string haze_colour;
    std::string haze_sun_colour;
    std::int64_t haze_density = 0;  // parts per million a metre
    std::string bounce_colour;

    template <typename V, typename Self>
    static void visit(V& v, Self& l) {
        using data::Affects;
        using data::Measure;
        v.whole({"sun_height", "how high the sun stands, in degrees above the horizon", Affects::look}, l.sun_height,
                {5, 89});
        v.whole({"sun_turn", "where the sun stands, in degrees clockwise from north", Affects::look}, l.sun_turn,
                {0, 359});
        v.text({"sun_colour", "the sun's light, as #rrggbb", Affects::look}, l.sun_colour);
        v.quantity({"sun_energy", "how strong the sun's light is, 100% being Godot's light at 1", Affects::look},
                   l.sun_energy, Measure::ratio, {100'000, 4'000'000});
        v.text({"sky_colour", "the sky's colour, as #rrggbb, which the picture's background takes", Affects::look},
               l.sky_colour);
        v.text({"ambient_colour", "the sky's fill in the shade, as #rrggbb", Affects::look}, l.ambient_colour);
        v.quantity({"ambient_energy", "how strong the sky's fill is", Affects::look}, l.ambient_energy, Measure::ratio,
                   {0, 4'000'000});
        v.text({"haze_colour", "the haze away from the sun, as #rrggbb", Affects::look}, l.haze_colour);
        v.text({"haze_sun_colour", "the haze toward the sun, as #rrggbb", Affects::look}, l.haze_sun_colour);
        v.quantity(
            {"haze_density", "how thick the haze is, as the share of the view it takes in a metre", Affects::look},
            l.haze_density, Measure::ratio, {0, 100'000});
        v.text({"bounce_colour", "the light bounced up from the sunlit ground, as #rrggbb", Affects::look},
               l.bounce_colour);
    }
};

/// Whether a text is a colour written #rrggbb, six hexadecimal digits in lower case, which is how Godot's own reading
/// of a colour is certain of it. Every tuning that holds a colour is checked by it (MAT-17).
[[nodiscard]] bool is_colour_text(std::string_view text);

/// Implements MAT-17 for the light: each colour is written #rrggbb.
void check_light(const data::Catalogue& cat, std::vector<data::Problem>& problems);

}  // namespace kd::look
