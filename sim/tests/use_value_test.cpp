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
#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/choice.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/save/snapshot.hpp"
namespace {
const kd::data::Catalogue& prerequisite_catalogue() {
    static const auto catalogue = [] {
        kd::data::Catalogue out;
        KD_CHECK(out.load(kd::data::read_folder(KD_REPO "/data")).empty(), "Trial catalogue loads");
        return out;
    }();
    return catalogue;
}
struct PrerequisiteFixture final : kd::world::System {
    kd::demo::CrowdWorld camp{9001, prerequisite_catalogue(), 1, true, true, true};
    kd::ecs::Id person{}, home{};
    std::vector<kd::world::CraftReason> reasons;
    bool visible = true, commit = false;
    PrerequisiteFixture() {
        auto& w = camp.world();
        home = camp.camp_ids().front();
        std::vector<kd::ecs::Id> others;
        w.beings().each([&](auto id, auto h) {
            if (!w.beings().raw().template all_of<kd::world::Person>(h)) return;
            if (!person.value)
                person = id;
            else
                others.push_back(id);
        });
        for (auto id : others) {
            for (std::uint32_t slot = 0; slot < 4; ++slot) w.cancel(id, slot);
            w.beings().end(id);
        }
        auto& raw = w.beings().raw();
        const auto h = w.beings().handle(person);
        auto& life = raw.get<kd::world::Life>(h);
        life.food = 0;
        life.water = 3000;
        life.awake = 0;
        life.gathering_skill = 0;  // Declared no-known-remedy fixture, not hidden extra material.
        auto& mind = raw.get<kd::world::Knowledge>(h);
        mind.skills.clear();
        mind.familiar.clear();
        mind.curiosity_need = 100;
        mind.hourly_draw = 0;
        w.set_command_taker(*this);
    }
    std::string_view name() const override { return "declared prerequisite question"; }
    void handle(kd::world::Context&, const kd::event::Event&) override {}
    void command(kd::world::Context& c, const kd::world::Command&) override {
        auto& w = c.world();
        c.touch(person);
        c.touch(home);
        const auto h = w.beings().handle(person);
        const auto here = w.beings().raw().get<kd::world::Place>(h).at;
        w.things().each([&](auto id, auto item) {
            if (w.things().raw().template all_of<kd::world::Fire>(item)) return;
            w.things().raw().template get<kd::world::Place>(item).at =
                visible ? here : w.torus().moved(here, {4000, 4000});
            c.item_changed(id);
        });
        kd::demo::ChoiceSet options;
        const auto* living = camp.living();
        KD_CHECK(living, "Trial camp has Living");
        auto& life = w.beings().raw().get<kd::world::Life>(h);
        life.decision_needs = kd::demo::Living::needs(life);
        life.unavailable.fill(0);
        life.unavailable[0] = life.gathering_skill ? 0 : 4;
        life.scores = {life.gathering_skill ? 10000 : -1000000, 0, 0, 0};
        for (std::uint8_t goal = 0; goal < 4; ++goal)
            options.add(kd::demo::Choices::body(life, goal), [](std::uint64_t) { return false; });
        kd::demo::Crafting::choose(*const_cast<kd::demo::Living*>(living), c, h, &options);
        reasons.assign(options.reasons().begin(), options.reasons().end());
        if (commit) (void)options.commit(c, h);
    }
    void choose() {
        auto& w = camp.world();
        (void)w.command(w.frontier(), 940, person.value, 0);
        w.run_to(w.frontier() + 1);
    }
    kd::world::Knowledge& mind() {
        auto& w = camp.world();
        return w.beings().raw().get<kd::world::Knowledge>(w.beings().handle(person));
    }
    std::vector<kd::world::CraftReason> questions() const {
        std::vector<kd::world::CraftReason> out;
        for (const auto& reason : reasons)
            if (reason.kind == 1 && reason.need != 3) out.push_back(reason);
        return out;
    }
};
}  // namespace
TEST_CASE("unanswered need exploration asks one saved finite question without installing an answer") {
    PrerequisiteFixture f;
    f.choose();
    const auto questions = f.questions();
    REQUIRE(questions.size() == 1);
    if (questions.size() != 1) return;
    const auto& question = questions.front();
    CHECK(question.need == 0);
    CHECK_FALSE(question.intended);
    CHECK(question.confidence == 0);
    CHECK(question.recipe == kd::world::kNoRecipe);
    CHECK(question.seconds <= 600);
    CHECK(question.seconds >= 300);
    CHECK(question.inputs.size() <= 2);
    CHECK_FALSE(question.inputs.empty());
    CHECK(f.mind().skills.empty());
    CHECK(f.mind().hourly_draw == 1);
    std::string why;
    const auto copy = kd::demo::CrowdWorld::open(prerequisite_catalogue(), f.camp.world().save(), why);
    INFO(why);
    REQUIRE(copy);
    if (!copy) return;
    CHECK(copy->world().digests().whole == f.camp.world().digests().whole);
    f.choose();
    CHECK(f.questions().empty());
}
TEST_CASE("unanswered need exploration protects a usable urgent remedy and cannot invent distant inputs") {
    for (const bool usable : {false, true}) {
        PrerequisiteFixture f;
        auto& w = f.camp.world();
        if (usable) {
            auto& life = w.beings().raw().get<kd::world::Life>(w.beings().handle(f.person));
            life.gathering_skill = 1;
            life.source[0] = 1;
        } else
            f.visible = false;
        f.choose();
        CHECK(f.questions().empty());
        CHECK(f.mind().hourly_draw == 0);
    }
}
TEST_CASE("unanswered questions use personal evidence rather than hidden burning values") {
    PrerequisiteFixture first, second;
    second.camp.world().things().each([&](auto, auto h) {
        auto& item = second.camp.world().things().raw().template get<kd::world::Item>(h);
        if (second.camp.world().things().raw().template all_of<kd::world::Fire>(h)) return;
        item.changed_mask |= 1U << 6U;
        item.changed[6] = 0;
    });
    first.choose();
    second.choose();
    const auto a = first.questions(), b = second.questions();
    REQUIRE(a.size() == 1);
    REQUIRE(b.size() == 1);
    if (a.size() != 1 || b.size() != 1) return;
    CHECK(a.front().action == b.front().action);
    CHECK(a.front().score == b.front().score);
    CHECK(a.front().inputs == b.front().inputs);
}

