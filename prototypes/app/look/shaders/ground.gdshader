// The ground of P1's close camp: each art pixel takes the strongest of four covers (meadow, worn dirt, mud,
// woods' floor) by the painter's weights, its edge moved a little by noise so it never runs straight, and that
// cover's pattern (look.gdshaderinc). Pre-production code (research 00).
shader_type spatial;
#if defined(PREPASS) || defined(SKYMAP)
render_mode world_vertex_coords, unshaded, cull_disabled, shadows_disabled;
#elif defined(HULL)
render_mode world_vertex_coords, cull_front, specular_disabled, ambient_light_disabled;
#else
render_mode world_vertex_coords, cull_disabled, specular_disabled, ambient_light_disabled;
#endif

#include "res://look/shaders/look.gdshaderinc"

uniform vec4 layers;
uniform vec4 layer_pat;
varying vec4 cover;

void vertex() {
	world = VERTEX;
	wnormal = NORMAL;
	cover = CUSTOM0;
#ifdef MIRROR
	VERTEX.y = 2.0 * look_mirror_y - VERTEX.y;
#endif
#ifdef HULL
	VERTEX += normalize(NORMAL) * look_mpp * 1.2;
#endif
}

void fragment() {
#ifdef MIRROR
	if (world.y < look_mirror_y - 0.02) {
		discard;
	}
	vec3 n = normalize(wnormal) * (FRONT_FACING ? -1.0 : 1.0);
#elif defined(HULL)
	vec3 n = normalize(wnormal);
#else
	vec3 n = normalize(wnormal) * (FRONT_FACING ? 1.0 : -1.0);
#endif
	DATA_OUT(n)
#ifndef DATA_PASS
	float j = 0.5 * (vnoise(world.xz * 0.9) - 0.5);
	vec4 w = cover + vec4(j, -j, j * 0.7, -j * 0.7);
	int k = 0;
	float best = w.x;
	if (w.y > best) {
		best = w.y;
		k = 1;
	}
	if (w.z > best) {
		best = w.z;
		k = 2;
	}
	if (w.w > best) {
		k = 3;
	}
	row = layers[k];
	flags = 0.0;
	bias = pattern(int(layer_pat[k] + 0.5), world, world, n);
#if defined(HULL)
	edge = 1.0;
#elif defined(MIRROR)
	edge = 0.0;
#else
	edge = outline_edge(SCREEN_UV, VERTEX, n, VIEW_MATRIX, flags);
#endif
	SKY_AND_FIRE(world, n)
	NORMAL = (VIEW_MATRIX * vec4(n, 0.0)).xyz;
	ALBEDO = vec3(1.0);
	ROUGHNESS = 1.0;
#endif
}

void light() {
	LOOK_LIGHT
}
