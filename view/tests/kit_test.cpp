#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "doctest.h"
#include "kd/look/model.hpp"
#include "kit.hpp"
#include "kit_assemble.hpp"

namespace kit = kd::view::kit;
namespace look = kd::look;

namespace {

// A flat square of a side in metres on the ground plane, its texture coordinates u along x and v along z scaled by
// the given amounts (1 is one texture metre to a metre): two triangles.
kit::Section square(double side, float u_scale, float v_scale, const std::string& role = "wood") {
    kit::Section s;
    s.role = role;
    const auto h = static_cast<float>(side);
    s.positions = {0, 0, 0, h, 0, 0, h, 0, h, 0, 0, h};
    s.normals = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};
    s.uvs = {0, 0, h * u_scale, 0, h * u_scale, h * v_scale, 0, h * v_scale};
    s.crease = {255, 255, 255, 255};
    s.indices = {0, 1, 2, 0, 2, 3};
    return s;
}

kit::Joint joint(const std::string& name, float x, float y, float z) {
    kit::Joint j;
    j.name = name;
    j.at = {x, y, z};
    return j;
}

kit::Part part(const std::string& name, std::array<float, 3> lowest, std::array<float, 3> highest,
               std::vector<kit::Joint> joints = {}, const std::string& role = "wood") {
    kit::Part p;
    p.name = name;
    p.lowest = lowest;
    p.highest = highest;
    p.sections.push_back(square(0.1, 1.0F, 1.0F, role));
    p.joints = std::move(joints);
    return p;
}

// The kit of a small cone tent: a cover with its apex and door, a pole with its foot, its crossing and its tip, two
// stones, and a binding.
kit::Family tent_family() {
    kit::Family f;
    f.parts.push_back(part("binding", {-0.1F, 0.0F, -0.1F}, {0.1F, 0.2F, 0.1F}, {joint("centre", 0.0F, 0.0F, 0.0F)}));
    f.parts.push_back(part("cover", {-1.9F, 0.0F, -1.9F}, {1.9F, 2.6F, 1.9F},
                           {joint("apex", 0.0F, 2.6F, 0.0F), joint("door_top", 0.0F, 1.2F, -1.2F)}, "hide"));
    f.parts.push_back(
        part("pole", {-0.03F, 0.0F, -0.03F}, {0.03F, 3.75F, 0.03F},
             {joint("foot", 0.0F, 0.0F, 0.0F), joint("bind", 0.0F, 3.27F, 0.0F), joint("tip", 0.0F, 3.75F, 0.0F)}));
    f.parts.push_back(part("stone_a", {-0.1F, 0.0F, -0.1F}, {0.1F, 0.15F, 0.1F}, {}, "stone"));
    f.parts.push_back(part("stone_b", {-0.12F, 0.0F, -0.1F}, {0.12F, 0.2F, 0.1F}, {}, "stone"));
    return f;
}

look::ModelPlace place(const std::string& name, std::vector<std::string> parts, const std::string& rule) {
    look::ModelPlace p;
    p.name = name;
    p.parts = std::move(parts);
    p.rule = rule;
    return p;
}

look::Model tent_model() {
    look::Model m;
    m.family = "camp";
    m.materials.push_back({"wood", {{"art:birch", 0, 0, 0}, {"art:pine", 0, 0, 0}}});
    m.materials.push_back({"hide", {{"art:deer_hide", 0, 0, 0}}});
    m.materials.push_back({"stone", {{"art:cobble", 0, 0, 0}}});
    m.places.push_back(place("cover", {"cover"}, "root"));
    look::ModelPlace poles = place("poles", {"pole"}, "span");
    poles.count = 10;
    poles.joint = "foot";
    poles.to_joint = "bind";
    poles.radius = 1850;
    poles.height = 2700;
    poles.jitter_radius = 20'000;  // 2%
    poles.jitter_size = 100'000;   // 10%
    m.places.push_back(poles);
    look::ModelPlace stones = place("stones", {"stone_a", "stone_b"}, "ring");
    stones.count = 24;
    stones.radius = 2100;
    stones.jitter_radius = 60'000;
    stones.jitter_turn = 3;
    m.places.push_back(stones);
    look::ModelPlace binding = place("binding", {"binding"}, "plug");
    binding.joint = "centre";
    binding.onto = "cover.apex";
    m.places.push_back(binding);
    return m;
}

