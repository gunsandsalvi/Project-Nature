#include "area_draw.hpp"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include "kd/look/area_tuning.hpp"
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

// A length in millimetres as metres.
double metres(std::int64_t millimetres) {
    return static_cast<double>(millimetres) / 1000.0;
}

// The area's shape from its tuning.
area::Shape shape_of(const look::AreaTuning& t) {
    area::Shape s;
    s.reach = metres(t.reach);
    s.strip = metres(t.strip);
    s.bank = metres(t.bank);
    s.width = metres(t.width);
    s.width_wobble = static_cast<double>(t.width_wobble) / 1e6;
    s.depth = metres(t.depth);
    s.depth_wobble = static_cast<double>(t.depth_wobble) / 1e6;
    s.wobble_length = metres(t.wobble_length);
    s.run = metres(t.run);
    s.spacing = metres(t.spacing);
    s.seed = static_cast<std::uint64_t>(t.seed);
    return s;
}

}  // namespace

KdArea::~KdArea() {
    clear();
}

godot::String KdArea::use_world(const godot::Ref<KdWorld>& world) {
    const data::Catalogue* cat = world.is_valid() ? world->catalogue() : nullptr;
    if (cat == nullptr || cat->kind<look::AreaTuning>().size() != 1) {
        return "the catalogue has no tuning/area";
    }
    const look::AreaTuning& tuning = cat->kind<look::AreaTuning>()[0];
    river_ = area::River(shape_of(tuning));
    ground_ = tuning.ground;
    bed_ = tuning.bed;
    marks_ = tuning.marks;
    earth_ = tuning.earth;
    worn_ = tuning.worn;
    gravel_ = tuning.gravel;
    return "";
}

godot::Dictionary KdArea::surfaces() const {
    godot::Dictionary out;
    out["ground"] = text(ground_);
    out["bed"] = text(bed_);
    out["marks"] = text(marks_);
    out["earth"] = text(earth_);
    out["worn"] = text(worn_);
    out["gravel"] = text(gravel_);
    return out;
}

godot::String KdArea::set_surface(const godot::String& role, const godot::PackedStringArray& paths,
                                  const godot::PackedInt32Array& count, const godot::PackedInt32Array& first) {
    if (count.size() != 3 || first.size() != 3) {
        return role + godot::String(": a surface has three tiles, near, middle and far");
    }
    std::int64_t layers = 0;
    for (std::int64_t i = 0; i < 3; ++i) {
        layers += count[i];
    }
    if (layers != paths.size() || layers == 0) {
        return role + godot::String(": its tiles' versions are not as many as the files given");
    }
    const Layered made = make_layered(paths);
    if (!made.problem.is_empty()) {
        return made.problem;
    }
    Surface& surface = surface_[text(role)];
    if (surface.texture.is_valid()) {
        server().free_rid(surface.texture);
    }
    surface.texture = made.texture;
    surface.count = {static_cast<float>(count[0]), static_cast<float>(count[1]), static_cast<float>(count[2])};
    surface.first = {static_cast<float>(first[0]), static_cast<float>(first[1]), static_cast<float>(first[2])};
    return "";
}

godot::RID KdArea::mesh_of(const area::Mesh& mesh, bool beds) {
    godot::PackedVector3Array positions;
    godot::PackedVector3Array normals;
    godot::PackedVector2Array second;
    for (std::size_t i = 0; i < mesh.vertices(); ++i) {
        positions.push_back({mesh.positions[3 * i], mesh.positions[3 * i + 1], mesh.positions[3 * i + 2]});
        normals.push_back({mesh.normals[3 * i], mesh.normals[3 * i + 1], mesh.normals[3 * i + 2]});
        second.push_back({mesh.beds[i], 0.0F});
    }
    godot::PackedInt32Array indices;
    for (const std::uint32_t i : mesh.indices) {
        indices.push_back(static_cast<int32_t>(i));
    }
    godot::Array arrays;
    arrays.resize(RS::ARRAY_MAX);
    arrays[RS::ARRAY_VERTEX] = positions;
    arrays[RS::ARRAY_NORMAL] = normals;
    if (beds) {
        arrays[RS::ARRAY_TEX_UV2] = second;
    }
    arrays[RS::ARRAY_INDEX] = indices;
    const godot::RID rid = server().mesh_create();
    server().mesh_add_surface_from_arrays(rid, RS::PRIMITIVE_TRIANGLES, arrays);
    return rid;
}

// A material over a shader, wearing the surfaces it is given as pairs of a role and the prefix of its uniforms: the
// ground's are kd_layers, kd_count and kd_first, the others' kd_<name>_layers, kd_<name>_count and kd_<name>_first.
godot::RID KdArea::material_of(const godot::RID& shader, const std::vector<std::pair<const char*, const char*>>& worn) {
    const godot::RID material = server().material_create();
    server().material_set_shader(material, shader);
    for (const auto& [role, prefix] : worn) {
        const Surface& surface = surface_[role];
        const godot::String stem(prefix);
        server().material_set_param(material, godot::StringName(stem + godot::String("layers")), surface.texture);
        server().material_set_param(material, godot::StringName(stem + godot::String("count")), surface.count);
        server().material_set_param(material, godot::StringName(stem + godot::String("first")), surface.first);
    }
    return material;
}

