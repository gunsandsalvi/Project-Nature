#include <string>
#include <vector>

#include "doctest.h"
#include "kd/data/catalogue.hpp"
#include "kd/demo/marker.hpp"
#include "kd/time/speeds.hpp"

namespace data = kd::data;

namespace {

const char* const kWalker =
    "speed = \"1.4 m/s\"\n"
    "reach = \"2 m\"\n"
    "greets = \"30%\"\n"
    "rest = { life = \"2 h\", game = \"2 h\" }\n"
    "colour = \"#e8c25a\"\n"
    "gait = \"walk\"\n"
    "walks_with = [\"demo:strider\"]\n";

const char* const kStrider =
    "speed = \"2 m/s\"\n"
    "reach = \"3 m\"\n"
    "greets = \"1 in 10\"\n"
    "rest = { life = \"45 min\", game = \"45 min\" }\n"
    "colour = \"#8fd18a\"\n"
    "gait = \"stride\"\n";

const char* const kTime =
    "person = \"1 min\"\nclose_camp = \"1 h\"\ncamp = \"8 h\"\nvalley = \"1 season\"\nregion = \"3 year\"\n";

std::vector<data::SourceFile> good() {
    return {
        {"demo/marker/walker.toml", kWalker}, {"demo/marker/strider.toml", kStrider}, {"base/tuning/time.toml", kTime}};
}

// The problems a catalogue finds when one file of the good ones is replaced or added.
std::vector<std::string> problems_with(const std::string& path, const std::string& text) {
    std::vector<data::SourceFile> files;
    for (data::SourceFile& f : good()) {
        if (f.path != path) {
            files.push_back(f);
        }
    }
    files.push_back({path, text});
    data::Catalogue cat;
    std::vector<std::string> out;
    for (const data::Problem& p : cat.load(files)) {
        out.push_back(data::problem_text(p));
    }
    return out;
}

// The walker with one line replaced, or added when the key is new.
std::string walker_with(const std::string& key, const std::string& line) {
    std::string text;
    bool replaced = false;
    for (std::size_t at = 0, end = 0; at < std::string(kWalker).size(); at = end + 1) {
        const std::string all(kWalker);
        end = all.find('\n', at);
        const std::string this_line = all.substr(at, end - at);
        if (this_line.rfind(key + " =", 0) == 0) {
            if (!line.empty()) {
                text += line + "\n";
            }
            replaced = true;
        } else {
            text += this_line + "\n";
        }
    }
    return replaced ? text : text + line + "\n";
}

}  // namespace

