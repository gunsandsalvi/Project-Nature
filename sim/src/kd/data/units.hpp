// Quantities as text (A3.6): "3.5 kg", "1 h 30 min", "15%" and "1 in 100" are read exactly into whole base units,
// with no floating point anywhere, so the phone's reading can never differ from the cloud's (research 18). A value
// finer than its base unit is refused, never rounded; "m" is only ever a metre, never a minute.
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "kd/num/probability.hpp"

namespace kd::data {

/// What a quantity measures; each has one base unit, the whole number the simulation counts in.
enum class Measure : std::uint8_t {
    mass,         // milligrams
    length,       // millimetres
    area,         // square millimetres
    volume,       // millilitres
    life_time,    // seconds of life, where a month is 365.25 / 12 days and a year 365.25 days
    game_time,    // game seconds, where a season is 15 days and a year 60 (TIM-18)
    speed,        // millimetres a second
    temperature,  // thousandths of a degree Celsius
    ratio,        // parts per million
};

/// A unit and what one of it is in base units, as a fraction that is exact.
struct Unit {
    std::string_view name;
    std::int64_t numerator;
    std::int64_t denominator;
};

/// The units a measure can be written in, the base unit first.
std::span<const Unit> units_of(Measure m);

/// The measure's name, for messages: "a mass".
std::string_view measure_name(Measure m);

/// The base unit's name in words: "milligram".
std::string_view base_name(Measure m);

/// A whole number of base units, or why the text is not one.
struct Amount {
    std::int64_t value = 0;
    std::string error;
};

/// Reads a quantity, such as "3.5 kg" or "1 h 30 min"; a ratio also as "15%", "0.15", "1 in 8" or "150 ppm".
/// Implements MAT-13, see A3.6.
Amount read_quantity(std::string_view text, Measure m);

/// A probability, or why the text is not one.
struct Chance {
    num::Probability value = num::Probability::never();
    std::string error;
};

/// Reads a probability: "15%", "1 in 100", "0.413", "0" or "1", exactly. Implements MAT-13 and TIM-16, see A3.6.
Chance read_probability(std::string_view text);

}  // namespace kd::data
