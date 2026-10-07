#include "figures.hpp"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include <chrono>
#include <cstring>

#include "poser.hpp"

namespace kd::view {

namespace {

godot::RenderingServer& server() {
    return *godot::RenderingServer::get_singleton();
}

const FigureMesh& stand_in() {
    static const FigureMesh mesh = figure_mesh();
    return mesh;
}

// A palette's texels a figure: its row's width.
int32_t row_texels() {
    return static_cast<int32_t>(figure_bones().size()) * kTexelsABone;
}

// The stand-in figure's mesh with its bones and weights, as triangles or as points, one a vertex in the same order. Its
// bounds take in every pose of the walk, so Godot never culls a figure for how it is posed.
godot::RID figure_rid(bool points) {
    const FigureMesh& m = stand_in();
    godot::PackedVector3Array vertices;
    godot::PackedVector3Array normals;
    for (std::size_t i = 0; i + 2 < m.positions.size(); i += 3) {
        vertices.push_back({m.positions[i], m.positions[i + 1], m.positions[i + 2]});
        normals.push_back({m.normals[i], m.normals[i + 1], m.normals[i + 2]});
    }
    godot::PackedInt32Array bones;
    bones.resize(static_cast<int64_t>(m.bones.size()));
    std::memcpy(bones.ptrw(), m.bones.data(), m.bones.size() * sizeof(std::int32_t));
    godot::PackedFloat32Array weights;
    weights.resize(static_cast<int64_t>(m.weights.size()));
    std::memcpy(weights.ptrw(), m.weights.data(), m.weights.size() * sizeof(float));
    godot::Array arrays;
    arrays.resize(godot::RenderingServer::ARRAY_MAX);
    arrays[godot::RenderingServer::ARRAY_VERTEX] = vertices;
    arrays[godot::RenderingServer::ARRAY_NORMAL] = normals;
    arrays[godot::RenderingServer::ARRAY_BONES] = bones;
    arrays[godot::RenderingServer::ARRAY_WEIGHTS] = weights;
    if (!points) {
        godot::PackedInt32Array indices;
        indices.resize(static_cast<int64_t>(m.indices.size()));
        std::memcpy(indices.ptrw(), m.indices.data(), m.indices.size() * sizeof(std::int32_t));
        arrays[godot::RenderingServer::ARRAY_INDEX] = indices;
    }
    const godot::RID mesh = server().mesh_create();
    server().mesh_add_surface_from_arrays(
        mesh, points ? godot::RenderingServer::PRIMITIVE_POINTS : godot::RenderingServer::PRIMITIVE_TRIANGLES, arrays);
    server().mesh_set_custom_aabb(mesh, godot::AABB({-1.0F, -0.2F, -1.0F}, {2.0F, 2.4F, 2.0F}));
    return mesh;
}

godot::Transform3D transform_of(const BoneRows& r) {
    const auto& m = r.m;
    return {godot::Basis(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]),
            godot::Vector3(m[0][3], m[1][3], m[2][3])};
}

// Where a figure stands, facing the way its seed gives.
godot::Transform3D standing(std::uint64_t seed, const godot::Vector3& at) {
    return {godot::Basis(godot::Vector3(0.0F, 1.0F, 0.0F), facing(seed)), at};
}

}  // namespace

KdFigures::~KdFigures() {
    clear();
}

