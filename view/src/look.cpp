#include "look.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <cstring>
#include <span>

#include "kd/look/blind.hpp"
#include "kd/num/maths.hpp"
#include "textures.hpp"

namespace kd::view {

namespace {

godot::RenderingServer& server() {
    return *godot::RenderingServer::get_singleton();
}

// The stand-in ground (T2.1a.3): tiles of 64 m over 2 km round the world's centre, and an 8 m test board at it.
constexpr std::uint32_t kGround = 1;  // the family
constexpr std::uint32_t kTile = 0;    // its forms: a ground tile
constexpr std::uint32_t kBoard = 1;   // and the test board
constexpr double kTileMetres = 64.0;
constexpr std::int64_t kTilesAcross = 32;
constexpr double kBoardMetres = 8.0;
constexpr double kTextureTile = 4.0;  // metres a texture tile spans (A5.3)

// A flat square of a side in metres, centred on its origin, facing up.
godot::RID square(double side) {
    const auto h = static_cast<float>(side / 2.0);
    godot::PackedVector3Array vertices;
    vertices.push_back({-h, 0.0F, -h});
    vertices.push_back({h, 0.0F, -h});
    vertices.push_back({h, 0.0F, h});
    vertices.push_back({-h, 0.0F, h});
    godot::PackedVector3Array normals;
    for (int i = 0; i < 4; ++i) {
        normals.push_back({0.0F, 1.0F, 0.0F});
    }
    // Godot's front faces wind clockwise seen from the front (A4.7)
    godot::PackedInt32Array indices;
    for (const int i : {0, 1, 2, 0, 2, 3}) {
        indices.push_back(i);
    }
    godot::Array arrays;
    arrays.resize(godot::RenderingServer::ARRAY_MAX);
    arrays[godot::RenderingServer::ARRAY_VERTEX] = vertices;
    arrays[godot::RenderingServer::ARRAY_NORMAL] = normals;
    arrays[godot::RenderingServer::ARRAY_INDEX] = indices;
    const godot::RID mesh = server().mesh_create();
    server().mesh_add_surface_from_arrays(mesh, godot::RenderingServer::PRIMITIVE_TRIANGLES, arrays);
    return mesh;
}

std::int64_t modulo(std::int64_t a, std::int64_t b) {
    const std::int64_t m = a % b;
    return m < 0 ? m + b : m;
}

}  // namespace

KdLook::~KdLook() {
    clear();
}

void KdLook::press(int64_t finger, godot::Vector2 at, double seconds) {
    gestures_.press(static_cast<int>(finger), static_cast<double>(at.x), static_cast<double>(at.y), seconds);
}

void KdLook::move(int64_t finger, godot::Vector2 at, double seconds) {
    gestures_.move(static_cast<int>(finger), static_cast<double>(at.x), static_cast<double>(at.y), seconds);
}

void KdLook::lift(int64_t finger, godot::Vector2 at, double seconds) {
    gestures_.lift(static_cast<int>(finger), static_cast<double>(at.x), static_cast<double>(at.y), seconds);
}

void KdLook::zoom_by(double ratio, godot::Vector2 at) {
    rig_.pinch(ratio, static_cast<double>(at.x), static_cast<double>(at.y));
    rig_.hold(true);
    rig_.hold(false);
}

void KdLook::turn_by(double degrees, godot::Vector2 at) {
    rig_.twist(degrees, static_cast<double>(at.x), static_cast<double>(at.y));
    rig_.hold(true);
    rig_.hold(false);
}

void KdLook::set_screen(godot::Vector2 size) {
    rig_.set_screen(static_cast<double>(size.x), static_cast<double>(size.y));
}

void KdLook::set_lens(double degrees) {
    rig_.set_lens(degrees);
}

void KdLook::frame(double seconds) {
    clock_ += seconds;
    const Motion m = gestures_.take();
    if (path_.kind() != Path::Kind::none) {
        path_.apply(rig_, clock_ - path_started_);
    } else {
        if (m.pan_x != 0.0 || m.pan_y != 0.0) {
            rig_.drag(m.pan_x, m.pan_y, m.at_x, m.at_y);
        }
        if (m.scale != 1.0) {
            rig_.pinch(m.scale, m.at_x, m.at_y);
        }
        if (m.twist != 0.0) {
            rig_.twist(m.twist, m.at_x, m.at_y);
        }
        rig_.hold(m.touching);
        rig_.step(seconds);
    }
    if (rig_.origin_east() != origin_east_ || rig_.origin_north() != origin_north_) {
        origin_east_ = rig_.origin_east();
        origin_north_ = rig_.origin_north();
        place_all();
    }
    publish();
}

void KdLook::publish() const {
    auto& rs = server();
    rs.global_shader_parameter_set("kd_texel_least", Rig::kTexelLeast * many_);
    const auto tile = static_cast<std::int64_t>(kTextureTile * 100.0);
    rs.global_shader_parameter_set(
        "kd_origin_tile", godot::Vector2(static_cast<float>(modulo(origin_east_, tile)) / static_cast<float>(tile),
                                         static_cast<float>(modulo(-origin_north_, tile)) / static_cast<float>(tile)));
}

godot::Dictionary KdLook::pose() const {
    const Pose p = rig_.pose();
    godot::Transform3D t;
    t.origin = godot::Vector3(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z));
    t = t.looking_at(
        godot::Vector3(static_cast<float>(p.look_x), static_cast<float>(p.look_y), static_cast<float>(p.look_z)),
        godot::Vector3(0.0F, 1.0F, 0.0F));
    godot::Dictionary out;
    out["transform"] = t;
    out["fov"] = p.lens;
    out["keep_width"] = !p.wide;
    out["near"] = p.near;
    out["far"] = p.far;
    return out;
}