std::array<double, 3> joint_at(const kit::Family& family, const kit::Placed& placed, const std::string& name) {
    return kit::carried(placed, family.part(placed.part)->joint(name)->at);
}

double reach(const std::array<double, 3>& p) {
    return std::sqrt(p[0] * p[0] + p[2] * p[2]);
}

bool names(const std::vector<std::string>& problems, const std::string& part) {
    return std::any_of(problems.begin(), problems.end(),
                       [&](const std::string& p) { return p.find(part) != std::string::npos; });
}

}  // namespace

// checks: PRE-46
TEST_CASE("a family written as a kit file reads back as it was, its parts in order of their names") {
    kit::Family f = tent_family();
    f.parts[1].sections.push_back(square(0.5, 1.0F, 1.0F, "bark"));
    f.parts[1].joints[0].rotation = {0, 1, 0, -1, 0, 0, 0, 0, 1};
    const std::vector<std::uint8_t> bytes = kit::write_family(f);
    const kit::Read read = kit::read_family(bytes);
    REQUIRE(read.problem.empty());
    CHECK(read.family == f);
    CHECK(read.family.part("pole") != nullptr);
    CHECK(read.family.part("pole")->joint("bind") != nullptr);
    CHECK(read.family.part("nothing") == nullptr);
    CHECK(read.family.part("cover")->triangles() == 4);
}

// checks: PRE-46
TEST_CASE("a kit file cut anywhere, of another version, out of order or with an index past its vertices is refused") {
    const kit::Family f = tent_family();
    const std::vector<std::uint8_t> bytes = kit::write_family(f);
    // cut short at every length: never read, never a crash
    for (std::size_t cut = 0; cut < bytes.size(); ++cut) {
        const kit::Read read = kit::read_family(std::span<const std::uint8_t>(bytes.data(), cut));
        CHECK_FALSE(read.problem.empty());
        CHECK(read.family.parts.empty());
    }
    std::vector<std::uint8_t> other = bytes;
    other[4] = 9;
    CHECK(kit::read_family(other).problem == "a kit file of another version");
    std::vector<std::uint8_t> not_one = bytes;
    not_one[0] = 'X';
    CHECK(kit::read_family(not_one).problem == "not a kit file");
    std::vector<std::uint8_t> extra = bytes;
    extra.push_back(0);
    CHECK(kit::read_family(extra).problem == "a kit file with bytes after its last part");
    kit::Family wrong = f;
    std::swap(wrong.parts[0], wrong.parts[1]);
    CHECK(kit::read_family(kit::write_family(wrong)).problem.find("out of order") != std::string::npos);
    kit::Family past = f;
    past.parts[0].sections[0].indices[2] = 9;
    CHECK(kit::read_family(kit::write_family(past)).problem.find("past its vertices") != std::string::npos);
    kit::Family twice = f;
    twice.parts[2].joints.push_back(joint("foot", 1.0F, 0.0F, 0.0F));
    CHECK(kit::read_family(kit::write_family(twice)).problem.find("twice") != std::string::npos);
    kit::Family bad = f;
    bad.parts[0].name = "Binding";
    CHECK_FALSE(kit::read_family(kit::write_family(bad)).problem.empty());
}

