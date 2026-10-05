// Every kind of catalogue entry the game knows, one line each (A3.6): a new kind is its header, its visit() and a
// line here.
#include "kd/data/catalogue.hpp"
#include "kd/demo/marker.hpp"
#include "kd/time/speeds.hpp"

namespace kd::data {

Catalogue::Catalogue() {
    add_kind<demo::Marker>("marker", "a kind of the demonstration's markers, which walk, meet and greet (MAT-16)");
    add_kind<time::ZoomSpeeds>("tuning/time", "the speeds of time at the zoom stops (TIM-01)", true);
}

}  // namespace kd::data
