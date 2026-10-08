// The stand-in area (A4.6, A5.3, T2.3b.2): base/tuning/area.toml, the square of meadow with a river across it that the
// pilot's pieces stand in until the world's own ground comes (M3). It names the three surfaces the area wears, each by
// its near tile's entry (art:meadow), the middle and far tiles and the versions being named by it (art:meadow/middle,
// art:meadow/v2), and gives the river's shape and the camp that stands on its bank: a tent and a club, each named by
// its recipe's entry, and where they stand, and the patch picture's numbers (A4.6): the masses of taller and
// shorter growth and the worn clearing round the camp. The names are text, not links, so the catalogue still loads with
// no art in it (the engine runs with none, PRE-20); when textures or recipes are loaded, a check holds each name to an
// entry. Only the screen reads it, so it counts in the look, never in the world's rules; view/src/area.hpp makes the
// land from it.
#pragma once

#include <cstdint>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"

namespace kd::look {

/// Implements PRE-23 and PRE-26, see A4.6 and A4.5: the stand-in area.
struct AreaTuning {
    std::string ground;
    std::string bed;
    std::string marks;
    std::string earth;
    std::string worn;
    std::string gravel;
    std::int64_t reach = 0;
    std::int64_t strip = 0;
    std::int64_t bank = 0;
    std::int64_t width = 0;
    std::int64_t width_wobble = 0;
    std::int64_t depth = 0;
    std::int64_t depth_wobble = 0;
    std::int64_t wobble_length = 0;
    std::int64_t run = 0;
    std::int64_t spacing = 0;
    std::int64_t seed = 0;
    std::string tent;
    std::string tent_simple;
    std::string tent_small;
    std::string club;
    std::int64_t camp_back = 0;
    std::int64_t club_away = 0;
    std::int64_t club_bearing = 0;
    std::int64_t tent_turn = 0;
    std::int64_t club_turn = 0;
    std::int64_t camp_seed = 0;
    std::int64_t patch_seed = 0;
    std::int64_t growth_scale = 0;   // millimetres
    std::int64_t growth_swing = 0;   // parts per million
    std::int64_t bare_below = 0;     // parts per million
    std::int64_t clearing = 0;       // millimetres
    std::int64_t clearing_fade = 0;  // millimetres
    std::int64_t form_simple = 0;    // screen pixels
    std::int64_t form_small = 0;     // screen pixels
    std::int64_t form_margin = 0;    // parts per million