// The demonstration's markers come from their own source, demo, never the game's (MAT-16).
// checks: MAT-13 MAT-16
TEST_CASE("a catalogue loads its entries in base units, numbered by sorted name, with links resolved") {
    data::Catalogue cat;
    const auto files = good();
    REQUIRE(cat.load(files).empty());
    const auto& markers = cat.kind<kd::demo::Marker>();
    REQUIRE(markers.size() == 2);
    CHECK(markers.name(0) == "demo:strider");
    CHECK(markers.name(1) == "demo:walker");
    const kd::demo::Marker& walker = markers[1];
    CHECK(walker.speed == 1'400);
    CHECK(walker.reach == 2'000);
    CHECK(walker.rest.game == 7'200);
    CHECK(walker.gait == "walk");
    REQUIRE(walker.walks_with.size() == 1);
    CHECK(walker.walks_with[0].name == "demo:strider");
    CHECK(walker.walks_with[0].index == 0);
    const auto& speeds = cat.kind<kd::time::ZoomSpeeds>();
    REQUIRE(speeds.size() == 1);
    CHECK(speeds.name(0) == "base:time");
    CHECK(speeds[0].valley == 1'296'000);
    // and the same whatever order the files come in
    std::vector<data::SourceFile> reversed(files.rbegin(), files.rend());
    data::Catalogue again;
    REQUIRE(again.load(reversed).empty());
    CHECK(again.kind<kd::demo::Marker>().digests(1).all == markers.digests(1).all);
    // numbered by name, not by file: "demo2:alpha" comes before "demo:strider", though its file comes after
    std::vector<data::SourceFile> more = files;
    more.push_back({"demo2/marker/alpha.toml", kStrider});
    data::Catalogue wider;
    REQUIRE(wider.load(more).empty());
    CHECK(wider.kind<kd::demo::Marker>().name(0) == "demo2:alpha");
    CHECK(wider.kind<kd::demo::Marker>()[2].walks_with[0].index == 1);
}

// checks: MAT-13 MAT-17 TIM-18
TEST_CASE("each fault in an entry is refused at its file, line and column, and only it") {
    const std::string w = "demo/marker/walker.toml";
    const auto one = [&](const std::string& text) {
        const std::vector<std::string> found = problems_with(w, text);
        CHECK_MESSAGE(found.size() == 1, text);
        return found.empty() ? std::string() : found.front();
    };
    CHECK(one(walker_with("speed", "speed = \"50 m/s\"")).rfind(w + ":1:9: speed: 50000 mm/s is out of its range", 0) ==
          0);
    CHECK(one(walker_with("speed", "speed = 1400")).rfind(w + ":1:9: speed: must be a text, not a whole number", 0) ==
          0);
    CHECK(one(walker_with("reach", "reach = \"2 furlong\"")).find(":2:9: reach: \"2 furlong\": \"furlong\" is not") !=
          std::string::npos);
    CHECK(one(walker_with("greets", "")).rfind(w + ":1:1: \"greets\" is missing", 0) == 0);
    CHECK(one(walker_with("gait", "gait = \"hop\"")).find(":6:8: gait: \"hop\" is not one of") != std::string::npos);
    CHECK(one(walker_with("walks_with", "walks_with = [\"demo:runner\"]"))
              .find(":7:15: walks_with: \"demo:runner\" names no entry of kind marker") != std::string::npos);
    CHECK(one(walker_with("rest", "rest = { life = \"3 month\", game = \"30 d\" }"))
              .find(":4:8: rest: breaks the rule of the game year (TIM-18)") != std::string::npos);
    CHECK(one(walker_with("colour", "colour = 0.5")).find(":5:10: a bare decimal number") != std::string::npos);
    CHECK(one(walker_with("height", "height = \"1 m\"")).find(":8:10: \"height\" is not a field of this kind") !=
          std::string::npos);
    CHECK(problems_with("demo/creature/fox.toml", kStrider).front().find("no kind of entry lives here") !=
          std::string::npos);
    CHECK(problems_with("demo/marker/Big-One.toml", kStrider).front().find("a name is in lower case") !=
          std::string::npos);
    CHECK(problems_with(w, "speed = = 3\n").front().rfind(w + ":1:9: the TOML is broken here", 0) == 0);
}

// checks: MAT-13 MAT-14
TEST_CASE("one description serves the loader, the schema and the fingerprints") {
    data::Catalogue cat;
    const auto files = good();
    REQUIRE(cat.load(files).empty());
    const data::KindBase* markers = cat.kind_in("marker");
    REQUIRE(markers != nullptr);
    for (const char* key : {"speed", "reach", "greets", "rest", "colour", "gait", "walks_with"}) {
        CHECK(markers->schema().find(std::string("  ") + key + ": ") != std::string::npos);
    }
    CHECK(markers->display(1).find("speed = 1400 mm/s") != std::string::npos);
    CHECK(markers->display(1).find("greets = 300000 ppm") != std::string::npos);
    // a change in how it looks changes only the look's digest; in how it walks, only the rules'
    const auto digests_with = [&](const std::string& key, const std::string& line) {
        std::vector<data::SourceFile> changed = files;
        changed[0].text = walker_with(key, line);
        data::Catalogue c;
        REQUIRE(c.load(changed).empty());
        return c.kind_in("marker")->digests(1);
    };
    const data::EntryDigests before = markers->digests(1);
    const data::EntryDigests colour = digests_with("colour", "colour = \"#ffffff\"");
    CHECK(colour.all != before.all);
    CHECK(colour.by_affects[0] == before.by_affects[0]);
    CHECK(colour.by_affects[2] != before.by_affects[2]);
    const data::EntryDigests speed = digests_with("speed", "speed = \"1.5 m/s\"");
    CHECK(speed.by_affects[0] != before.by_affects[0]);
    CHECK(speed.by_affects[2] == before.by_affects[2]);
    // the length in life only checks the game's, so it changes no rule
    const data::EntryDigests months = digests_with("rest", "rest = { life = \"3 month\", game = \"15 d\" }");
    const data::EntryDigests weeks = digests_with("rest", "rest = { life = \"13 week\", game = \"15 d\" }");
    CHECK(weeks.all != months.all);
    CHECK(weeks.by_affects[0] == months.by_affects[0]);
}
