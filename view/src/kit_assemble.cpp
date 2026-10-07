#include "kit_assemble.hpp"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <map>
#include <string_view>

#include "kd/chance/chance.hpp"
#include "kd/num/maths.hpp"

namespace kd::view::kit {

namespace {

using Vec = std::array<double, 3>;
using Mat = std::array<double, 9>;  // by rows

// How far a span's parts may differ from the span's length, as a share of it: a pole 15% too short is a recipe or a
// part that does not fit, not the seed's variety.
constexpr double kSpanFit = 0.15;
// How far apart two joints that should meet may lie, in metres.
constexpr double kMeet = 0.001;

Vec add(const Vec& a, const Vec& b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

Vec sub(const Vec& a, const Vec& b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

Vec scale(const Vec& a, double s) {
    return {a[0] * s, a[1] * s, a[2] * s};
}

double dot(const Vec& a, const Vec& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

Vec cross(const Vec& a, const Vec& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

double length(const Vec& a) {
    return std::sqrt(dot(a, a));
}

Vec unit(const Vec& a) {
    const double l = length(a);
    return l > 0.0 ? scale(a, 1.0 / l) : a;
}

Vec mul(const Mat& m, const Vec& v) {
    return {m[0] * v[0] + m[1] * v[1] + m[2] * v[2], m[3] * v[0] + m[4] * v[1] + m[5] * v[2],
            m[6] * v[0] + m[7] * v[1] + m[8] * v[2]};
}

Mat times(const Mat& a, const Mat& b) {
    Mat out{};
    for (std::size_t r = 0; r < 3; ++r) {
        for (std::size_t c = 0; c < 3; ++c) {
            out[r * 3 + c] = a[r * 3] * b[c] + a[r * 3 + 1] * b[3 + c] + a[r * 3 + 2] * b[6 + c];
        }
    }
    return out;
}

Mat transposed(const Mat& m) {
    return {m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8]};
}

Mat identity() {
    return {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
}

// The sine and cosine of an angle in degrees, by the project's own maths.
double sine(double degrees) {
    return num::sinpi(degrees / 180.0);
}

double cosine(double degrees) {
    return num::cospi(degrees / 180.0);
}

// Turns about +y by an angle: a part's forward, -z, comes round to the azimuth that is minus the angle.
Mat about_y(double degrees) {
    const double c = cosine(degrees);
    const double s = sine(degrees);
    return {c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c};
}

Mat about_z(double degrees) {
    const double c = cosine(degrees);
    const double s = sine(degrees);
    return {c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0};
}

Mat about_x(double degrees) {
    const double c = cosine(degrees);
    const double s = sine(degrees);
    return {1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c};
}

// Rodrigues' turn about a unit axis.
Mat about(const Vec& axis, double degrees) {
    const double c = cosine(degrees);
    const double s = sine(degrees);
    const double t = 1.0 - c;
    const double x = axis[0];
    const double y = axis[1];
    const double z = axis[2];
    return {t * x * x + c,     t * x * y - s * z, t * x * z + s * y, t * x * y + s * z, t * y * y + c,
            t * y * z - s * x, t * x * z - s * y, t * y * z + s * x, t * z * z + c};
}

// The smallest turn that takes one unit vector to another.
Mat aligning(const Vec& from, const Vec& to) {
    const Vec axis = cross(from, to);
    const double s = length(axis);
    const double c = dot(from, to);
    if (s < 1e-12) {
        if (c > 0.0) {
            return identity();
        }
        // opposite: half a turn about any axis across them
        const Vec across = std::abs(from[0]) < 0.9 ? Vec{1.0, 0.0, 0.0} : Vec{0.0, 1.0, 0.0};
        return about(unit(cross(from, across)), 180.0);
    }
    const Vec k = scale(axis, 1.0 / s);
    // sin and cos of the angle are known, so build the turn from them directly
    const double t = 1.0 - c;
    return {t * k[0] * k[0] + c,        t * k[0] * k[1] - s * k[2], t * k[0] * k[2] + s * k[1],
            t * k[0] * k[1] + s * k[2], t * k[1] * k[1] + c,        t * k[1] * k[2] - s * k[0],
            t * k[0] * k[2] - s * k[1], t * k[1] * k[2] + s * k[0], t * k[2] * k[2] + c};
}

Mat from_rotation(const std::array<float, 9>& r) {
    Mat m{};
    for (std::size_t i = 0; i < 9; ++i) {
        m[i] = static_cast<double>(r[i]);
    }
    return m;
}

Vec from_point(const std::array<float, 3>& p) {
    return {static_cast<double>(p[0]), static_cast<double>(p[1]), static_cast<double>(p[2])};
}

// A frame whose three axes are the matrix's columns made square again: its first and second kept as the joint's x
// and y, the third across them.
Mat squared(const Mat& m) {
    const Vec x = unit({m[0], m[3], m[6]});
    const Vec z = unit(cross(x, {m[1], m[4], m[7]}));
    const Vec y = cross(z, x);
    return {x[0], y[0], z[0], x[1], y[1], z[1], x[2], y[2], z[2]};
}

std::array<double, 12> carrying(const Mat& linear, const Vec& origin_in_part, const Vec& place) {
    const Vec moved = mul(linear, origin_in_part);
    return {linear[0], linear[1],           linear[2], place[0] - moved[0], linear[3], linear[4],
            linear[5], place[1] - moved[1], linear[6], linear[7],           linear[8], place[2] - moved[2]};
}

Mat linear_of(const std::array<double, 12>& m) {
    return {m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10]};
}

Vec translation_of(const std::array<double, 12>& m) {
    return {m[3], m[7], m[11]};
}

Vec carry(const std::array<double, 12>& m, const Vec& p) {
    return add(mul(linear_of(m), p), translation_of(m));
}

// The draws for one placement of a seed's thing.
chance::Draws dice(std::uint64_t seed, const std::string& what) {
    return chance::Draws(seed, chance::name("kit"), 0, 0, chance::name(what));
}

// How each copy's draws are numbered: this many for each.
constexpr std::uint64_t kDrawsEach = 8;
enum class Draw : std::uint8_t { which_part = 0, radius = 1, turn = 2, thickness = 3, roll = 4 };

std::uint64_t draw_index(std::uint64_t copy, Draw what) {
    return copy * kDrawsEach + static_cast<std::uint64_t>(what);
}

// A number from -1 up to 1 for a draw.
double either_way(const chance::Draws& d, std::uint64_t copy, Draw what) {
    return 2.0 * d.fraction(draw_index(copy, what)) - 1.0;
}

// A message made of its parts with one allocation, for those made inside loops.
std::string sentence(std::initializer_list<std::string_view> parts) {
    std::string out;
    for (const std::string_view part : parts) {
        out += part;
    }
    return out;
}

// A placement's quantities as the metres and degrees the maths wants.
double metres(std::int64_t millimetres) {
    return static_cast<double>(millimetres) / 1000.0;
}

double share(std::int64_t parts_per_million) {
    return static_cast<double>(parts_per_million) / 1e6;
}

// Where a placement's joint of a part is in the part's metres, and its frame.
struct JointFrame {
    Vec at{};
    Mat axes = identity();
};

JointFrame frame_of(const Part& part, const std::string& joint) {
    JointFrame f;
    if (const Joint* j = part.joint(joint)) {
        f.at = from_point(j->at);
        f.axes = from_rotation(j->rotation);
    }
    return f;
}

// A joint of a placed part as the thing holds it: its axes there, and its place there lifted along its main axis by a
// length in metres.
struct Socket {
    Vec at{};
    Mat axes = identity();
};

Socket socket_of(const Placed& placed, const Part& part, const std::string& joint, double lift) {
    const JointFrame frame = frame_of(part, joint);
    Socket s;
    s.axes = squared(times(linear_of(placed.matrix), frame.axes));
    s.at = add(carry(placed.matrix, frame.at), scale({s.axes[2], s.axes[5], s.axes[8]}, lift));
    return s;
}

}  // namespace

std::array<double, 3> carried(const Placed& placed, const std::array<float, 3>& point) {
    return carry(placed.matrix, from_point(point));
}

Assembly assemble(const kd::look::Model& model, const Family& family, std::uint64_t seed) {
    Assembly out;
    const auto fail = [&out](std::string why) {
        out.placed.clear();
        out.problem = std::move(why);
        return std::move(out);
    };
    // what each role wears: one texture for the whole thing
    for (const kd::look::ModelMaterial& m : model.materials) {
        if (!m.textures.empty()) {
            const std::uint64_t pick = dice(seed, "wears:" + m.role).below(0, m.textures.size());
            out.wears.emplace_back(m.role, m.textures[pick].name);
        }
    }
    // where each placement's copies went, in the order they were placed, for a plug to find its joints
    std::map<std::string, std::vector<std::size_t>> copies_of;
    for (const kd::look::ModelPlace& place : model.places) {
        if (place.parts.empty()) {
            return fail("the placement \"" + place.name + "\" names no part");
        }
        const chance::Draws draws = dice(seed, place.name);
        // the part a copy is, by the seed
        const auto part_of = [&](std::uint64_t copy) -> const Part* {
            const std::string& name = place.parts[draws.below(draw_index(copy, Draw::which_part), place.parts.size())];
            return family.part(name);
        };
        const auto missing = [&](std::uint64_t copy) {
            return "the part \"" + place.parts[draws.below(draw_index(copy, Draw::which_part), place.parts.size())] +
                   "\" of the placement \"" + place.name + "\" is not in the family \"" + model.family + "\"";
        };
        const auto lacks = [&](const Part& part, const std::string& joint) {
            return "the part \"" + part.name + "\" has no joint \"" + joint + "\", which the placement \"" +
                   place.name + "\" needs";
        };
        const auto put = [&](const Part& part, const std::array<double, 12>& matrix) {
            copies_of[place.name].push_back(out.placed.size());
            out.placed.push_back({place.name, part.name, matrix});
        };
        const auto count = static_cast<std::uint64_t>(place.count);
        if (place.rule == "root") {
            const Part* part = part_of(0);
            if (part == nullptr) {
                return fail(missing(0));
            }
            if (!place.joint.empty() && part->joint(place.joint) == nullptr) {
                return fail(lacks(*part, place.joint));
            }
            const Mat linear =
                times(about_y(-static_cast<double>(place.turn)), about_x(static_cast<double>(place.tilt)));
            put(*part, carrying(linear, frame_of(*part, place.joint).at, {0.0, metres(place.height), 0.0}));
        } else if (place.rule == "ring" || place.rule == "span") {
            for (std::uint64_t k = 0; k < count; ++k) {
                const Part* part = part_of(k);
                if (part == nullptr) {
                    return fail(missing(k));
                }
                if (!place.joint.empty() && part->joint(place.joint) == nullptr) {
                    return fail(lacks(*part, place.joint));
                }
                const double azimuth = static_cast<double>(place.turn) +
                                       360.0 * static_cast<double>(k) / static_cast<double>(count) +
                                       static_cast<double>(place.jitter_turn) * either_way(draws, k, Draw::turn);
                const double r =
                    metres(place.radius) * (1.0 + share(place.jitter_radius) * either_way(draws, k, Draw::radius));
                const Vec on_circle{r * sine(azimuth), metres(place.base), -r * cosine(azimuth)};
                const JointFrame from = frame_of(*part, place.joint);
                if (place.rule == "ring") {
                    const Mat facing = about_y(-(azimuth + static_cast<double>(place.face)));
                    put(*part, carrying(facing, from.at, on_circle));
                    continue;
                }
                if (part->joint(place.to_joint) == nullptr) {
                    return fail(lacks(*part, place.to_joint));
                }
                const Vec along = sub(frame_of(*part, place.to_joint).at, from.at);
                const Vec meet{0.0, metres(place.height), 0.0};
                const Vec span = sub(meet, on_circle);
                if (length(along) < 1e-9 || length(span) < 1e-9) {
                    return fail("the placement \"" + place.name + "\" has a span of no length");
                }
                // the thickness varies; the length is the span's, so the two joints meet exactly
                const double thick = 1.0 + share(place.jitter_size) * either_way(draws, k, Draw::thickness);
                const double stretch = length(span) / length(along);
                const Vec axis = unit(along);
                const Vec heading = unit(span);
                const Mat aim = aligning(axis, heading);
                const Mat roll_about =
                    about(heading, static_cast<double>(place.face) +
                                       static_cast<double>(place.jitter_turn) * either_way(draws, k, Draw::roll));
                Mat shape{};
                for (std::size_t i = 0; i < 3; ++i) {
                    for (std::size_t j = 0; j < 3; ++j) {
                        shape[i * 3 + j] = (i == j ? thick : 0.0) + (stretch - thick) * axis[i] * axis[j];
                    }
                }
                put(*part, carrying(times(roll_about, times(aim, shape)), from.at, on_circle));
            }
        } else if (place.rule == "plug") {
            const std::size_t dot_at = place.onto.find('.');
            const std::string target = place.onto.substr(0, dot_at);
            const std::string target_joint =
                dot_at == std::string::npos ? std::string() : place.onto.substr(dot_at + 1);
            const auto found = copies_of.find(target);
            if (found == copies_of.end()) {
                return fail("the plug \"" + place.name + "\" goes into \"" + target +
                            "\", which is not placed before it");
            }
            // one copy for each copy of what it plugs into
            const std::vector<std::size_t> targets = found->second;
            for (std::uint64_t k = 0; k < targets.size(); ++k) {
                const Part* part = part_of(k);
                if (part == nullptr) {
                    return fail(missing(k));
                }
                if (part->joint(place.joint) == nullptr) {
                    return fail(lacks(*part, place.joint));
                }
                const Placed& to = out.placed[targets[k]];
                const Part* to_part = family.part(to.part);
                if (to_part == nullptr || to_part->joint(target_joint) == nullptr) {
                    return fail(
                        sentence({"the placement \"", target, "\" has no joint \"", target_joint, "\" to plug into"}));
                }
                const Socket there = socket_of(to, *to_part, target_joint, metres(place.height));
                const JointFrame own = frame_of(*part, place.joint);
                // the part's metres into its joint's own, turned about the joint's main axis (its third) clockwise seen
                // from its tip, then into the thing's
                const Mat linear =
                    times(there.axes, times(about_z(-static_cast<double>(place.turn)), transposed(squared(own.axes))));
                put(*part, carrying(linear, own.at, there.at));
            }
        }
    }
    // the thing's bounds, from its parts' corners
    bool first = true;
    for (const Placed& p : out.placed) {
        const Part* part = family.part(p.part);
        for (unsigned corner = 0; corner < 8; ++corner) {
            const std::array<float, 3> c{(corner & 1U) != 0 ? part->highest[0] : part->lowest[0],
                                         (corner & 2U) != 0 ? part->highest[1] : part->lowest[1],
                                         (corner & 4U) != 0 ? part->highest[2] : part->lowest[2]};
            const Vec w = carried(p, c);
            for (std::size_t i = 0; i < 3; ++i) {
                out.lowest[i] = first ? w[i] : std::min(out.lowest[i], w[i]);
                out.highest[i] = first ? w[i] : std::max(out.highest[i], w[i]);
            }
            first = false;
        }
    }
    return out;
}

std::vector<std::string> check_model(const std::string& name, const kd::look::Model& model, const Family& family) {
    std::vector<std::string> found;
    const auto say = [&](const std::string& what) { found.push_back(name + ": " + what); };
    // every part and joint a placement names, in each alternative
    std::map<std::string, const kd::look::ModelPlace*> placed;
    std::vector<std::string> roles;
    for (const kd::look::ModelPlace& place : model.places) {
        const auto needs = [&](const std::string& joint, const char* why) {
            if (joint.empty()) {
                return;
            }
            for (const std::string& part_name : place.parts) {
                const Part* part = family.part(part_name);
                if (part != nullptr && part->joint(joint) == nullptr) {
                    say(sentence({"the part \"", part_name, "\" has no joint \"", joint, "\", ", why,
                                  " of the placement \"", place.name, "\""}));
                }
            }
        };
        for (const std::string& part_name : place.parts) {
            const Part* part = family.part(part_name);
            if (part == nullptr) {
                say("the part \"" + part_name + "\" of the placement \"" + place.name + "\" is not in the family \"" +
                    model.family + "\"");
                continue;
            }
            for (const Section& s : part->sections) {
                if (std::find(roles.begin(), roles.end(), s.role) == roles.end()) {
                    roles.push_back(s.role);
                }
            }
        }
        needs(place.joint, "which goes to the place");
        needs(place.to_joint, "which meets the others");
        if (place.rule == "plug") {
            const std::size_t dot = place.onto.find('.');
            const std::string target = place.onto.substr(0, dot);
            const auto at = placed.find(target);
            if (at != placed.end() && dot != std::string::npos) {
                const std::string joint = place.onto.substr(dot + 1);
                for (const std::string& part_name : at->second->parts) {
                    const Part* part = family.part(part_name);
                    if (part != nullptr && part->joint(joint) == nullptr) {
                        say(sentence({"the part \"", part_name, "\" has no joint \"", joint, "\", which the plug \"",
                                      place.name, "\" goes into"}));
                    }
                }
            }
        }
        // a span's parts as long as the span, with the radius straying as far as it may
        if (place.rule == "span" && !place.joint.empty() && !place.to_joint.empty()) {
            for (const std::string& part_name : place.parts) {
                const Part* part = family.part(part_name);
                if (part == nullptr || part->joint(place.joint) == nullptr || part->joint(place.to_joint) == nullptr) {
                    continue;
                }
                const double own =
                    length(sub(from_point(part->joint(place.to_joint)->at), from_point(part->joint(place.joint)->at)));
                for (const double r : {metres(place.radius) * (1.0 - share(place.jitter_radius)),
                                       metres(place.radius) * (1.0 + share(place.jitter_radius))}) {
                    const double need = std::sqrt(r * r + (metres(place.height) - metres(place.base)) *
                                                              (metres(place.height) - metres(place.base)));
                    if (own > 0.0 && std::abs(need / own - 1.0) > kSpanFit) {
                        say("the span \"" + place.name + "\" is " + std::to_string(need) +
                            " m from joint to joint, but the part \"" + part_name + "\" is " + std::to_string(own) +
                            " m between them: it would be stretched past " +
                            std::to_string(static_cast<int>(kSpanFit * 100.0)) + "%");
                        break;
                    }
                }
            }
        }
        placed[place.name] = &place;
    }
    // each role the parts wear has a texture
    for (const std::string& role : roles) {
        const bool given = std::any_of(model.materials.begin(), model.materials.end(),
                                       [&](const kd::look::ModelMaterial& m) { return m.role == role; });
        if (!given) {
            say("its parts wear the role \"" + role + "\", which it gives no texture");
        }
    }
    if (!found.empty()) {
        return found;
    }
    // and the joints really meet when it is put together
    const Assembly made = assemble(model, family, 1);
    if (!made.problem.empty()) {
        say(made.problem);
        return found;
    }
    std::map<std::string, std::vector<const Placed*>> by_place;
    for (const Placed& p : made.placed) {
        by_place[p.place].push_back(&p);
    }
    for (const kd::look::ModelPlace& place : model.places) {
        if (place.rule != "plug") {
            continue;
        }
        const std::size_t dot = place.onto.find('.');
        const std::vector<const Placed*>& into = by_place[place.onto.substr(0, dot)];
        const std::vector<const Placed*>& own = by_place[place.name];
        for (std::size_t k = 0; k < own.size() && k < into.size(); ++k) {
            const Part* a = family.part(own[k]->part);
            const Part* b = family.part(into[k]->part);
            const Vec here = carry(own[k]->matrix, from_point(a->joint(place.joint)->at));
            const Vec there = socket_of(*into[k], *b, place.onto.substr(dot + 1), metres(place.height)).at;
            if (length(sub(here, there)) > kMeet) {
                say("the plug \"" + place.name + "\" and what it goes into do not meet at their joints");
                break;
            }
        }
    }
    return found;
}

}  // namespace kd::view::kit
