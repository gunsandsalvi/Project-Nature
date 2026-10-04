// P1's look (IMPLEMENTATION α0.2a): the art book's painter (art/book/paint/www/glsl.js) ported to Godot's Mobile
// renderer, which draws forward with no buffers to read back. Each material works out its art pixel's material,
// step bias, sky light and fire light in fragment(); light() adds the sun after its shadow, rounds the light to a
// step of the material's ramp, gives it the hour's colour and the haze, and writes it as the whole of the light.
// The environment's ambient light is black and every albedo white, so a pixel is exactly its colour.
// Pre-production code (research 00): thrown away with the prototypes.

// The variants a material is compiled in, by a #define put after its shader_type (look.gd): PREPASS, the second
// camera's normals and depths for outline method (c); SKYMAP, the heights seen from straight above; MIRROR, the
// scene reflected in the water, lit where it really stands; HULL, outline method (d), the shape drawn again a
// little larger from behind in its own darker shade; OCCMAP, the heights of what stands round the fires, for the
// fires' shadows, leaving out what stands higher than a fire's flames reach, foliage, and what glows or casts no
// shadow, such as a hearth's own embers, which would shade its fire's nearest neighbours: seen from above, its tops,
// and from below, its undersides, the faces turned down, so light passes under a roof but not through a tent.
// DATA_OUT writes a data pass's picture; the rest of a fragment() is left out of a data pass (DATA_PASS).
#if defined(PREPASS)
#define DATA_PASS
#define DATA_OUT(n) ALBEDO = vec3(oct_encode(n), clamp((-VERTEX.z - look_gbuf_depth.x) / look_gbuf_depth.y, 0.0, 1.0));
#elif defined(SKYMAP)
#define DATA_PASS
#define DATA_OUT(n) ALBEDO = vec3(clamp((world.y - look_sky_height.x) / look_sky_height.y, 0.0, 1.0));
#elif defined(OCCMAP)
#define DATA_PASS
#define DATA_OUT(n) if (world.y > look_occ_cap || bit(flags, FLAG_FOLIAGE) || bit(flags, FLAG_EMISSIVE) || bit(flags, FLAG_NOSHADOW) || (INV_VIEW_MATRIX[2].y < 0.0 && wnormal.y > -0.05)) { discard; } ALBEDO = vec3(clamp((world.y - look_sky_height.x) / look_sky_height.y, 0.0, 1.0));
#else
#define DATA_OUT(n)
#endif
#ifdef MIRROR
#define SHADOW_OF(a) 1.0
#define DEPTH_OF(v) -1e9
#else
#define SHADOW_OF(a) a
#define DEPTH_OF(v) v
#endif
// The light of a pixel, from light(): the whole of it, so the pixel is exactly its colour.
#ifdef DATA_PASS
#define LOOK_LIGHT
#else
// the sun gives the stepped colour; Godot's own lights (P2's comparison with our firelight term) add a plain warm glow
#define LOOK_LIGHT if (LIGHT_IS_DIRECTIONAL) { DIFFUSE_LIGHT = to_linear(shade_pixel(dot(NORMAL, LIGHT), SHADOW_OF(ATTENUATION), DEPTH_OF(vdepth), row, bias, flags, sky, fire, edge)); } else { DIFFUSE_LIGHT += to_linear(look_fire_tint) * ATTENUATION * max(dot(NORMAL, LIGHT), 0.0) * 0.8; } SPECULAR_LIGHT = vec3(0.0);
// The sky's and fire's light of a pixel, from fragment().
#define SKY_AND_FIRE(p, n) vec2 sf_ = sky_and_fire(p, n, flags); sky = sf_.x; fire = sf_.y; vdepth = -VERTEX.z;
#endif

