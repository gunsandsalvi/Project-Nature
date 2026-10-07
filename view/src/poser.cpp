#include "poser.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <utility>

#include "kd/num/maths.hpp"

namespace kd::view {

namespace {

using Vec = std::array<float, 3>;

// A rigid place: a rotation, by rows, and a translation.
struct Place {
    std::array<Vec, 3> r{{{1.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F, 1.0F}}};
    Vec t{};
};

Vec add(const Vec& a, const Vec& b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

Vec sub(const Vec& a, const Vec& b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

Vec scaled(const Vec& a, float s) {
    return {a[0] * s, a[1] * s, a[2] * s};
}

float dot(const Vec& a, const Vec& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vec cross(const Vec& a, const Vec& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

Vec unit(const Vec& a) {
    const float length = std::sqrt(dot(a, a));
    return length > 0.0F ? scaled(a, 1.0F / length) : a;
}

// The sine and cosine of an angle in radians by the project's own maths, the same on the cloud and the phone.
float sine(float radians) {
    return static_cast<float>(num::sinpi(static_cast<double>(radians) / std::numbers::pi));
}

float cosine(float radians) {
    return static_cast<float>(num::cospi(static_cast<double>(radians) / std::numbers::pi));
}

// The place a then b: b's points carried by b, then by a.
Place times(const Place& a, const Place& b) {
    Place out;
    for (std::size_t i = 0; i < 3; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            out.r[i][j] = a.r[i][0] * b.r[0][j] + a.r[i][1] * b.r[1][j] + a.r[i][2] * b.r[2][j];
        }
        out.t[i] = dot(a.r[i], b.t) + a.t[i];
    }
    return out;
}

// A turn by joint angles in radians: pitch about x, then yaw about y, then roll about z, as a joint bends.
Place turned(float pitch, float yaw, float roll) {
    const float cx = cosine(pitch);
    const float sx = sine(pitch);
    const float cy = cosine(yaw);
    const float sy = sine(yaw);
    const float cz = cosine(roll);
    const float sz = sine(roll);
    Place x;
    x.r = {{{1.0F, 0.0F, 0.0F}, {0.0F, cx, -sx}, {0.0F, sx, cx}}};
    Place y;
    y.r = {{{cy, 0.0F, sy}, {0.0F, 1.0F, 0.0F}, {-sy, 0.0F, cy}}};
    Place z;
    z.r = {{{cz, -sz, 0.0F}, {sz, cz, 0.0F}, {0.0F, 0.0F, 1.0F}}};
    return times(z, times(y, x));
}

Place moved(const Vec& t) {
    Place out;
    out.t = t;
    return out;
}

// The bones' places at rest, in the figure's metres: each its parent's and its own offset.
std::vector<Vec> rest_places() {
    const std::vector<Bone>& bones = figure_bones();
    std::vector<Vec> out(bones.size());
    for (std::size_t b = 0; b < bones.size(); ++b) {
        out[b] = bones[b].parent < 0 ? bones[b].at : add(out[static_cast<std::size_t>(bones[b].parent)], bones[b].at);
    }
    return out;
}

// A number from 0 to 1 for a seed and a salt, the same on every machine.
float fraction(std::uint64_t seed, double salt) {
    const double v = static_cast<double>(seed % 100'003U) * salt;
    return static_cast<float>(v - std::floor(v));
}

// The joints' angles of a walk at a phase, a turn for each bone, and the hips' bob in metres.
struct Walk {
    std::vector<Place> turns;
    float bob = 0.0F;
};

Walk walk_at(float phase) {
    const std::size_t count = figure_bones().size();
    std::vector<std::array<float, 3>> angles(count, {0.0F, 0.0F, 0.0F});
    const float s = sine(phase);
    const float knee = 0.5F * std::numbers::pi_v<float>;
    angles[1] = {0.0F, 0.08F * s, 0.0F};   // hips turn with the stride
    angles[2] = {0.0F, -0.05F * s, 0.0F};  // spine and chest turn back against them
    angles[3] = {0.0F, -0.06F * s, 0.0F};
    angles[5] = {0.06F * sine(2.0F * phase), 0.0F, 0.0F};  // the head nods twice a stride
    angles[7] = {0.45F * s, 0.0F, 0.0F};                   // arms swing against the legs
    angles[8] = {-0.35F - 0.2F * s, 0.0F, 0.0F};
    angles[12] = {-0.45F * s, 0.0F, 0.0F};
    angles[13] = {-0.35F + 0.2F * s, 0.0F, 0.0F};
    angles[16] = {-0.5F * s, 0.0F, 0.0F};  // legs swing, knees bend as each foot swings through
    angles[17] = {0.8F * std::max(0.0F, sine(phase + knee)), 0.0F, 0.0F};
    angles[18] = {0.25F * s, 0.0F, 0.0F};
    angles[20] = {0.5F * s, 0.0F, 0.0F};
    angles[21] = {0.8F * std::max(0.0F, sine(phase + knee + std::numbers::pi_v<float>)), 0.0F, 0.0F};
    angles[22] = {-0.25F * s, 0.0F, 0.0F};
    Walk w;
    for (const std::array<float, 3>& a : angles) {
        w.turns.push_back(turned(a[0], a[1], a[2]));
    }
    w.bob = 0.02F * cosine(2.0F * phase);
    return w;
}

// Each bone's posed place, in the figure's metres, at a pose step.
std::vector<Place> posed(std::uint64_t seed, std::int64_t step) {
    const std::vector<Bone>& bones = figure_bones();
    const double seconds = static_cast<double>(step) / kPosesASecond;
    // about 0.9 strides a second, each figure's stride offset by its seed
    const auto phase =
        static_cast<float>(2.0 * std::numbers::pi * (0.9 * seconds + static_cast<double>(fraction(seed, 0.4142))));
    const Walk w = walk_at(phase);
    std::vector<Place> out(bones.size());
    for (std::size_t b = 0; b < bones.size(); ++b) {
        Vec at = bones[b].at;
        if (b == 1) {
            at[1] += w.bob;
        }
        const Place local = times(moved(at), w.turns[b]);
        out[b] = bones[b].parent < 0 ? local : times(out[static_cast<std::size_t>(bones[b].parent)], local);
    }
    return out;
}

BoneRows rows_of(const Place& p) {
    BoneRows out;
    for (std::size_t i = 0; i < 3; ++i) {
        out.m[i] = {p.r[i][0], p.r[i][1], p.r[i][2], p.t[i]};
    }
    return out;
}

// A vertex's four bones and their weights.
struct Weights {
    std::array<std::int32_t, 4> bones{};
    std::array<float, 4> weights{};
};

// A body part's points, normals and weights added to a mesh, and its triangles turned so each faces out: Godot draws
// a triangle whose corners wind clockwise seen from outside (A4.7).
struct Builder {
    FigureMesh mesh;
    void point(const Vec& p, const Vec& n, const Weights& w) {
        mesh.positions.insert(mesh.positions.end(), p.begin(), p.end());
        mesh.normals.insert(mesh.normals.end(), n.begin(), n.end());
        mesh.bones.insert(mesh.bones.end(), w.bones.begin(), w.bones.end());
        mesh.weights.insert(mesh.weights.end(), w.weights.begin(), w.weights.end());
    }
    [[nodiscard]] Vec at(std::int32_t i) const {
        const auto k = static_cast<std::size_t>(i) * 3;
        return {mesh.positions[k], mesh.positions[k + 1], mesh.positions[k + 2]};
    }
    void triangle(std::int32_t a, std::int32_t b, std::int32_t c, const Vec& out) {
        const Vec facing = cross(sub(at(b), at(a)), sub(at(c), at(a)));
        if (dot(facing, out) > 0.0F) {
            std::swap(b, c);
        }
        mesh.indices.insert(mesh.indices.end(), {a, b, c});
    }
};

constexpr int kSides = 8;
constexpr int kRings = 4;

}  // namespace

const std::vector<Bone>& figure_bones() {
    // the figure's left is +x and it faces +z; a part's tube runs from its bone to its tail
    static const std::vector<Bone> bones = {
        {-1, {0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 0.0F},        // 0 root, on the ground
        {0, {0.0F, 0.95F, 0.0F}, {0.0F, 0.12F, 0.0F}, 0.15F},      // 1 hips
        {1, {0.0F, 0.12F, 0.0F}, {0.0F, 0.2F, 0.0F}, 0.14F},       // 2 spine
        {2, {0.0F, 0.2F, 0.0F}, {0.0F, 0.24F, 0.0F}, 0.16F},       // 3 chest
        {3, {0.0F, 0.24F, 0.0F}, {0.0F, 0.09F, 0.0F}, 0.05F},      // 4 neck
        {4, {0.0F, 0.09F, 0.0F}, {0.0F, 0.2F, 0.0F}, 0.1F},        // 5 head
        {3, {0.03F, 0.2F, 0.0F}, {0.15F, 0.0F, 0.0F}, 0.05F},      // 6 left collarbone
        {6, {0.15F, 0.0F, 0.0F}, {0.0F, -0.28F, 0.0F}, 0.05F},     // 7 left upper arm
        {7, {0.0F, -0.28F, 0.0F}, {0.0F, -0.25F, 0.0F}, 0.04F},    // 8 left forearm
        {8, {0.0F, -0.25F, 0.0F}, {0.0F, -0.08F, 0.0F}, 0.035F},   // 9 left hand
        {9, {0.0F, -0.08F, 0.0F}, {0.0F, -0.07F, 0.0F}, 0.025F},   // 10 left fingers
        {3, {-0.03F, 0.2F, 0.0F}, {-0.15F, 0.0F, 0.0F}, 0.05F},    // 11 right collarbone
        {11, {-0.15F, 0.0F, 0.0F}, {0.0F, -0.28F, 0.0F}, 0.05F},   // 12 right upper arm
        {12, {0.0F, -0.28F, 0.0F}, {0.0F, -0.25F, 0.0F}, 0.04F},   // 13 right forearm
        {13, {0.0F, -0.25F, 0.0F}, {0.0F, -0.08F, 0.0F}, 0.035F},  // 14 right hand
        {14, {0.0F, -0.08F, 0.0F}, {0.0F, -0.07F, 0.0F}, 0.025F},  // 15 right fingers
        {1, {0.09F, -0.02F, 0.0F}, {0.0F, -0.43F, 0.0F}, 0.07F},   // 16 left thigh
        {16, {0.0F, -0.43F, 0.0F}, {0.0F, -0.43F, 0.0F}, 0.05F},   // 17 left shin
        {17, {0.0F, -0.43F, 0.0F}, {0.0F, -0.04F, 0.13F}, 0.04F},  // 18 left foot
        {18, {0.0F, -0.04F, 0.13F}, {0.0F, 0.0F, 0.06F}, 0.03F},   // 19 left toes
        {1, {-0.09F, -0.02F, 0.0F}, {0.0F, -0.43F, 0.0F}, 0.07F},  // 20 right thigh
        {20, {0.0F, -0.43F, 0.0F}, {0.0F, -0.43F, 0.0F}, 0.05F},   // 21 right shin
        {21, {0.0F, -0.43F, 0.0F}, {0.0F, -0.04F, 0.13F}, 0.04F},  // 22 right foot
        {22, {0.0F, -0.04F, 0.13F}, {0.0F, 0.0F, 0.06F}, 0.03F},   // 23 right toes
    };
    return bones;
}

FigureMesh figure_mesh() {
    const std::vector<Bone>& bones = figure_bones();
    const std::vector<Vec> rest = rest_places();
    Builder build;
    for (std::size_t b = 0; b < bones.size(); ++b) {
        const Bone& bone = bones[b];
        if (bone.radius <= 0.0F) {
            continue;
        }
        const float length = std::sqrt(dot(bone.tail, bone.tail));
        const Vec along = unit(bone.tail);
        const Vec side = unit(std::fabs(along[1]) < 0.9F ? cross(along, Vec{0.0F, 1.0F, 0.0F})
                                                         : cross(along, Vec{1.0F, 0.0F, 0.0F}));
        const Vec up = cross(along, side);
        // near its joint a part follows its bone and its parent's half each, so it bends smoothly; the hips follow
        // their own bone alone, the root having no part
        Weights own;
        own.bones = {static_cast<std::int32_t>(b), 0, 0, 0};
        own.weights = {1.0F, 0.0F, 0.0F, 0.0F};
        Weights joint = own;
        if (bones[static_cast<std::size_t>(bone.parent)].radius > 0.0F) {
            joint.bones = {static_cast<std::int32_t>(b), bone.parent, 0, 0};
            joint.weights = {0.5F, 0.5F, 0.0F, 0.0F};
        }
        const auto first = static_cast<std::int32_t>(build.mesh.positions.size() / 3);
        for (int k = 0; k < kRings; ++k) {
            const float s = static_cast<float>(k) / static_cast<float>(kRings - 1);
            const float radius = bone.radius * (1.0F - 0.15F * s);
            const Vec centre = add(rest[b], scaled(along, s * length));
            for (int j = 0; j < kSides; ++j) {
                const float angle = 2.0F * std::numbers::pi_v<float> * static_cast<float>(j) / kSides;
                const Vec out = add(scaled(side, cosine(angle)), scaled(up, sine(angle)));
                build.point(add(centre, scaled(out, radius)), out, k == 0 ? joint : own);
            }
        }
        const std::int32_t foot = first + kRings * kSides;
        build.point(rest[b], scaled(along, -1.0F), joint);
        build.point(add(rest[b], scaled(along, length)), along, own);
        for (int k = 0; k + 1 < kRings; ++k) {
            for (int j = 0; j < kSides; ++j) {
                const std::int32_t a = first + k * kSides + j;
                const std::int32_t n = first + k * kSides + (j + 1) % kSides;
                const std::int32_t c = a + kSides;
                const std::int32_t e = n + kSides;
                const Vec mid = scaled(add(add(build.at(a), build.at(n)), add(build.at(c), build.at(e))), 0.25F);
                const Vec axis = add(rest[b], scaled(along, dot(sub(mid, rest[b]), along)));
                build.triangle(a, n, c, sub(mid, axis));
                build.triangle(n, e, c, sub(mid, axis));
            }
        }
        for (int j = 0; j < kSides; ++j) {
            const int next = (j + 1) % kSides;
            build.triangle(foot, first + j, first + next, scaled(along, -1.0F));
            const std::int32_t top = first + (kRings - 1) * kSides;
            build.triangle(foot + 1, top + j, top + next, along);
        }
    }
    return build.mesh;
}

std::int64_t pose_step(std::uint64_t seed, double seconds) {
    return static_cast<std::int64_t>(std::floor(seconds * kPosesASecond + static_cast<double>(fraction(seed, 0.618))));
}

std::vector<BoneRows> figure_pose(std::uint64_t seed, std::int64_t step) {
    const std::vector<Vec> rest = rest_places();
    const std::vector<Place> places = posed(seed, step);
    std::vector<BoneRows> out;
    out.reserve(places.size());
    for (std::size_t b = 0; b < places.size(); ++b) {
        out.push_back(rows_of(times(places[b], moved(scaled(rest[b], -1.0F)))));
    }
    return out;
}

float facing(std::uint64_t seed) {
    return 2.0F * std::numbers::pi_v<float> * fraction(seed, 0.7071);
}

std::vector<BoneRows> rest_pose() {
    return std::vector<BoneRows>(figure_bones().size(), rows_of(Place{}));
}

std::vector<std::array<float, 3>> bone_places(std::uint64_t seed, std::int64_t step) {
    std::vector<std::array<float, 3>> out;
    for (const Place& p : posed(seed, step)) {
        out.push_back(p.t);
    }
    return out;
}

std::uint16_t half_bits(float value) {
    // after F. Giesen's rounding of a float to a half, ties to even
    constexpr std::uint32_t kInfinity = 255U << 23U;
    constexpr std::uint32_t kTooLarge = (127U + 16U) << 23U;  // 65536 and up are infinite as halves
    constexpr std::uint32_t kSmallest = 113U << 23U;          // 2^-14, the least normal half
    constexpr std::uint32_t kAlignBits = ((127U - 15U) + (23U - 10U) + 1U) << 23U;
    std::uint32_t bits = std::bit_cast<std::uint32_t>(value);
    const std::uint32_t sign = bits & 0x8000'0000U;
    bits ^= sign;
    std::uint32_t out = 0;
    if (bits >= kTooLarge) {
        out = bits > kInfinity ? 0x7E00U : 0x7C00U;  // not a number stays one; infinity and the too large, infinity
    } else if (bits < kSmallest) {
        // a subnormal half or zero: adding this lines the half's 10 bits up at the bottom of the float, rounded to the
        // nearest by the addition itself
        const float aligned = std::bit_cast<float>(bits) + std::bit_cast<float>(kAlignBits);
        out = std::bit_cast<std::uint32_t>(aligned) - kAlignBits;
    } else {
        const std::uint32_t odd = (bits >> 13U) & 1U;
        // the exponent rebased, and just under half of the dropped bits' last place added, plus one if the kept bits
        // are odd: ties go to the even
        bits += ((15U - 127U) << 23U) + 0xFFFU + odd;
        out = bits >> 13U;
    }
    return static_cast<std::uint16_t>(out | (sign >> 16U));
}

std::vector<std::uint16_t> palette_row(const std::vector<BoneRows>& pose) {
    std::vector<std::uint16_t> out;
    out.reserve(pose.size() * kTexelsABone * 4);
    for (const BoneRows& bone : pose) {
        for (const std::array<float, 4>& row : bone.m) {
            for (const float value : row) {
                out.push_back(half_bits(value));
            }
        }
    }
    return out;
}

}  // namespace kd::view
