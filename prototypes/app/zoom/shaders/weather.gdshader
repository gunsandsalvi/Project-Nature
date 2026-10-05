// P8's weather (A8.6, WLD-16): the clouds' cover everywhere in the world and how high they build, drawn as a picture
// of the whole world that every other shader reads (zoom_weather). The cover follows the climate's wetness and the
// belts of rising and sinking air, which move with the sun through the seasons; its shapes are weather systems: noise
// warped by noise (Quílez), so it swirls and streams, twisted round the storms of the storm tracks and, in summer, a
// tropical storm; carried by the winds, from the west in the middle latitudes, from the east in the tropics and by the
// poles. Its edges are sharp, so clouds are masses with clear sky between them, as seen from space, thicker inside.
shader_type canvas_item;
render_mode blend_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"

// The pattern's largest cells, round the world and from pole to pole: 167 km.
const float AROUND = 12.0;
const float POLES = 6.0;
// How long the winds carry one copy of the pattern before it begins again, seconds; and a day.
const float CYCLE = 10800.0;
const float DAY = 86400.0;

// Fractal value noise, 0 to 1, wrapping round the world.
float fbm(vec2 p, int octaves) {
	float sum = 0.0;
	float amp = 0.5;
	float around = AROUND;
	for (int o = 0; o < octaves; o++) {
		sum += amp * value_noise(p, around);
		p = p * 2.0 + vec2(0.31, 0.67);
		around *= 2.0;
		amp *= 0.5;
	}
	return sum / (1.0 - 2.0 * amp);
}

// The metres from c to p, east the shorter way round.
vec2 apart(vec2 p, vec2 c) {
	vec2 d = p - c;
	d.x -= zoom_world.x * round(d.x / zoom_world.x);
	return d;
}

// A place twisted round a storm at c, `turn` radians at its middle (counterclockwise positive), less farther out.
vec2 twist(vec2 w, vec2 c, float turn, float radius) {
	vec2 d = apart(w, c);
	float a = turn * exp(-dot(d, d) / (radius * radius));
	float s = sin(a);
	float k = cos(a);
	return c + vec2(k * d.x - s * d.y, s * d.x + k * d.y);
}

// The storms at a time, at a place in the world's metres: xy the place twisted round each storm; z how stormy it is
// there, 0 to 1; w how much of a storm's clear eye it is in. The storm tracks' lows, two each side of the equator,
// each living about four days, carried east by the westerlies; and in each hemisphere's summer, a tropical storm
// carried west.
vec4 storms(vec2 w, float t) {
	float storm = 0.0;
	float eye = 0.0;
	float year = sin(TAU * zoom_clock.y);
	for (int k = 0; k < 6; k++) {
		float fk = float(k);
		float tropical = k >= 4 ? 1.0 : 0.0;
		float north = (k % 2 == 0) ? 1.0 : -1.0;
		float life_days = mix(4.0, 7.0, tropical);
		float start = hash12(vec2(fk, 1.7));
		float age = t / (life_days * DAY) + start;
		float which = floor(age);
		float life = fract(age);
		float strength = sin(PI * life);
		strength *= strength;
		// a tropical storm only in its hemisphere's summer, where the sea is warm enough
		strength *= mix(1.0, smoothstep(0.2, 0.7, north * year), tropical);
		float lat = mix(50.0 + 12.0 * (hash12(vec2(which, fk)) - 0.5), 16.0 + 6.0 * life, tropical);
		float speed = mix(14.0, -6.0, tropical);
		float born = (which - start) * life_days * DAY;
		vec2 c = vec2(
			zoom_world.x * hash12(vec2(fk + 0.5, which)) + speed * (t - born),
			zoom_world.y * (0.5 + north * lat / 180.0)
		);
		float radius = mix(110000.0, 60000.0, tropical);
		vec2 d = apart(w, c);
		storm = max(storm, strength * exp(-dot(d, d) / (radius * radius)));
		eye = max(eye, tropical * strength * smoothstep(9000.0, 4000.0, length(d)));
		w = twist(w, c, north * strength * mix(2.6, 6.0, tropical), radius);
	}
	return vec4(w, storm, eye);
}

