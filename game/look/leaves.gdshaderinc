// The leaves family's shader body (A4.6): cards of a plant's leaves and blades, their design read through
// the one sampling function and lit by the one shared light function, with light through them from
// behind. The way calibration scene C2 draws them comes from the file that includes this: KD_CUT, cut out
// where the design's alpha falls under a half; KD_SOLID, a solid core cut nowhere; KD_COVERAGE, cut with
// its edge smoothed by alpha to coverage. Written once for every way (CLAUDE.md rule 4). Implements
// PRE-46 and PRE-30.

#include "res://look/picture.gdshaderinc"
#include "res://look/sample.gdshaderinc"
#include "res://look/light.gdshaderinc"

// The leaves' designs, a layer a form, at 64 texture pixels a metre (A5.3).
uniform sampler2DArray kd_leaves : source_color, filter_linear_mipmap, repeat_disable;

// How much light comes through a leaf from behind (A4.3's backlight).
const float KD_LEAF_BACKLIT = 0.6;

varying vec3 kd_world;
varying flat float kd_layer;
varying float kd_big_shadow;

void vertex() {
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	// a card's design is its form's layer of the array
	kd_layer = UV2.x;
}

void fragment() {
	vec4 texel = kd_sample(kd_leaves, UV, kd_layer);
	ALBEDO = texel.rgb;
	ROUGHNESS = 1.0;
#if defined(KD_CUT) || defined(KD_COVERAGE)
	ALPHA = texel.a;
	ALPHA_SCISSOR_THRESHOLD = 0.5;
#endif
#ifdef KD_COVERAGE
	ALPHA_ANTIALIASING_EDGE = 0.3;
	ALPHA_TEXTURE_COORDINATE = UV * vec2(textureSize(kd_leaves, 0).xy);
#endif
	vec3 normal = (INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	vec3 colour;
	float openness;
	vec3 glow;
	vec4 haze;
	float big_shadow;
	kd_light_surface(ALBEDO, kd_world, normal, CAMERA_POSITION_WORLD, 1.0, colour, openness, glow, haze,
			big_shadow);
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

void light() {
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, ATTENUATION, kd_big_shadow, KD_LEAF_BACKLIT);
}
