// Smoke rising from a fire as a lit volume (PRE-30), in place of the painter's flat puffs. Its path comes from the
// scene (look.gd): up from the fire, along the underside of an overhang to its edge, then up and away on the wind;
// along it, round blobs that widen as they rise, stirred by noise that climbs with time. Each pixel walks its ray
// through the box round the path, stopping at whatever stands there, so what stands in the smoke shows through it.
// The fire below lights it; its colour is a step of the smoke's ramp, its thickness a few steps of opacity, and it is
// mixed over the picture's stored colours as the painter mixes. Pre-production code (research 00).
shader_type spatial;
render_mode world_vertex_coords, unshaded, cull_front, depth_draw_never, depth_test_disabled, blend_mix, shadows_disabled;

#include "res://look/shaders/look.gdshaderinc"

const int NODES = 16;
const int STEPS = 24;

uniform sampler2D screen : hint_screen_texture, filter_nearest;
uniform sampler2D depth : hint_depth_texture, filter_nearest;
// the path: each node's centre and radius, its thickness falling with height
uniform vec4 nodes[16];
uniform int node_count;
uniform vec3 box_min;
uniform vec3 box_max;
uniform float smoke_row;
// the hour's step of the smoke's ramp, and how thick the smoke is
uniform float tone = 4.0;
uniform float strength = 1.0;
// 1 by day, 0 at night: by day the smoke is torn into drifting puffs with gaps between them, lit by the sun on
// its thin edges, where a smooth sheet read as pale glass
uniform float day = 0.0;

float thickness(vec3 x) {
	float d = 0.0;
	for (int i = 0; i < NODES; i++) {
		if (i >= node_count) {
			break;
		}
		vec3 q = x - nodes[i].xyz;
		float k = dot(q, q) / (nodes[i].w * nodes[i].w);
		if (k < 4.0) {
			d += exp(-1.6 * k) * (1.0 - 0.55 * float(i) / float(node_count));
		}
	}
	float curl = vnoise3(x * 1.4 + vec3(0.0, -TIME * 0.8, TIME * 0.15));
	float n = mix(curl, smoothstep(0.35, 0.65, curl), day);
	return clamp(d * (0.5 + 0.9 * n) - 0.1 - 0.3 * day, 0.0, 1.5);
}

// The fires' glow at a point of the smoke, without facing: the fire term's reach and power.
float glow(vec3 x) {
	float f = 0.0;
	for (int i = 0; i < 4; i++) {
		if (look_fire_powers[i] <= 0.0) {
			continue;
		}
		float att = clamp(1.0 - length(look_fires[i].xyz - x) / (look_fires[i].w * 0.8), 0.0, 1.0);
		f += look_fire_powers[i] * att * att;
	}
	return f;
}

void fragment() {
	vec3 ro = (INV_VIEW_MATRIX * vec4(VERTEX.xy, 0.0, 1.0)).xyz;
	vec3 rd = normalize((INV_VIEW_MATRIX * vec4(0.0, 0.0, -1.0, 0.0)).xyz);
	vec3 inv = 1.0 / rd;
	vec3 t_a = (box_min - ro) * inv;
	vec3 t_b = (box_max - ro) * inv;
	vec3 t_lo = min(t_a, t_b);
	vec3 t_hi = max(t_a, t_b);
	float t0 = max(max(t_lo.x, t_lo.y), max(t_lo.z, 0.0));
	float t1 = min(min(t_hi.x, t_hi.y), t_hi.z);
	float raw = texture(depth, SCREEN_UV).r;
	if (raw > 0.0) {
		vec4 v = INV_PROJECTION_MATRIX * vec4(SCREEN_UV * 2.0 - 1.0, raw, 1.0);
		t1 = min(t1, -v.z / v.w);
	}
	if (t1 <= t0) {
		discard;
	}
	float dt = (t1 - t0) / float(STEPS);
	float acc = 0.0;
	float lit = 0.0;
	for (int k = 0; k < STEPS; k++) {
		vec3 x = ro + rd * (t0 + (float(k) + 0.5) * dt);
		float a = 1.0 - exp(-thickness(x) * dt * 1.5 * strength);
		lit += (1.0 - acc) * a * glow(x);
		acc += (1.0 - acc) * a;
	}
	// a few steps of thickness, so the smoke has clean edges like everything else
	float a = floor(acc * 5.0 + 0.4) / 5.0 * 0.85;
	if (a <= 0.0) {
		discard;
	}
	float f = lit / max(acc, 1e-4);
	float s = clamp(floor(tone + f * 4.0 + 0.5), 0.0, 6.0);
	// firelit from below; by day its thin parts take the sun's tint, so it reads grey rather than blue
	float cls = f > 0.36 ? 4.0 : (f > 0.15 ? 2.0 : (day > 0.5 && acc < 0.7 ? 1.0 : 0.0));
	vec3 c = grade(pal(smoke_row, s), cls);
	vec3 smoke = hazed(c, t0);
	// Godot blends in linear light; the blend is solved for the painter's mix over the picture's stored colours
	vec3 back = texture(screen, SCREEN_UV).rgb;
	vec3 want = to_linear(mix(to_srgb(back), smoke, a));
	vec3 need = 1.0 - want / max(back, vec3(1e-5));
	float alpha = clamp(max(a, max(need.r, max(need.g, need.b))), 1e-3, 1.0);
	ALBEDO = clamp((want - back * (1.0 - alpha)) / alpha, 0.0, 1.0);
	ALPHA = alpha;
}