// The weather's shapes at p in the pattern's cells: x 0 to 1, noise warped twice by noise, slowly changing; y the
// heaps the clouds gather in, rounded cells of about 28 and 10 km, 1 in their middles.
vec2 pattern(vec2 p, float t) {
	float drift = t / DAY;
	vec2 q = vec2(fbm(p + vec2(0.0, 0.05 * drift), 4), fbm(p + vec2(5.2, 1.3), 4));
	vec2 r = vec2(fbm(p + 2.0 * q + vec2(1.7, 9.2), 4), fbm(p + 2.0 * q + vec2(8.3, 2.8 + 0.03 * drift), 4));
	vec2 warped = p + 2.0 * r;
	vec4 big = worley(warped * 6.0, AROUND * 6.0);
	vec4 small = worley(warped * 16.0 + vec2(5.2, 0.7), AROUND * 16.0);
	float heaps = (1.0 - big.x * big.x) * 0.6 + (1.0 - small.x * small.x) * 0.4;
	return vec2(fbm(warped, 6), heaps);
}

void fragment() {
	vec2 w = UV * zoom_world;
	float lat = PI * (UV.y - 0.5);
	float deg = degrees(lat);
	float t = zoom_clock.x;
	vec4 twisted = storms(w, t);
	w = twisted.xy;
	vec2 storm = twisted.zw;
	// two copies of the pattern, each carried by the winds and begun again in turn, crossfaded so neither's
	// beginning shows; the winds in metres a second east
	float wind = -7.0 + 21.0 * smoothstep(12.0, 40.0, abs(deg)) - 19.0 * smoothstep(52.0, 78.0, abs(deg));
	float ta = mod(t, CYCLE);
	float tb = mod(t + 0.5 * CYCLE, CYCLE);
	float fade = 1.0 - abs(2.0 * ta / CYCLE - 1.0);
	vec2 cells = vec2(AROUND, POLES) / zoom_world;
	vec2 a = pattern((w - vec2(wind * ta, 0.0)) * cells, t);
	vec2 b = pattern((w - vec2(wind * tb, 0.0)) * cells + vec2(3.3, 1.9), t);
	float f = mix(b.x, a.x, fade);
	float heaps = mix(b.y, a.y, fade);
	// the belts: the rising air by the thermal equator, which follows the sun; the sinking air of the subtropics; the
	// storm tracks; the dry cold by the poles
	float itcz = degrees(zoom_clock.w) * 0.6;
	float belts = 0.24 * exp(-pow((deg - itcz) / 7.0, 2.0))
			- 0.26 * exp(-pow((abs(deg) - 26.0) / 8.0, 2.0))
			+ 0.12 * exp(-pow((abs(deg) - 56.0) / 10.0, 2.0))
			- 0.45 * smoothstep(64.0, 80.0, abs(deg));
	vec4 clim = texture(zoom_climate, UV);
	float wet = clamp(clim.a, 0.0, 1.2);
	float c = 0.30 + 0.25 * (wet - 0.5) + belts + 1.6 * (f - 0.5) + 0.4 * storm.x;
	float masses = smoothstep(0.44, 0.74, c) * (1.0 - storm.y);
	// thicker and thinner within the masses, in cells of a few tens of kilometres
	float within = fbm(w * cells * 5.0 + vec2(3.7, 8.1), 4);
	masses *= mix(0.5, 1.0, smoothstep(0.3, 0.7, within));
	// heaped: thick in the heaps' middles, thin or clear between them
	masses *= smoothstep(0.15, 0.6, heaps + 0.3 * (masses - 0.5));
	// fair-weather cumulus between the masses, where it is warm and moist enough: a thin cover, which the clouds' own
	// noise makes into small clouds close up and a faint veil from far off
	float warm = smoothstep(2.0, 16.0, clim.r) * clamp(wet * 2.0, 0.0, 1.0);
	float small = fbm(w * cells * 3.0 + vec2(7.1, 2.3), 3);
	float cumulus = 0.52 * smoothstep(0.38, 0.58, small) * warm * (1.0 - storm.y);
	float cover = max(masses, cumulus);
	// how high they build: towers in the warm and wet masses and in storms, low elsewhere
	float tops = clamp(0.1 + 0.75 * masses * warm + 0.5 * storm.x, 0.0, 1.0);
	tops = mix(0.12, tops, smoothstep(0.3, 0.6, masses + storm.x)) * mix(0.7, 1.0, within) * mix(0.55, 1.0, heaps);
	// and two smooth random fields, 12.5 km across, that shift the clouds' noise from place to place (cloud_density), so
	// its pattern, which repeats every few kilometres, never shows repeating
	vec2 shift = vec2(value_noise(UV * 160.0, 160.0), value_noise(UV * 160.0 + vec2(37.0, 11.0), 160.0));
	COLOR = vec4(cover, tops, shift);
}
