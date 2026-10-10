#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/crafting.hpp"
namespace {
std::uint32_t use_entry(const kd::data::Catalogue& c, std::string_view kind, std::string_view name) {
    const auto found = c.find(kind, name);
    REQUIRE(found);
    return found.value_or(UINT32_MAX);
}
kd::world::Knowledge use_mind(const kd::data::Catalogue& c, const kd::world::Item& tool) {
    // Labelled value fixture: this person knows butchering and has perceived an edge.
    kd::world::Knowledge mind;
    kd::world::Skill skill;
    skill.recipe = use_entry(c, "blueprint", "base:butcher");
    skill.known = 1;
    mind.skills.push_back(skill);
    kd::world::Familiar familiar;
    familiar.kind = tool.kind;
    familiar.material = tool.material;
    familiar.mask = 1U << 1U;
    familiar.values[1] = 4;
    familiar.certainty[1] = 100;
    mind.familiar.push_back(familiar);
    return mind;
}
kd::world::Item use_tool(const kd::data::Catalogue& c, std::string_view name) {
    kd::world::Item tool;
    tool.kind = use_entry(c, "item", name);
    tool.material = use_entry(c, "item", "base:flint");
    tool.mass = 20000;
    tool.length = 50;
    return tool;
}
}  // namespace
TEST_CASE("a renamed identical small tool has the same personally known downstream use value") {
    auto files = kd::data::read_folder(KD_REPO "/data");
    kd::data::Catalogue original, renamed;
    REQUIRE(original.load(files).empty());
    for (auto& file : files) {
        if (file.path == "base/item/flake.toml") file.path = "base/item/another_edge.toml";
        const std::string_view token = "result = \"flake\"";
        const auto at = file.text.find(token);
        if (at != std::string::npos) file.text.replace(at, token.size(), "result = \"another_edge\"");
    }
    REQUIRE(renamed.load(files).empty());
    const auto first = use_tool(original, "base:flake"), second = use_tool(renamed, "base:another_edge");
    const auto before = use_mind(original, first), after = use_mind(renamed, second);
    const std::array<std::int64_t, 3> needs{40, 100, 100};
    const auto a = kd::demo::Crafting::known_uses(original, before, needs, first, before.familiar.front());
    const auto b = kd::demo::Crafting::known_uses(renamed, after, needs, second, after.familiar.front());
    REQUIRE(a.size() == 1);
    REQUIRE(b.size() == 1);
    CHECK(a[0].score > 0);
    CHECK(a[0].score == b[0].score);
    CHECK(a[0].benefit == b[0].benefit);
    CHECK(a[0].role == 1);
    CHECK(a[0].need == 0);
    auto smaller = first;
    smaller.mass = 5000;
    const auto small = kd::demo::Crafting::known_uses(original, before, needs, smaller, before.familiar.front());
    REQUIRE(small.size() == 1);
    CHECK(small[0].score == a[0].score);
}
TEST_CASE("use value follows this person's evidence even when physical truth or other knowledge differs") {
    kd::data::Catalogue catalogue;
    REQUIRE(catalogue.load(kd::data::read_folder(KD_REPO "/data")).empty());
    auto tool = use_tool(catalogue, "base:flake");
    auto mind = use_mind(catalogue, tool);
    const std::array<std::int64_t, 3> needs{40, 100, 100};
    const auto value = kd::demo::Crafting::known_uses(catalogue, mind, needs, tool, mind.familiar.front());
    REQUIRE(value.size() == 1);
    tool.changed_mask = 1U << 1U;
    tool.changed[1] = 0;  // A false belief remains possible; hidden truth cannot improve the chooser.
    tool.quality = 0;
    CHECK(kd::demo::Crafting::known_uses(catalogue, mind, needs, tool, mind.familiar.front())[0].score ==
          value[0].score);
    mind.familiar.front().mask = 0;
    CHECK(kd::demo::Crafting::known_uses(catalogue, mind, needs, tool, mind.familiar.front()).empty());
    mind = use_mind(catalogue, tool);
    mind.skills.front().known = 0;
    CHECK(kd::demo::Crafting::known_uses(catalogue, mind, needs, tool, mind.familiar.front()).empty());
}
