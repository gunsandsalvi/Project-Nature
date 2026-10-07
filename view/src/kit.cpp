#include "kit.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <limits>

namespace kd::view::kit {

namespace {

constexpr std::size_t kMostParts = 100'000;
constexpr std::size_t kMostSections = 64;
constexpr std::size_t kMostJoints = 256;
// What a triangle that has no area on the surface or in its texture is held to, so it carries nothing; and the stretch
// given to one that has an area on the surface and none in its texture, which carries a whole line of texture pixels
constexpr double kNoArea = 1e-16;
constexpr double kBroken = 1e9;

// A name is lower case letters, digits and _: it is a part's, a role's or a joint's, and is written as such in recipes.
bool valid_name(const std::string& name) {
    return !name.empty() && std::all_of(name.begin(), name.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
    });
}

// The file's bytes taken in order, each checked against what is left.
class Bytes {
public:
    explicit Bytes(std::span<const std::uint8_t> file) : file_(file) {}

    [[nodiscard]] std::size_t left() const { return file_.size() - at_; }
    [[nodiscard]] bool ok() const { return ok_; }

    std::uint32_t u32() {
        if (left() < 4) {
            ok_ = false;
            return 0;
        }
        const std::uint32_t v =
            static_cast<std::uint32_t>(file_[at_]) | static_cast<std::uint32_t>(file_[at_ + 1]) << 8U |
            static_cast<std::uint32_t>(file_[at_ + 2]) << 16U | static_cast<std::uint32_t>(file_[at_ + 3]) << 24U;
        at_ += 4;
        return v;
    }

    std::uint16_t u16() {
        if (left() < 2) {
            ok_ = false;
            return 0;
        }
        const auto v = static_cast<std::uint16_t>(static_cast<std::uint32_t>(file_[at_]) |
                                                  static_cast<std::uint32_t>(file_[at_ + 1]) << 8U);
        at_ += 2;
        return v;
    }

    std::uint8_t u8() {
        if (left() < 1) {
            ok_ = false;
            return 0;
        }
        return file_[at_++];
    }

    float f32() { return std::bit_cast<float>(u32()); }

    std::string text() {
        const std::size_t length = u16();
        if (left() < length) {
            ok_ = false;
            return {};
        }
        std::string out(reinterpret_cast<const char*>(file_.data() + at_), length);
        at_ += length;
        return out;
    }

    // n floats, or none and a failure if the file is too short for them
    bool floats(std::vector<float>& out, std::size_t n) {
        if (left() / 4 < n) {
            ok_ = false;
            return false;
        }
        out.resize(n);
        for (float& f : out) {
            f = f32();
        }
        return true;
    }

private:
    std::span<const std::uint8_t> file_;
    std::size_t at_ = 0;
    bool ok_ = true;
};

void put32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    for (unsigned i = 0; i < 4; ++i) {
        out.push_back(static_cast<std::uint8_t>(v >> (8U * i)));
    }
}

void put16(std::vector<std::uint8_t>& out, std::uint16_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFU));
    out.push_back(static_cast<std::uint8_t>(v >> 8U));
}

void put_text(std::vector<std::uint8_t>& out, const std::string& text) {
    put16(out, static_cast<std::uint16_t>(text.size()));
    out.insert(out.end(), text.begin(), text.end());
}

void put_float(std::vector<std::uint8_t>& out, float v) {
    put32(out, std::bit_cast<std::uint32_t>(v));
}

