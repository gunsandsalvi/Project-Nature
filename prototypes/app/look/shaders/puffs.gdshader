// Smoke over P1's close camp: round puffs with ragged edges, turned to the camera, lit from the upper left in
// steps of the smoke's ramp, hazed with distance, as the painter's puffs are. Pre-production code (research 00).
shader_type spatial;
render_mode cull_disabled, unshaded, blend_mix, depth_draw_never, shadows_disabled;

#include "res://look/shaders/look.gdshaderinc"

uniform float puff_row;
uniform float puff_shift;
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
	float lit = dot(normalize(vec3(puff.xy, 0.6)), normalize(vec3(-0.55, 0.6, 0.6)));
	float s = clamp(floor(puff.w + (lit > 0.55 ? 1.0 : (lit < 0.1 ? -1.0 : 0.0)) + puff_shift + 0.5), 0.0, 6.0);
	vec3 c = grade(pal(puff_row, s), lit > 0.3 ? 1.0 : 0.0);
	float hz = floor(clamp((-VERTEX.z - look_haze_range.x) / max(look_haze_range.y - look_haze_range.x, 0.001), 0.0, 1.0) * 4.0) / 4.0;
	ALBEDO = to_linear(mix(c, look_haze_col, hz * look_haze_k));
	ALPHA = puff_alpha;
}