godot::Dictionary KdLook::state() const {
    godot::Dictionary out;
    out["focus_east"] = rig_.focus_east();
    out["focus_north"] = rig_.focus_north();
    out["heading"] = rig_.heading();
    out["metres_per_pixel"] = rig_.metres_per_pixel();
    out["metres_across"] = rig_.metres_per_pixel() * rig_.short_side();
    out["band"] = rig_.band();
    out["texel_pixels"] = rig_.texel_pixels(rig_.band());
    out["resting"] = rig_.resting();
    return out;
}

godot::String KdLook::load_layers(const godot::PackedStringArray& paths) {
    godot::TypedArray<godot::Image> images;
    std::int64_t side = 0;
    for (const godot::String& path : paths) {
        const godot::PackedByteArray bytes = godot::FileAccess::get_file_as_bytes(path);
        if (bytes.is_empty()) {
            return godot::String("no texture at ") + path;
        }
        const Levels levels = split_levels(std::span<const std::uint8_t>(bytes.ptr(), bytes.size()));
        if (!levels.problem.empty()) {
            return path + godot::String(": ") + godot::String(levels.problem.c_str());
        }
        godot::PackedByteArray all;
        std::int64_t width = 0;
        for (std::size_t i = 0; i < levels.pictures.size(); ++i) {
            godot::PackedByteArray png;
            png.resize(static_cast<int64_t>(levels.pictures[i].size()));
            std::memcpy(png.ptrw(), levels.pictures[i].data(), levels.pictures[i].size());
            godot::Ref<godot::Image> image;
            image.instantiate();
            if (image->load_png_from_buffer(png) != godot::OK) {
                return path + godot::String(": level ") + godot::String::num_int64(static_cast<int64_t>(i)) +
                       godot::String(" is not a PNG");
            }
            image->convert(godot::Image::FORMAT_RGBA8);
            if (i == 0) {
                width = image->get_width();
            }
            if (image->get_width() != (width >> i) || image->get_height() != (width >> i)) {
                return path + godot::String(": level ") + godot::String::num_int64(static_cast<int64_t>(i)) +
                       godot::String(" is not half the size of the one before");
            }
            all.append_array(image->get_data());
        }
        if (width == 0 || (width >> (levels.pictures.size() - 1)) != 1) {
            return path + godot::String(": its levels do not run down to one texture pixel");
        }
        if (side != 0 && width != side) {
            return path + godot::String(": a layer of another size");
        }
        side = width;
        images.push_back(godot::Image::create_from_data(static_cast<int32_t>(width), static_cast<int32_t>(width), true,
                                                        godot::Image::FORMAT_RGBA8, all));
    }
    if (layers_.is_valid()) {
        server().free_rid(layers_);
    }
    layers_ = server().texture_2d_layered_create(images, godot::RenderingServer::TEXTURE_LAYERED_2D_ARRAY);
    return "";
}