godot::String KdArea::build(const godot::RID& scenario, const godot::RID& land, const godot::RID& river,
                            const godot::RID& water) {
    if (!river_) {
        return "the area has no world: call use_world first";
    }
    for (const char* role : {"ground", "bed", "marks", "earth", "worn", "gravel"}) {
        const auto found = surface_.find(role);
        if (found == surface_.end() || !found->second.texture.is_valid()) {
            return godot::String("the area has no surface for ") + role + godot::String(": call set_surface first");
        }
    }
    clear_drawing_only();
    scenario_ = scenario;
    // the carpet wears the ground and its two other grounds, the strip also the bed and the bank's gravel, the water
    // the marks
    struct Part {
        area::Mesh mesh;
        bool beds;
        godot::RID shader;
        std::vector<std::pair<const char*, const char*>> worn;
    };
    const std::vector<std::pair<const char*, const char*>> ground{
        {"ground", "kd_"}, {"earth", "kd_earth_"}, {"worn", "kd_worn_"}};
    std::vector<std::pair<const char*, const char*>> strip = ground;
    strip.push_back({"bed", "kd_bed_"});
    strip.push_back({"gravel", "kd_gravel_"});
    const std::vector<Part> parts{
        {river_->carpet(), false, land, ground},
        {river_->strip(), false, river, strip},
        {river_->surface(), true, water, {{"marks", "kd_"}}},
    };
    for (const Part& part : parts) {
        const godot::RID mesh = mesh_of(part.mesh, part.beds);
        const godot::RID material = material_of(part.shader, part.worn);
        server().mesh_surface_set_material(mesh, 0, material);
        const godot::RID instance = server().instance_create2(mesh, scenario);
        // the ground is a big caster that shades through the height-field sun map, never Godot's (A4.4), and the
        // water casts none
        server().instance_geometry_set_cast_shadows_setting(instance, RS::SHADOW_CASTING_SETTING_OFF);
        server().instance_set_layer_mask(instance, layers_);
        meshes_.push_back(mesh);
        materials_.push_back(material);
        instances_.push_back(instance);
        triangles_.push_back(static_cast<std::int64_t>(part.mesh.triangles()));
    }
    place_all();
    return "";
}

void KdArea::set_origin(int64_t east, int64_t north) {
    origin_east_ = east;
    origin_north_ = north;
    place_all();
}

void KdArea::place_all() {
    // the area's centre is the world's: it lies at minus the origin, east is x and north is -z
    godot::Transform3D t;
    t.origin = godot::Vector3(static_cast<float>(static_cast<double>(-origin_east_) / 100.0), 0.0F,
                              static_cast<float>(static_cast<double>(origin_north_) / 100.0));
    for (const godot::RID& instance : instances_) {
        server().instance_set_transform(instance, t);
    }
}

void KdArea::set_layers(int64_t layers) {
    layers_ = static_cast<std::uint32_t>(layers);
    for (const godot::RID& instance : instances_) {
        server().instance_set_layer_mask(instance, layers_);
    }
}

double KdArea::ground_height(int64_t east, int64_t north) const {
    return river_ ? river_->height(static_cast<double>(east) / 100.0, static_cast<double>(north) / 100.0) : 0.0;
}

godot::Vector2 KdArea::banks_at(int64_t east) const {
    if (!river_) {
        return {};
    }
    const area::Banks b = river_->banks(static_cast<double>(east) / 100.0);
    return {static_cast<float>(b.north), static_cast<float>(b.south)};
}

godot::Dictionary KdArea::info() const {
    godot::Dictionary out;
    out["built"] = !instances_.empty();
    if (river_) {
        out["level"] = river_->level();
        out["reach"] = river_->shape().reach;
        out["strip"] = river_->shape().strip;
        out["deepest"] = river_->deepest(0.0);
        out["fine"] = river_->fine();
    }
    godot::Array triangles;
    for (const std::int64_t n : triangles_) {
        triangles.append(n);
    }
    out["triangles"] = triangles;
    return out;
}

void KdArea::clear_drawing_only() {
    if (RS::get_singleton() == nullptr) {
        return;  // the engine is closing, and frees what is left itself
    }
    for (const godot::RID& instance : instances_) {
        server().free_rid(instance);
    }
    instances_.clear();
    for (const godot::RID& mesh : meshes_) {
        server().free_rid(mesh);
    }
    meshes_.clear();
    for (const godot::RID& material : materials_) {
        server().free_rid(material);
    }
    materials_.clear();
    triangles_.clear();
}

void KdArea::clear() {
    clear_drawing_only();
    if (RS::get_singleton() == nullptr) {
        return;
    }
    for (const auto& [role, surface] : surface_) {
        if (surface.texture.is_valid()) {
            server().free_rid(surface.texture);
        }
    }
    surface_.clear();
}

void KdArea::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("use_world", "world"), &KdArea::use_world);
    ClassDB::bind_method(D_METHOD("surfaces"), &KdArea::surfaces);
    ClassDB::bind_method(D_METHOD("set_surface", "role", "paths", "count", "first"), &KdArea::set_surface);
    ClassDB::bind_method(D_METHOD("build", "scenario", "land", "river", "water"), &KdArea::build);
    ClassDB::bind_method(D_METHOD("set_origin", "east", "north"), &KdArea::set_origin);
    ClassDB::bind_method(D_METHOD("set_layers", "layers"), &KdArea::set_layers);
    ClassDB::bind_method(D_METHOD("ground_height", "east", "north"), &KdArea::ground_height);
    ClassDB::bind_method(D_METHOD("banks_at", "east"), &KdArea::banks_at);
    ClassDB::bind_method(D_METHOD("info"), &KdArea::info);
    ClassDB::bind_method(D_METHOD("clear"), &KdArea::clear);
}

}  // namespace kd::view
