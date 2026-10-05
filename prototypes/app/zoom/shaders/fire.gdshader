// P8's hearth fire (PRE-30, MAT-18): flames standing over the camp's hearth, worked out pixel by pixel on an upright
// card turned to the camera, as the art book draws its flame pictures (art/book/paint/www/atlas.js): three tongues
// licking up, red at their edges and white-hot at their core in the steps of the fire's ramp, with a spark or two,
// changing about 10 times a second; so they are as crisp as everything else at every pixel size. Unlit: they are the
// light. Pre-production code (research 00).
shader_type spatial;
render_mode unshaded, world_vertex_coords, cull_disabled, shadows_disabled, fog_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://look/shaders/look_core.gdshaderinc"

uniform float fire_row = 59.0;  // the palette's row of fire
uniform float tall = 0.75;      // the middle flame's height at full burn, metres
uniform float power = 1.0;      // how high it burns, 0 to 1

varying vec2 v_at;  // metres across the card from the middle of its foot, and up from it

void vertex() {
	vec3 flat_foot = MODEL_MATRIX[3].xyz;
	vec3 foot = planet_place(vec2(flat_foot.x - zoom_focus_at.x, zoom_focus_at.z - flat_foot.z), flat_foot.y);
	vec3 up = planet_frame(foot)[1];
	vec3 right = INV_VIEW_MATRIX[0].xyz;
	right = normalize(right - up * dot(right, up));
	vec2 size = vec2(1.4, 1.8) * tall;
	v_at = vec2((UV.x - 0.5) * size.x, (1.0 - UV.y) * size.y);
	VERTEX = foot + right * v_at.x + up * v_at.y;
}

// A flame's tongue as the art book draws one, from its foot at x0, `h` metres tall: whether a place is inside it, and
// how far out from its core, 0 to about 1.
vec2 tongue(vec2 p, float x0, float h, float frame, float k) {
	float w0 = h * 0.3;
	float base = max(0.0, 1.0 - p.y / h);
	float dx = p.x - x0;
	float w = w0 * sqrt(base) * (1.0 - max(0.0, (p.y - h * 0.65) / (h * 0.4)));
	bool inside = abs(dx) < w && p.y >= 0.0;
	// licks: thin flames reaching up from the foot, a new set each frame
	for (int i = 0; i < 3; i++) {
		float lx = (hash12(vec2(frame, k * 7.0 + float(i))) - 0.5) * 0.3 * h;
		float lh = mix(0.45, 0.95, hash12(vec2(frame + 13.0, k * 5.0 + float(i)))) * h;
		if (p.y >= 0.0 && p.y < lh && abs(dx - lx * (p.y / lh)) < max(0.03, w0 * 0.3 * (1.0 - p.y / lh))) {
			inside = true;
		}
	}
	return vec2(inside ? 1.0 : 0.0, abs(dx) / max(w, 0.02) + p.y / (h * 1.15));
}

void fragment() {
	float frame = floor(TIME * 10.0);
	float burn = mix(0.45, 1.0, power);
	vec2 best = vec2(0.0, 9.0);
	for (int k = 0; k < 3; k++) {
		float x0 = (float(k) - 1.0) * 0.21 * tall + (hash12(vec2(frame, float(k))) - 0.5) * 0.04;
		float h = tall * burn * (k == 1 ? 1.0 : 0.72) * mix(0.85, 1.1, hash12(vec2(frame + 5.0, float(k))));
		vec2 t = tongue(v_at, x0, h, frame, float(k));
		if (t.x > 0.5 && t.y < best.y) {
			best = t;
		}
	}
	float step_of = best.y < 0.35 ? 6.0 : (best.y < 0.6 ? 5.0 : (best.y < 0.85 ? 4.0 : 3.0));
	if (best.x < 0.5) {
		// a spark or two above the flames
		vec2 spark = vec2((hash12(vec2(frame, 31.0)) - 0.5) * 0.5, mix(1.0, 1.6, hash12(vec2(frame, 37.0)))) * tall * burn;
		float px = max(length(fwidth(v_at)), 0.01);
		if (length(v_at - spark) > px * 0.75 || power < 0.3) {
			discard;
		}
		step_of = 5.0;
	}
	ALBEDO = to_linear(pal(fire_row, step_of));
}
