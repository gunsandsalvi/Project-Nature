#include <limits>

#include "doctest.h"
#include "heat.hpp"

using kd::view::HeatGovernor;
using kd::view::HeatRules;

namespace {

HeatRules rules() {
    HeatRules r;
    r.near = 0.85;
    r.margin = 0.05;
    r.cut = 0.5;
    r.floor = 0.25;
    r.calm_readings = 3;
    r.give_back = 0.05;
    return r;
}

}  // namespace

// checks: PLT-04
TEST_CASE("the heat guard acts at the phone's light threshold less its margin, and at its own line without one") {
    HeatGovernor guard(rules());
    CHECK(guard.near() == 0.85);
    guard.set_light(0.7);
    CHECK(guard.near() == doctest::Approx(0.65));
    CHECK(guard.read(0.64) == 1.0);
    CHECK(guard.read(0.66) == 0.5);  // past the light threshold less 0.05: the share is cut at once
    CHECK(guard.read(0.9) == 0.25);  // the moderate level: cut again, to the floor at most
    CHECK(guard.read(0.95) == 0.25);
    // a threshold that is no headroom is no threshold
    guard.set_light(1.5);
    CHECK(guard.near() == 0.85);
}

// checks: PLT-04
TEST_CASE("a missing heat reading is no reading, never a cool phone") {
    HeatGovernor guard(rules());
    guard.set_light(0.7);
    CHECK(guard.read(0.8) == 0.5);
    const double nothing = std::numeric_limits<double>::quiet_NaN();
    // a phone that stops answering keeps the share where it was, however long it stays silent
    for (int i = 0; i < 20; ++i) {
        CHECK(guard.read(nothing) == 0.5);
        CHECK(guard.read(-1.0) == 0.5);
    }
    // calm readings give it back slowly, after three
    CHECK(guard.read(0.5) == 0.5);
    CHECK(guard.read(0.5) == 0.5);
    CHECK(guard.read(0.5) == 0.5);
    CHECK(guard.read(0.5) == doctest::Approx(0.55));
}
