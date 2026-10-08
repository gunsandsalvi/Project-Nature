// Implements PRE-31, RES-05 (T2.7a.4): semantic captures preserve the colour frame's alpha coverage.
shader_type canvas_item;
render_mode unshaded;
#include "res://fixtures/height.gdshaderinc"
uniform vec4 identity_colour;
uniform bool inspect_alpha = false;
// T2.8a PRE-26/PRE-31: semantic coverage uses the same wading cutoff and optional material IDs.
uniform bool clip_water = false;
uniform float water_height = -100.0;
uniform vec4 water_bounds = vec4(0.0);
uniform vec3 foot;
uniform vec2 pivot;
uniform float density = 32.0;
uniform float height_basis = 1.0;
uniform bool use_material_index = false;
uniform sampler2D material_atlas : filter_nearest, repeat_disable;
varying vec3 world_point;
void vertex() { world_point = fixture_sprite_point(foot, pivot, VERTEX, density, height_basis); }
void fragment() {
    float alpha = texture(TEXTURE, UV).a;
    if (alpha <= 0.1 || (clip_water && fixture_inside_water(world_point,water_bounds) && world_point.z < water_height)) { discard; }
    vec3 code = identity_colour.rgb;
    if (use_material_index) { code = vec3(round(texture(material_atlas,UV).r*255.0)/255.0,0.0,0.0); }
    COLOR = inspect_alpha ? vec4(vec3(alpha), 1.0) : vec4(code, 1.0);
}
