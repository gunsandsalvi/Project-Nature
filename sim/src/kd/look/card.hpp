// The target card's goals and bands (A5.5): the tuning file tuning/card.toml, the goals every moment keeps and how far
// past a band an alarm turns amber, and the card/ entries, each moment's bands as tools/art/card.py measures them from
// the pictures you chose; and each statistic's alarm, green, amber or red. The card warns and never decides, so it
// counts in the look.
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
#include "kd/look/frame.hpp"

namespace kd::look {

/// Implements PRE-01, see A5.5: the goals every moment keeps, and the slack past each moment's bands.
struct CardTuning {
    std::int64_t flat_most = 0;            // parts per million of squares
    std::int64_t strong_colour_most = 0;   // parts per million of cells
    std::int64_t largest_colour_most = 0;  // parts per million of cells
    std::int64_t lightness_slack = 0;      // parts per million of OKLab's lightness
    std::int64_t dark_slack = 0;           // parts per million of cells
    std::int64_t lights_hue_slack = 0;     // degrees
    std::int64_t yellowness_slack = 0;     // parts per million of OKLab's scale
    std::int64_t green_slack = 0;          // parts per million of cells
    std::int64_t things_slack = 0;         // small things a thousand cells
    std::int64_t contrast_slack = 0;       // parts per million of OKLab's lightness

    template <typename V, typename Self>
    static void visit(V& v, Self& t) {
        using data::Affects;
        using data::Measure;
        const data::Range share{0, 1'000'000};
        v.quantity({"flat_most", "the most flat patches any moment may show: no flat ground", Affects::look},
                   t.flat_most, Measure::ratio, share);
        v.quantity({"strong_colour_most", "the most strong colour any moment may show: specks only", Affects::look},
                   t.strong_colour_most, Measure::ratio, share);
        v.quantity({"largest_colour_most", "the most of a frame one colour may cover", Affects::look},
                   t.largest_colour_most, Measure::ratio, share);
        v.quantity({"lightness_slack", "how far past a moment's lightness an alarm stays amber, and its bands' margin",
                    Affects::look},
                   t.lightness_slack, Measure::ratio, share);
        v.quantity({"dark_slack", "the same for the share of dark cells", Affects::look}, t.dark_slack, Measure::ratio,
                   share);
        v.whole({"lights_hue_slack", "the same for the lights' hue, in degrees", Affects::look}, t.lights_hue_slack,
                {0, 180});
        v.quantity({"yellowness_slack", "the same for the lights' and the shade's yellowness", Affects::look},
                   t.yellowness_slack, Measure::ratio, share);
        v.quantity({"green_slack", "the same for the share of green cells", Affects::look}, t.green_slack,
                   Measure::ratio, share);
        v.whole({"things_slack", "the same for small things, a thousand cells", Affects::look}, t.things_slack,
                {0, 1'000});
        v.quantity({"contrast_slack", "the same for the fine texture and the big masses", Affects::look},
                   t.contrast_slack, Measure::ratio, share);
    }
};

/// Implements PRE-01, see A4.3 and A5.5: a moment's bands on the card, green from each low to its high; ratios in
/// parts per million, as OKLab's scale or a share of the frame, the lights' hue in degrees and small things a
/// thousand cells.
struct Moment {
    std::string pictures;  // the chosen pictures its bands are measured from, in art/targets/, without .webp
    std::int64_t lightness_low = 0;
    std::int64_t lightness_high = 0;
    std::int64_t dark_low = 0;
    std::int64_t dark_high = 0;
    std::int64_t lights_hue_low = 0;  // a low above its high wraps through 0 degrees
    std::int64_t lights_hue_high = 0;
    std::int64_t lights_low = 0;
    std::int64_t lights_high = 0;
    std::int64_t shade_low = 0;
    std::int64_t shade_high = 0;
    std::int64_t green_low = 0;
    std::int64_t green_high = 0;
    std::int64_t things_low = 0;
    std::int64_t things_high = 0;
    std::int64_t texture_low = 0;
    std::int64_t texture_high = 0;
    std::int64_t masses_low = 0;
    std::int64_t masses_high = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& m) {
        using data::Affects;
        using data::Measure;
        const data::Range share{0, 1'000'000};
        const data::Range axis{-500'000, 500'000};
        v.text({"pictures", "the chosen pictures in art/targets/ its bands are measured from", Affects::look},
               m.pictures);
        v.quantity({"lightness_low", "the mean lightness", Affects::look}, m.lightness_low, Measure::ratio, share);
        v.quantity({"lightness_high", "the top of its band", Affects::look}, m.lightness_high, Measure::ratio, share);
        v.quantity({"dark_low", "the share of cells darker than 45%", Affects::look}, m.dark_low, Measure::ratio,
                   share);
        v.quantity({"dark_high", "the top of its band", Affects::look}, m.dark_high, Measure::ratio, share);
        v.whole({"lights_hue_low", "the brightest twentieth's hue, in degrees; a low above the high wraps through 0",
                 Affects::look},
                m.lights_hue_low, {0, 359});
        v.whole({"lights_hue_high", "the top of its band", Affects::look}, m.lights_hue_high, {0, 359});
        v.quantity({"lights_low", "the lightest fifth's yellowness, OKLab's b", Affects::look}, m.lights_low,
                   Measure::ratio, axis);
        v.quantity({"lights_high", "the top of its band", Affects::look}, m.lights_high, Measure::ratio, axis);
        v.quantity({"shade_low", "the darkest fifth's yellowness", Affects::look}, m.shade_low, Measure::ratio, axis);
        v.quantity({"shade_high", "the top of its band", Affects::look}, m.shade_high, Measure::ratio, axis);
        v.quantity({"green_low", "the share of green cells", Affects::look}, m.green_low, Measure::ratio, share);
        v.quantity({"green_high", "the top of its band", Affects::look}, m.green_high, Measure::ratio, share);
        v.whole({"things_low", "small things a thousand cells", Affects::look}, m.things_low, {0, 1'000});
        v.whole({"things_high", "the top of its band", Affects::look}, m.things_high, {0, 1'000});
        v.quantity({"texture_low", "the fine texture", Affects::look}, m.texture_low, Measure::ratio, share);
        v.quantity({"texture_high", "the top of its band", Affects::look}, m.texture_high, Measure::ratio, share);
        v.quantity({"masses_low", "the big masses", Affects::look}, m.masses_low, Measure::ratio, share);
        v.quantity({"masses_high", "the top of its band", Affects::look}, m.masses_high, Measure::ratio, share);
    }
};

/// A statistic's alarm on the card.
enum class Alarm : std::uint8_t { green, amber, red };

/// "green", "amber" or "red".
[[nodiscard]] std::string_view alarm_name(Alarm a);

/// One statistic's value on the card and its alarm.
struct Reading {
    std::string_view name;
    double value = 0.0;
    Alarm alarm = Alarm::green;
};

/// Implements PRE-01, see A5.5: each of the card's statistics against the moment's bands and the goals every moment
/// keeps, in the card's order: green inside, amber within the slack past a band or up to twice a goal, red beyond.
[[nodiscard]] std::vector<Reading> alarms(const Card& card, const CardTuning& tuning, const Moment& moment);

}  // namespace kd::look
