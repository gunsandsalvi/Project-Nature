#include <cstdint>
#include <string>

#include "doctest.h"
#include "kd/data/toml.hpp"
#include "kd/data/units.hpp"

namespace data = kd::data;
using data::Measure;

namespace {

std::int64_t amount(const char* text, Measure m) {
    const data::Amount a = data::read_quantity(text, m);
    REQUIRE_MESSAGE(a.error.empty(), a.error);
    return a.value;
}

std::string refusal(const char* text, Measure m) {
    const data::Amount a = data::read_quantity(text, m);
    CHECK_MESSAGE(!a.error.empty(), text);
    return a.error;
}

}  // namespace

// checks: MAT-13
TEST_CASE("TOML becomes a tree whose every value knows its line and column") {
    const data::Parsed p = data::parse_toml(
        "name = \"walker\"\n"
        "speed = \"1.4 m/s\"\n"
        "count = 12\n"
        "shy = true\n"
        "colours = [\"ochre\", \"ash\"]\n"
        "[keep]\n"
        "life = \"3 month\"\n",
        "demo/marker/walker.toml");
    REQUIRE(p.problems.empty());
    const data::Value& root = p.root;
    REQUIRE(root.items.size() == 6);
    CHECK(root.items.front().key == "colours");
    CHECK(root.find("name")->text == "walker");
    CHECK(root.find("count")->whole == 12);
    CHECK(root.find("shy")->truth);
    CHECK(root.find("colours")->items.size() == 2);
    CHECK(root.find("speed")->line == 2);
    CHECK(root.find("speed")->column == 9);
    CHECK(root.find("keep")->find("life")->text == "3 month");
    CHECK(root.find("absent") == nullptr);
}

// checks: MAT-13
TEST_CASE("floats, dates and broken TOML are each refused where they are written") {
    const data::Parsed floats = data::parse_toml("a = 1\nb = 3.5\nc = 1979-05-27\n", "f.toml");
    REQUIRE(floats.problems.size() == 2);
    CHECK(data::problem_text(floats.problems[0]).rfind("f.toml:2:5: a bare decimal number", 0) == 0);
    CHECK(data::problem_text(floats.problems[1]).rfind("f.toml:3:5: a date or a time", 0) == 0);
    const data::Parsed broken = data::parse_toml("a = 1\nb = = 2\n", "g.toml");
    REQUIRE(broken.problems.size() == 1);
    CHECK(broken.problems[0].line == 2);
    CHECK(broken.problems[0].what.rfind("the TOML is broken here", 0) == 0);
}

// checks: MAT-13
TEST_CASE("quantities are read exactly into whole base units") {
    CHECK(amount("3.5 kg", Measure::mass) == 3'500'000);
    CHECK(amount("250g", Measure::mass) == 250'000);
    CHECK(amount("2 t", Measure::mass) == 2'000'000'000);
    CHECK(amount("1.75 m", Measure::length) == 1'750);
    CHECK(amount("2.5 ha", Measure::area) == 25'000'000'000);
    CHECK(amount("1.5 m²", Measure::area) == 1'500'000);
    CHECK(amount("0.75 l", Measure::volume) == 750);
    CHECK(amount("1 h 30 min", Measure::life_time) == 5'400);
    CHECK(amount("3 month", Measure::life_time) == 7'889'400);
    CHECK(amount("1 year", Measure::life_time) == 31'557'600);
    CHECK(amount("15 d", Measure::game_time) == 1'296'000);
    CHECK(amount("1 season", Measure::game_time) == 1'296'000);
    CHECK(amount("2 years", Measure::game_time) == 10'368'000);
    CHECK(amount("36 km/h", Measure::speed) == 10'000);
    CHECK(amount("1.4 m/s", Measure::speed) == 1'400);
    CHECK(amount("1200 °C", Measure::temperature) == 1'200'000);
    CHECK(amount("-5 °C", Measure::temperature) == -5'000);
    CHECK(amount("15%", Measure::ratio) == 150'000);
    CHECK(amount("0.15", Measure::ratio) == 150'000);
    CHECK(amount("1 in 8", Measure::ratio) == 125'000);
    CHECK(amount("150 ppm", Measure::ratio) == 150);
}

// checks: MAT-13
TEST_CASE("a quantity finer than its base unit, or written wrongly, is refused with what to write") {
    CHECK(refusal("0.5 mg", Measure::mass) ==
          "\"0.5 mg\": finer than one milligram, which is as fine as a mass is counted");
    CHECK(refusal("1 km/h", Measure::speed).find("finer than one millimetre a second") != std::string::npos);
    CHECK(refusal("1 in 3", Measure::ratio).find("not a whole number of parts per million") != std::string::npos);
    CHECK(refusal("3.5", Measure::mass) == "\"3.5\": a mass needs its unit, such as \"3.5 g\"");
    CHECK(refusal("3.5 kgg", Measure::mass) ==
          "\"3.5 kgg\": \"kgg\" is not a unit of a mass, which is written in mg, g, kg or t");
    CHECK(refusal("1,5 kg", Measure::mass) == "\"1,5 kg\": write the number with a point, such as \"1.5 kg\"");
    CHECK(refusal("3 month", Measure::game_time).find("not a unit of a length of game time") != std::string::npos);
    CHECK(refusal("5 m", Measure::game_time).find("\"m\" is not a unit") != std::string::npos);
    CHECK(!refusal("", Measure::mass).empty());
    CHECK(!refusal("1e3 kg", Measure::mass).empty());
    CHECK(!refusal(".5 kg", Measure::mass).empty());
    CHECK(!refusal("-1 h 30 min", Measure::life_time).empty());
    CHECK(!refusal("9999999999999999999 kg", Measure::mass).empty());
    CHECK(!refusal("10000000000000 t", Measure::mass).empty());
}

// checks: MAT-13 TIM-16
TEST_CASE("chances are read exactly into thresholds") {
    CHECK(data::read_probability("50%").value == kd::num::Probability::ratio(1, 2));
    CHECK(data::read_probability("1 in 100").value == kd::num::Probability::ratio(1, 100));
    CHECK(data::read_probability("0.413").value == kd::num::Probability::ratio(413, 1'000));
    CHECK(data::read_probability("12.5%").value == kd::num::Probability::ratio(1, 8));
    CHECK(data::read_probability("1").value == kd::num::Probability::always());
    CHECK(data::read_probability("100%").value == kd::num::Probability::always());
    CHECK(data::read_probability("0").value == kd::num::Probability::never());
    CHECK(data::read_probability("0%").error.empty());
    CHECK(!data::read_probability("150%").error.empty());
    CHECK(!data::read_probability("-1%").error.empty());
    CHECK(!data::read_probability("1 in 0").error.empty());
    CHECK(!data::read_probability("0.5 in 2").error.empty());
    CHECK(!data::read_probability("one in ten").error.empty());
    CHECK(!data::read_probability("0.123456789012345678").error.empty());
}
