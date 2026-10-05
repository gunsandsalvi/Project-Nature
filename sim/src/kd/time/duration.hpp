// Durations with two lengths (A3.6, TIM-18): every duration in the catalogues records how long it lasts in life and
// how long in the game, and the rule of the game year holds the two together, so a squeezed year never squeezes what
// takes only days.
#pragma once

#include <string>

#include "kd/time/calendar.hpp"

namespace kd::time {

/// A duration: its length in life and in the game, both in seconds.
struct Duration {
    Seconds life = 0;
    Seconds game = 0;
};

/// Whether a duration keeps the rule, and if not, why, in a sentence for the catalogue's error.
struct RuleCheck {
    bool kept = false;
    std::string why;
};

/// TIM-18's rule, "about" read as within 10% (A3.6): up to about two weeks in life, the game length equals it; a
/// month or more, about a sixth, 60 days for 365.25; between, anywhere from about the shortened length to the real
/// one. Implements TIM-18, see A3.6.
RuleCheck check_rule(Duration d);

}  // namespace kd::time