// checks: PRE-22 PRE-46
TEST_CASE("a texture pixel's stretch on a triangle is the ratio of the longest way it is carried to the shortest") {
    kit::Part p;
    p.name = "square";
    // one texture metre to a metre: no stretch
    p.sections.push_back(square(1.0, 1.0F, 1.0F));
    // twice as many texture metres along u: each texture pixel carried half as long that way, so 2 to 1
    p.sections.push_back(square(1.0, 2.0F, 1.0F, "stone"));
    // a mirrored texture is no stretch
    p.sections.push_back(square(1.0, -1.0F, 1.0F, "bark"));
    // a sheared one: u runs along the diagonal
    kit::Section sheared = square(1.0, 1.0F, 1.0F, "hide");
    sheared.uvs = {0, 0, 1, 0, 2, 1, 1, 1};
    p.sections.push_back(sheared);
    const std::vector<double> each = kit::stretches(p);
    REQUIRE(each.size() == 8);
    CHECK(each[0] == doctest::Approx(1.0));
    CHECK(each[1] == doctest::Approx(1.0));
    CHECK(each[2] == doctest::Approx(2.0));
    CHECK(each[3] == doctest::Approx(2.0));
    CHECK(each[4] == doctest::Approx(1.0));
    CHECK(each[5] == doctest::Approx(1.0));
    // a unit shear takes a round pixel to an ellipse whose axes are as the golden ratio's square
    CHECK(each[6] == doctest::Approx((3.0 + std::sqrt(5.0)) / 2.0).epsilon(1e-9));
    CHECK(each[7] == doctest::Approx((3.0 + std::sqrt(5.0)) / 2.0).epsilon(1e-9));
}

// checks: PRE-22 PRE-46
TEST_CASE(
    "a triangle with an area on the surface and none in its texture stretches without end, and one with no "
    "area stretches nothing") {
    kit::Part p;
    p.name = "odd";
    kit::Section s = square(1.0, 1.0F, 1.0F);
    s.uvs = {0, 0, 1, 1, 2, 2, 0, 0};  // the first triangle's texture coordinates lie on a line
    p.sections.push_back(s);
    kit::Section flat = square(1.0, 1.0F, 1.0F, "stone");
    flat.positions = {0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 1};  // the first triangle's corners lie on a line
    p.sections.push_back(flat);
    const std::vector<double> each = kit::stretches(p);
    REQUIRE(each.size() == 4);
    CHECK(each[0] > 1e6);
    CHECK(each[2] == 0.0);
}

// checks: PRE-22 PRE-46
TEST_CASE("the stretch check lists every triangle over the line, worst first, and the worst of all") {
    kit::Family f;
    kit::Part a;
    a.name = "a";
    a.sections.push_back(square(1.0, 1.0F, 1.0F));
    a.sections.push_back(square(1.0, 1.4F, 1.0F, "stone"));
    a.sections.push_back(square(1.0, 3.0F, 1.0F, "bark"));
    kit::Part b;
    b.name = "b";
    b.sections.push_back(square(1.0, 1.0F, 2.0F));
    f.parts = {a, b};
    const kit::StretchReport report = kit::check_stretch(f);
    CHECK(report.triangles == 8);
    REQUIRE(report.over.size() == 4);
    CHECK(report.over[0].part == "a");
    CHECK(report.over[0].role == "bark");
    CHECK(report.over[0].stretch == doctest::Approx(3.0));
    CHECK(report.over[2].part == "b");
    CHECK(report.over[2].stretch == doctest::Approx(2.0));
    CHECK(report.worst == doctest::Approx(3.0));
    // at a line of 3.5 nothing is over
    CHECK(kit::check_stretch(f, 3.5).over.empty());
}

