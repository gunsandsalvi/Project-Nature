#include "kd/time/duration.hpp"

namespace kd::time {

namespace {

using Wide = __int128;

// About two weeks: up to 14 days and a tenth more.
constexpr Seconds kShort = 14 * kDay + 14 * kDay / 10;
// A month, a twelfth of 365.25 days, and about it: from a tenth less.
constexpr Seconds kMonth = 365 * kDay / 12 + kDay / 48;
constexpr Seconds kLong = kMonth - kMonth / 10;

// A length in days to a tenth, for the reason a duration is refused: "1 day", "13.5 days".
std::string days(Seconds s) {
    const Seconds tenths = (s * 10 + kDay / 2) / kDay;
    const std::string n = std::to_string(tenths / 10) + (tenths % 10 != 0 ? "." + std::to_string(tenths % 10) : "");
    return n + (tenths == 10 ? " day" : " days");
}

// The game length a life length shortens to, 60 days for 365.25, that is 240 for 1461, times 1461 and the tenths
// given, so it compares with a game length times 1461 x 10 in whole numbers.
constexpr Wide kScale = Wide{1461} * 10;

Wide shortened_tenths(Seconds life, int tenths) {
    return static_cast<Wide>(life) * 240 * tenths;
}

}  // namespace

RuleCheck check_rule(Duration d) {
    if (d.life < 0 || d.game < 0) {
        return {false, "a duration cannot be negative"};
    }
    const std::string lasts = "lasts " + days(d.life) + " in life";
    if (d.life <= kShort) {
        if (d.game == d.life) {
            return {true, {}};
        }
        return {false, lasts + ", so its game length must be the same, not " + days(d.game)};
    }
    // within a tenth of the shortened length: game x 1461 x 10 against life x 240 x 9 and x 11
    const Wide game = static_cast<Wide>(d.game) * kScale;
    const bool above_lowest = game >= shortened_tenths(d.life, 9);
    const Seconds shortest = static_cast<Seconds>(shortened_tenths(d.life, 9) / kScale);
    if (d.life >= kLong) {
        if (above_lowest && game <= shortened_tenths(d.life, 11)) {
            return {true, {}};
        }
        const Seconds longest = static_cast<Seconds>(shortened_tenths(d.life, 11) / kScale);
        return {false, lasts + ", so about a sixth in the game, " + days(shortest) + " to " + days(longest) + ", not " +
                           days(d.game)};
    }
    if (above_lowest && d.game <= d.life) {
        return {true, {}};
    }
    return {false, lasts + ", so from " + days(shortest) + " to " + days(d.life) + " in the game, not " + days(d.game)};
}

}  // namespace kd::time
