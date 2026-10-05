// Game time (A3.3): whole game seconds from the start of history, Year 1, spring, day 1 at midnight, and the 60-day
// year of TIM-18 in four seasons of 15 days of 24 hours, with dates as TIM-14 writes them.
#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace kd::time {

/// A moment: whole game seconds since Year 1, spring, day 1 at midnight; moments before history are negative.
using Seconds = std::int64_t;

inline constexpr Seconds kMinute = 60;
inline constexpr Seconds kHour = 60 * kMinute;
inline constexpr Seconds kDay = 24 * kHour;
inline constexpr std::int64_t kDaysPerSeason = 15;
inline constexpr std::int64_t kSeasonsPerYear = 4;
inline constexpr Seconds kSeason = kDaysPerSeason * kDay;
inline constexpr Seconds kYear = kSeasonsPerYear * kSeason;

/// The seasons in the order the year has them, as the half of the world where history begins names them.
enum class Season : std::uint8_t { spring, summer, autumn, winter };

/// A moment as the calendar reads it.
struct Date {
    std::int64_t year = 1;  // from 1; 0 and below before history
    Season season = Season::spring;
    int day = 1;     // 1 to 15
    int hour = 0;    // 0 to 23
    int minute = 0;  // 0 to 59
    int second = 0;  // 0 to 59
    friend constexpr bool operator==(const Date&, const Date&) = default;
};

/// The date of a moment. Implements TIM-14 and TIM-18, see A3.3.
Date date_of(Seconds moment);

/// The moment a date names; every part must be in its range. Implements TIM-14 and TIM-18, see A3.3.
Seconds moment_of(const Date& date);

/// The season the other half of the world has at the same time: spring and autumn swap, and summer and winter.
/// Implements TIM-14, see A3.3 (WLD-01).
Season season_in_other_half(Season season);

/// A season's name: "spring", "summer", "autumn" or "winter".
std::string_view season_name(Season season);

/// A date as TIM-14 writes it: "Year 112, autumn, day 6". Implements TIM-14, see A3.3.
std::string date_text(const Date& date);

/// The same for a place in the other half of the world, its own season beside the calendar's:
/// "Year 140, winter (their summer), day 3". Implements TIM-14, see A3.3.
std::string date_text_in_other_half(const Date& date);

/// The hour and minute: "06:05".
std::string time_of_day_text(const Date& date);

}  // namespace kd::time