void KdLook::build(const godot::RID& scenario, const godot::RID& shader) {
    clear_drawing();
    scenario_ = scenario;
    material_ = server().material_create();
    server().material_set_shader(material_, shader);
    server().material_set_param(material_, "kd_layers", layers_);
    server().material_set_param(material_, "kd_tile_metres", kTextureTile);
    meshes_ = {square(kTileMetres), square(kBoardMetres)};
    for (const godot::RID& mesh : meshes_) {
        server().mesh_surface_set_material(mesh, 0, material_);
    }
    // the stand-in feed: the meadow's tiles, then the board just above them at the centre
    std::uint64_t id = 1;
    const auto tile_cm = static_cast<std::int64_t>(kTileMetres * 100.0);
    for (std::int64_t j = 0; j < kTilesAcross; ++j) {
        for (std::int64_t i = 0; i < kTilesAcross; ++i) {
            Copy c;
            c.family = kGround;
            c.form = kTile;
            c.east = (i - kTilesAcross / 2) * tile_cm + tile_cm / 2;
            c.north = (j - kTilesAcross / 2) * tile_cm + tile_cm / 2;
            c.look[0] = 0.0F;  // the meadow's layer
            stage_.put(id++, c);
        }
    }
    Copy board;
    board.family = kGround;
    board.form = kBoard;
    board.up = 1;          // a centimetre above the meadow
    board.look[0] = 1.0F;  // the test pattern's layer
    stage_.put(id, board);
    for (const Change& change : stage_.drain()) {
        if (change.gone) {
            const auto it = instances_.find(change.id);
            if (it != instances_.end()) {
                server().free_rid(it->second);
                instances_.erase(it);
            }
        } else if (const Copy* copy = stage_.find(change.id)) {
            place(change.id, *copy);
        }
    }
}

void KdLook::place(std::uint64_t id, const Copy& copy) {
    auto it = instances_.find(id);
    if (it == instances_.end()) {
        const godot::RID instance = server().instance_create2(meshes_[copy.form], scenario_);
        it = instances_.emplace(id, instance).first;
        server().instance_geometry_set_shader_parameter(instance, "kd_layer", copy.look[0]);
        // the ground is a big caster: it shades through the height-field sun map, never Godot's (A4.4)
        server().instance_geometry_set_cast_shadows_setting(instance,
                                                            godot::RenderingServer::SHADOW_CASTING_SETTING_OFF);
    }
    godot::Transform3D t;
    t.origin = godot::Vector3(static_cast<float>(static_cast<double>(copy.east - origin_east_) / 100.0),
                              static_cast<float>(static_cast<double>(copy.up) / 100.0),
                              static_cast<float>(-static_cast<double>(copy.north - origin_north_) / 100.0));
    server().instance_set_transform(it->second, t);
    const auto hidden = hidden_.find(copy.form);
    server().instance_set_visible(it->second, hidden == hidden_.end() || !hidden->second);
}

void KdLook::place_all() {
    for (const auto& [id, copy] : stage_.copies()) {
        place(id, copy);
    }
}

void KdLook::set_part(const godot::String& part, bool on) {
    if (part == "ground") {
        hidden_[kTile] = !on;
    } else if (part == "pattern") {
        hidden_[kBoard] = !on;
    } else {
        return;
    }
    place_all();
}

void KdLook::clear_drawing() {
    if (godot::RenderingServer::get_singleton() == nullptr) {
        return;  // the engine is closing, and frees what is left itself
    }
    for (const auto& [id, instance] : instances_) {
        server().free_rid(instance);
    }
    instances_.clear();
    for (const godot::RID& mesh : meshes_) {
        server().free_rid(mesh);
    }
    meshes_.clear();
    if (material_.is_valid()) {
        server().free_rid(material_);
        material_ = godot::RID();
    }
    stage_ = Stage();
}

void KdLook::clear() {
    clear_drawing();
    if (layers_.is_valid() && godot::RenderingServer::get_singleton() != nullptr) {
        server().free_rid(layers_);
        layers_ = godot::RID();
    }
}

