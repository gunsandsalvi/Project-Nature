#include "kd/look/card.hpp"

#include <algorithm>

namespace kd::look {

namespace {

// Parts per million as the card's hundredths: 569,300 is 56.93.
double hundredths(std::int64_t ppm) {
    return static_cast<double>(ppm) / 10'000.0;
}

Alarm within(double value, double low, double high, double slack) {
    if (value >= low && value <= high) {
        return Alarm::green;
    }
    const double past = value < low ? low - value : value - high;
    return past <= slack ? Alarm::amber : Alarm::red;
}

// Hues on the circle: a low above its high wraps through 0 degrees.
Alarm within_hue(double hue, double low, double high, double slack) {
    const bool inside = low <= high ? hue >= low && hue <= high : hue >= low || hue <= high;
    if (inside) {
        return Alarm::green;
    }
    const auto apart = [](double a, double b) {
        const double d = a > b ? a - b : b - a;
        return d > 180.0 ? 360.0 - d : d;
    };
    return std::min(apart(hue, low), apart(hue, high)) <= slack ? Alarm::amber : Alarm::red;
}

Alarm at_most(double value, double most) {
    if (value <= most) {
        return Alarm::green;
    }
    return value <= 2.0 * most ? Alarm::amber : Alarm::red;
}

}  // namespace

std::string_view alarm_name(Alarm a) {
    switch (a) {
        case Alarm::green:
            return "green";
        case Alarm::amber:
            return "amber";
        case Alarm::red:
            return "red";
    }
    return "red";
}

std::vector<Reading> alarms(const Card& c, const CardTuning& t, const Moment& m) {
    const double yellowness = hundredths(t.yellowness_slack);
    const double contrast = hundredths(t.contrast_slack);
    return {
        {"lightness", c.lightness,
         within(c.lightness, hundredths(m.lightness_low), hundredths(m.lightness_high), hundredths(t.lightness_slack))},
        {"dark", c.dark, within(c.dark, hundredths(m.dark_low), hundredths(m.dark_high), hundredths(t.dark_slack))},
        {"lights_hue", c.lights_hue,
         within_hue(c.lights_hue, static_cast<double>(m.lights_hue_low), static_cast<double>(m.lights_hue_high),
                    static_cast<double>(t.lights_hue_slack))},
        {"lights", c.lights, within(c.lights, hundredths(m.lights_low), hundredths(m.lights_high), yellowness)},
        {"shade", c.shade, within(c.shade, hundredths(m.shade_low), hundredths(m.shade_high), yellowness)},
        {"strong_colour", c.strong_colour, at_most(c.strong_colour, hundredths(t.strong_colour_most))},
        {"green", c.green,
         within(c.green, hundredths(m.green_low), hundredths(m.green_high), hundredths(t.green_slack))},
        {"flat", c.flat, at_most(c.flat, hundredths(t.flat_most))},
        {"things", c.things,
         within(c.things, static_cast<double>(m.things_low), static_cast<double>(m.things_high),
                static_cast<double>(t.things_slack))},
        {"largest_colour", c.largest_colour, at_most(c.largest_colour, hundredths(t.largest_colour_most))},
        {"texture", c.texture, within(c.texture, hundredths(m.texture_low), hundredths(m.texture_high), contrast)},
        {"masses", c.masses, within(c.masses, hundredths(m.masses_low), hundredths(m.masses_high), contrast)},
    };
}

}  // namespace kd::look
