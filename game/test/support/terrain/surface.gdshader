// Implements PRE-20 PRE-21 PRE-23 PRE-24 PRE-30: receivers use their measured world surface and normal.
shader_type canvas_item;
render_mode unshaded;
#include "res://test/support/terrain/light.gdshaderinc"
uniform vec3 corner0;
uniform vec3 corner1;
uniform vec3 corner2;
uniform vec3 corner3;
uniform vec3 surface_normal = vec3(0.0, 0.0, 1.0);
uniform sampler2D visibility_mask : filter_linear, repeat_disable;
uniform vec2 repeats = vec2(1.0);
uniform bool world_tiled = false;
uniform float tile_metres = 4.0;
uniform vec2 art_phase = vec2(0.0);
uniform bool ground_variants = false;
uniform sampler2D variant_field : filter_nearest, repeat_disable;
uniform vec4 variant_grid;
uniform vec2 variant_phase;
uniform vec2 variant_origin_tile;
uniform vec2 variant_tile_count;
uniform vec4 variant_edges;
uniform vec2 variant_world_span;
uniform float variant_cell_metres = 16.0;
uniform sampler2D variant_b_colour : filter_nearest, repeat_enable;
uniform sampler2D variant_b_normal : filter_nearest, repeat_enable;
uniform sampler2D variant_b_material : filter_nearest, repeat_enable;
uniform sampler2D variant_c_colour : filter_nearest, repeat_enable;
uniform sampler2D variant_c_normal : filter_nearest, repeat_enable;
uniform sampler2D variant_c_material : filter_nearest, repeat_enable;
uniform int material_kind = 4;
uniform sampler2D normal_atlas : filter_nearest, repeat_enable;
uniform sampler2D material_atlas : filter_nearest, repeat_enable;
uniform bool has_normal = false;
uniform bool has_material = false;
varying vec3 world_point;
varying vec4 vertex_colour;
void vertex() {
    vertex_colour = COLOR;
    world_point = mix(mix(corner0, corner1, UV.x), mix(corner3, corner2, UV.x), UV.y);
}
int ground_variant(vec2 point) {
    if (!ground_variants) { return 0; }
    vec2 tile = variant_origin_tile + floor((point + variant_phase) / variant_cell_metres);
    // Keep close local coordinates precise; account for the world's last partial cells at wraps.
    if (point.x >= variant_edges.z) { tile.x = floor(mod(point.x - variant_edges.z,variant_world_span.x)/variant_cell_metres); }
    if (point.y >= variant_edges.w) { tile.y = floor(mod(point.y - variant_edges.w,variant_world_span.y)/variant_cell_metres); }
    if (point.x < -variant_edges.x) { tile.x = floor(mod(point.x + variant_edges.x,variant_world_span.x)/variant_cell_metres); }
    if (point.y < -variant_edges.y) { tile.y = floor(mod(point.y + variant_edges.y,variant_world_span.y)/variant_cell_metres); }
    vec2 cell = mod(tile - variant_grid.xy, variant_tile_count);
    if (any(greaterThanEqual(cell,variant_grid.zw))) { return 0; }
    return int(round(texture(variant_field,(cell+vec2(0.5))/variant_grid.zw).r*255.0));
}
void fragment() {
    vec2 art_uv = world_tiled ? vec2(world_point.x + art_phase.x,-world_point.y - art_phase.y) / tile_metres : UV * repeats;
    int variant = ground_variant(world_point.xy);
    vec4 albedo = variant == 1 ? texture(variant_b_colour,art_uv) : variant == 2 ? texture(variant_c_colour,art_uv) : texture(TEXTURE, art_uv);
    vec3 normal_value = variant == 1 ? texture(variant_b_normal,art_uv).rgb : variant == 2 ? texture(variant_c_normal,art_uv).rgb : texture(normal_atlas,art_uv).rgb;
    float category = variant == 1 ? texture(variant_b_material,art_uv).r : variant == 2 ? texture(variant_c_material,art_uv).r : texture(material_atlas,art_uv).r;
    if (material_kind == 3) {
        float layer = mod(floor(world_point.z * 1.4), 3.0);
        albedo.rgb *= layer < 1.0 ? vec3(0.71, 0.67, 0.61) : layer < 2.0 ? vec3(0.83, 0.72, 0.63) : vec3(0.66, 0.70, 0.72);
    }
    if (material_kind == 2) { albedo.rgb *= vec3(0.89, 0.72, 0.51); }
    vec3 n = has_normal ? normalize(surface_normal + (normal_value*2.0-1.0-vec3(0.0,0.0,1.0))*0.3) : surface_normal;
    int material_index = has_material ? int(round(category*255.0)) : material_kind;
    vec3 lit = fixture_light_linear(albedo.rgb, n, world_point, texture(visibility_mask, UV), 0.0, material_index);
    COLOR = vec4(fixture_encode(lit * fixture_decode(vertex_colour.rgb)), albedo.a * vertex_colour.a);
}
