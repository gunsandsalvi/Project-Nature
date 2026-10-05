// The look's light (A4.1, PRE-20, PRE-30), the core every lit material shares: the master palette's ramps, the step the
// sun and the sky give a pixel, the hour's colours and the haze, with no texture but the palette. look.gdshaderinc builds
// P1's materials on it; P8's ground uses it alone, so the zoom's ground and what stands on it are lit as one.
// Pre-production code (research 00): thrown away with the prototypes.
#ifndef LOOK_CORE
#define LOOK_CORE

global uniform sampler2D look_palette : filter_nearest;
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
// how much of a fire's light shows on sunlit ground: none at noon, all at night (look.gd's _set_hour)
global uniform float look_fire_in_sun;
global uniform float look_sky_cover;
global uniform vec4 look_edges;
// up to four fires: each column the fire's place and reach, and their powers (0 for none)
global uniform mat4 look_fires;
global uniform vec4 look_fire_powers;
// each fire's flicker, a share of its power, felt only near its flames, so the pool's rim holds still
global uniform vec4 look_fire_flicker;

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

vec3 to_linear(vec3 c) {
	return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}

vec3 to_srgb(vec3 c) {
	return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), c));
}

// Fire i's light on a point `p` facing `n` (PRE-30), before anything standing between them: fading to its reach,
// which `ragged`, 0 to 1, moves in and out so the pool's edge is not a circle; `wrap` of it reaches round the point's
// sides; flickering near its flames.
float fire_reach(int i, vec3 p, vec3 n, float wrap, float ragged) {
	vec3 l = look_fires[i].xyz - p;
	float d = length(l);
	float att = clamp(1.0 - d / (look_fires[i].w * (0.78 + 0.44 * ragged)), 0.0, 1.0);
	if (att <= 0.0) {
		return 0.0;
	}
	float lit = wrap + (1.0 - wrap) * max(dot(n, l / max(d, 0.001)), 0.0);
	float flicker = mix(look_fire_flicker[i], 1.0, smoothstep(1.0, 2.5, d));
	return look_fire_powers[i] * flicker * pow(att, 1.4) * lit;
}

// The sky's share of a point's light, before the sun, for its facing `n` and how open to the sky it is, 0 to 1.
float sky_light(vec3 n, float open) {
	return (0.7 + 0.3 * n.y) * mix(1.0 - look_sky_cover, 1.0, open);
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
		// firelit colour leans to a warm ramp of the pixel's own brightness, so lit grass reads amber, not olive:
		// the fire's tint of green is olive, so the ramp, its brightness in the fire's colour, carries the hue
		g = mix(g, mix(tint(c, look_fire_tint), look_fire_tint * lc * 1.55, 0.9), fb > 1.5 ? 0.95 : 0.8);
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
		// the sun's shadow is either there or not, as the painter's is: a low sun stretches the shadow map's
		// texels along the ground, and their filtered edges read as soft blobs of a step or two
		float sh = n > 0.0 ? step(0.5, shadow) : 0.0;
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
#endif
