// P8's clouds (A8.6): what the ground's shadows and the sky's clouds share, read from the weather's picture of the
// whole world (weather.gdshader), which must not include this, since it draws that picture. Include planet.gdshaderinc
// first, and cloud_noise.gdshaderinc after it where CLOUD_NOISE is defined, for the clouds' shapes close up.

global uniform sampler2D zoom_weather : filter_linear, repeat_enable;  // the clouds' cover and how high they build

// Metres above the sea (WLD-30: real heights): the clouds' bases, the highest a storm's towers build, the deck of the
// flat clouds; and their density's extinction, a metre.
const float CLOUD_BASE = 1500.0;
const float CLOUD_TOP = 9500.0;
const float CLOUD_DECK = 3000.0;
const float CLOUD_SIGMA = 0.012;

// The weather's clouds at a place in the world's metres (weather.gdshader): x their cover, 0 to 1; y how high they
// build, 0 to 1; zw how far their noise is shifted there, in its periods.
vec4 cloud_weather(vec2 w) {
	return textureLod(zoom_weather, w / zoom_world, 0.0);
}

// The top of the clouds where they build `tops` high, metres above the sea.
float cloud_top(float tops) {
	return CLOUD_BASE + mix(900.0, CLOUD_TOP - CLOUD_BASE, tops);
}

// The clouds' density as seen from far off, 0 to 1, at a height above the sea for the weather there: the cover's
// masses with crisp edges, rounded below and narrowing to their tops, without flicker; the thin cover of small
// fair-weather clouds shows only close up, where the clouds' noise shapes them (cloud_noise.gdshaderinc).
float cloud_far(float alt, vec4 weather) {
	float h = (alt - CLOUD_BASE) / (cloud_top(weather.y) - CLOUD_BASE);
	if (weather.x <= 0.02 || h <= 0.0 || h >= 1.0) {
		return 0.0;
	}
	float profile = smoothstep(0.0, 0.08, h) * smoothstep(1.0, 0.55, h);
	return profile * 0.35 * smoothstep(0.5, 0.56, weather.x * mix(1.0, 0.75, h));
}

// The art book's flat clouds (variant C): one deck, the weather's cover worn at its edges by finer noise, hard-edged:
// 1 cloud, 0 clear.
float cloud_puff(vec2 w, vec4 weather, float fp) {
	float edge = value_noise(w / 4000.0, zoom_world.x / 4000.0) * 0.6
			+ value_noise(w / 1600.0, zoom_world.x / 1600.0) * 0.4 * smoothstep(1600.0, 400.0, fp);
	return step(0.5, weather.x + 0.45 * (edge - 0.5));
}