global uniform sampler2D look_palette : filter_nearest;
global uniform vec3 look_sun_dir;
global uniform float look_sun_k;
global uniform float look_sky_k;
global uniform float look_sun_pow;
global uniform float look_shift;
global uniform float look_contrast;
global uniform vec3 look_sun_tint;
global uniform vec3 look_shade_tint;
global uniform vec3 look_fire_tint;
global uniform vec3 look_haze_col;
global uniform float look_haze_k;
global uniform float look_desat;
global uniform vec2 look_haze_range;
// up to four fires: each column the fire's place and reach, and their powers (0 for none)
global uniform mat4 look_fires;
global uniform vec4 look_fire_powers;
// how much of a fire's light shows on sunlit ground: none at noon, all at night (look.gd's _set_hour)
global uniform float look_fire_in_sun;
global uniform float look_sky_cover;
#ifndef SKYMAP
global uniform sampler2D look_sky_map : filter_nearest;
#endif
global uniform vec4 look_sky_box;
// the heights round the fires (OCCMAP), the tops from above and the undersides from below, the second mirrored
// left to right: its square's corner and side, on when w is 1; and the height above which nothing is drawn into it
#ifndef OCCMAP
global uniform sampler2D look_occ_map : filter_nearest;
global uniform sampler2D look_occ_floor : filter_nearest;
#endif
global uniform vec4 look_occ_box;
global uniform float look_occ_cap;
global uniform vec2 look_sky_height;
global uniform vec4 look_edges;
global uniform float look_mpp;
global uniform int look_outline;
#ifndef PREPASS
global uniform sampler2D look_gbuf : filter_nearest;
#endif
global uniform float look_mirror_y;

global uniform vec2 look_gbuf_depth;

// From vertex() to fragment(): the point and its normal in the world (every mesh is in world coordinates).
varying vec3 world;
varying vec3 wnormal;
// From fragment() to light(): the pixel's material row, step bias and flags, its sky and fire light, and whether
// it is an outline (1) or a lit edge (2).
varying float row;
varying float bias;
varying float flags;
varying float sky;
varying float fire;
varying float edge;
varying float vdepth;

const float FLAG_EMISSIVE = 1.0;
const float FLAG_FOLIAGE = 2.0;
const float FLAG_NOOUTLINE = 4.0;
const float FLAG_NOSHADOW = 16.0;
const float FLAG_CREATURE = 32.0;
// parts that take their copy's own material, pattern and wear from a MultiMesh's per-copy data (A6.2)
const float FLAG_COPY = 64.0;

bool bit(float f, float b) {
	return mod(floor(f / b), 2.0) > 0.5;
}

float hash12(vec2 p) {
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

float hash13(vec3 p3) {
	p3 = fract(p3 * 0.1031);
	p3 += dot(p3, p3.zyx + 31.32);
	return fract((p3.x + p3.y) * p3.z);
}

float vnoise(vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(mix(hash12(i), hash12(i + vec2(1.0, 0.0)), u.x), mix(hash12(i + vec2(0.0, 1.0)), hash12(i + vec2(1.0, 1.0)), u.x), u.y);
}

float vnoise3(vec3 p) {
	vec3 i = floor(p);
	vec3 f = fract(p);
	vec3 u = f * f * (3.0 - 2.0 * f);
	float a = mix(mix(hash13(i), hash13(i + vec3(1, 0, 0)), u.x), mix(hash13(i + vec3(0, 1, 0)), hash13(i + vec3(1, 1, 0)), u.x), u.y);
	float b = mix(mix(hash13(i + vec3(0, 0, 1)), hash13(i + vec3(1, 0, 1)), u.x), mix(hash13(i + vec3(0, 1, 1)), hash13(i + vec3(1, 1, 1)), u.x), u.y);
	return mix(a, b, u.z);
}

float fbm2(vec2 p) {
	float a = 0.5;
	float s = 0.0;
	for (int i = 0; i < 4; i++) {
		s += a * vnoise(p);
		p = p * 2.03 + 7.1;
		a *= 0.5;
	}
	return s / 0.9375;
}

vec3 to_linear(vec3 c) {
	return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}

vec3 to_srgb(vec3 c) {
	return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), c));
}

// A unit vector in two numbers from 0 to 1 (octahedral), for the second camera's picture, and back.
vec2 oct_encode(vec3 n) {
	n /= abs(n.x) + abs(n.y) + abs(n.z);
	vec2 e = n.z >= 0.0 ? n.xy : (1.0 - abs(n.yx)) * vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
	return e * 0.5 + 0.5;
}

