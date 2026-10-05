#include "kd/data/units.hpp"

#include <array>
#include <optional>

namespace kd::data {

namespace {

using Wide = __int128;

constexpr std::int64_t kMax = INT64_MAX;

constexpr std::array kMass = {Unit{"mg", 1, 1}, Unit{"g", 1'000, 1}, Unit{"kg", 1'000'000, 1},
                              Unit{"t", 1'000'000'000, 1}};
constexpr std::array kLength = {Unit{"mm", 1, 1}, Unit{"cm", 10, 1}, Unit{"m", 1'000, 1}, Unit{"km", 1'000'000, 1}};
constexpr std::array kArea = {Unit{"mm2", 1, 1}, Unit{"cm2", 100, 1}, Unit{"m2", 1'000'000, 1},
                              Unit{"ha", 10'000'000'000, 1}, Unit{"km2", 1'000'000'000'000, 1}};
constexpr std::array kVolume = {Unit{"ml", 1, 1}, Unit{"cl", 10, 1}, Unit{"dl", 100, 1}, Unit{"l", 1'000, 1},
                                Unit{"m3", 1'000'000, 1}};
constexpr std::array kLifeTime = {
    Unit{"s", 1, 1},          Unit{"min", 60, 1},          Unit{"h", 3'600, 1},        Unit{"d", 86'400, 1},
    Unit{"week", 604'800, 1}, Unit{"month", 2'629'800, 1}, Unit{"year", 31'557'600, 1}};
constexpr std::array kGameTime = {Unit{"s", 1, 1},      Unit{"min", 60, 1},           Unit{"h", 3'600, 1},
                                  Unit{"d", 86'400, 1}, Unit{"season", 1'296'000, 1}, Unit{"year", 5'184'000, 1}};
constexpr std::array kSpeed = {Unit{"mm/s", 1, 1}, Unit{"cm/s", 10, 1}, Unit{"m/s", 1'000, 1}, Unit{"km/h", 2'500, 9}};
constexpr std::array kTemperature = {Unit{"m°C", 1, 1}, Unit{"°C", 1'000, 1}};
constexpr std::array kRatio = {Unit{"ppm", 1, 1}, Unit{"%", 10'000, 1}, Unit{"", 1'000'000, 1}};

// The written forms that mean the same unit: plurals, and the ASCII and superscript squares and cubes.
struct Alias {
    std::string_view written;
    std::string_view unit;
};
constexpr std::array kAliases = {
    Alias{"mm²", "mm2"},        Alias{"cm²", "cm2"},    Alias{"m²", "m2"},        Alias{"km²", "km2"},
    Alias{"m³", "m3"},          Alias{"weeks", "week"}, Alias{"months", "month"}, Alias{"years", "year"},
    Alias{"seasons", "season"}, Alias{"days", "d"},     Alias{"day", "d"},        Alias{"hours", "h"},
    Alias{"hour", "h"},
};

// A number as written: its digits as one whole number and how many of them follow the point.
struct Number {
    Wide digits = 0;
    int decimals = 0;
    bool negative = false;
};

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

// Reads a number at the start of text, moving past it; at most 18 digits, a point between digits, no exponent.
std::optional<Number> read_number(std::string_view& text) {
    Number n;
    std::size_t i = 0;
    if (i < text.size() && text[i] == '-') {
        n.negative = true;
        ++i;
    }
    int digits = 0;
    bool point = false;
    for (; i < text.size(); ++i) {
        const char c = text[i];
        if (is_digit(c)) {
            if (++digits > 18) {
                return std::nullopt;
            }
            n.digits = n.digits * 10 + (c - '0');
            if (point) {
                ++n.decimals;
            }
        } else if (c == '.' && !point && i + 1 < text.size() && is_digit(text[i + 1]) && digits > 0) {
            point = true;
        } else {
            break;
        }
    }
    if (digits == 0) {
        return std::nullopt;
    }
    text.remove_prefix(i);
    return n;
}

void skip_spaces(std::string_view& text) {
    while (!text.empty() && text.front() == ' ') {
        text.remove_prefix(1);
    }
}

// The unit's name at the start of text: everything up to the next space or digit.
std::string_view take_unit(std::string_view& text) {
    std::size_t i = 0;
    while (i < text.size() && text[i] != ' ' && !is_digit(text[i]) && text[i] != '-') {
        ++i;
    }
    const std::string_view unit = text.substr(0, i);
    text.remove_prefix(i);
    return unit;
}

std::string_view canonical(std::string_view written) {
    for (const Alias& a : kAliases) {
        if (a.written == written) {
            return a.unit;
        }
    }
    return written;
}

std::string unit_list(Measure m) {
    std::string out;
    const auto units = units_of(m);
    for (std::size_t i = 0; i < units.size(); ++i) {
        if (units[i].name.empty()) {
            continue;
        }
        if (!out.empty()) {
            out += i + 1 == units.size() ? " or " : ", ";
        }
        out += units[i].name;
    }
    return out;
}

Wide pow10(int n) {
    Wide p = 1;
    for (int i = 0; i < n; ++i) {
        p *= 10;
    }
    return p;
}

std::string quoted(std::string_view text) {
    return "\"" + std::string(text) + "\"";
}

}  // namespace

std::span<const Unit> units_of(Measure m) {
    switch (m) {
        case Measure::mass:
            return kMass;
        case Measure::length:
            return kLength;
        case Measure::area:
            return kArea;
        case Measure::volume:
            return kVolume;
        case Measure::life_time:
            return kLifeTime;
        case Measure::game_time:
            return kGameTime;
        case Measure::speed:
            return kSpeed;
        case Measure::temperature:
            return kTemperature;
        case Measure::ratio:
            return kRatio;
    }
    return {};
}

std::string_view base_name(Measure m) {
    switch (m) {
        case Measure::mass:
            return "milligram";
        case Measure::length:
            return "millimetre";
        case Measure::area:
            return "square millimetre";
        case Measure::volume:
            return "millilitre";
        case Measure::life_time:
        case Measure::game_time:
            return "second";
        case Measure::speed:
            return "millimetre a second";
        case Measure::temperature:
            return "thousandth of a degree";
        case Measure::ratio:
            return "part per million";
    }
    return "base unit";
}

std::string_view measure_name(Measure m) {
    switch (m) {
        case Measure::mass:
            return "a mass";
        case Measure::length:
            return "a length";
        case Measure::area:
            return "an area";
        case Measure::volume:
            return "a volume";
        case Measure::life_time:
            return "a length of time in life";
        case Measure::game_time:
            return "a length of game time";
        case Measure::speed:
            return "a speed";
        case Measure::temperature:
            return "a temperature";
        case Measure::ratio:
            return "a ratio";
    }
    return "a quantity";
}

Amount read_quantity(std::string_view text, Measure m) {
    const std::string whole_text = quoted(text);
    if (text.empty()) {
        return {0, "empty: write a number and its unit, such as \"3.5 kg\""};
    }
    // "1 in 8" for a ratio
    if (m == Measure::ratio) {
        std::string_view rest = text;
        if (auto one = read_number(rest); one && !one->negative && one->decimals == 0) {
            skip_spaces(rest);
            if (rest.substr(0, 3) == "in ") {
                rest.remove_prefix(3);
                const auto many = read_number(rest);
                if (!many || !rest.empty() || many->negative || many->decimals != 0 || many->digits == 0) {
                    return {0, whole_text + ": write a ratio of whole numbers, such as \"1 in 8\""};
                }
                const Wide ppm = one->digits * 1'000'000;
                if (ppm % many->digits != 0) {
                    return {0, whole_text +
                                   ": not a whole number of parts per million; write it as a percentage "
                                   "with enough places, such as \"33.33%\""};
                }
                return {static_cast<std::int64_t>(ppm / many->digits), {}};
            }
        }
    }
    Wide total = 0;
    std::string_view rest = text;
    bool first = true;
    while (!rest.empty()) {
        const std::optional<Number> n = read_number(rest);
        if (!n) {
            return {0, whole_text + ": write each number with at most 18 digits and a point, such as \"1.5 kg\""};
        }
        if (!first && (n->negative || total < 0)) {
            return {0, whole_text + ": a negative quantity is written in one part, such as \"-90 min\""};
        }
        skip_spaces(rest);
        const std::string_view written = take_unit(rest);
        skip_spaces(rest);
        const std::string_view name = canonical(written);
        const Unit* unit = nullptr;
        for (const Unit& u : units_of(m)) {
            if (u.name == name) {
                unit = &u;
            }
        }
        if (unit == nullptr) {
            if (!written.empty() && written.front() == ',') {
                return {0, whole_text + ": write the number with a point, such as \"1.5 kg\""};
            }
            if (written.empty()) {
                return {0, whole_text + ": " + std::string(measure_name(m)) + " needs its unit, such as \"3.5 " +
                               std::string(units_of(m)[1].name) + "\""};
            }
            return {0, whole_text + ": " + quoted(written) + " is not a unit of " + std::string(measure_name(m)) +
                           ", which is written in " + unit_list(m)};
        }
        const Wide scaled = n->digits * unit->numerator;
        const Wide divisor = pow10(n->decimals) * unit->denominator;
        if (scaled % divisor != 0) {
            return {0, whole_text + ": finer than one " + std::string(base_name(m)) + ", which is as fine as " +
                           std::string(measure_name(m)) + " is counted"};
        }
        const Wide value = scaled / divisor;
        total += n->negative ? -value : value;
        if (total > kMax || total < -kMax) {
            return {0, whole_text + ": too large to count"};
        }
        first = false;
    }
    return {static_cast<std::int64_t>(total), {}};
}

Chance read_probability(std::string_view text) {
    const std::string whole_text = quoted(text);
    std::string_view rest = text;
    const std::optional<Number> n = read_number(rest);
    if (!n || n->negative) {
        return {num::Probability::never(), whole_text + ": write a chance as \"15%\", \"1 in 100\" or \"0.413\""};
    }
    skip_spaces(rest);
    Wide numerator = n->digits;
    Wide denominator = pow10(n->decimals);
    if (rest == "%") {
        denominator *= 100;
    } else if (rest.substr(0, 3) == "in ") {
        rest.remove_prefix(3);
        const std::optional<Number> many = read_number(rest);
        if (!many || !rest.empty() || many->negative || many->decimals != 0 || n->decimals != 0 || many->digits == 0) {
            return {num::Probability::never(), whole_text + ": write a chance in whole numbers, such as \"1 in 100\""};
        }
        denominator = many->digits;
    } else if (!rest.empty()) {
        return {num::Probability::never(), whole_text + ": write a chance as \"15%\", \"1 in 100\" or \"0.413\""};
    }
    if (numerator > denominator) {
        return {num::Probability::never(), whole_text + ": a chance is at most certain, \"100%\""};
    }
    if (denominator > Wide{INT64_MAX}) {
        return {num::Probability::never(), whole_text + ": too many digits for a chance"};
    }
    return {num::Probability::ratio(static_cast<std::uint64_t>(numerator), static_cast<std::uint64_t>(denominator)),
            {}};
}

}  // namespace kd::data
