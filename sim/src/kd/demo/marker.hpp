// The demonstration's marker (MAT-16): how a marker of the foundations' crowd walks, meets and greets. It lives in
// its own source, data/demo/, and never enters the game's catalogue.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "kd/data/schema.hpp"
#include "kd/data/units.hpp"
#include "kd/num/probability.hpp"
#include "kd/time/duration.hpp"

namespace kd::demo {

/// Implements MAT-16, see A3.6: a kind of marker.
struct Marker {
    std::int64_t speed = 0;  // millimetres a second
    std::int64_t reach = 0;  // millimetres
    num::Probability greets = num::Probability::never();
    time::Duration rest{};
    std::string colour;
    std::string gait;
    std::vector<data::Ref> walks_with;

    template <typename V, typename Self>
    static void visit(V& v, Self& m) {
        using data::Affects;
        using data::Measure;
        v.quantity({"speed", "how fast it walks", Affects::rules}, m.speed, Measure::speed, {100, 3'000});
        v.quantity({"reach", "how close another marker must come to be met", Affects::rules}, m.reach, Measure::length,
                   {100, 100'000});
        v.chance({"greets", "the chance it greets a marker it meets", Affects::rules}, m.greets);
        v.duration({"rest", "how long it rests at a camp between walks", Affects::rules}, m.rest);
        v.text({"colour", "its colour on the screen, as #rrggbb", Affects::look}, m.colour);
        v.choice({"gait", "how it is drawn walking", Affects::look}, m.gait, {"walk", "amble", "stride"});
        v.links({"walks_with", "the kinds of marker it keeps company with", Affects::rules, false}, m.walks_with,
                "marker");
    }
};

}  // namespace kd::demo