vec3 oct_decode(vec2 e) {
	e = e * 2.0 - 1.0;
	vec3 n = vec3(e, 1.0 - abs(e.x) - abs(e.y));
	float t = clamp(-n.z, 0.0, 1.0);
	n.xy += vec2(n.x >= 0.0 ? -t : t, n.y >= 0.0 ? -t : t);
	return normalize(n);
}

// The surfaces' patterns in the art book's sharp look (no grain, no brush): the lines and broad shapes of what a
// thing is made of, as steps up or down (PRE-20).
float pattern(int k, vec3 p, vec3 l, vec3 n) {
	if (k == 1) {
		return fbm2(p.xz * 0.11 + 3.0) < 0.31 ? -1.0 : 0.0;
	}
	if (k == 2) {
		float b = 0.0;
		float side = 1.0 - smoothstep(0.55, 0.8, abs(n.y));
		float y = p.y + (vnoise(p.xz * 0.8) - 0.5) * 0.22;
		float sp = 0.78;
		float bed = floor(y / sp);
		float f = fract(y / sp);
		if (f < 0.1 * 0.42 / sp && side > 0.5 && vnoise(vec2(dot(p.xz, vec2(0.7, 0.7)) * 1.3, bed)) > 0.45) {
			b -= 1.0;
		}
		float cr = vnoise(vec2(dot(p.xz, vec2(0.71, -0.71)) * 3.1, bed * 7.3));
		if (cr > 0.93 && side > 0.5) {
			b -= 1.0;
		}
		return b;
	}
	if (k == 3) {
		float a = l.x * 5.0 + vnoise(vec2(l.y * 2.0, l.x * 3.0)) * 1.2;
		return fract(a) < 0.24 ? -1.0 : 0.0;
	}
	if (k == 5) {
		float b = 0.0;
		if (fract(l.x / 0.85 + 0.5) < 0.06) {
			b -= 1.0;
		}
		if (fract(l.y / 0.62) < 0.05) {
			b -= 1.0;
		}
		return b;
	}
	if (k == 6) {
		float r = l.y / 0.3 + (vnoise(vec2(l.x * 0.9, 3.0)) - 0.5) * 0.35;
		return fract(r) < 0.13 ? -1.0 : 0.0;
	}
	if (k == 7) {
		if (l.z < 0.45) {
			return 0.0;
		}
		if (l.y > 0.04 && l.y < 0.19 && abs(l.x) > 0.1 && abs(l.x) < 0.27) {
			return -3.0;
		}
		if (l.y > -0.29 && l.y < -0.19 && abs(l.x) < 0.13) {
			return -2.0;
		}
		return 0.0;
	}
	if (k == 9) {
		return 1.0 + (fbm2(p.xz * 0.16 + 9.0) > 0.7 ? 1.0 : 0.0);
	}
	if (k == 10) {
		float r = (p.x * 0.8 + p.z * 0.35) / 0.5 + vnoise(p.xz * 0.6) * 1.5;
		return fract(r) < 0.16 ? -1.0 : 0.0;
	}
	if (k == 11) {
		return fbm2(p.xz * 0.5) < 0.32 ? -1.0 : 0.0;
	}
	if (k == 13) {
		return fract(l.y / 0.1) < 0.2 ? -1.0 : 0.0;
	}
	if (k == 15) {
		return fract(l.x / 0.22) < 0.14 ? -1.0 : 0.0;
	}
	if (k == 16) {
		float r = l.y / 0.12;
		float w = sin(l.x * 6.283 / 0.5 + floor(r) * 3.14159);
		float b = fract(r) < 0.25 ? -1.0 : 0.0;
		return b + (w > 0.6 ? 1.0 : (w < -0.6 ? -1.0 : 0.0));
	}
	return 0.0;
}

