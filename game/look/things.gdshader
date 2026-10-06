// Things of one colour lit by the one shared light function (A4.3): the sun, the sky's fill and the fires
// of the light grid, as every family is lit: the stand-ins for what stands round calibration scene C5's
// fires. Implements PLT-04 and PRE-30.
shader_type spatial;
render_mode cull_back, specular_disabled;

#include "res://look/light.gdshaderinc"
#include "res://look/picture.gdshaderinc"

uniform vec3 kd_colour : source_color = vec3(0.45, 0.36, 0.28);

varying vec3 kd_world;
varying float kd_big_shadow;

void vertex() {
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

void fragment() {
	vec3 normal = (INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	vec3 colour;
	float openness;
	vec3 glow;
	vec4 haze;
	float big_shadow;
	kd_light_surface(kd_colour, kd_world, normal, CAMERA_POSITION_WORLD, 1.0, vec4(1.0), colour, openness,
			glow, haze, big_shadow);
	kd_big_shadow = big_shadow;
	ALBEDO = colour;
	ROUGHNESS = 1.0;
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
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, ATTENUATION, kd_big_shadow, 0.0);
}
