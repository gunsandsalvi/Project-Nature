#include <map>
#include <string>

#include "doctest.h"
#include "kd/look/card.hpp"

namespace look = kd::look;

namespace {

look::CardTuning tuning() {
    look::CardTuning t;
    t.flat_most = 50'000;  // 5%
    t.strong_colour_most = 35'000;
    t.largest_colour_most = 70'000;
    t.lightness_slack = 30'000;  // 3 hundredths
    t.dark_slack = 60'000;
    t.lights_hue_slack = 10;
    t.yellowness_slack = 15'000;
    t.green_slack = 40'000;
    t.things_slack = 3;
    t.contrast_slack = 10'000;
    return t;
}

look::Moment moment() {
    look::Moment m;
    m.lightness_low = 500'000;  // 50 to 60
    m.lightness_high = 600'000;
    m.dark_low = 200'000;
    m.dark_high = 300'000;
    m.lights_hue_low = 340;  // round through 0 to 30 degrees
    m.lights_hue_high = 30;
    m.lights_low = 50'000;
    m.lights_high = 90'000;
    m.shade_low = -20'000;
    m.shade_high = 20'000;
    m.green_low = 0;
    m.green_high = 120'000;
    m.things_low = 20;
    m.things_high = 30;
    m.texture_low = 50'000;
    m.texture_high = 80'000;
    m.masses_low = 60'000;
    m.masses_high = 100'000;
    return m;
}

std::map<std::string, std::string> named(const look::Card& c) {
    std::map<std::string, std::string> out;
    for (const look::Reading& r : look::alarms(c, tuning(), moment())) {
        out[std::string(r.name)] = std::string(look::alarm_name(r.alarm));
    }
    return out;
}

}  // namespace

// checks: PRE-01
TEST_CASE("the card's alarms are green inside a band, amber within the slack past it, red beyond") {
    look::Card c;
    c.lightness = 55.0;       // inside
    c.dark = 32.0;            // 2 past 30, within 6
    c.lights_hue = 350.0;     // inside the band round 0
    c.lights = 11.0;          // 2 past 9, beyond 1.5
    c.shade = -3.0;           // 1 below -2, within 1.5
    c.strong_colour = 3.0;    // under 3.5
    c.green = 12.0;           // at the top
    c.flat = 8.0;             // over 5, under twice it
    c.things = 34.0;          // 4 past 30, beyond 3
    c.largest_colour = 15.0;  // over twice 7
    c.texture = 4.5;          // 0.5 below 5, within 1
    c.masses = 6.0;           // at the bottom
    const auto a = named(c);
    REQUIRE(a.size() == 12);
    CHECK(a.at("lightness") == "green");
    CHECK(a.at("dark") == "amber");
    CHECK(a.at("lights_hue") == "green");
    CHECK(a.at("lights") == "red");
    CHECK(a.at("shade") == "amber");
    CHECK(a.at("strong_colour") == "green");
    CHECK(a.at("green") == "green");
    CHECK(a.at("flat") == "amber");
    CHECK(a.at("things") == "red");
    CHECK(a.at("largest_colour") == "red");
    CHECK(a.at("texture") == "amber");
    CHECK(a.at("masses") == "green");
}

// checks: PRE-01
TEST_CASE("a hue band round 0 degrees measures how far past it round the circle") {
    look::Card c;
    c.lights_hue = 40.0;  // 10 past 30
    CHECK(named(c).at("lights_hue") == "amber");
    c.lights_hue = 45.0;
    CHECK(named(c).at("lights_hue") == "red");
    c.lights_hue = 335.0;  // 5 before 340
    CHECK(named(c).at("lights_hue") == "amber");
    c.lights_hue = 5.0;
    CHECK(named(c).at("lights_hue") == "green");
    c.lights_hue = 180.0;
    CHECK(named(c).at("lights_hue") == "red");
}