void KdLook::set_view(int64_t east, int64_t north, double heading, double metres_per_pixel) {
    path_.start(Path::Kind::none, rig_);
    rig_.set_focus(east, north);
    rig_.set_heading(heading);
    rig_.set_metres_per_pixel(metres_per_pixel);
}

void KdLook::set_many(int64_t times) {
    many_ = static_cast<double>(std::max<int64_t>(1, times));
}

namespace {

kd::look::BlindTest blind_test(int64_t seed, const godot::Array& chose_first) {
    kd::look::BlindTest test{kd::look::Comparison::msaa, static_cast<std::uint32_t>(seed & 0xFFFF), {}};
    for (int64_t i = 0; i < chose_first.size(); ++i) {
        test.chose_first.push_back(static_cast<bool>(chose_first[i]));
    }
    return test;
}

}  // namespace

godot::Array KdLook::blind_pairs(int64_t seed) const {
    godot::Array out;
    for (const kd::look::BlindPair& p : kd::look::blind_pairs(static_cast<std::uint32_t>(seed & 0xFFFF))) {
        godot::Dictionary d;
        d["better_first"] = p.better_first;
        d["heading"] = p.heading;
        d["east"] = p.east;
        d["north"] = p.north;
        out.push_back(d);
    }
    return out;
}

godot::String KdLook::blind_code(int64_t seed, const godot::Array& chose_first) const {
    if (chose_first.size() != kd::look::kBlindPairs) {
        return {};
    }
    const std::string code = kd::look::blind_code(blind_test(seed, chose_first));
    return godot::String::utf8(code.c_str());
}

int64_t KdLook::blind_right(int64_t seed, const godot::Array& chose_first) const {
    if (chose_first.size() != kd::look::kBlindPairs) {
        return 0;
    }
    return kd::look::blind_right(blind_test(seed, chose_first));
}

void KdLook::play(const godot::String& path) {
    path_.start(Path::named(path.utf8().get_data()), rig_);
    path_started_ = clock_;
}

godot::String KdLook::playing() const {
    const std::string_view name = Path::name(path_.kind());
    return godot::String::utf8(name.data(), static_cast<int64_t>(name.size()));
}

void KdLook::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("press", "finger", "at", "seconds"), &KdLook::press);
    ClassDB::bind_method(D_METHOD("move", "finger", "at", "seconds"), &KdLook::move);
    ClassDB::bind_method(D_METHOD("lift", "finger", "at", "seconds"), &KdLook::lift);
    ClassDB::bind_method(D_METHOD("zoom_by", "ratio", "at"), &KdLook::zoom_by);
    ClassDB::bind_method(D_METHOD("turn_by", "degrees", "at"), &KdLook::turn_by);
    ClassDB::bind_method(D_METHOD("set_screen", "size"), &KdLook::set_screen);
    ClassDB::bind_method(D_METHOD("set_lens", "degrees"), &KdLook::set_lens);
    ClassDB::bind_method(D_METHOD("frame", "seconds"), &KdLook::frame);
    ClassDB::bind_method(D_METHOD("pose"), &KdLook::pose);
    ClassDB::bind_method(D_METHOD("state"), &KdLook::state);
    ClassDB::bind_method(D_METHOD("load_layers", "paths"), &KdLook::load_layers);
    ClassDB::bind_method(D_METHOD("build", "scenario", "shader"), &KdLook::build);
    ClassDB::bind_method(D_METHOD("set_part", "part", "on"), &KdLook::set_part);
    ClassDB::bind_method(D_METHOD("clear"), &KdLook::clear);
    ClassDB::bind_method(D_METHOD("set_view", "east", "north", "heading", "metres_per_pixel"), &KdLook::set_view);
    ClassDB::bind_method(D_METHOD("set_many", "times"), &KdLook::set_many);
    ClassDB::bind_method(D_METHOD("blind_pairs", "seed"), &KdLook::blind_pairs);
    ClassDB::bind_method(D_METHOD("blind_code", "seed", "chose_first"), &KdLook::blind_code);
    ClassDB::bind_method(D_METHOD("blind_right", "seed", "chose_first"), &KdLook::blind_right);
    ClassDB::bind_method(D_METHOD("play", "path"), &KdLook::play);
    ClassDB::bind_method(D_METHOD("playing"), &KdLook::playing);
}

}  // namespace kd::view
