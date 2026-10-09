#include "kd/demo/learning.hpp"
#include "doctest.h"
#include "kd/chance/chance.hpp"
#include "kd/ecs/component.hpp"

namespace {
kd::world::Practice reopen(const kd::world::Practice& p) {
    kd::ByteWriter writer;
    kd::ecs::write_component(p, writer);
    const auto bytes = writer.take();
    kd::ByteReader reader(bytes);
    kd::world::Practice out;
    REQUIRE(kd::ecs::read_component(out, reader, {}));
    REQUIRE(reader.finished());
    return out;
}
std::vector<std::byte> encoded(const kd::world::Practice& p) {
    kd::ByteWriter writer;
    kd::ecs::write_component(p, writer);
    return writer.take();
}
}  // namespace

TEST_CASE("skill_curve conserves fractional effort and uses the declared two slopes") {
    kd::world::Practice skill{1000, 1000, 0, 0};
    kd::demo::Learning::practice(skill, 0, 180 * kd::time::kHour, false);
    CHECK(skill.level == 5000);
    CHECK(skill.seconds == 180 * kd::time::kHour);
    kd::demo::Learning::practice(skill, 0, 780 * kd::time::kHour, false);
    CHECK(skill.level == 10000);
    CHECK(skill.best == 10000);
    CHECK(skill.fraction == 0);
    kd::world::Practice successful{1000, 1000, 0, 0};
    kd::demo::Learning::practice(successful, 0, 90 * kd::time::kHour, true);
    CHECK(successful.level == 5000);
    kd::world::Practice whole{1000, 1000, 0, 0}, pieces = whole;
    kd::demo::Learning::practice(whole, 0, 317, true, 917123, 5137777);
    for (int i = 0; i < 317; ++i) {
        kd::demo::Learning::practice(pieces, 0, 1, true, 917123, 5137777);
        if (i % 7 == 0) pieces = reopen(pieces);
    }
    CHECK(encoded(pieces) == encoded(whole));
    CHECK(whole.scale_remainder != 0);
    CHECK(whole.seconds_remainder != 0);
}

TEST_CASE("unused skill fades from a saved fixed anchor and never below half its best") {
    kd::world::Practice direct{8000, 10000, 960 * kd::time::kHour, 0}, split = direct;
    kd::demo::Learning::fade(direct, 10 * kd::time::kYear);
    for (int year = 1; year <= 10; ++year) {
        kd::demo::Learning::fade(split, year * kd::time::kYear);
        split = reopen(split);
    }
    CHECK(direct.level == 5750);
    CHECK(encoded(split) == encoded(direct));
    kd::demo::Learning::fade(direct, 1000 * kd::time::kYear);
    CHECK(direct.level == 5000);
    CHECK(direct.best == 10000);
    kd::demo::Learning::practice(direct, 1000 * kd::time::kYear, 562, false);
    CHECK(direct.level == 5001);
    CHECK(direct.level < direct.best);
}

TEST_CASE("unfinished practice does not unlock a recipe and a known faded recipe remains known") {
    kd::world::Knowledge mind;
    kd::world::Skill unfinished;
    unfinished.recipe = 9;
    unfinished.known = 0;
    kd::demo::Learning::practice(unfinished.practice, 0, kd::time::kDay, true);
    REQUIRE(unfinished.practice.level > 1000);
    mind.skills.push_back(unfinished);
    CHECK_FALSE(kd::demo::Learning::knows(mind, 9));
    mind.skills.back().known = 1;
    kd::demo::Learning::fade(mind.skills.back().practice, 10 * kd::time::kYear);
    CHECK(kd::demo::Learning::knows(mind, 9));
    CHECK_FALSE(kd::demo::Learning::knows(mind, 10));
    CHECK(kd::demo::Learning::taught_multiplier({0, 0, 0, 0}) == 4000000);
    CHECK(kd::demo::Learning::taught_multiplier({5000, 5000, 0, 0}) == 6000000);
    CHECK(kd::demo::Learning::taught_multiplier({10000, 10000, 0, 0}) == 8000000);
}

TEST_CASE("no_global_unlock and last_holder_removed leave only personal skill holders") {
    kd::world::Knowledge holder, other;
    kd::world::Skill skill;
    skill.recipe = 17;
    skill.known = 1;
    skill.practice = {1000, 1000, 0, -1};
    holder.skills.push_back(skill);
    CHECK(kd::demo::Learning::knows(holder, 17));
    CHECK_FALSE(kd::demo::Learning::knows(other, 17));
    other.performed = 1;
    CHECK_FALSE(kd::demo::Learning::knows(other, 17));
    // Labelled holder removal, not a disease or death implementation.
    holder.skills.clear();
    CHECK_FALSE(kd::demo::Learning::knows(holder, 17));
    CHECK_FALSE(kd::demo::Learning::knows(other, 17));
}
TEST_CASE("sector and recipe practice share effort scaling and saved fading") {
    kd::world::Knowledge mind;
    kd::world::Skill skill;
    skill.recipe = 17;
    skill.known = 1;
    skill.practice = {1000, 1000, 0, 0};
    mind.skills.push_back(skill);
    mind.sectors[0] = skill.practice;
    int first_five = -1;
    for (int day = 0; day < 180; ++day) {
        if (day % 7 == 6) continue;
        const auto now = day * kd::time::kDay;
        kd::demo::Learning::practice(mind.skills.front().practice, now, kd::time::kHour, true, 917123);
        kd::demo::Learning::practice(mind.sectors[0], now, kd::time::kHour, true, 917123);
        if (first_five == -1 && mind.sectors[0].level >= 5000) first_five = day;
    }
    CHECK(first_five >= 90);
    CHECK(first_five < 180);
    CHECK(encoded(mind.sectors[0]) == encoded(mind.skills.front().practice));
    CHECK(mind.sectors[0].level >= 5000);
    auto saved = reopen(mind.sectors[0]);
    kd::demo::Learning::fade(saved, saved.last_use + 10 * kd::time::kYear);
    CHECK(saved.level >= (saved.best + 1) / 2);
}

TEST_CASE("zero elapsed practice neither credits effort nor records a fresh use") {
    kd::world::Practice skill{8000, 10000, 0, 0};
    const auto before = encoded(skill);
    kd::demo::Learning::practice(skill, 10 * kd::time::kYear, 0, true);
    CHECK(encoded(skill) == before);
    kd::demo::Learning::fade(skill, 10 * kd::time::kYear);
    CHECK(skill.level == 5750);
    CHECK(skill.last_use == 0);
}