#ifndef DATA_PASS
// How much open sky a point sees, from the sky map: the height of whatever lies highest over each spot, drawn
// once from straight above into a half-float picture (look_sky_box: its corner x, z and size; look_sky_height:
// the base and range of the heights it holds). Anything well above the point (an overhang, a roof, a crown)
// takes some sky away, as the painter's skyOpen() does.
float sky_open(vec3 p, vec3 n) {
	float texel = look_sky_box.z / float(textureSize(look_sky_map, 0).x);
	float open = 0.0;
	for (int i = 0; i < 5; i++) {
		vec2 o = i == 0 ? vec2(0.0) : (i == 1 ? vec2(0.5, 0.0) : (i == 2 ? vec2(-0.5, 0.0) : (i == 3 ? vec2(0.0, 0.5) : vec2(0.0, -0.5))));
		vec3 q = p + n * max(0.2, texel * 2.0) + vec3(o.x, 0.0, o.y) * max(1.0, texel);
		vec2 uv = (q.xz - look_sky_box.xy) / look_sky_box.z;
		if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
			open += 1.0;  // beyond the map nothing is known to stand over it
			continue;
		}
		vec4 s = texture(look_sky_map, uv);
		float top = s.a > 0.5 ? look_sky_height.x + s.r * look_sky_height.y : -1e6;
		open += top > q.y + max(0.35, texel * 3.0) ? 0.0 : 1.0;
	}
	return open / 5.0;
}

// The top of what stands round the fires at a place of their heights' map, or none.
float occ_top(vec2 uv) {
	vec4 s = textureLod(look_occ_map, uv, 0.0);
	return s.a > 0.5 ? look_sky_height.x + s.r * look_sky_height.y : -1e9;
}

// Whether a fire reaches a point, or what stands round it shades the point (PRE-30): the line from the point to
// the fire's flames, walked across the heights round the fires (OCCMAP) every 0.2 m, from past the point's own
// shape. The line is in shadow where it runs through something, between its underside and its top, or where it
// passes from above a thing to below its underside between two steps, as through a roof from a point on top of
// it; under a roof it passes. 1 lit, 0 in shadow.
float fire_shadow(vec3 p, vec3 f) {
	if (look_occ_box.w < 0.5) {
		return 1.0;
	}
	vec2 d = f.xz - p.xz;
	float len = length(d);
	float start = 0.35;
	if (len <= start + 0.1) {
		return 1.0;
	}
	vec2 dir = d / len;
	float steps = clamp(floor((len - start) / 0.2), 1.0, 32.0);
	// the top the line was last above: the point's own, when it stands on top of something
	vec2 uv0 = (p.xz - look_occ_box.xy) / look_occ_box.z;
	float own = occ_top(uv0);
	float last = p.y > own - 0.15 ? own : -1e9;
	for (int k = 0; k < 32; k++) {
		if (float(k) >= steps) {
			break;
		}
		float t = start + (len - start) * (float(k) + 0.5) / steps;
		vec2 uv = (p.xz + dir * t - look_occ_box.xy) / look_occ_box.z;
		if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
			last = -1e9;
			continue;
		}
		float top = occ_top(uv);
		float h = mix(p.y + 0.15, f.y + 1.5, t / len);
		if (h < top) {
			vec4 b = textureLod(look_occ_floor, vec2(1.0 - uv.x, uv.y), 0.0);
			float bottom = b.a > 0.5 ? look_sky_height.x + b.r * look_sky_height.y : -1e9;
			if (h > bottom || last > bottom - 0.3) {
				return 0.0;
			}
		}
		last = h >= top ? top : -1e9;
	}
	return 1.0;
}

