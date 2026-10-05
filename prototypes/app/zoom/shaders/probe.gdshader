// P8's probe (A8.3): the picture's own rules for rivers, forests and coasts read round the focus once, so the camp is
// set where the picture shows open land beside a river (zoom.gd's _find_camp). Each texel a place `spacing` metres
// apart round the focus, north up: red how far it lies outside the nearest river's banks, a metre a level from 20 m
// inside; green how much of it is a forest's stand; blue its height, metres, 0 where it is sea or lake.
shader_type canvas_item;

#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/rivers.gdshaderinc"
#include "res://zoom/shaders/forest.gdshaderinc"

uniform float spacing = 6.0;
uniform vec2 texels = vec2(64.0, 64.0);

void fragment() {
	vec2 d = vec2(UV.x - 0.5, 0.5 - UV.y) * texels * spacing;
	vec3 river = river_near(d, 1.0);
	float stand = forest_stand(d, textureLod(zoom_cover, cell_uv(d), 0.0).a, 0.0);
	vec2 shore = shore_uv(d);
	float land = textureLod(zoom_height, shore, 0.0).r;
	float lake = textureLod(zoom_water_soft, shore, 0.0).b * 255.0;
	land = lake > 1.0 ? 0.0 : max(land, 0.0);
	COLOR = vec4(clamp((river.z + 20.0) / 255.0, 0.0, 1.0), stand, clamp(land / 255.0, 0.0, 1.0), 1.0);
}
