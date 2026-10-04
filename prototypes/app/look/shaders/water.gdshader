// The clear water of P1's close camp (PRE-26), as the painter's style W1 draws it: the bed shows through the
// shallows, deeper water darkens in steps, a bright line marks the shore, the sky's colour lies on the surface,
// thin lines follow the flow, and what stands above is mirrored when the mirrored pass is on. Forward drawing
// gives the water no record of the land around it, so the shore is where the water is thinnest: within about an
// art pixel of the bank. Pre-production code (research 00).
shader_type spatial;
render_mode world_vertex_coords, cull_disabled, specular_disabled, ambient_light_disabled, depth_draw_never, blend_mix;

#include "res://look/shaders/look.gdshaderinc"

global uniform vec3 look_sky_col;
global uniform vec3 look_water_tint;
global uniform vec3 look_cam_right;
global uniform float look_elev_sin;
global uniform float look_reflect;
global uniform sampler2D look_mirror : filter_nearest;
uniform sampler2D screen : hint_screen_texture, filter_nearest;
uniform sampler2D depth : hint_depth_texture, filter_nearest;
uniform float water_row;
uniform float foam_row;
varying vec4 flow;
varying vec3 under;
varying float thick;
varying vec2 frag;

void vertex() {
	world = VERTEX;
	wnormal = vec3(0.0, 1.0, 0.0);
	flow = CUSTOM0;
}

// Thin lines on the water along the flow, one art pixel thick, every gap rows, dashes about len metres long,
// more of them where the wind ruffles the water (the painter's dash()).
float dash(vec3 p, vec2 fl, float gap, float len, float amount) {
	float across = dot(p.xz, vec2(-fl.y, fl.x));
	float along = dot(p.xz, fl);
	float roww = look_mpp * (1.0 + abs(dot(fl, normalize(look_cam_right.xz))));
	float i = floor(across / roww);
	if (mod(i, gap) > 0.5) {
		return 0.0;
	}
	float sp = along / len + hash12(vec2(i, 7.1)) * 7.0;
	float h = hash12(vec2(i, floor(sp)));
	float wind = vnoise(p.xz * 0.22 + 5.0);
	return (h < amount * (0.35 + wind) && fract(sp) < 0.6) ? 1.0 : 0.0;
}

void fragment() {
	float raw = texture(depth, SCREEN_UV).r;
	vec4 behind = INV_PROJECTION_MATRIX * vec4(SCREEN_UV * 2.0 - 1.0, raw, 1.0);
	thick = (-behind.z / behind.w) - (-VERTEX.z);
	under = texture(screen, SCREEN_UV).rgb;
	frag = FRAGCOORD.xy;
	row = water_row;
	flags = 0.0;
	bias = 0.0;
	edge = 0.0;
	SKY_AND_FIRE(world, vec3(0.0, 1.0, 0.0))
	NORMAL = (VIEW_MATRIX * vec4(0.0, 1.0, 0.0, 0.0)).xyz;
	ALBEDO = vec3(1.0);
	ROUGHNESS = 1.0;
}

void light() {
	// the surface's own light, as a step and its class (sunlit, firelit), from the painter's light()
	float ndl = dot(NORMAL, LIGHT);
	float sh = ndl > 0.0 ? ATTENUATION : 0.0;
	float sun = pow(max(ndl, 0.0), look_sun_pow) * sh;
	float wst = 3.0 + ((look_sky_k * sky + look_sun_k * sun) * 6.0 + look_shift - 3.0) * look_contrast + fire * 4.5;
	float wcls = (sh > 0.5 && ndl > 0.06) ? 1.0 : 0.0;
	bool sunlit = wcls > 0.5;
	float dep = thick * look_elev_sin;
	float shade = clamp(floor((wst - 4.2) * 0.5 + 0.5), -2.0, 1.0);
	vec2 fl = flow.z < 0.5 ? normalize(look_cam_right.xz) : (length(flow.xy) > 0.01 ? normalize(flow.xy) : vec2(1.0, 0.0));
	vec3 sky_r = mix(look_sky_col * 1.03, look_sky_col * vec3(0.74, 0.8, 0.93), clamp(SCREEN_UV.y, 0.0, 1.0));
	vec3 refl = sky_r;
	if (look_reflect > 0.5) {
		vec4 r = texture(look_mirror, SCREEN_UV);
		refl = r.a > 0.5 ? r.rgb : sky_r;
	}
	vec3 foam = grade(pal(foam_row, clamp(floor(wst + 0.5) + 1.0, 1.0, 5.0)), wcls);
	float bs = (dep < 0.12 ? 5.0 : (dep < 0.4 ? 4.0 : (dep < 0.9 ? 3.0 : 2.0))) + shade;
	vec3 body = grade(pal(water_row, bs), wcls);
	float a = dep < 0.12 ? 0.3 : (dep < 0.4 ? 0.55 : (dep < 0.9 ? 0.8 : 1.0));
	vec3 bed = to_srgb(under);
	vec3 c = mix(bed * look_water_tint, body, a);
	c = mix(c, refl, dep < 0.12 ? 0.2 : (dep < 0.4 ? 0.3 : 0.38));
	bool d1 = dash(world, fl, 3.0, 1.0, 0.13) > 0.5;
	if (d1) {
		c = mix(c, sunlit ? sky_r * 1.12 : body * 1.12, 0.45);
	}
	if (d1 && sunlit && hash12(floor(frag)) > 0.97) {
		c = vec3(1.0);
	}
	if (thick * look_elev_sin < look_mpp * 0.45) {
		c = mix(c, foam, 0.8);
	}
	float hz = floor(clamp((vdepth - look_haze_range.x) / max(look_haze_range.y - look_haze_range.x, 0.001), 0.0, 1.0) * 4.0) / 4.0;
	DIFFUSE_LIGHT = to_linear(mix(c, look_haze_col, hz * look_haze_k));
	SPECULAR_LIGHT = vec3(0.0);
}