void KdFigures::build(const godot::RID& scenario, const godot::String& way, const godot::RID& shader,
                      const godot::PackedVector3Array& places, bool points) {
    clear();
    auto& rs = server();
    palette_way_ = way == "palette";
    mesh_ = figure_rid(points);
    material_ = rs.material_create();
    rs.material_set_shader(material_, shader);
    rs.mesh_surface_set_material(mesh_, 0, material_);
    const auto count = static_cast<std::size_t>(places.size());
    steps_.assign(count, 0);
    if (count == 0) {
        return;
    }
    if (palette_way_) {
        multimesh_ = rs.multimesh_create();
        rs.multimesh_set_mesh(multimesh_, mesh_);
        rs.multimesh_allocate_data(multimesh_, static_cast<int32_t>(count),
                                   godot::RenderingServer::MULTIMESH_TRANSFORM_3D, false, true);
        for (std::size_t i = 0; i < count; ++i) {
            const auto copy = static_cast<int32_t>(i);
            rs.multimesh_instance_set_transform(multimesh_, copy, standing(i, places[static_cast<int64_t>(i)]));
            // the copy's row of the palette
            rs.multimesh_instance_set_custom_data(multimesh_, copy, godot::Color(static_cast<float>(i), 0.0F, 0.0F));
        }
        rows_.resize(static_cast<int64_t>(count) * row_texels() * 4 * 2);
        for (std::size_t i = 0; i < count; ++i) {
            pose(i, pose_step(i, clock_));
        }
        image_ = godot::Image::create_from_data(row_texels(), static_cast<int32_t>(count), false,
                                                godot::Image::FORMAT_RGBAH, rows_);
        palette_ = rs.texture_2d_create(image_);
        rows_changed_ = false;
        rs.material_set_param(material_, "kd_palette", palette_);
        instances_.push_back(rs.instance_create2(multimesh_, scenario));
        return;
    }
    const auto bones = static_cast<int32_t>(figure_bones().size());
    for (std::size_t i = 0; i < count; ++i) {
        const godot::RID skeleton = rs.skeleton_create();
        rs.skeleton_allocate_data(skeleton, bones, false);
        skeletons_.push_back(skeleton);
        const godot::RID instance = rs.instance_create2(mesh_, scenario);
        rs.instance_attach_skeleton(instance, skeleton);
        rs.instance_set_transform(instance, standing(i, places[static_cast<int64_t>(i)]));
        instances_.push_back(instance);
        pose(i, pose_step(i, clock_));
    }
}

void KdFigures::pose(std::size_t figure, std::int64_t step) {
    steps_[figure] = step;
    const std::vector<BoneRows> rows = figure_pose(figure, step);
    if (palette_way_) {
        const std::vector<std::uint16_t> halves = palette_row(rows);
        std::memcpy(rows_.ptrw() + figure * halves.size() * sizeof(std::uint16_t), halves.data(),
                    halves.size() * sizeof(std::uint16_t));
        rows_changed_ = true;
        return;
    }
    for (std::size_t b = 0; b < rows.size(); ++b) {
        server().skeleton_bone_set_transform(skeletons_[figure], static_cast<int32_t>(b), transform_of(rows[b]));
    }
}

double KdFigures::frame(double seconds) {
    const auto started = std::chrono::steady_clock::now();
    clock_ += seconds;
    for (std::size_t i = 0; i < steps_.size(); ++i) {
        const std::int64_t step = pose_step(i, clock_);
        if (step != steps_[i]) {
            pose(i, step);
        }
    }
    if (rows_changed_) {
        image_->set_data(row_texels(), static_cast<int32_t>(steps_.size()), false, godot::Image::FORMAT_RGBAH, rows_);
        server().texture_2d_update(palette_, image_, 0);
        rows_changed_ = false;
    }
    return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - started).count();
}

int64_t KdFigures::triangles() const {
    return static_cast<int64_t>(stand_in().indices.size() / 3);
}

int64_t KdFigures::vertices() const {
    return static_cast<int64_t>(stand_in().positions.size() / 3);
}

void KdFigures::clear() {
    steps_.clear();
    rows_changed_ = false;
    if (godot::RenderingServer::get_singleton() == nullptr) {
        return;  // the engine is closing, and frees what is left itself
    }
    auto& rs = server();
    for (const godot::RID& instance : instances_) {
        rs.free_rid(instance);
    }
    instances_.clear();
    for (const godot::RID& skeleton : skeletons_) {
        rs.free_rid(skeleton);
    }
    skeletons_.clear();
    for (godot::RID* rid : {&multimesh_, &mesh_, &material_, &palette_}) {
        if (rid->is_valid()) {
            rs.free_rid(*rid);
            *rid = godot::RID();
        }
    }
}

void KdFigures::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("build", "scenario", "way", "shader", "places", "points"), &KdFigures::build);
    ClassDB::bind_method(D_METHOD("frame", "seconds"), &KdFigures::frame);
    ClassDB::bind_method(D_METHOD("triangles"), &KdFigures::triangles);
    ClassDB::bind_method(D_METHOD("vertices"), &KdFigures::vertices);
    ClassDB::bind_method(D_METHOD("clear"), &KdFigures::clear);
}

}  // namespace kd::view
