// Implements PRE-20 PRE-21 PRE-23 PRE-24 PRE-30: receivers use their measured world surface and normal.
shader_type canvas_item;
render_mode unshaded;
#include "res://terrain/light.gdshaderinc"
uniform vec3 corner0;
uniform vec3 corner1;
uniform vec3 corner2;
uniform vec3 corner3;
uniform vec3 surface_normal = vec3(0.0, 0.0, 1.0);
uniform sampler2D visibility_mask : filter_linear, repeat_disable;
uniform vec2 repeats = vec2(1.0);
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
void fragment() {
    vec4 albedo = texture(TEXTURE, UV * repeats);
    if (material_kind == 3) {
        float layer = mod(floor(world_point.z * 1.4), 3.0);
        albedo.rgb *= layer < 1.0 ? vec3(0.71, 0.67, 0.61) : layer < 2.0 ? vec3(0.83, 0.72, 0.63) : vec3(0.66, 0.70, 0.72);
    }
    if (material_kind == 2) { albedo.rgb *= vec3(0.89, 0.72, 0.51); }
    vec3 n = has_normal ? normalize(surface_normal + (texture(normal_atlas,UV*repeats).rgb*2.0-1.0-vec3(0.0,0.0,1.0))*0.3) : surface_normal;
    int material_index = has_material ? int(round(texture(material_atlas,UV*repeats).r*255.0)) : material_kind;
    COLOR = vec4(fixture_light(albedo.rgb, n, world_point, texture(visibility_mask, UV), 0.0, material_index), albedo.a) * vertex_colour;
}