// The sky's and the fire's share of a point's light, before the sun (the painter's light(), less its sun).
vec2 sky_and_fire(vec3 p, vec3 n, float fl) {
	bool foliage = bit(fl, FLAG_FOLIAGE);
	float s = (0.7 + 0.3 * n.y) * mix(1.0 - look_sky_cover, 1.0, foliage ? 1.0 : sky_open(p, n));
	float f = 0.0;
	for (int i = 0; i < 4; i++) {
		if (look_fire_powers[i] <= 0.0) {
			continue;
		}
		vec3 l = look_fires[i].xyz - p;
		float d = length(l);
		float reach = look_fires[i].w * (0.78 + 0.44 * vnoise(p.xz * 1.3 + float(i) * 7.0));
		float att = clamp(1.0 - d / reach, 0.0, 1.0);
		if (att <= 0.0) {
			continue;
		}
		// small creatures take a fire's light round their sides too, so a figure by a fire reads as lit from
		// wherever the camera stands
		float wrap = bit(fl, FLAG_CREATURE) ? 0.65 : 0.35;
		float lit = wrap + (1.0 - wrap) * max(dot(n, l / max(d, 0.001)), 0.0);
		f += look_fire_powers[i] * pow(att, 1.4) * lit * fire_shadow(p, look_fires[i].xyz);
	}
	return vec2(s, f);
}

vec3 tint(vec3 c, vec3 t) {
	float l = dot(t, vec3(0.299, 0.587, 0.114));
	return c * t / max(l, 0.05);
}

// The hour's colour: sunlit colour leans to the sun's tint, shade to the sky's, firelit to the fire's (PRE-30).
vec3 grade(vec3 c, float cls) {
	if (cls > 6.5) {
		return c;
	}
	float sunlit = mod(cls, 2.0);
	float fb = floor(cls / 2.0 + 0.01);
	vec3 g = tint(c, sunlit > 0.5 ? look_sun_tint : look_shade_tint);
	float lc = dot(c, vec3(0.299, 0.587, 0.114));
	if (fb > 0.5) {
		// firelit colour leans to a warm ramp of the pixel's own brightness, so lit grass reads amber, not olive
		g = mix(g, mix(tint(c, look_fire_tint), look_fire_tint * lc * 1.55, 0.6), fb > 1.5 ? 0.85 : 0.6);
	}
	float l = dot(g, vec3(0.299, 0.587, 0.114));
	g = mix(g, vec3(l), look_desat * (fb > 0.5 ? 0.0 : 1.0));
	return clamp(g, 0.0, 1.0);
}

vec3 pal(float m, float s) {
	return texelFetch(look_palette, ivec2(int(clamp(s, 0.0, 6.0) + 0.5), int(m + 0.5)), 0).rgb;
}


// The step and colour of a pixel from its light, as the painter's resolve and final passes do (PRE-20, PRE-30):
// sun after shadow and sky make a continuous step, rounded with no pattern where steps meet (the sharp look);
// the pattern's bias moves it; an outline darkens and a lit edge brightens by whole steps (PRE-21); haze in steps.
// The step of a material's ramp the light gives (the painter's light()): from the sky's and the sun's light, by the
// hour's shift and contrast, raised by firelight.
float light_step(float sky_light, float sun, float fire_light) {
	return 3.0 + ((look_sky_k * sky_light + look_sun_k * sun) * 6.0 + look_shift - 3.0) * look_contrast + fire_light * 4.5;
}

// The haze by distance from the camera, in four steps (PRE-30).
vec3 hazed(vec3 c, float depth) {
	float hz = floor(clamp((depth - look_haze_range.x) / max(look_haze_range.y - look_haze_range.x, 0.001), 0.0, 1.0) * 4.0) / 4.0;
	return mix(c, look_haze_col, hz * look_haze_k);
}

