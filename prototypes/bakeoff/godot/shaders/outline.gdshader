// Outlines and lit edges (research 03, step 3), drawn by a quad over the whole low-resolution image after the opaque
// pass. Each pixel looks at its 4 neighbours: where a neighbour lies farther away, this pixel is the near side of a
// silhouette and darkens; where the surface turns sharply on an outward edge, it lightens.
// Ported from Leo Peltola's shader (MIT License, Copyright (c) 2023 Leo Peltola,
// github.com/leopeltola/Godot-3d-pixelart-demo), itself after Kody King's three.js pixel pass; changed to sample
// nearest texels and to draw over the whole screen.
shader_type spatial;
render_mode unshaded, depth_draw_never, depth_test_disabled, cull_disabled, blend_mix;

uniform sampler2D screen_tex : hint_screen_texture, filter_nearest;
uniform sampler2D depth_tex : hint_depth_texture, filter_nearest;
uniform sampler2D normal_tex : hint_normal_roughness_texture, filter_nearest;
uniform float outline_strength = 0.5;
uniform float rim_strength = 0.3;

void vertex() {
	POSITION = vec4(VERTEX.xy * 2.0, 1.0, 1.0);
}

float view_depth(vec2 uv, mat4 inv_proj) {
	float raw = texture(depth_tex, uv).r;
	vec4 v = inv_proj * vec4(uv * 2.0 - 1.0, raw, 1.0);
	return -v.z / v.w;
}

float normal_indicator(vec3 base_n, vec3 new_n, float depth_diff) {
	float normal_diff = dot(base_n - new_n, vec3(1.0));
	float side = clamp(smoothstep(-0.01, 0.01, normal_diff), 0.0, 1.0);
	float near = clamp(sign(depth_diff * 0.25 + 0.0025), 0.0, 1.0);
	return (1.0 - dot(base_n, new_n)) * near * side;
}

void fragment() {
	vec2 e = 1.0 / VIEWPORT_SIZE;
	vec2 offs[4] = vec2[4](vec2(0.0, -1.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(-1.0, 0.0));
	float d = view_depth(SCREEN_UV, INV_PROJECTION_MATRIX);
	vec3 n = texture(normal_tex, SCREEN_UV).xyz * 2.0 - 1.0;
	float farther = 0.0;
	float nearer = 0.5;
	float rim = 0.0;
	for (int i = 0; i < 4; i++) {
		vec2 uv = SCREEN_UV + offs[i] * e;
		float dn = view_depth(uv, INV_PROJECTION_MATRIX);
		farther += clamp(dn - d, 0.0, 1.0);
		nearer += d - dn;
		rim += normal_indicator(n, texture(normal_tex, uv).xyz * 2.0 - 1.0, dn - d);
	}
	float outline = smoothstep(0.2, 0.3, farther);
	nearer = clamp(step(0.5, clamp(nearer, 0.0, 1.0)) * 10.0, 0.0, 1.0);
	rim = clamp(smoothstep(0.2, 0.8, rim) - nearer, 0.0, 1.0);
	vec3 c = texture(screen_tex, SCREEN_UV).rgb;
	vec3 lit = mix(c, vec3(1.0), rim_strength);
	vec3 dark = c * (1.0 - outline_strength);
	ALBEDO = mix(mix(c, lit, step(0.5, rim)), dark, step(0.5, outline));
	ALPHA = max(step(0.5, rim), step(0.5, outline));
}
