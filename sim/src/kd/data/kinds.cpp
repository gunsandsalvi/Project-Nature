// Every kind of catalogue entry the game knows, one line each, and the checks they bring (A3.6): a new kind is its
// header, its visit(), a line here and its checks' lines below.
#include <array>

#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/orders.hpp"
#include "kd/demo/checks.hpp"
#include "kd/demo/crowd.hpp"
#include "kd/demo/marker.hpp"
#include "kd/time/speeds.hpp"

namespace kd::data {

Catalogue::Catalogue() {
    add_kind<demo::Marker>("marker", "a kind of the demonstration's markers, which walk, meet and greet (MAT-16)");
    add_kind<demo::Crowd>("tuning/crowd", "the demonstration's crowd: its camps, markers and greetings (MAT-16)", true);
    add_kind<time::ZoomSpeeds>("tuning/time", "the speeds of time at the zoom stops (TIM-01)", true);
}

namespace {

// In the order of their names.
constexpr std::array<Check, 2> kChecks{{
    {"company", "MAT-17", "every demonstration marker can greet or be greeted", demo::check_company},
    {"orders", "MAT-05", "every entry in its place in the real order of things, as checks/orders.toml lists them",
     check_orders},
}};

}  // namespace

std::span<const Check> checks() {
    return kChecks;
}

}  // namespace kd::data