TEST_CASE("retained actual trial memories bound repeating a question regardless of input order") {
    PrerequisiteFixture f;
    f.choose();
    const auto questions = f.questions();
    REQUIRE(questions.size() == 1);
    if (questions.size() != 1) return;
    const auto& question = questions.front();
    // Declared past-trial state uses the actual candidate's personal evidence;
    // the fixture never installs a successful result or recipe.
    for (int n = 0; n < 3; ++n) {
        kd::world::Memory memory;
        memory.id = f.mind().next_memory++;
        memory.at = f.camp.world().frontier();
        memory.action = question.action;
        for (const auto& link : question.inputs) {
            const auto h = f.camp.world().things().handle(link.id);
            const auto& item = f.camp.world().things().raw().get<kd::world::Item>(h);
            const auto* familiar = kd::demo::Discovery::familiar(f.mind(), item);
            REQUIRE(familiar);
            if (!familiar) return;
            memory.inputs.push_back(*familiar);
        }
        std::reverse(memory.inputs.begin(), memory.inputs.end());
        f.mind().memories.push_back(std::move(memory));
    }
    f.mind().hourly_draw = 0;  // Declared pre-proposal state in the same keyed hour.
    f.choose();
    CHECK(f.questions().empty());
    CHECK(f.mind().skills.empty());
}
