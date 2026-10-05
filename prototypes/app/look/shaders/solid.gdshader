// Everything solid in P1's close camp but the ground: the cliff, trees, people, tents and things. Each vertex carries
// its material row, step bias, pattern and flags (custom0), its place in its own shape for the pattern (custom1),
// and its object (custom3). A MultiMesh's copies of one shape may each carry their own material, pattern and wear
// (A6.2, P3's huts). Soot darkens the rock over the shelter's hearth. Compiled with ZOOM (P8's camp, zoom.gd), each
// point is set on P8's planet where it stands, as the ground there is. Pre-production code (research 00).
shader_type spatial;
#if defined(PREPASS) || defined(SKYMAP) || defined(OCCMAP)
render_mode world_vertex_coords, unshaded, cull_disabled, shadows_disabled;
#elif defined(HULL)
render_mode world_vertex_coords, cull_front, specular_disabled, ambient_light_disabled;
#else
render_mode world_vertex_coords, cull_disabled, specular_disabled, ambient_light_disabled;
#endif

#include "res://look/shaders/look.gdshaderinc"
#ifdef ZOOM
#include "res://zoom/shaders/planet.gdshaderinc"
#endif

global uniform vec4 look_soot;
varying flat vec4 attrs;
// a copy's wear, the share of its patches a step darker, and its seed, which places them
varying flat vec2 copy;
varying vec3 loc;

void vertex() {
	world = VERTEX;
	wnormal = NORMAL;
	attrs = vec4(round(CUSTOM0.x * 255.0), round(CUSTOM0.y * 255.0) - 128.0, round(CUSTOM0.z * 255.0), round(CUSTOM0.w * 255.0));
	copy = vec2(0.0);
	if (bit(attrs.w, FLAG_COPY) && INSTANCE_CUSTOM.a > 0.0) {
		// the copy's material row, pattern, wear and seed (its seed above 0 marks the data as there)
		attrs.x = round(INSTANCE_CUSTOM.r * 255.0);
		attrs.z = round(INSTANCE_CUSTOM.g * 255.0);
		copy = INSTANCE_CUSTOM.ba;
	}
	loc = CUSTOM1.xyz;
#ifdef MIRROR
	VERTEX.y = 2.0 * look_mirror_y - VERTEX.y;
#endif
#ifdef HULL
	VERTEX += normalize(NORMAL) * look_mpp * 1.2;
#endif
#ifdef ZOOM
	VERTEX = planet_place(vec2(VERTEX.x - zoom_focus_at.x, zoom_focus_at.z - VERTEX.z), VERTEX.y);
#endif
}

void fragment() {
	flags = attrs.w;
	if (IN_SHADOW_PASS && bit(flags, FLAG_NOSHADOW)) {
		discard;
	}
#ifdef MIRROR
	if (world.y < look_mirror_y - 0.02) {
		discard;
	}
	vec3 n = normalize(wnormal) * (FRONT_FACING ? -1.0 : 1.0);
#elif defined(HULL)
	if (bit(flags, FLAG_NOOUTLINE) || bit(flags, FLAG_EMISSIVE)) {
		discard;
	}
	vec3 n = normalize(wnormal);
#else
	vec3 n = normalize(wnormal) * (FRONT_FACING ? 1.0 : -1.0);
#endif
	DATA_OUT(n)
#ifndef DATA_PASS
	row = attrs.x;
	int pat = int(attrs.z + 0.5);
	bias = attrs.y + pattern(pat, world, loc, n);
	// wear and weather: patches of the copy a step darker, in places of its own (PRE-42, PRE-43)
	if (copy.x > 0.0 && hash13(floor(loc * 2.5) + copy.y * 61.0) < copy.x) {
		bias -= 1.0;
	}
	if (pat == 2 && look_soot.w != 0.0) {
		vec3 d = (world - look_soot.xyz) / vec3(1.3, 1.0, 1.0);
		float k = 1.0 - length(d) / abs(look_soot.w);
		if (k > 0.0) {
			bias -= floor((look_soot.w > 0.0 ? 2.6 : 1.4) * k * (0.7 + 0.6 * vnoise3(world * 2.0)) + 0.3);
		}
	}
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