vec3 shade_pixel(float ndl, float shadow, float depth, float m_row, float m_bias, float m_flags, float m_sky, float m_fire, float m_edge) {
	bool foliage = bit(m_flags, FLAG_FOLIAGE);
	float f;
	float cls;
	if (bit(m_flags, FLAG_EMISSIVE)) {
		f = 0.0;
		cls = 7.0;
	} else {
		float n = foliage ? ndl * 0.55 + 0.45 : ndl;
		float sh = n > 0.0 ? shadow : 0.0;
		float sun = pow(max(n, 0.0), look_sun_pow) * sh;
		float sunlit = (sh > 0.5 && n > 0.06) ? 1.0 : 0.0;
		// in full daylight a fire's light is lost: by day it shows only in shade, so sunlit ground round a
		// fire takes no pale disc (look_fire_in_sun: 0 at noon, 1 at night, when the "sun" is the moon)
		float fl = sunlit > 0.5 ? m_fire * look_fire_in_sun : m_fire;
		f = light_step(m_sky, sun, fl);
		if (bit(m_flags, FLAG_CREATURE)) {
			f += 0.9;
		}
		if (foliage) {
			f -= 0.3;
		}
		float fb = fl > 0.36 ? 2.0 : (fl > 0.15 ? 1.0 : 0.0);
		cls = sunlit + fb * 2.0;
	}
	float st = cls > 6.5 ? clamp(m_bias, 0.0, 6.0) : clamp(floor(f + 0.5) + m_bias, 0.0, 6.0);
	if (m_edge > 0.5 && m_edge < 1.5) {
		st = max(st - (foliage ? look_edges.y : look_edges.x), 0.0);
	} else if (m_edge > 1.5 && mod(cls, 2.0) > 0.5) {
		st = min(st + look_edges.z, 6.0);
	}
	return hazed(grade(pal(m_row, st), cls), depth);
}

// Outline method (c): a second camera draws every pixel's normal and depth at the same low resolution
// (look_gbuf: the normal's x and y in red and green, the depth past look_gbuf_depth.x in blue, over
// look_gbuf_depth.y metres), and each material reads its four neighbours there, as the painter's final pass reads
// its buffers: a neighbour well behind the plane this pixel lies in makes an outline (1), or a lit edge (2) on the
// sun's side of a sunlit shape, and a sunlit fold makes a lit edge too.
float outline_edge(vec2 uv, vec3 view_pos, vec3 n, mat4 view, float fl) {
	if (look_outline != 3 || bit(fl, FLAG_NOOUTLINE) || bit(fl, FLAG_EMISSIVE)) {
		return 0.0;
	}
	vec2 px = 1.0 / vec2(textureSize(look_gbuf, 0));
	float d = -view_pos.z;
	vec3 nv = (view * vec4(n, 0.0)).xyz;
	vec2 sv = normalize((view * vec4(look_sun_dir, 0.0)).xy + vec2(1e-4));
	bool foliage = bit(fl, FLAG_FOLIAGE);
	bool sunward = dot(n, look_sun_dir) > 0.25;
	float e = 0.0;
	for (int k = 0; k < 4; k++) {
		vec2 o = k == 0 ? vec2(1.0, 0.0) : (k == 1 ? vec2(-1.0, 0.0) : (k == 2 ? vec2(0.0, 1.0) : vec2(0.0, -1.0)));
		vec4 q = texture(look_gbuf, uv + vec2(o.x, -o.y) * px);
		float qd = look_gbuf_depth.x + q.b * look_gbuf_depth.y;
		float pred = d + (nv.x * o.x + nv.y * o.y) * look_mpp / max(nv.z, 0.25);
		float thr = (foliage ? 1.6 : 0.32) * look_edges.w;
		if (q.a < 0.5 || qd > pred + thr) {
			bool rim = dot(o, sv) > 0.35 && sunward;
			if (rim && e == 0.0) {
				e = 2.0;
			} else if (!rim) {
				e = 1.0;
			}
		}
	}
	if (e == 0.0 && !foliage && sunward) {
		// a fold: a neighbour of the same surface turned away from the sun catches a lit edge on this side
		for (int k = 0; k < 4; k++) {
			vec2 o = k == 0 ? vec2(1.0, 0.0) : (k == 1 ? vec2(-1.0, 0.0) : (k == 2 ? vec2(0.0, 1.0) : vec2(0.0, -1.0)));
			vec4 q = texture(look_gbuf, uv + vec2(o.x, -o.y) * px);
			vec3 qn = oct_decode(q.rg);
			float qd = look_gbuf_depth.x + q.b * look_gbuf_depth.y;
			float pred = d + (nv.x * o.x + nv.y * o.y) * look_mpp / max(nv.z, 0.25);
			if (q.a > 0.5 && dot(n, qn) < 0.8 && dot(n, look_sun_dir) > dot(qn, look_sun_dir) + 0.2 && qd > pred - 0.03) {
				e = 2.0;
			}
		}
	}
	return e;
}
#endif
