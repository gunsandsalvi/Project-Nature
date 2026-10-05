#include <cstdint>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/time/calendar.hpp"

namespace kt = kd::time;
using kt::Season;

// checks: TIM-14 TIM-18
TEST_CASE("history begins at second 0, on Year 1, spring, day 1, and a year is 60 days of 24 hours") {
    CHECK(kt::date_of(0) == kt::Date{1, Season::spring, 1, 0, 0, 0});
    CHECK(kt::moment_of({1, Season::spring, 1, 0, 0, 0}) == 0);
    CHECK(kt::kYear == 5'184'000);
    CHECK(kt::date_of(kt::kYear - 1) == kt::Date{1, Season::winter, 15, 23, 59, 59});
    CHECK(kt::date_of(kt::kYear) == kt::Date{2, Season::spring, 1, 0, 0, 0});
    CHECK(kt::date_of(-1) == kt::Date{0, Season::winter, 15, 23, 59, 59});
}

// checks: TIM-14 TIM-18
TEST_CASE("every boundary of a sample of years converts both ways") {
    for (std::int64_t year : {-2, 0, 1, 2, 7, 112, 250, 10'000, 999'999'999}) {
        for (int season = 0; season < 4; ++season) {
            for (int day : {1, 2, 8, 14, 15}) {
                for (int hour : {0, 11, 23}) {
                    const kt::Date d{year, static_cast<Season>(season), day, hour, 0, 0};
                    const kt::Seconds m = kt::moment_of(d);
                    REQUIRE(kt::date_of(m) == d);
                    for (kt::Seconds near : {m - 1, m, m + 1, m + 59, m + 3599}) {
                        REQUIRE(kt::moment_of(kt::date_of(near)) == near);
                    }
                }
            }
        }
    }
    CHECK(kd::test::stops([] { static_cast<void>(kt::moment_of({1, Season::spring, 16, 0, 0, 0})); }));
    CHECK(kd::test::stops([] { static_cast<void>(kt::moment_of({1, Season::spring, 1, 24, 0, 0})); }));
}

// checks: TIM-14
TEST_CASE("dates read as year, season and day, and the other half of the world adds its own season") {
    const kt::Date d = kt::date_of(kt::moment_of({112, Season::autumn, 6, 14, 5, 0}));
    CHECK(kt::date_text(d) == "Year 112, autumn, day 6");
    CHECK(kt::time_of_day_text(d) == "14:05");
    CHECK(kt::time_of_day_text(kt::date_of(6 * kt::kHour + 5 * kt::kMinute)) == "06:05");
    CHECK(kt::date_text_in_other_half({140, Season::winter, 3, 0, 0, 0}) == "Year 140, winter (their summer), day 3");
    CHECK(kt::season_in_other_half(Season::spring) == Season::autumn);
    CHECK(kt::season_in_other_half(Season::summer) == Season::winter);
    CHECK(kt::season_in_other_half(Season::autumn) == Season::spring);
    CHECK(kt::season_in_other_half(Season::winter) == Season::summer);
}
