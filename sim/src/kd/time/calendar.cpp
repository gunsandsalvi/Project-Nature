#include "kd/time/calendar.hpp"

#include <array>

#include "kd/core/check.hpp"
#include "kd/num/whole.hpp"

namespace kd::time {

namespace {

constexpr std::array<std::string_view, 4> kSeasonNames = {"spring", "summer", "autumn", "winter"};

}  // namespace

Date date_of(Seconds moment) {
    // rounding down, so a moment before history still falls in a year
    const std::int64_t years = num::floor_div(moment, kYear);
    Seconds left = moment - years * kYear;
    Date d;
    d.year = years + 1;
    d.season = static_cast<Season>(left / kSeason);
    left %= kSeason;
    d.day = static_cast<int>(left / kDay) + 1;
    left %= kDay;
    d.hour = static_cast<int>(left / kHour);
    left %= kHour;
    d.minute = static_cast<int>(left / kMinute);
    d.second = static_cast<int>(left % kMinute);
    return d;
}

Seconds moment_of(const Date& date) {
    KD_CHECK(static_cast<int>(date.season) < kSeasonsPerYear && date.day >= 1 && date.day <= kDaysPerSeason &&
                 date.hour >= 0 && date.hour < 24 && date.minute >= 0 && date.minute < 60 && date.second >= 0 &&
                 date.second < 60,
             "time::moment_of: a part of the date is out of its range");
    KD_CHECK(date.year > -1'000'000'000 && date.year < 1'000'000'000, "time::moment_of: the year is out of range");
    return (date.year - 1) * kYear + static_cast<std::int64_t>(date.season) * kSeason + (date.day - 1) * kDay +
           date.hour * kHour + date.minute * kMinute + date.second;
}

Season season_in_other_half(Season season) {
    return static_cast<Season>((static_cast<int>(season) + 2) % 4);
}

std::string_view season_name(Season season) {
    return kSeasonNames[static_cast<std::size_t>(season)];
}

std::string date_text(const Date& date) {
    return "Year " + std::to_string(date.year) + ", " + std::string(season_name(date.season)) + ", day " +
           std::to_string(date.day);
}

std::string date_text_in_other_half(const Date& date) {
    return "Year " + std::to_string(date.year) + ", " + std::string(season_name(date.season)) + " (their " +
           std::string(season_name(season_in_other_half(date.season))) + "), day " + std::to_string(date.day);
}

std::string time_of_day_text(const Date& date) {
    const auto two = [](int v) { return (v < 10 ? "0" : "") + std::to_string(v); };
    return two(date.hour) + ":" + two(date.minute);
}

}  // namespace kd::time
