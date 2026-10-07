#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "doctest.h"
#include "poser.hpp"

namespace view = kd::view;

namespace {

using Vec = std::array<float, 3>;

Vec point(const std::vector<float>& xyz, std::size_t i) {
    return {xyz[i * 3], xyz[i * 3 + 1], xyz[i * 3 + 2]};
}

// A bone's skinning matrix applied to a point, as three rows.
Vec carried(const view::BoneRows& rows, const Vec& p) {
    Vec out{};
    for (std::size_t r = 0; r < 3; ++r) {
        out[r] = rows.m[r][0] * p[0] + rows.m[r][1] * p[1] + rows.m[r][2] * p[2] + rows.m[r][3];
    }
    return out;
}

float distance(const Vec& a, const Vec& b) {
    return std::sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]));
}

}  // namespace

// checks: PRE-27
TEST_CASE("the stand-in figure has 24 bones and about 1,500 triangles, each vertex's weights adding to one") {
    CHECK(view::figure_bones().size() == 24);
    const view::FigureMesh mesh = view::figure_mesh();
    const std::size_t vertices = mesh.positions.size() / 3;
    CHECK(mesh.indices.size() / 3 >= 1'400);
    CHECK(mesh.indices.size() / 3 <= 1'600);
    REQUIRE(mesh.bones.size() == vertices * 4);
    REQUIRE(mesh.weights.size() == vertices * 4);
    for (std::size_t v = 0; v < vertices; ++v) {
        float sum = 0.0F;
        for (std::size_t k = 0; k < 4; ++k) {
            sum += mesh.weights[v * 4 + k];
            CHECK(mesh.bones[v * 4 + k] >= 0);
            CHECK(mesh.bones[v * 4 + k] < 24);
        }
        CHECK(std::fabs(sum - 1.0F) < 1e-6F);
    }
    // a standing adult about 1.7 to 1.8 m tall, feet on the ground
    float low = 10.0F;
    float high = -10.0F;
    for (std::size_t v = 0; v < vertices; ++v) {
        low = std::fmin(low, mesh.positions[v * 3 + 1]);
        high = std::fmax(high, mesh.positions[v * 3 + 1]);
    }
    CHECK(std::fabs(low) < 0.03F);
    CHECK(high > 1.65F);
    CHECK(high < 1.85F);
}

// checks: PRE-27
TEST_CASE("every triangle of the figure winds clockwise seen from outside, as Godot draws a front face") {
    const view::FigureMesh mesh = view::figure_mesh();
    for (std::size_t t = 0; t < mesh.indices.size(); t += 3) {
        const auto a = static_cast<std::size_t>(mesh.indices[t]);
        const auto b = static_cast<std::size_t>(mesh.indices[t + 1]);
        const auto c = static_cast<std::size_t>(mesh.indices[t + 2]);
        const Vec pa = point(mesh.positions, a);
        const Vec pb = point(mesh.positions, b);
        const Vec pc = point(mesh.positions, c);
        const Vec ab{pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2]};
        const Vec ac{pc[0] - pa[0], pc[1] - pa[1], pc[2] - pa[2]};
        const Vec turn{ab[1] * ac[2] - ab[2] * ac[1], ab[2] * ac[0] - ab[0] * ac[2], ab[0] * ac[1] - ab[1] * ac[0]};
        Vec out{};
        for (const std::size_t i : {a, b, c}) {
            const Vec n = point(mesh.normals, i);
            out = {out[0] + n[0], out[1] + n[1], out[2] + n[2]};
        }
        CHECK(turn[0] * out[0] + turn[1] * out[1] + turn[2] * out[2] < 0.0F);
    }
}

// checks: PRE-27, PRE-44
TEST_CASE("a figure takes a new pose ten times a second, its steps offset by its seed") {
    CHECK(view::pose_step(7, 0.35) + 1 == view::pose_step(7, 0.45));
    CHECK(view::pose_step(7, 1.0) - view::pose_step(7, 0.0) == 10);
    // across 40 figures the steps fall at different moments
    int changing = 0;
    for (std::uint64_t seed = 0; seed < 40; ++seed) {
        changing += view::pose_step(seed, 0.5) != view::pose_step(seed, 0.52) ? 1 : 0;
    }
    CHECK(changing > 2);
    CHECK(changing < 20);
}

// checks: PRE-27
TEST_CASE("a pose's skinning matrices carry each bone from where it rests to where it is posed") {
    for (const view::BoneRows& rows : view::rest_pose()) {
        CHECK(rows.m[0][0] == 1.0F);
        CHECK(rows.m[1][1] == 1.0F);
        CHECK(rows.m[2][3] == 0.0F);
    }
    const std::vector<view::Bone>& bones = view::figure_bones();
    std::vector<Vec> rest(bones.size());
    for (std::size_t b = 0; b < bones.size(); ++b) {
        rest[b] = bones[b].parent < 0 ? bones[b].at
                                      : Vec{rest[static_cast<std::size_t>(bones[b].parent)][0] + bones[b].at[0],
                                            rest[static_cast<std::size_t>(bones[b].parent)][1] + bones[b].at[1],
                                            rest[static_cast<std::size_t>(bones[b].parent)][2] + bones[b].at[2]};
    }
    const std::vector<view::BoneRows> pose = view::figure_pose(5, 13);
    const std::vector<std::array<float, 3>> places = view::bone_places(5, 13);
    for (std::size_t b = 0; b < bones.size(); ++b) {
        CHECK(distance(carried(pose[b], rest[b]), places[b]) < 1e-5F);
    }
}

// checks: PRE-27
TEST_CASE("a half float keeps a number to the nearest of its own, ties to the even") {
    CHECK(view::half_bits(0.0F) == 0x0000);
    CHECK(view::half_bits(-0.0F) == 0x8000);
    CHECK(view::half_bits(1.0F) == 0x3C00);
    CHECK(view::half_bits(-2.0F) == 0xC000);
    CHECK(view::half_bits(0.5F) == 0x3800);
    CHECK(view::half_bits(65504.0F) == 0x7BFF);
    // past the largest half, infinity
    CHECK(view::half_bits(65520.0F) == 0x7C00);
    CHECK(view::half_bits(1.0e6F) == 0x7C00);
    CHECK(view::half_bits(-1.0e6F) == 0xFC00);
    // halfway between two halves: the even one; just past halfway: the nearer
    CHECK(view::half_bits(1.0F + 1.0F / 2048.0F) == 0x3C00);
    CHECK(view::half_bits(1.0F + 3.0F / 2048.0F) == 0x3C02);
    CHECK(view::half_bits(1.0F + 1.0F / 2048.0F + 1.0F / 65536.0F) == 0x3C01);
    // the least normal half, the least subnormal, and under half of it
    CHECK(view::half_bits(1.0F / 16384.0F) == 0x0400);
    CHECK(view::half_bits(1.0F / 16'777'216.0F) == 0x0001);
    CHECK(view::half_bits(1.0F / 67'108'864.0F) == 0x0000);
}

// checks: PRE-27
TEST_CASE("a figure's palette row holds each bone's three rows, four half floats a texel") {
    const std::vector<std::uint16_t> rest = view::palette_row(view::rest_pose());
    REQUIRE(rest.size() == 24 * 3 * 4);
    for (std::size_t bone = 0; bone < 24; ++bone) {
        for (std::size_t row = 0; row < 3; ++row) {
            for (std::size_t column = 0; column < 4; ++column) {
                CHECK(rest[(bone * 3 + row) * 4 + column] == (row == column ? 0x3C00 : 0x0000));
            }
        }
    }
    const std::vector<view::BoneRows> pose = view::figure_pose(5, 13);
    const std::vector<std::uint16_t> row = view::palette_row(pose);
    CHECK(row[(18 * 3 + 1) * 4 + 3] == view::half_bits(pose[18].m[1][3]));
}

// checks: PRE-27
TEST_CASE("the walk moves the feet and hands, keeps the feet near the ground, and differs from figure to figure") {
    const std::vector<std::array<float, 3>> now = view::bone_places(3, 0);
    const std::vector<std::array<float, 3>> later = view::bone_places(3, 5);
    // half a second later the left foot and the left hand have moved
    CHECK(distance(now[18], later[18]) > 0.05F);
    CHECK(distance(now[9], later[9]) > 0.05F);
    for (std::int64_t step = 0; step < 20; ++step) {
        for (const std::size_t foot : {18U, 22U}) {
            const float up = view::bone_places(3, step)[foot][1];
            CHECK(up > -0.05F);
            CHECK(up < 0.3F);
        }
    }
    CHECK(distance(view::bone_places(3, 0)[18], view::bone_places(4, 0)[18]) > 1e-3F);
    // and figures face their own ways
    CHECK(std::fabs(view::facing(3) - view::facing(4)) > 1e-3F);
    for (std::uint64_t seed = 0; seed < 40; ++seed) {
        CHECK(view::facing(seed) >= 0.0F);
        CHECK(view::facing(seed) < 6.2832F);
    }
}
