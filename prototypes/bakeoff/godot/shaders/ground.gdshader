// The ground (research 04, rule 6): the patch value in UV.x is cut into three greens, with a fine pattern fixed to the
// ground (16 texture pixels a metre) only where two meet. COLOR holds the weights of path, bank, bed and rock (grass
// is what is left); the strongest wins, its edge broken by the same pattern. Steep rock shows the cliff's strata.
shader_type spatial;
render_mode cull_back, specular_disabled;
#include "res://shaders/light.gdshaderinc"

uniform vec3 g0 : source_color;
uniform vec3 g1 : source_color;
uniform vec3 g2 : source_color;
uniform vec3 sand : source_color;
uniform vec3 bed : source_color;
uniform sampler2D dirt : source_color, filter_nearest_mipmap, repeat_enable;
uniform sampler2D strata : source_color, filter_nearest_mipmap, repeat_enable;
varying vec3 world;
varying vec3 wnormal;

void vertex() {
	world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	wnormal = normalize((MODEL_MATRIX * vec4(NORMAL, 0.0)).xyz);
}

void fragment() {
	ivec2 px = ivec2(floor(world.xz * 16.0));
	float d = bayer4(px) - 0.5;
	vec4 w = COLOR + vec4(d * 0.35);
	float grass = 1.0 - (COLOR.r + COLOR.g + COLOR.b + COLOR.a) + d * 0.35;
	float best = max(max(w.r, w.g), max(w.b, w.a));
	vec3 c;
	if (grass >= best) {
		float q = UV.x + d * 0.08;
		c = q < 0.36 ? g0 : (q < 0.64 ? g1 : g2);
	} else if (best == w.r) {
		c = texture(dirt, world.xz / 2.0).rgb;
	} else if (best == w.g) {
		c = mix(sand, texture(dirt, world.xz / 2.0).rgb, 0.3);
	} else if (best == w.b) {
		c = bed;
	} else {
		vec2 t = normalize(vec2(-wnormal.z, wnormal.x) + vec2(1e-4));
		c = texture(strata, vec2(dot(world.xz, t), -world.y) / 4.0).rgb;
	}
	ALBEDO = c;
	ROUGHNESS = 1.0;
}
