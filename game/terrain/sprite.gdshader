// Implements PRE-20 PRE-21 PRE-24 PRE-28 PRE-30: atlas-aligned masks/normals, body bands and reveal.
shader_type canvas_item;
render_mode unshaded;
#include "res://terrain/light.gdshaderinc"
uniform sampler2D normal_atlas : filter_nearest, repeat_disable;
uniform sampler2D material_atlas : filter_nearest, repeat_disable;
uniform bool has_normal = false;
uniform bool has_material = false;
uniform vec3 foot;
uniform vec2 pivot;
uniform float density = 32.0;
uniform float height_basis;
uniform vec3 sun_bands = vec3(1.0);
uniform vec3 sky_bands = vec3(1.0);
uniform vec3 fire_bands = vec3(1.0);
uniform float band_min_height = 0.0;
uniform float band_max_height = 1.65;
uniform float fade = 1.0;
uniform float water_height = -100.0;
uniform bool silhouette = false;
uniform float emission = 0.0;
uniform int material_kind = 5;
uniform vec3 emission_colour = vec3(1.0);
varying vec3 world_point;
varying vec4 vertex_colour;
void vertex() {
    vertex_colour = COLOR;
    world_point = foot + vec3((VERTEX.x - pivot.x) / density, 0.0, (pivot.y - VERTEX.y) / (density * height_basis));
}
void fragment() {
    vec4 albedo = texture(TEXTURE, UV);
    if (albedo.a <= 0.1 || world_point.z < water_height) { discard; }
    vec3 n = has_normal ? normalize(texture(normal_atlas, UV).rgb * 2.0 - 1.0) : vec3(0.0, 0.0, 1.0);
    float band = clamp((world_point.z - foot.z - band_min_height) / max(0.01, band_max_height - band_min_height), 0.0, 1.0);
    vec3 visible = band < 0.5 ? mix(vec3(sun_bands.x,sky_bands.x,fire_bands.x),vec3(sun_bands.y,sky_bands.y,fire_bands.y),band*2.0) : mix(vec3(sun_bands.y,sky_bands.y,fire_bands.y),vec3(sun_bands.z,sky_bands.z,fire_bands.z),(band-0.5)*2.0);
    int material_index = has_material ? int(round(texture(material_atlas,UV).r*255.0)) : material_kind;
    float emit = emission;
    vec3 colour = fixture_light(albedo.rgb * emission_colour, n, world_point, vec4(visible.x,visible.y,1.0,visible.z), emit, material_index);
    if (silhouette) { colour = vec3(0.64,0.69,0.66); }
    COLOR = vec4(colour, albedo.a * fade * (silhouette ? 0.24 : 1.0)) * vertex_colour;
}
