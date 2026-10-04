// Smoke over P1's close camp: round puffs with ragged edges, turned to the camera, lit from the upper left in
// steps of the smoke's ramp (PRE-20), hazed with distance, as the painter's puffs are. The painter keeps only the
// nearest puff at each pixel, hides a puff only behind what stands more than half its size in front of its middle,
// and mixes it over the finished picture's stored colours. Here a first pass (PUFF_DEPTH) leaves each pixel's nearest
// puff depth, so the second pass draws that puff alone, its blend solved for the painter's mix. Pre-production code
// (research 00).
shader_type spatial;
#ifdef PUFF_DEPTH
render_mode cull_disabled, unshaded, blend_mix, depth_draw_always, shadows_disabled;
#else
render_mode cull_disabled, unshaded, blend_mix, depth_draw_never, shadows_disabled;
#endif

#include "res://look/shaders/look.gdshaderinc"

uniform float puff_row;
uniform float puff_shift;
#ifndef PUFF_DEPTH
uniform sampler2D screen : hint_screen_texture, filter_nearest;
#endif
varying vec4 puff;
varying float puff_alpha;

void vertex() {
	vec3 right = INV_VIEW_MATRIX[0].xyz;
	vec3 up = INV_VIEW_MATRIX[1].xyz;
	VERTEX += (right * CUSTOM0.x + up * CUSTOM0.y) * CUSTOM0.z;
	puff = CUSTOM0;
	puff_alpha = CUSTOM1.x;
}

void fragment() {
	float r = length(puff.xy) * 2.0;
	if (r > 0.82 + 0.22 * (vnoise(puff.xy * 5.0 + puff.w * 13.0) - 0.5)) {
		discard;
	}
	// the puff's depth, half its size nearer than its middle; the drawing pass a hair nearer still, so that it passes
	// where its own first pass left the nearest depth, and nowhere else
#ifdef PUFF_DEPTH
	vec4 near = PROJECTION_MATRIX * vec4(VERTEX.xy, VERTEX.z + puff.z * 0.5, 1.0);
	DEPTH = near.z / near.w;
	ALBEDO = vec3(0.0);
	ALPHA = 0.0;
#else
	vec4 near = PROJECTION_MATRIX * vec4(VERTEX.xy, VERTEX.z + puff.z * 0.5 + 0.005, 1.0);
	DEPTH = near.z / near.w;
	float lit = dot(normalize(vec3(puff.xy, 0.6)), normalize(vec3(-0.55, 0.6, 0.6)));
	float s = clamp(floor(puff.w + (lit > 0.55 ? 1.0 : (lit < 0.1 ? -1.0 : 0.0)) + puff_shift + 0.5), 0.0, 6.0);
	vec3 c = grade(pal(puff_row, s), lit > 0.3 ? 1.0 : 0.0);
	float hz = floor(clamp((-VERTEX.z - look_haze_range.x) / max(look_haze_range.y - look_haze_range.x, 0.001), 0.0, 1.0) * 4.0) / 4.0;
	vec3 smoke = mix(c, look_haze_col, hz * look_haze_k);
	// Godot blends in linear light, which would brighten smoke over dark ground; so the blend is solved for the
	// painter's mix over the picture the opaque pass left: exact there, close where water or outlines lie under
	vec3 back = texture(screen, SCREEN_UV).rgb;
	vec3 want = to_linear(mix(to_srgb(back), smoke, puff_alpha));
	vec3 need = 1.0 - want / max(back, vec3(1e-5));
	float a = clamp(max(puff_alpha, max(need.r, max(need.g, need.b))), 1e-3, 1.0);
	ALBEDO = clamp((want - back * (1.0 - a)) / a, 0.0, 1.0);
	ALPHA = a;
#endif
}
