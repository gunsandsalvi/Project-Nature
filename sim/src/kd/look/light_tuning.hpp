// The light of the moment the look shows (A4.3, PRE-30): base/tuning/light.toml, where the sun stands and the colours
// of its light, the sky's fill, the haze and the light bounced up from the ground. The light function reads them
// through the view's globals (game/look/afternoon.gd); they are stand-ins until α2.3b tunes the light against the
// target card, and only the screen reads them, so they count in the look, never in the world's rules. The same entry
// holds the numbers of the view's maps round things (A4.4: openness, contact and the sun's angle, which
// view/src/maps.hpp makes) and the strength of the lit edge a low sun gives a shape (A5.2).
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
    std::int64_t maps_texel = 0;        // millimetres
    std::int64_t open_reach = 0;        // millimetres
    std::int64_t open_strength = 0;     // parts per million
    std::int64_t contact_width = 0;     // millimetres
    std::int64_t contact_strength = 0;  // parts per million
    std::int64_t shadow_reach = 0;      // millimetres
    std::int64_t rim = 0;               // parts per million

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
        v.quantity({"maps_texel", "the side of a texel of the view's maps round things", Affects::look}, l.maps_texel,
                   Measure::length, {20, 500});
        v.quantity(
            {"open_reach", "how far round a point on the ground openness looks for what blocks the sky", Affects::look},
            l.open_reach, Measure::length, {200, 10'000});
        v.quantity({"open_strength", "how much of the sky the nearest, tallest things take from the ground at most",
                    Affects::look},
                   l.open_strength, Measure::ratio, {0, 1'000'000});
        v.quantity({"contact_width", "how far from a thing's foot the ground is darkened", Affects::look},
                   l.contact_width, Measure::length, {20, 2'000});
        v.quantity({"contact_strength", "how much the ground is darkened at a thing's foot", Affects::look},
                   l.contact_strength, Measure::ratio, {0, 1'000'000});
        v.quantity({"shadow_reach", "the farthest caster's distance the maps hold, over which a shadow softens",
                    Affects::look},
                   l.shadow_reach, Measure::length, {2'000, 100'000});
        v.quantity({"rim", "how bright the edge is where the low sun grazes a shape, against the sun's own light",
                    Affects::look},
                   l.rim, Measure::ratio, {0, 4'000'000});
    }
};

/// Whether a text is a colour written #rrggbb, six hexadecimal digits in lower case, which is how Godot's own reading
/// of a colour is certain of it. Every tuning that holds a colour is checked by it (MAT-17).
[[nodiscard]] bool is_colour_text(std::string_view text);

/// Implements MAT-17 for the light: each colour is written #rrggbb.
void check_light(const data::Catalogue& cat, std::vector<data::Problem>& problems);

}  // namespace kd::look
