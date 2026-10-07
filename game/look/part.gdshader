// The kit's parts (A6.1, A6.2, A6.4): every part of every thing, a part's role wearing its texture, read through the one
// sampling function with texture coordinates in metres, so a texture pixel is 1/64 m at band 0 on every part, and lit by
// the one shared light function; the crease baked into the part in Blender darkens its corners and undersides.
// One material for each texture a role may wear: the texture's array and the width of its tile in metres. Drawn as
// MultiMesh copies, each carrying a number of its own in its custom data's red, for the variety of a made thing.
// Implements PRE-46, PRE-22 and PRE-30.
shader_type spatial;
render_mode cull_back, specular_disabled;

#include "res://look/picture.gdshaderinc"
#include "res://look/sample.gdshaderinc"
#include "res://look/light.gdshaderinc"

uniform sampler2DArray kd_layers : source_color, filter_linear_mipmap, repeat_enable;
// the texture tile's width on the part, in metres: 4 m for a tile of 256 texture pixels at 64 a metre (A5.3)
uniform float kd_tile_metres = 4.0;

varying vec3 kd_world;
varying float kd_big_shadow;

void vertex() {
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

void fragment() {
	vec3 texel = kd_sample(kd_layers, UV / kd_tile_metres, 0.0).rgb;
	ROUGHNESS = 1.0;
	vec3 normal = (INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	vec3 colour;
	float openness;
	vec3 glow;
	vec4 haze;
	float big_shadow;
	kd_light_surface(texel, kd_world, normal, CAMERA_POSITION_WORLD, COLOR.r, vec4(1.0), colour, openness, glow,
			haze, big_shadow);
	kd_big_shadow = big_shadow;
	ALBEDO = colour;
	AO = openness;
	AO_LIGHT_AFFECT = 0.0;
	EMISSION = glow;
	FOG = haze;
	// in a test picture: no material of the world's yet, and no object
	if (kd_picture != 0) {
		ALBEDO = vec3(0.0);
		EMISSION = kd_number_colour(0);
		FOG = vec4(0.0);
	}
}

// How bright the edge is where the low sun grazes a shape, against the sun's own light (tuning/light's rim, A5.2).
global uniform float kd_rim;

void light() {
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, ATTENUATION, kd_big_shadow, 0.0);
	// the lit edge: where the surface turns edge on to the eye, on the side that faces the sun, and only where the
	// sun reaches
	float edge = pow(1.0 - clamp(dot(NORMAL, VIEW), 0.0, 1.0), 3.0);
	float toward = smoothstep(-0.2, 0.6, dot(NORMAL, LIGHT));
	DIFFUSE_LIGHT += LIGHT_COLOR * kd_rim * edge * toward * ATTENUATION * kd_big_shadow / PI;
}
