// The river's water (A4.5, PRE-26, T2.3b.4): base/tuning/water.toml, how the water takes the bed's colour as light is
// lost in it, how wet the bank is, what the surface mirrors, how fast the current runs and how its marks, glints and
// shore line look. The ground's shader and the surface's read them through the view's globals (game/look/water.gd),
// so no number of the water is in code. Only the screen reads them, so they count in the look, never in the world's
// rules.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"

namespace kd::look {

/// Implements PRE-26, see A4.5: the water's look.
struct WaterTuning {
    std::int64_t fade_red = 0;
    std::int64_t fade_green = 0;
    std::int64_t fade_blue = 0;
    std::int64_t murk = 0;
    std::string deep_colour;
    std::int64_t wet_margin = 0;
    std::int64_t wet_darkening = 0;
    std::int64_t ragged = 0;
    std::int64_t beach = 0;
    std::int64_t calm_marks = 0;
    std::int64_t calm_scale = 0;
    std::int64_t marks_depth = 0;
    std::int64_t shore_wander = 0;
    std::int64_t shore_scale = 0;
    std::int64_t murk_curve = 0;
    std::int64_t gravel_depth = 0;
    std::int64_t shore_line_share = 0;
    std::int64_t sky_share = 0;
    std::int64_t flow = 0;
    std::int64_t step_rate = 0;
    std::string glint_colour;
    std::int64_t glint_share = 0;
    std::int64_t glint_life = 0;
    std::string shore_colour;
    std::int64_t shore_width = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& w) {
        using data::Affects;
        using data::Measure;
        v.quantity({"fade_red", "how far red light goes in the water before a third of it is left", Affects::look},
                   w.fade_red, Measure::length, {50, 100'000});
        v.quantity({"fade_green", "how far green light goes before a third of it is left", Affects::look}, w.fade_green,
                   Measure::length, {50, 100'000});
        v.quantity({"fade_blue", "how far blue light goes before a third of it is left", Affects::look}, w.fade_blue,
                   Measure::length, {50, 100'000});
        v.quantity({"murk", "how deep the water must be for its own colour to have taken two thirds", Affects::look},
                   w.murk, Measure::length, {50, 100'000});
        v.text({"deep_colour", "the colour deep water sends back, as #rrggbb", Affects::look}, w.deep_colour);
        v.quantity({"wet_margin", "how far above the water's level the bed is still wet", Affects::look}, w.wet_margin,
                   Measure::length, {0, 2'000});
        v.quantity({"wet_darkening", "the share of its colour wet bed keeps", Affects::look}, w.wet_darkening,
                   Measure::ratio, {0, 1'000'000});
        v.quantity({"ragged", "how ragged the shore is, as a share of the depth a texture pixel spans", Affects::look},
                   w.ragged, Measure::ratio, {0, 2'000'000});
        v.quantity({"beach", "how high above the wet margin gravel and grass share the bank, up to the bank's top",
                    Affects::look},
                   w.beach, Measure::length, {0, 2'000});
        v.quantity({"calm_marks", "the share of its marks the water keeps where it is calmest", Affects::look},
                   w.calm_marks, Measure::ratio, {0, 1'000'000});
        v.quantity({"calm_scale", "how wide the calm and the rough stretches of water are", Affects::look},
                   w.calm_scale, Measure::length, {500, 100'000});
        v.quantity(
            {"marks_depth", "the depth over which the marks fade in from nothing at the water's edge", Affects::look},
            w.marks_depth, Measure::length, {0, 2'000});
        v.quantity(
            {"shore_wander",
             "how many millimetres of depth the shore is moved by, so the waterline bends round the bank instead "
             "of running straight",
             Affects::look},
            w.shore_wander, Measure::length, {0, 500});
        v.quantity({"shore_scale", "how wide the bends of the waterline are", Affects::look}, w.shore_scale,
                   Measure::length, {200, 20'000});
        v.quantity({"murk_curve",
                    "how sharply the water's own colour comes in with depth: 100% is an even rate, more is clearer at "
                    "first and then quicker",
                    Affects::look},
                   w.murk_curve, Measure::ratio, {500'000, 4'000'000});
        v.quantity({"gravel_depth",
                    "how deep under the water the bank's gravel goes before the river bed's stones take over, mixed in "
                    "clumps",
                    Affects::look},
                   w.gravel_depth, Measure::length, {0, 2'000});
        v.quantity({"shore_line_share", "the share of the waterline's texture pixels the bright shore line lights",
                    Affects::look},
                   w.shore_line_share, Measure::ratio, {0, 1'000'000});
        v.quantity({"sky_share", "the share of the sky's light the surface mirrors when seen square on", Affects::look},
                   w.sky_share, Measure::ratio, {0, 1'000'000});
        v.quantity({"flow", "how fast the current runs, toward the east", Affects::look}, w.flow, Measure::speed,
                   {0, 10'000});
        v.whole({"step_rate", "how many times a second the marks and glints step", Affects::look}, w.step_rate,
                {1, 30});
        v.text({"glint_colour", "the glints' colour, as #rrggbb", Affects::look}, w.glint_colour);
        v.quantity({"glint_share", "the most share of the texture pixels a glint lights at one time", Affects::look},
                   w.glint_share, Measure::ratio, {0, 100'000});
        v.whole({"glint_life", "how many steps a glint lives", Affects::look}, w.glint_life, {1, 30});
        v.text({"shore_colour", "the line where water meets land, as #rrggbb", Affects::look}, w.shore_colour);
        v.whole({"shore_width", "the shore line's width in texture pixels", Affects::look}, w.shore_width, {1, 4});
    }
};

/// Implements MAT-17 for the water: each colour is written #rrggbb, as the light's are.
void check_water(const data::Catalogue& cat, std::vector<data::Problem>& problems);

}  // namespace kd::look