// checks: PRE-46 PRE-42
TEST_CASE(
    "a cone tent's parts go where its recipe says: poles from the ring to the meeting point, stones round the "
    "ring, and the binding plugged into the apex") {
    const kit::Family f = tent_family();
    const look::Model m = tent_model();
    const kit::Assembly tent = kit::assemble(m, f, 7);
    REQUIRE(tent.problem.empty());
    REQUIRE(tent.placed.size() == 1 + 10 + 24 + 1);
    // every pole's foot is on its circle and its crossing is at the same point of the axis, 2.7 m up
    int poles = 0;
    for (const kit::Placed& p : tent.placed) {
        if (p.place != "poles") {
            continue;
        }
        ++poles;
        const std::array<double, 3> foot = joint_at(f, p, "foot");
        const std::array<double, 3> bind = joint_at(f, p, "bind");
        CHECK(foot[1] == doctest::Approx(0.0).epsilon(1e-9));
        CHECK(reach(foot) == doctest::Approx(1.85).epsilon(0.021));
        CHECK(reach(bind) < 1e-9);
        CHECK(bind[1] == doctest::Approx(2.7).epsilon(1e-9));
        // and the tip is past the crossing, on the far side of the axis
        const std::array<double, 3> tip = joint_at(f, p, "tip");
        CHECK(tip[1] > bind[1]);
    }
    CHECK(poles == 10);
    // the stones lie on a ring of about 2.1 m, in two shapes, and sit on the ground
    int stones = 0;
    int first_kind = 0;
    for (const kit::Placed& p : tent.placed) {
        if (p.place != "stones") {
            continue;
        }
        ++stones;
        first_kind += p.part == "stone_a" ? 1 : 0;
        const std::array<double, 3> at = kit::carried(p, {0.0F, 0.0F, 0.0F});
        CHECK(reach(at) == doctest::Approx(2.1).epsilon(0.061));
        CHECK(at[1] == doctest::Approx(0.0).epsilon(1e-9));
    }
    CHECK(stones == 24);
    CHECK(first_kind > 0);
    CHECK(first_kind < 24);
    // the binding's joint is where the cover's apex is
    const kit::Placed& cover = tent.placed.front();
    const kit::Placed& binding = tent.placed.back();
    REQUIRE(binding.place == "binding");
    const std::array<double, 3> apex = joint_at(f, cover, "apex");
    const std::array<double, 3> centre = joint_at(f, binding, "centre");
    for (std::size_t i = 0; i < 3; ++i) {
        CHECK(centre[i] == doctest::Approx(apex[i]).epsilon(1e-9));
    }
    // the thing's bounds hold its parts: the stones' ring, and the poles' tips above the cover
    CHECK(tent.highest[1] > 3.0);
    CHECK(tent.lowest[1] > -0.1);
    CHECK(tent.lowest[1] < 0.01);
    CHECK(tent.highest[0] > 2.0);
}

// checks: PRE-46 PRE-42 TIM-16
TEST_CASE(
    "the same seed makes the same thing, another seed another: its stones, its poles' thickness and what its "
    "roles wear") {
    const kit::Family f = tent_family();
    const look::Model m = tent_model();
    const kit::Assembly a = kit::assemble(m, f, 11);
    const kit::Assembly b = kit::assemble(m, f, 11);
    const kit::Assembly c = kit::assemble(m, f, 12);
    REQUIRE(a.placed.size() == b.placed.size());
    for (std::size_t i = 0; i < a.placed.size(); ++i) {
        CHECK(a.placed[i].part == b.placed[i].part);
        CHECK(a.placed[i].matrix == b.placed[i].matrix);
    }
    CHECK(a.wears == b.wears);
    bool differs = false;
    for (std::size_t i = 0; i < a.placed.size(); ++i) {
        differs = differs || a.placed[i].matrix != c.placed[i].matrix || a.placed[i].part != c.placed[i].part;
    }
    CHECK(differs);
    // each role wears one of its textures; across seeds both of wood's appear
    bool birch = false;
    bool pine = false;
    for (std::uint64_t seed = 0; seed < 20; ++seed) {
        for (const auto& [role, texture] : kit::assemble(m, f, seed).wears) {
            if (role == "wood") {
                birch = birch || texture == "art:birch";
                pine = pine || texture == "art:pine";
            } else {
                CHECK((texture == "art:deer_hide" || texture == "art:cobble"));
            }
        }
    }
    CHECK(birch);
    CHECK(pine);
}

