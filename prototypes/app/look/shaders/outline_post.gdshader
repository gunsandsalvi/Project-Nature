// Outline methods (a) and (b) of P1 (PRE-21): a quad over the whole low-resolution picture, drawn after the opaque
// pass, compares each pixel's depth with its four neighbours'. A neighbour well behind the plane this pixel lies in
// makes an outline, darkened about two steps; with (a), the pixel's normal rebuilt from the depths also lights the
// sun's side of a shape by a step. (b) uses depth alone, with no plane, so no lit edges. Reading the depth and the
// picture costs the Mobile renderer a copy of both, which P1 measures. Pre-production code (research 00).
shader_type spatial;
render_mode unshaded, depth_draw_never, depth_test_disabled, cull_disabled, blend_mix, shadows_disabled, fog_disabled;

global uniform int look_outline;
global uniform float look_mpp;
global uniform vec4 look_edges;
global uniform vec3 look_sun_dir;
uniform sampler2D screen : hint_screen_texture, filter_nearest;
uniform sampler2D depth : hint_depth_texture, filter_nearest;

void vertex() {
	POSITION = vec4(VERTEX.xy * 2.0, 0.0, 1.0);
}

vec3 view_at(vec2 uv, mat4 inv_proj) {
	float raw = texture(depth, uv).r;
	vec4 v = inv_proj * vec4(uv * 2.0 - 1.0, raw, 1.0);
	return v.xyz / v.w;
}

bool empty(vec2 uv) {
	return texture(depth, uv).r <= 0.0;
}

void fragment() {
	if (look_outline != 1 && look_outline != 2 || empty(SCREEN_UV)) {
		discard;
	}
	vec2 px = 1.0 / VIEWPORT_SIZE;
	vec2 offs[4] = vec2[4](vec2(1.0, 0.0), vec2(-1.0, 0.0), vec2(0.0, 1.0), vec2(0.0, -1.0));
	vec3 p = view_at(SCREEN_UV, INV_PROJECTION_MATRIX);
	float d = -p.z;
	// (a): the normal from the nearer of each pair of neighbours, so it does not bend over an edge (atyuwen)
	vec3 l = view_at(SCREEN_UV - vec2(px.x, 0.0), INV_PROJECTION_MATRIX);
	vec3 r = view_at(SCREEN_UV + vec2(px.x, 0.0), INV_PROJECTION_MATRIX);
	vec3 u = view_at(SCREEN_UV - vec2(0.0, px.y), INV_PROJECTION_MATRIX);
	vec3 b = view_at(SCREEN_UV + vec2(0.0, px.y), INV_PROJECTION_MATRIX);
	vec3 dx = abs(r.z - p.z) < abs(p.z - l.z) ? r - p : p - l;
	vec3 dy = abs(u.z - p.z) < abs(p.z - b.z) ? u - p : p - b;
	vec3 nv = normalize(cross(dy, dx));
	if (nv.z < 0.0) {
		nv = -nv;
	}
	vec2 sv = normalize((VIEW_MATRIX * vec4(look_sun_dir, 0.0)).xy + vec2(1e-4));
	bool sunward = dot((INV_VIEW_MATRIX * vec4(nv, 0.0)).xyz, look_sun_dir) > 0.25;
	float e = 0.0;
	for (int k = 0; k < 4; k++) {
		vec2 o = offs[k];
		vec2 q = SCREEN_UV + vec2(o.x, -o.y) * px;
		float qd = empty(q) ? 1e9 : -view_at(q, INV_PROJECTION_MATRIX).z;
		bool behind;
		if (look_outline == 1) {
			float pred = d + (nv.x * o.x + nv.y * o.y) * look_mpp / max(nv.z, 0.25);
			behind = qd > pred + 0.32 * look_edges.w;
		} else {
			behind = qd > d + 0.6 * look_edges.w;
		}
		if (behind) {
			bool rim = look_outline == 1 && dot(o, sv) > 0.35 && sunward;
			if (rim && e == 0.0) {
				e = 2.0;
			} else if (!rim) {
				e = 1.0;
			}
		}
	}
	if (e == 0.0) {
		discard;
	}
	vec3 c = texture(screen, SCREEN_UV).rgb;
	ALBEDO = e > 1.5 ? min(c * 1.7, vec3(1.0)) : c * 0.32;
}
