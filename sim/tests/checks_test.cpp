#include <string>
#include <string_view>
#include <vector>

#include "catalogue_files.hpp"
#include "doctest.h"
#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"

namespace data = kd::data;
using kd::test::checked_with;
using kd::test::good;
using kd::test::kOrders;
using kd::test::kStrider;

namespace {

// The orders file with one line replaced, or the whole order when the line is empty.
std::string orders_with(const std::string& from, const std::string& to) {
    std::string text(kOrders);
    const std::size_t at = text.find(from);
    REQUIRE(at != std::string::npos);
    return text.replace(at, from.size(), to);
}

// The one problem the checks find, or a failed check when there are none or several.
std::string one(const std::vector<std::string>& found) {
    CHECK(found.size() == 1);
    return found.empty() ? std::string() : found.front();
}

}  // namespace

// checks: MAT-17
TEST_CASE("the catalogue's checks run on the whole catalogue, each named, in the order of their names") {
    data::Catalogue cat;
    const auto files = good();
    REQUIRE(cat.load(files).empty());
    CHECK(data::run_checks(cat).empty());
    REQUIRE(data::checks().size() >= 2);
    for (std::size_t i = 0; i < data::checks().size(); ++i) {
        const data::Check& c = data::checks()[i];
        CHECK(!c.serves.empty());
        CHECK(!c.about.empty());
        if (i > 0) {
            CHECK(data::checks()[i - 1].name < c.name);
        }
    }
    REQUIRE(cat.check_files().size() == 1);
    CHECK(cat.check_files()[0].path == "demo/checks/orders.toml");
}

// checks: MAT-05 MAT-17
TEST_CASE("an entry out of the real order of things is refused at its place in the order, and only it") {
    const std::string o = "demo/checks/orders.toml";
    // a strider slower than a walker breaks the order
    const std::string slow =
        one(checked_with("demo/marker/strider.toml", "speed = \"1 m/s\"\n" + std::string(kStrider).substr(16)));
    CHECK(slow ==
          o + ":4:33: least_to_most: demo:strider's speed, 1000 mm/s, is not more than demo:walker's, 1400 mm/s, "
              "against the order of things (MAT-05): a stride covers more ground than a walk");
    // and each fault in the order itself, at its place
    CHECK(one(checked_with(o, orders_with("kind = \"marker\"", "kind = \"creature\"")))
              .rfind(o + ":2:8: kind: \"creature\" is no kind's folder", 0) == 0);
    CHECK(one(checked_with(o, orders_with("field = \"speed\"", "field = \"height\"")))
              .rfind(o + ":3:9: field: \"height\" is not a field of kind marker", 0) == 0);
    CHECK(one(checked_with(o, orders_with("field = \"speed\"", "field = \"colour\"")))
              .rfind(o + ":3:9: field: \"colour\" cannot be put in order", 0) == 0);
    CHECK(one(checked_with(o, orders_with("\"demo:strider\"]", "\"demo:runner\"]")))
              .rfind(o + ":4:33: least_to_most: \"demo:runner\" names no entry of kind marker", 0) == 0);
    CHECK(one(checked_with(o, orders_with("\"demo:walker\", ", "")))
              .rfind(o + ":4:17: least_to_most: an order names two entries or more", 0) == 0);
    CHECK(one(checked_with(o, orders_with("why", "how = \"x\"\nwhy")))
              .rfind(o + ":5:7: \"how\" is not a field of this kind", 0) == 0);
    CHECK(one(checked_with(o, "colour = \"#000000\"\n")).rfind(o + ":1:10: an orders file holds only [[order]]", 0) ==
          0);
}

// checks: MAT-17
TEST_CASE("a demonstration marker that keeps company with no one is refused") {
    const std::string loner = "demo/marker/loner.toml";
    CHECK(one(checked_with(loner, kStrider)) ==
          loner +
              ":1:1: demo:loner keeps company with no one: it names no marker in walks_with and no marker names it, so "
              "it would walk the crowd alone");
    // named by another, it has company
    CHECK(checked_with(loner, std::string(kStrider) + "walks_with = [\"demo:walker\"]\n").empty());
}