// checks: PRE-46
TEST_CASE("a root is turned, leaned and lifted, and a ring turns each part to face outward") {
    kit::Family f;
    f.parts.push_back(part("stick", {0.0F, 0.0F, 0.0F}, {1.0F, 0.1F, 0.1F}, {joint("end", 1.0F, 0.0F, 0.0F)}));
    look::Model m;
    m.family = "camp";
    look::ModelPlace root = place("stick", {"stick"}, "root");
    root.turn = 90;
    root.height = 500;
    m.places.push_back(root);
    const kit::Assembly lying = kit::assemble(m, f, 1);
    REQUIRE(lying.problem.empty());
    // turned 90 degrees clockwise from north: the stick's own -z, its forward, points east; its +x, sideways, then
    // points to the south, and it is lifted half a metre
    const std::array<double, 3> end = kit::carried(lying.placed[0], {1.0F, 0.0F, 0.0F});
    CHECK(end[0] == doctest::Approx(0.0).epsilon(1e-9));
    CHECK(end[1] == doctest::Approx(0.5).epsilon(1e-9));
    CHECK(end[2] == doctest::Approx(1.0).epsilon(1e-9));
    // the same stick set by its end joint has that end at the origin
    m.places[0].joint = "end";
    const kit::Assembly by_end = kit::assemble(m, f, 1);
    const std::array<double, 3> at = kit::carried(by_end.placed[0], {1.0F, 0.0F, 0.0F});
    CHECK(at[0] == doctest::Approx(0.0).epsilon(1e-9));
    CHECK(at[2] == doctest::Approx(0.0).epsilon(1e-9));
    // a ring of four: the first copy to the north, its forward pointing north too; the second to the east
    look::Model ring;
    ring.family = "camp";
    look::ModelPlace round = place("round", {"stick"}, "ring");
    round.count = 4;
    round.radius = 2000;
    ring.places.push_back(round);
    const kit::Assembly four = kit::assemble(ring, f, 1);
    REQUIRE(four.placed.size() == 4);
    const std::array<double, 3> north = kit::carried(four.placed[0], {0.0F, 0.0F, 0.0F});
    CHECK(north[0] == doctest::Approx(0.0).epsilon(1e-9));
    CHECK(north[2] == doctest::Approx(-2.0).epsilon(1e-9));
    const std::array<double, 3> east = kit::carried(four.placed[1], {0.0F, 0.0F, 0.0F});
    CHECK(east[0] == doctest::Approx(2.0).epsilon(1e-9));
    CHECK(east[2] == doctest::Approx(0.0).epsilon(1e-9));
    // the second copy's own forward (-z) points east, away from the middle
    const std::array<double, 3> forward = kit::carried(four.placed[1], {0.0F, 0.0F, -1.0F});
    CHECK(forward[0] == doctest::Approx(3.0).epsilon(1e-9));
}

// checks: PRE-46 PRE-22
TEST_CASE("a recipe that fits its parts passes the kit's check, and each way it can fail is named") {
    const kit::Family f = tent_family();
    const look::Model good = tent_model();
    CHECK(kit::check_model("art:tent", good, f).empty());

    look::Model missing_part = good;
    missing_part.places[2].parts.push_back("boulder");
    CHECK(names(kit::check_model("art:tent", missing_part, f),
                "\"boulder\" of the placement \"stones\" is not in the family"));

    look::Model missing_joint = good;
    missing_joint.places[1].to_joint = "crossing";
    CHECK(names(kit::check_model("art:tent", missing_joint, f), "no joint \"crossing\""));

    look::Model bad_plug = good;
    bad_plug.places[3].onto = "cover.hinge";
    CHECK(names(kit::check_model("art:tent", bad_plug, f), "no joint \"hinge\""));

    // poles 40% too short for the span they must cross
    look::Model far_span = good;
    far_span.places[1].height = 4000;
    CHECK(names(kit::check_model("art:tent", far_span, f), "stretched past 15%"));

    // a role its parts wear that the recipe gives no texture
    look::Model unclothed = good;
    unclothed.materials.pop_back();
    CHECK(names(kit::check_model("art:tent", unclothed, f), "the role \"stone\""));
}

// checks: PRE-46
TEST_CASE("a recipe the assembler cannot put together gives its reason and places nothing") {
    const kit::Family f = tent_family();
    look::Model m = tent_model();
    m.places[3].onto = "roof.apex";
    const kit::Assembly bad = kit::assemble(m, f, 1);
    CHECK(bad.placed.empty());
    CHECK(bad.problem.find("not placed before it") != std::string::npos);
    look::Model nothing = tent_model();
    nothing.places[0].parts.clear();
    CHECK_FALSE(kit::assemble(nothing, f, 1).problem.empty());
}
