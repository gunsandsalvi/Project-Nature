// The model kit on the screen (A6.1, A6.2, A4.6): families of parts read from their .kdkit files, recipes read from the
// world's catalogue, and things put together by the assembler and drawn as copies: for each part of a thing and each
// role it wears, one MultiMesh of its copies in the thing's own metres, drawn through Godot's RenderingServer by an
// instance that carries the thing to its place. The textures the roles wear are read from .kdtex files and bound to the
// family's shader, which reads them through the one sampling function (A4.2) with texture coordinates in metres (A6.4).
// This class converts and draws, and never decides for the world (WLD-13); the assembler's rules are
// view/src/kit_assemble.hpp's.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>

#include "kit.hpp"
#include "kit_assemble.hpp"
#include "world.hpp"

namespace kd::view {

/// Implements PRE-46, PRE-42 and PRE-22, see A6.1, A6.2 and A6.4: the kit's things, assembled and drawn.
class KdKit : public godot::RefCounted {
    GDCLASS(KdKit, godot::RefCounted)

public:
    ~KdKit() override;

    /// Reads a family's .kdkit file under the name its recipes give it, the file's name without its extension: "" or
    /// the problem.
    godot::String load_family(const godot::String& family, const godot::String& path);
    /// The world whose catalogue holds the recipes (A3.6), loaded first.
    void use_world(const godot::Ref<KdWorld>& world);
    /// The recipes' names in the catalogue, such as "art:standin_club", in order.
    [[nodiscard]] godot::PackedStringArray models() const;
    /// Every texture the recipes' roles may wear, by name, such as "art:standin_wood": the ones to hand set_texture.
    [[nodiscard]] godot::PackedStringArray texture_names() const;
    /// A recipe as the sheet shows it, for the seed 0: about, family, approved, roles (each role's textures), parts
    /// (the names of the parts it uses), copies, triangles, and its lowest and highest points in metres; or problem.
    [[nodiscard]] godot::Dictionary model_info(const godot::String& model) const;
    /// A family's parts, by name, in order.
    [[nodiscard]] godot::PackedStringArray parts(const godot::String& family) const;
    /// A part as the sheet shows it: its triangles, its roles with their triangles, its joints, its bounds, and the
    /// most its texture pixels stretch on any triangle (A6.4).
    [[nodiscard]] godot::Dictionary part_info(const godot::String& family, const godot::String& part) const;

    /// Makes a texture from a .kdtex file for the roles to wear, and its tile's width in metres, which sets how its
    /// pixels sit on a part (A6.4): "" or the problem.
    godot::String set_texture(const godot::String& name, const godot::String& path, double tile_metres);
    /// The shader every part is drawn with.
    void set_shader(const godot::RID& shader);
    /// The place the rig's origin is, in centimetres east and north, which every thing is placed about (A8.2).
    void set_origin(int64_t east, int64_t north);

    /// Puts a recipe's thing for a seed in a scenario at a place: centimetres east, north and up, and its turn in
    /// degrees clockwise from north. wear names a texture for a role, in place of the one the seed gave it. Gives id
    /// (to move or remove it), problem ("" if all is well), lowest and highest, in the thing's own metres, and
    /// triangles.
    godot::Dictionary place(const godot::RID& scenario, const godot::String& model, int64_t seed, int64_t east,
                            int64_t north, int64_t up, double turn, const godot::Dictionary& wear);
    /// Puts one part of a family alone, as the model sheet shows it, wearing the textures wear names for its roles.
    godot::Dictionary place_part(const godot::RID& scenario, const godot::String& family, const godot::String& part,
                                 int64_t east, int64_t north, int64_t up, double turn, const godot::Dictionary& wear);
    /// The triangles of a thing as it stands, for the view's maps (A4.4): every three points one triangle, in metres
    /// about the world's centre, x east, y up, z south, none for a thing not placed.
    [[nodiscard]] godot::PackedVector3Array thing_triangles(int64_t id) const;
    /// Moves a thing already placed.
    void move(int64_t id, int64_t east, int64_t north, int64_t up, double turn);
    /// Takes a thing off the screen.
    void remove(int64_t id);
    /// Puts the things on these visual layers, so only the cameras that see them draw them.
    void set_layers(int64_t layers);
    /// Frees everything it made in the RenderingServer.
    void clear();

protected:
    static void _bind_methods();

private:
    // a texture the roles wear: the array made from its file, the material over the family's shader, its tile's width
    struct Wearable {
        godot::RID texture;
        godot::RID material;
        double tile = 4.0;
    };
    // a thing on the screen: each draw is one part's role, its copies in a MultiMesh and the instance that shows it
    struct Draw {
        godot::RID multimesh;
        godot::RID instance;
    };
    struct Thing {
        std::vector<Draw> draws;
        std::vector<float> shape;  // its triangles in its own metres, nine numbers each
        std::int64_t east = 0;
        std::int64_t north = 0;
        std::int64_t up = 0;
        double turn = 0.0;
    };

    godot::Dictionary draw(const godot::RID& scenario, const kit::Assembly& assembly, const std::string& family_name,
                           const godot::Dictionary& wear, std::int64_t east, std::int64_t north, std::int64_t up,
                           double turn, std::uint64_t seed);
    godot::RID mesh_of(const std::string& family, const kit::Part& part, const kit::Section& section);
    void put(Thing& thing);

    godot::Ref<KdWorld> world_;
    std::map<std::string, kit::Family> families_;
    std::map<std::string, Wearable> wearables_;
    std::map<std::string, godot::RID> meshes_;  // by family/part/role
    std::map<std::int64_t, Thing> things_;
    godot::RID shader_;
    std::int64_t next_ = 1;
    std::int64_t origin_east_ = 0;
    std::int64_t origin_north_ = 0;
    std::uint32_t layers_ = 1;
};

}  // namespace kd::view