struct Vec {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

Vec sub(const Vec& a, const Vec& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec scaled(const Vec& a, double s) {
    return {a.x * s, a.y * s, a.z * s};
}

double dot(const Vec& a, const Vec& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec cross(const Vec& a, const Vec& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec position(const Section& s, std::uint32_t vertex) {
    const std::size_t i = vertex;
    return {static_cast<double>(s.positions[3 * i]), static_cast<double>(s.positions[3 * i + 1]),
            static_cast<double>(s.positions[3 * i + 2])};
}

// A vertex's texture coordinates.
std::array<double, 2> texture_at(const Section& s, std::uint32_t vertex) {
    const std::size_t i = vertex;
    return {static_cast<double>(s.uvs[2 * i]), static_cast<double>(s.uvs[2 * i + 1])};
}

}  // namespace

const Joint* Part::joint(const std::string& joint_name) const {
    for (const Joint& j : joints) {
        if (j.name == joint_name) {
            return &j;
        }
    }
    return nullptr;
}

std::size_t Part::triangles() const {
    std::size_t n = 0;
    for (const Section& s : sections) {
        n += s.triangles();
    }
    return n;
}

const Part* Family::part(const std::string& part_name) const {
    const auto at = std::lower_bound(parts.begin(), parts.end(), part_name,
                                     [](const Part& p, const std::string& n) { return p.name < n; });
    return at != parts.end() && at->name == part_name ? &*at : nullptr;
}

Read read_family(std::span<const std::uint8_t> file) {
    Read out;
    Bytes in(file);
    const auto fail = [&out](std::string why) {
        out.family = {};
        out.problem = std::move(why);
        return std::move(out);
    };
    if (file.size() < 12 || file[0] != 'K' || file[1] != 'D' || file[2] != 'K' || file[3] != 'T') {
        return fail("not a kit file");
    }
    in.u32();  // the magic, already read
    if (in.u32() != kVersion) {
        return fail("a kit file of another version");
    }
    const std::uint32_t parts = in.u32();
    if (parts > kMostParts) {
        return fail("a kit file with too many parts");
    }
    for (std::uint32_t p = 0; p < parts; ++p) {
        Part part;
        part.name = in.text();
        for (float& f : part.lowest) {
            f = in.f32();
        }
        for (float& f : part.highest) {
            f = in.f32();
        }
        const std::uint32_t sections = in.u32();
        if (!in.ok() || !valid_name(part.name) || sections > kMostSections) {
            return fail("a kit file cut short, or a part with a bad name");
        }
        if (!out.family.parts.empty() && !(out.family.parts.back().name < part.name)) {
            return fail("the part \"" + part.name + "\" is out of order or twice in the file");
        }
        for (std::uint32_t s = 0; s < sections; ++s) {
            Section section;
            section.role = in.text();
            const std::uint32_t vertices = in.u32();
            const std::uint32_t indices = in.u32();
            if (!in.ok() || !valid_name(section.role) || indices % 3 != 0) {
                return fail("a kit file cut short, or a section of the part \"" + part.name + "\" with a bad role");
            }
            // 3 + 3 + 2 floats and a byte a vertex, and an index is 4 bytes: all must fit in what is left
            if (static_cast<std::uint64_t>(vertices) * 33U + static_cast<std::uint64_t>(indices) * 4U > in.left()) {
                return fail("a kit file cut short in the part \"" + part.name + "\"");
            }
            const std::size_t n = vertices;
            if (!in.floats(section.positions, 3 * n) || !in.floats(section.normals, 3 * n) ||
                !in.floats(section.uvs, 2 * n)) {
                return fail("a kit file cut short in the part \"" + part.name + "\"");
            }
            section.crease.resize(n);
            for (std::uint8_t& c : section.crease) {
                c = in.u8();
            }
            section.indices.resize(indices);
            for (std::uint32_t& index : section.indices) {
                index = in.u32();
                if (index >= vertices) {
                    return fail("the part \"" + part.name + "\" has an index past its vertices");
                }
            }
            if (!in.ok()) {
                return fail("a kit file cut short in the part \"" + part.name + "\"");
            }
            const bool twice = std::any_of(part.sections.begin(), part.sections.end(),
                                           [&](const Section& other) { return other.role == section.role; });
            if (twice) {
                return fail("the part \"" + part.name + "\" has the role \"" + section.role + "\" twice");
            }
            part.sections.push_back(std::move(section));
        }
        const std::uint32_t joints = in.u32();
        if (!in.ok() || joints > kMostJoints) {
            return fail("a kit file cut short in the part \"" + part.name + "\"");
        }
        for (std::uint32_t j = 0; j < joints; ++j) {
            Joint joint;
            joint.name = in.text();
            for (float& f : joint.rotation) {
                f = in.f32();
            }
            for (float& f : joint.at) {
                f = in.f32();
            }
            if (!in.ok() || !valid_name(joint.name)) {
                return fail("a kit file cut short, or a joint of the part \"" + part.name + "\" with a bad name");
            }
            if (part.joint(joint.name) != nullptr) {
                return fail("the part \"" + part.name + "\" has the joint \"" + joint.name + "\" twice");
            }
            part.joints.push_back(std::move(joint));
        }
        out.family.parts.push_back(std::move(part));
    }
    if (in.left() != 0) {
        return fail("a kit file with bytes after its last part");
    }
    return out;
}

std::vector<std::uint8_t> write_family(const Family& family) {
    std::vector<std::uint8_t> out{'K', 'D', 'K', 'T'};
    put32(out, kVersion);
    put32(out, static_cast<std::uint32_t>(family.parts.size()));
    for (const Part& part : family.parts) {
        put_text(out, part.name);
        for (float f : part.lowest) {
            put_float(out, f);
        }
        for (float f : part.highest) {
            put_float(out, f);
        }
        put32(out, static_cast<std::uint32_t>(part.sections.size()));
        for (const Section& s : part.sections) {
            put_text(out, s.role);
            put32(out, static_cast<std::uint32_t>(s.vertices()));
            put32(out, static_cast<std::uint32_t>(s.indices.size()));
            for (float f : s.positions) {
                put_float(out, f);
            }
            for (float f : s.normals) {
                put_float(out, f);
            }
            for (float f : s.uvs) {
                put_float(out, f);
            }
            out.insert(out.end(), s.crease.begin(), s.crease.end());
            for (std::uint32_t index : s.indices) {
                put32(out, index);
            }
        }
        put32(out, static_cast<std::uint32_t>(part.joints.size()));
        for (const Joint& j : part.joints) {
            put_text(out, j.name);
            for (float f : j.rotation) {
                put_float(out, f);
            }
            for (float f : j.at) {
                put_float(out, f);
            }
        }
    }
    return out;
}

std::vector<double> stretches(const Part& part) {
    std::vector<double> out;
    for (const Section& s : part.sections) {
        for (std::size_t t = 0; t < s.triangles(); ++t) {
            const std::uint32_t a = s.indices[3 * t];
            const std::uint32_t b = s.indices[3 * t + 1];
            const std::uint32_t c = s.indices[3 * t + 2];
            const Vec e1 = sub(position(s, b), position(s, a));
            const Vec e2 = sub(position(s, c), position(s, a));
            const double area2 = dot(cross(e1, e2), cross(e1, e2));
            if (area2 < kNoArea) {
                out.push_back(0.0);
                continue;
            }
            const std::array<double, 2> ta = texture_at(s, a);
            const std::array<double, 2> tb = texture_at(s, b);
            const std::array<double, 2> tc = texture_at(s, c);
            const double d1x = tb[0] - ta[0];
            const double d1y = tb[1] - ta[1];
            const double d2x = tc[0] - ta[0];
            const double d2y = tc[1] - ta[1];
            const double det = d1x * d2y - d1y * d2x;
            if (det * det < kNoArea) {
                out.push_back(kBroken);
                continue;
            }
            // the surface's steps for a step of one texture pixel along u and along v, and the lengths of the
            // ellipse a round texture pixel becomes: the roots of the eigenvalues of J^T J
            const Vec ju = scaled(sub(scaled(e1, d2y), scaled(e2, d1y)), 1.0 / det);
            const Vec jv = scaled(sub(scaled(e2, d1x), scaled(e1, d2x)), 1.0 / det);
            const double p = dot(ju, ju);
            const double q = dot(ju, jv);
            const double r = dot(jv, jv);
            const double mean = 0.5 * (p + r);
            const double spread = std::sqrt(0.25 * (p - r) * (p - r) + q * q);
            const double longest = mean + spread;
            const double shortest = mean - spread;
            out.push_back(shortest <= longest * 1e-18 ? kBroken : std::sqrt(longest / shortest));
        }
    }
    return out;
}

StretchReport check_stretch(const Family& family, double line) {
    StretchReport report;
    for (const Part& part : family.parts) {
        const std::vector<double> each = stretches(part);
        std::size_t at = 0;
        for (const Section& s : part.sections) {
            for (std::size_t t = 0; t < s.triangles(); ++t, ++at) {
                ++report.triangles;
                report.worst = std::max(report.worst, each[at]);
                if (each[at] > line) {
                    report.over.push_back({part.name, s.role, t, each[at]});
                }
            }
        }
    }
    std::stable_sort(report.over.begin(), report.over.end(),
                     [](const Stretched& a, const Stretched& b) { return a.stretch > b.stretch; });
    return report;
}

}  // namespace kd::view::kit
