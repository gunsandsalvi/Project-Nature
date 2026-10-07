#include "kit_draw.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <algorithm>
#include <cmath>
#include <set>
#include <span>

#include "kd/look/model.hpp"
#include "kd/num/maths.hpp"
#include "texture_images.hpp"

namespace kd::view {

namespace {

using RS = godot::RenderingServer;

RS& server() {
    return *RS::get_singleton();
}

godot::String text(const std::string& s) {
    return godot::String::utf8(s.c_str());
}

std::string text(const godot::String& s) {
    return std::string(s.utf8().get_data());
}

godot::Vector3 point(const std::array<float, 3>& p) {
    return {p[0], p[1], p[2]};
}

// A placed part's carrying as Godot's transform: its rows of a rotation and stretch, and its place.
godot::Transform3D carrying(const kit::Placed& placed) {
    const std::array<double, 12>& m = placed.matrix;
    const auto f = [](double v) { return static_cast<float>(v); };
    return {godot::Basis(f(m[0]), f(m[1]), f(m[2]), f(m[4]), f(m[5]), f(m[6]), f(m[8]), f(m[9]), f(m[10])),
            godot::Vector3(f(m[3]), f(m[7]), f(m[11]))};
}

// A number from 0 up to 1 for a copy's number, the golden ratio's steps, so neighbours differ.
float spread(std::uint64_t i) {
    const double v = static_cast<double>(i) * 0.6180339887498949;
    return static_cast<float>(v - std::floor(v));
}

}  // namespace

KdKit::~KdKit() {
    clear();
}

godot::String KdKit::load_family(const godot::String& family, const godot::String& path) {
    const godot::PackedByteArray bytes = godot::FileAccess::get_file_as_bytes(path);
    if (bytes.is_empty()) {
        return godot::String("no kit file at ") + path;
    }
    const kit::Read read = kit::read_family(std::span<const std::uint8_t>(bytes.ptr(), bytes.size()));
    if (!read.problem.empty()) {
        return path + godot::String(": ") + text(read.problem);
    }
    families_[text(family)] = read.family;
    return "";
}

void KdKit::use_world(const godot::Ref<KdWorld>& world) {
    world_ = world;
}

godot::PackedStringArray KdKit::models() const {
    godot::PackedStringArray out;
    if (world_.is_valid() && world_->catalogue() != nullptr) {
        const data::Kind<look::Model>& kind = world_->catalogue()->kind<look::Model>();
        for (std::uint32_t i = 0; i < kind.size(); ++i) {
            out.push_back(text(kind.name(i)));
        }
    }
    return out;
}

godot::PackedStringArray KdKit::texture_names() const {
    std::set<std::string> names;
    if (world_.is_valid() && world_->catalogue() != nullptr) {
        const data::Kind<look::Model>& kind = world_->catalogue()->kind<look::Model>();
        for (std::uint32_t i = 0; i < kind.size(); ++i) {
            for (const look::ModelMaterial& m : kind[i].materials) {
                for (const data::Ref& r : m.textures) {
                    names.insert(r.name);
                }
            }
        }
    }
    godot::PackedStringArray out;
    for (const std::string& n : names) {
        out.push_back(text(n));
    }
    return out;
}

godot::Dictionary KdKit::model_info(const godot::String& model) const {
    godot::Dictionary out;
    const data::Catalogue* cat = world_.is_valid() ? world_->catalogue() : nullptr;
    const std::optional<std::uint32_t> at = cat != nullptr ? cat->find("models", text(model)) : std::nullopt;
    if (!at) {
        out["problem"] = godot::String("no model ") + model;
        return out;
    }
    const look::Model& recipe = cat->kind<look::Model>()[*at];
    out["about"] = text(recipe.about);
    out["family"] = text(recipe.family);
    out["approved"] = text(recipe.approved);
    godot::Dictionary roles;
    for (const look::ModelMaterial& m : recipe.materials) {
        godot::PackedStringArray textures;
        for (const data::Ref& r : m.textures) {
            textures.push_back(text(r.name));
        }
        roles[text(m.role)] = textures;
    }
    out["roles"] = roles;
    const auto family = families_.find(recipe.family);
    if (family == families_.end()) {
        out["problem"] = godot::String("the family ") + text(recipe.family) + godot::String(" is not loaded");
        return out;
    }
    const kit::Assembly made = kit::assemble(recipe, family->second, 0);
    if (!made.problem.empty()) {
        out["problem"] = text(made.problem);
        return out;
    }
    std::set<std::string> used;
    std::int64_t triangles = 0;
    for (const kit::Placed& p : made.placed) {
        used.insert(p.part);
        triangles += static_cast<std::int64_t>(family->second.part(p.part)->triangles());
    }
    godot::PackedStringArray parts;
    for (const std::string& n : used) {
        parts.push_back(text(n));
    }
    out["parts"] = parts;
    out["copies"] = static_cast<int64_t>(made.placed.size());
    out["triangles"] = triangles;
    out["lowest"] = godot::Vector3(static_cast<float>(made.lowest[0]), static_cast<float>(made.lowest[1]),
                                   static_cast<float>(made.lowest[2]));
    out["highest"] = godot::Vector3(static_cast<float>(made.highest[0]), static_cast<float>(made.highest[1]),
                                    static_cast<float>(made.highest[2]));
    out["problem"] = "";
    return out;
}

godot::PackedStringArray KdKit::parts(const godot::String& family) const {
    godot::PackedStringArray out;
    const auto at = families_.find(text(family));
    if (at != families_.end()) {
        for (const kit::Part& p : at->second.parts) {
            out.push_back(text(p.name));
        }
    }
    return out;
}

godot::Dictionary KdKit::part_info(const godot::String& family, const godot::String& part) const {
    godot::Dictionary out;
    const auto at = families_.find(text(family));
    const kit::Part* p = at != families_.end() ? at->second.part(text(part)) : nullptr;
    if (p == nullptr) {
        out["problem"] = godot::String("no part ") + part;
        return out;
    }
    godot::Dictionary roles;
    for (const kit::Section& s : p->sections) {
        roles[text(s.role)] = static_cast<int64_t>(s.triangles());
    }
    godot::PackedStringArray joints;
    for (const kit::Joint& j : p->joints) {
        joints.push_back(text(j.name));
    }
    double worst = 1.0;
    for (const double s : kit::stretches(*p)) {
        worst = std::max(worst, s);
    }
    out["triangles"] = static_cast<int64_t>(p->triangles());
    out["roles"] = roles;
    out["joints"] = joints;
    out["lowest"] = point(p->lowest);
    out["highest"] = point(p->highest);
    out["stretch"] = worst;
    out["problem"] = "";
    return out;
}

godot::String KdKit::set_texture(const godot::String& name, const godot::String& path, double tile_metres) {
    const ReadLevels read = read_levels(path);
    if (!read.problem.is_empty()) {
        return read.problem;
    }
    godot::TypedArray<godot::Image> images;
    images.push_back(with_levels(read.images));
    Wearable w;
    w.tile = tile_metres;
    w.texture = server().texture_2d_layered_create(images, RS::TEXTURE_LAYERED_2D_ARRAY);
    w.material = server().material_create();
    if (shader_.is_valid()) {
        server().material_set_shader(w.material, shader_);
    }
    server().material_set_param(w.material, "kd_layers", w.texture);
    server().material_set_param(w.material, "kd_tile_metres", tile_metres);
    const auto old = wearables_.find(text(name));
    if (old != wearables_.end()) {
        server().free_rid(old->second.material);
        server().free_rid(old->second.texture);
    }
    wearables_[text(name)] = w;
    return "";
}

void KdKit::set_shader(const godot::RID& shader) {
    shader_ = shader;
    for (const auto& [name, w] : wearables_) {
        server().material_set_shader(w.material, shader_);
    }
}

void KdKit::set_origin(int64_t east, int64_t north) {
    origin_east_ = east;
    origin_north_ = north;
    for (auto& [id, thing] : things_) {
        put(thing);
    }
}

// The thing's place on the screen: its turn about up, and its centimetres east, up and north about the rig's origin.
void KdKit::put(Thing& thing) {
    const double angle = -thing.turn / 180.0;
    const auto c = static_cast<float>(num::cospi(angle));
    const auto s = static_cast<float>(num::sinpi(angle));
    const godot::Transform3D at(
        godot::Basis(c, 0.0F, s, 0.0F, 1.0F, 0.0F, -s, 0.0F, c),
        godot::Vector3(static_cast<float>(static_cast<double>(thing.east - origin_east_) / 100.0),
                       static_cast<float>(static_cast<double>(thing.up) / 100.0),
                       -static_cast<float>(static_cast<double>(thing.north - origin_north_) / 100.0)));
    for (const Draw& d : thing.draws) {
        server().instance_set_transform(d.instance, at);
    }
}

// A part's role as a one-surface mesh, made once for every thing that uses it; its texture coordinates in metres, its
// crease in the colour's red.
godot::RID KdKit::mesh_of(const std::string& family, const kit::Part& part, const kit::Section& section) {
    const std::string key = family + "/" + part.name + "/" + section.role;
    const auto have = meshes_.find(key);
    if (have != meshes_.end()) {
        return have->second;
    }
    godot::PackedVector3Array vertices;
    godot::PackedVector3Array normals;
    godot::PackedVector2Array uvs;
    godot::PackedColorArray colours;
    for (std::size_t i = 0; i < section.vertices(); ++i) {
        vertices.push_back({section.positions[3 * i], section.positions[3 * i + 1], section.positions[3 * i + 2]});
        normals.push_back({section.normals[3 * i], section.normals[3 * i + 1], section.normals[3 * i + 2]});
        uvs.push_back({section.uvs[2 * i], section.uvs[2 * i + 1]});
        const float crease = static_cast<float>(section.crease[i]) / 255.0F;
        colours.push_back(godot::Color(crease, crease, crease, 1.0F));
    }
    godot::PackedInt32Array indices;
    for (const std::uint32_t index : section.indices) {
        indices.push_back(static_cast<int32_t>(index));
    }
    godot::Array arrays;
    arrays.resize(RS::ARRAY_MAX);
    arrays[RS::ARRAY_VERTEX] = vertices;
    arrays[RS::ARRAY_NORMAL] = normals;
    arrays[RS::ARRAY_TEX_UV] = uvs;
    arrays[RS::ARRAY_COLOR] = colours;
    arrays[RS::ARRAY_INDEX] = indices;
    const godot::RID mesh = server().mesh_create();
    server().mesh_add_surface_from_arrays(mesh, RS::PRIMITIVE_TRIANGLES, arrays);
    meshes_[key] = mesh;
    return mesh;
}

godot::Dictionary KdKit::draw(const godot::RID& scenario, const kit::Assembly& assembly, const std::string& family_name,
                              const godot::Dictionary& wear, std::int64_t east, std::int64_t north, std::int64_t up,
                              double turn, std::uint64_t seed) {
    godot::Dictionary out;
    const kit::Family& family = families_.at(family_name);
    // the texture each role wears: the seed's, or the one asked for
    std::map<std::string, std::string> wears(assembly.wears.begin(), assembly.wears.end());
    const godot::Array asked = wear.keys();
    for (int64_t i = 0; i < asked.size(); ++i) {
        wears[text(godot::String(asked[i]))] = text(godot::String(wear[asked[i]]));
    }
    std::map<std::string, std::vector<const kit::Placed*>> by_part;
    for (const kit::Placed& p : assembly.placed) {
        by_part[p.part].push_back(&p);
    }
    Thing thing;
    thing.east = east;
    thing.north = north;
    thing.up = up;
    thing.turn = turn;
    std::int64_t triangles = 0;
    std::string problem;
    for (const auto& [part_name, copies] : by_part) {
        const kit::Part* part = family.part(part_name);
        for (const kit::Section& section : part->sections) {
            const auto role = wears.find(section.role);
            if (role == wears.end()) {
                problem = "the role \"" + section.role + "\" of the part \"" + part_name + "\" wears no texture";
                break;
            }
            const auto texture = wearables_.find(role->second);
            if (texture == wearables_.end()) {
                problem = "the texture \"" + role->second + "\" is not set: call set_texture first";
                break;
            }
            const godot::RID mesh = mesh_of(family_name, *part, section);
            const godot::RID multimesh = server().multimesh_create();
            server().multimesh_allocate_data(multimesh, static_cast<int32_t>(copies.size()), RS::MULTIMESH_TRANSFORM_3D,
                                             false, true);
            server().multimesh_set_mesh(multimesh, mesh);
            for (std::size_t i = 0; i < copies.size(); ++i) {
                server().multimesh_instance_set_transform(multimesh, static_cast<int32_t>(i), carrying(*copies[i]));
                server().multimesh_instance_set_custom_data(multimesh, static_cast<int32_t>(i),
                                                            godot::Color(spread(seed * 131U + i), 0.0F, 0.0F, 0.0F));
            }
            const godot::RID instance = server().instance_create2(multimesh, scenario);
            server().instance_geometry_set_material_override(instance, texture->second.material);
            server().instance_set_layer_mask(instance, layers_);
            thing.draws.push_back({multimesh, instance});
            triangles += static_cast<std::int64_t>(section.triangles() * copies.size());
        }
        if (!problem.empty()) {
            break;
        }
    }
    if (!problem.empty()) {
        for (const Draw& d : thing.draws) {
            server().free_rid(d.instance);
            server().free_rid(d.multimesh);
        }
        out["problem"] = text(problem);
        return out;
    }
    put(thing);
    const std::int64_t id = next_++;
    things_[id] = thing;
    out["id"] = id;
    out["problem"] = "";
    out["lowest"] = godot::Vector3(static_cast<float>(assembly.lowest[0]), static_cast<float>(assembly.lowest[1]),
                                   static_cast<float>(assembly.lowest[2]));
    out["highest"] = godot::Vector3(static_cast<float>(assembly.highest[0]), static_cast<float>(assembly.highest[1]),
                                    static_cast<float>(assembly.highest[2]));
    out["triangles"] = triangles;
    return out;
}

godot::Dictionary KdKit::place(const godot::RID& scenario, const godot::String& model, int64_t seed, int64_t east,
                               int64_t north, int64_t up, double turn, const godot::Dictionary& wear) {
    godot::Dictionary out;
    const data::Catalogue* cat = world_.is_valid() ? world_->catalogue() : nullptr;
    const std::optional<std::uint32_t> at = cat != nullptr ? cat->find("models", text(model)) : std::nullopt;
    if (!at) {
        out["problem"] = godot::String("no model ") + model;
        return out;
    }
    const look::Model& recipe = cat->kind<look::Model>()[*at];
    const auto family = families_.find(recipe.family);
    if (family == families_.end()) {
        out["problem"] = godot::String("the family ") + text(recipe.family) + godot::String(" is not loaded");
        return out;
    }
    const kit::Assembly made = kit::assemble(recipe, family->second, static_cast<std::uint64_t>(seed));
    if (!made.problem.empty()) {
        out["problem"] = text(made.problem);
        return out;
    }
    return draw(scenario, made, family->first, wear, east, north, up, turn, static_cast<std::uint64_t>(seed));
}

godot::Dictionary KdKit::place_part(const godot::RID& scenario, const godot::String& family, const godot::String& part,
                                    int64_t east, int64_t north, int64_t up, double turn,
                                    const godot::Dictionary& wear) {
    godot::Dictionary out;
    const auto have = families_.find(text(family));
    const kit::Part* p = have != families_.end() ? have->second.part(text(part)) : nullptr;
    if (p == nullptr) {
        out["problem"] = godot::String("no part ") + part;
        return out;
    }
    kit::Assembly alone;
    alone.placed.push_back({"part", p->name, {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0}});
    alone.lowest = {static_cast<double>(p->lowest[0]), static_cast<double>(p->lowest[1]),
                    static_cast<double>(p->lowest[2])};
    alone.highest = {static_cast<double>(p->highest[0]), static_cast<double>(p->highest[1]),
                     static_cast<double>(p->highest[2])};
    return draw(scenario, alone, have->first, wear, east, north, up, turn, 0);
}

void KdKit::move(int64_t id, int64_t east, int64_t north, int64_t up, double turn) {
    const auto it = things_.find(id);
    if (it == things_.end()) {
        return;
    }
    it->second.east = east;
    it->second.north = north;
    it->second.up = up;
    it->second.turn = turn;
    put(it->second);
}

void KdKit::remove(int64_t id) {
    const auto it = things_.find(id);
    if (it == things_.end()) {
        return;
    }
    if (RS::get_singleton() != nullptr) {
        for (const Draw& d : it->second.draws) {
            server().free_rid(d.instance);
            server().free_rid(d.multimesh);
        }
    }
    things_.erase(it);
}

void KdKit::set_layers(int64_t layers) {
    layers_ = static_cast<std::uint32_t>(layers);
    for (const auto& [id, thing] : things_) {
        for (const Draw& d : thing.draws) {
            server().instance_set_layer_mask(d.instance, layers_);
        }
    }
}

void KdKit::clear() {
    if (RS::get_singleton() == nullptr) {
        return;  // the engine is closing, and frees what is left itself
    }
    for (const auto& [id, thing] : things_) {
        for (const Draw& d : thing.draws) {
            server().free_rid(d.instance);
            server().free_rid(d.multimesh);
        }
    }
    things_.clear();
    for (const auto& [key, mesh] : meshes_) {
        server().free_rid(mesh);
    }
    meshes_.clear();
    for (const auto& [name, w] : wearables_) {
        server().free_rid(w.material);
        server().free_rid(w.texture);
    }
    wearables_.clear();
}

void KdKit::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("load_family", "family", "path"), &KdKit::load_family);
    ClassDB::bind_method(D_METHOD("use_world", "world"), &KdKit::use_world);
    ClassDB::bind_method(D_METHOD("models"), &KdKit::models);
    ClassDB::bind_method(D_METHOD("texture_names"), &KdKit::texture_names);
    ClassDB::bind_method(D_METHOD("model_info", "model"), &KdKit::model_info);
    ClassDB::bind_method(D_METHOD("parts", "family"), &KdKit::parts);
    ClassDB::bind_method(D_METHOD("part_info", "family", "part"), &KdKit::part_info);
    ClassDB::bind_method(D_METHOD("set_texture", "name", "path", "tile_metres"), &KdKit::set_texture);
    ClassDB::bind_method(D_METHOD("set_shader", "shader"), &KdKit::set_shader);
    ClassDB::bind_method(D_METHOD("set_origin", "east", "north"), &KdKit::set_origin);
    ClassDB::bind_method(D_METHOD("place", "scenario", "model", "seed", "east", "north", "up", "turn", "wear"),
                         &KdKit::place);
    ClassDB::bind_method(D_METHOD("place_part", "scenario", "family", "part", "east", "north", "up", "turn", "wear"),
                         &KdKit::place_part);
    ClassDB::bind_method(D_METHOD("move", "id", "east", "north", "up", "turn"), &KdKit::move);
    ClassDB::bind_method(D_METHOD("remove", "id"), &KdKit::remove);
    ClassDB::bind_method(D_METHOD("set_layers", "layers"), &KdKit::set_layers);
    ClassDB::bind_method(D_METHOD("clear"), &KdKit::clear);
}

}  // namespace kd::view
