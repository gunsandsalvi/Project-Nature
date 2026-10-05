#include "kd/time/duration.hpp"
#include "doctest.h"

namespace kt = kd::time;

namespace {

constexpr kt::Seconds kDays = kt::kDay;
// A month in life: a twelfth of 365.25 days.
constexpr kt::Seconds kMonth = 365 * kt::kDay / 12 + kt::kDay / 48;

}  // namespace

// Smoked meat keeps about three months in life and 15 days in the game (RCK-14).
// checks: TIM-18
TEST_CASE("a duration keeps its real length up to about two weeks and a sixth from a month") {
    CHECK(kt::check_rule({3 * kMonth, 15 * kDays}).kept);
    CHECK(kt::check_rule({2 * kDays, 2 * kDays}).kept);
    CHECK(kt::check_rule({15 * kDays, 15 * kDays}).kept);
    CHECK(kt::check_rule({1 * kt::kHour, 1 * kt::kHour}).kept);
    CHECK(kt::check_rule({0, 0}).kept);
    // a year in life is a game year, within a tenth
    CHECK(kt::check_rule({365 * kDays + kDays / 4, 60 * kDays}).kept);
    CHECK(kt::check_rule({365 * kDays + kDays / 4, 54 * kDays}).kept);
    CHECK(kt::check_rule({365 * kDays + kDays / 4, 66 * kDays}).kept);
    // between two weeks and a month, anything from about the shortened length to the real one
    CHECK(kt::check_rule({21 * kDays, 21 * kDays}).kept);
    CHECK(kt::check_rule({21 * kDays, 10 * kDays}).kept);
    CHECK(kt::check_rule({21 * kDays, 4 * kDays}).kept);
}

// checks: TIM-18
TEST_CASE("a duration that breaks the rule is refused with the reason") {
    const kt::RuleCheck meat = kt::check_rule({3 * kMonth, 30 * kDays});
    CHECK_FALSE(meat.kept);
    CHECK(meat.why == "lasts 91.3 days in life, so about a sixth in the game, 13.5 days to 16.5 days, not 30 days");
    const kt::RuleCheck short_one = kt::check_rule({2 * kDays, 1 * kDays});
    CHECK_FALSE(short_one.kept);
    CHECK(short_one.why == "lasts 2 days in life, so its game length must be the same, not 1 day");
    CHECK_FALSE(kt::check_rule({365 * kDays + kDays / 4, 53 * kDays}).kept);
    CHECK_FALSE(kt::check_rule({365 * kDays + kDays / 4, 67 * kDays}).kept);
    CHECK_FALSE(kt::check_rule({21 * kDays, 22 * kDays}).kept);
    CHECK_FALSE(kt::check_rule({21 * kDays, 3 * kDays}).kept);
    CHECK_FALSE(kt::check_rule({-1, 0}).kept);
}
