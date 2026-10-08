// The stand-in area on the screen (A4.6, A5.3, A4.5, T2.3b.2, T2.3b.4): the land and the river's water that
// view/src/area.hpp makes, drawn through Godot's RenderingServer with the surfaces the tuning names, their tiles and
// versions read through the ladder (game/look/ladder.gdshaderinc). It draws three things: the meadow's carpet, the
// strip of heightfield the river runs in, with the bed below the water's level, and the water's surface over it. This
// class converts and draws, and never decides for the world (WLD-13).
#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "area.hpp"
#include "world.hpp"

namespace kd::view {

/// Implements PRE-23, PRE-26 and PRE-22, see A4.6, A4.5 and A5.3: the stand-in area, drawn.
class KdArea : public godot::RefCounted {
    GDCLASS(KdArea, godot::RefCounted)

public:
    ~KdArea() override;

    /// Reads the area's shape from the world's catalogue, loaded first (A3.6): "" or the problem. The surfaces it
    /// names are in surfaces().
    godot::String use_world(const godot::Ref<KdWorld>& world);
    /// The surfaces the tuning names, by their role: ground, bed, marks, earth, worn and gravel, each the entry of its
    /// near tile, such as art:meadow.
    [[nodiscard]] godot::Dictionary surfaces() const;
    /// Makes the surface of a role from its tiles' .kdtex files: paths, the near tile's versions first, then the
    /// middle tile's and the far's; count, how many versions each tile has; first, the band each tile starts at, a
    /// band past the last for a tile it has none of (A5.3). "" or the problem.
    godot::String set_surface(const godot::String& role, const godot::PackedStringArray& paths,
                              const godot::PackedInt32Array& count, const godot::PackedInt32Array& first);
    /// Puts the land and the water into a scenario, drawn by these shaders: the flat land's, the strip's with the
    /// river and the water's; "" or the problem. Call after use_world and the three surfaces.
    godot::String build(const godot::RID& scenario, const godot::RID& land, const godot::RID& river,
                        const godot::RID& water);
    /// The place the rig's origin is, in centimetres east and north, which everything is placed about (A8.2).
    void set_origin(int64_t east, int64_t north);
    /// Puts the land and the water on these visual layers, so only the cameras that see them draw them.
    void set_layers(int64_t layers);
    /// The ground's height at a place in centimetres east and north of the area's centre, in metres up from the
    /// meadow.
    [[nodiscard]] double ground_height(int64_t east, int64_t north) const;
    /// Where the river's banks lie at a place along it, in centimetres east: how far north and south of its middle
    /// line, in metres, as a vector of the north bank and the south.
    [[nodiscard]] godot::Vector2 banks_at(int64_t east) const;
    /// The river as the pages show it: level and deepest (metres), reach and strip (metres), triangles of the three
    /// meshes, and whether it is built.
    [[nodiscard]] godot::Dictionary info() const;
    /// Frees everything it made in the RenderingServer.
    void clear();

protected:
    static void _bind_methods();

private:
    // a surface: its texture array, how many versions each tile has and the band each starts at
    struct Surface {
        godot::RID texture;
        godot::Vector3 count;
        godot::Vector3 first;
    };

    godot::RID mesh_of(const area::Mesh& mesh, bool beds);
    godot::RID material_of(const godot::RID& shader, const std::vector<std::pair<const char*, const char*>>& worn);
    void place_all();
    void clear_drawing_only();

    std::optional<area::River> river_;  // none until the world is read
    std::string ground_;
    std::string bed_;
    std::string marks_;
    std::string earth_;
    std::string worn_;
    std::string gravel_;
    std::map<std::string, Surface> surface_;
    godot::RID scenario_;
    std::vector<godot::RID> meshes_;
    std::vector<godot::RID> materials_;
    std::vector<godot::RID> instances_;
    std::vector<std::int64_t> triangles_;
    std::int64_t origin_east_ = 0;
    std::int64_t origin_north_ = 0;
    std::uint32_t layers_ = 1;
};

}  // namespace kd::view
