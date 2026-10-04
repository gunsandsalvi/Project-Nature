// Cards of P1's close camp: leaf clumps, grass tufts, flowers, reeds and flames, small pictures from the painter's
// atlas standing in 3D and turned to the camera (in the shadow pass, to the sun), lit by the normal they were
// given, each picture's own shading bending it (the atlas's red and green) and moving its step (its blue).
// Pre-production code (research 00).
shader_type spatial;
#if defined(PREPASS) || defined(SKYMAP)
render_mode world_vertex_coords, unshaded, cull_disabled, shadows_disabled;
#elif defined(HULL)
render_mode world_vertex_coords, cull_front, specular_disabled, ambient_light_disabled;
#else
render_mode world_vertex_coords, cull_disabled, specular_disabled, ambient_light_disabled;
#endif

#include "res://look/shaders/look.gdshaderinc"

global uniform sampler2D look_atlas : filter_nearest;
global uniform float look_atlas_grid;
varying flat vec4 attrs;
varying vec2 tile_uv;
varying vec3 card_right;
varying vec3 card_up;

void vertex() {
	vec3 right = INV_VIEW_MATRIX[0].xyz;
	vec3 up = INV_VIEW_MATRIX[1].xyz;
	vec2 corner = CUSTOM0.xy;
	vec2 size = CUSTOM0.zw;
	if (CUSTOM2.x > 0.5) {
		right = normalize(vec3(right.x, 0.0, right.z));
		up = vec3(0.0, 1.0, 0.0);
	}
	float c = cos(CUSTOM2.y);
	float s = sin(CUSTOM2.y);
	vec2 k = vec2(c * corner.x - s * corner.y, s * corner.x + c * corner.y);
	VERTEX += right * k.x * size.x + up * k.y * size.y;
	float tile = round(CUSTOM1.x * 255.0);
	tile_uv = (vec2(mod(tile, look_atlas_grid), floor(tile / look_atlas_grid)) + corner + 0.5) / look_atlas_grid;
	attrs = vec4(round(CUSTOM1.y * 255.0), round(CUSTOM1.z * 255.0) - 128.0, 0.0, round(CUSTOM1.w * 255.0));
	world = VERTEX;
	wnormal = NORMAL;
	card_right = right;
	card_up = up;
#ifdef MIRROR
	VERTEX.y = 2.0 * look_mirror_y - VERTEX.y;
#endif
}

void fragment() {
	vec4 t = texture(look_atlas, tile_uv);
	flags = attrs.w;
	if (t.a < 0.5 || (IN_SHADOW_PASS && bit(flags, 16.0))) {
		discard;
	}
#ifdef MIRROR
	if (world.y < look_mirror_y - 0.02) {
		discard;
	}
#endif
	vec2 bump = (t.rg - 0.5) * 2.0;
	vec3 n = normalize(wnormal + (card_right * bump.x + card_up * bump.y) * 0.9);
	DATA_OUT(n)
#ifndef DATA_PASS
	row = attrs.x;
	bias = attrs.y + floor((t.b * 255.0 - 128.0) / 40.0 + 0.5);
#ifdef MIRROR
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