    template <typename V, typename Self>
    static void visit(V& v, Self& a) {
        using data::Affects;
        using data::Measure;
        v.text({"ground",
                "the meadow's texture, written as its near tile's entry, art:meadow; its middle and far tiles and its "
                "versions are named by it: art:meadow/middle, art:meadow/far, art:meadow/v2",
                Affects::look},
               a.ground);
        v.text({"bed", "the river bed's texture, named as the ground's is", Affects::look}, a.bed);
        v.text(
            {"marks", "the water's marks, flow lines and foam on see-through, named as the ground's is", Affects::look},
            a.marks);
        v.text(
            {"earth", "the open dry ground's texture, where growth is thin, named as the ground's is", Affects::look},
            a.earth);
        v.text({"worn", "the trodden ground's texture, round the camp's tent, named as the ground's is", Affects::look},
               a.worn);
        v.text({"gravel", "the river bank's gravel texture, named as the ground's is", Affects::look}, a.gravel);
        v.quantity({"reach", "half the side of the square the area covers", Affects::look}, a.reach, Measure::length,
                   {64'000, 8'000'000});
        v.quantity({"strip", "half the width of the strip of heightfield the river runs in", Affects::look}, a.strip,
                   Measure::length, {8'000, 500'000});
        v.quantity({"bank", "how far the meadow stands above the river's level", Affects::look}, a.bank,
                   Measure::length, {50, 5'000});
        v.quantity({"width", "the river's mean width, bank to bank", Affects::look}, a.width, Measure::length,
                   {1'000, 200'000});
        v.quantity(
            {"width_wobble", "how far each bank strays from the mean, as a share of half the width", Affects::look},
            a.width_wobble, Measure::ratio, {0, 900'000});
        v.quantity({"depth", "the river's mean depth along its deepest line", Affects::look}, a.depth, Measure::length,
                   {100, 20'000});
        v.quantity({"depth_wobble", "how far the depth strays from the mean, as a share", Affects::look},
                   a.depth_wobble, Measure::ratio, {0, 900'000});
        v.quantity({"wobble_length", "how far along the river its widths and depths take to repeat", Affects::look},
                   a.wobble_length, Measure::length, {5'000, 2'000'000});
        v.quantity({"run", "how far the bank runs from the water's edge up to the meadow", Affects::look}, a.run,
                   Measure::length, {100, 20'000});
        v.quantity({"spacing", "the strip's rows round the banks: as fine as the shore is drawn", Affects::look},
                   a.spacing, Measure::length, {50, 4'000});
        v.whole({"seed", "which way the river wanders", Affects::look}, a.seed, {0, 2'147'483'647});
        v.text({"tent", "the camp's tent, a recipe's entry, such as art:hide_tent_cone", Affects::look}, a.tent);
        v.text({"tent_simple", "the tent's simple form, a recipe's entry, such as art:hide_tent_cone_simple",
                Affects::look},
               a.tent_simple);
        v.text(
            {"tent_small", "the tent's small form, a recipe's entry, such as art:hide_tent_cone_small", Affects::look},
            a.tent_small);
        v.text({"club", "the camp's club, a recipe's entry, such as art:club", Affects::look}, a.club);
        v.quantity({"camp_back", "how far north of the river's north bank, at the strip's middle, the tent stands",
                    Affects::look},
                   a.camp_back, Measure::length, {1'000, 100'000});
        v.quantity({"club_away", "how far from the tent the club lies", Affects::look}, a.club_away, Measure::length,
                   {500, 50'000});
        v.whole(
            {"club_bearing", "which way from the tent the club lies, in degrees clockwise from north", Affects::look},
            a.club_bearing, {0, 359});
        v.whole({"tent_turn", "how far the tent is turned from the way its recipe stands it, in degrees clockwise",
                 Affects::look},
                a.tent_turn, {0, 359});
        v.whole({"club_turn", "how far the club is turned from the way its recipe lays it, in degrees clockwise",
                 Affects::look},
                a.club_turn, {0, 359});
        v.whole({"camp_seed", "the seed the camp's things are put together with", Affects::look}, a.camp_seed,
                {0, 2'147'483'647});
        v.whole({"patch_seed", "which way the patch picture's masses of growth lie", Affects::look}, a.patch_seed,
                {0, 2'147'483'647});
        v.quantity({"growth_scale", "how wide the patch picture's biggest masses of taller or shorter growth are",
                    Affects::look},
                   a.growth_scale, Measure::length, {8'000, 500'000});
        v.quantity({"growth_swing",
                    "how far the ground's colour swings, golden where growth is short and dry and deeper "
                    "green where it is tall, at the extremes",
                    Affects::look},
                   a.growth_swing, Measure::ratio, {0, 1'000'000});
        v.quantity({"bare_below", "the growth under which the ground shows bare earth between its blades, as a share",
                    Affects::look},
                   a.bare_below, Measure::ratio, {0, 1'000'000});
        v.quantity(
            {"clearing", "the radius round the camp's tent where the ground is worn to bare earth", Affects::look},
            a.clearing, Measure::length, {1'000, 100'000});
        v.quantity({"clearing_fade", "how far past the clearing the wear takes to leave", Affects::look},
                   a.clearing_fade, Measure::length, {500, 100'000});
        v.whole({"form_simple", "the tent's width on the screen, in pixels, under which its simple form is drawn",
                 Affects::look},
                a.form_simple, {2, 4'000});
        v.whole({"form_small", "the tent's width on the screen, in pixels, under which its small form is drawn",
                 Affects::look},
                a.form_small, {1, 4'000});
        v.quantity({"form_margin",
                    "how far past a switch, as a share of its pixels, the tent's size must go before its form changes "
                    "back, so a form never flickers at a switch",
                    Affects::look},
                   a.form_margin, Measure::ratio, {0, 500'000});
    }
};

/// Implements MAT-17 for the area: the strip is wide enough to hold the river at its widest and its banks, the square
/// is wider than the strip, so the meadow's carpet has a north and a south, the camp stands inside the strip, and,
/// where textures or recipes are loaded, each surface and each thing it names is an entry of them.
void check_area(const data::Catalogue& cat, std::vector<data::Problem>& problems);

}  // namespace kd::look
