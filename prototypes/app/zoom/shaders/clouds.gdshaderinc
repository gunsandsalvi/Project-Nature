// P8's clouds (A8.6): what the ground's shadows and the sky's clouds share, read from the weather's picture of the
// whole world (weather.gdshader), which must not include this, since it draws that picture. Include planet.gdshaderinc
// first.

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

// The clouds' density at a place in the world's metres and a height above the sea, 0 to 1, for the weather there.
// Close up, the cover's masses, rounded below and narrowing to their tops, shaped by Perlin's noise eroded by Worley's
// cells (Schneider 2015) at two scales turned to each other and shifted from place to place, so no pattern repeats,
// and worn at their edges by finer cells. A pixel `fp` metres across shows only the shaping larger than it: from far
// off, the clouds are their cover's masses with crisp edges, without flicker; the thin cover of small fair-weather
// clouds shows only once they can be seen. The ground's shadows read the same clouds.
float cloud_density(vec2 w, float alt, vec4 weather, float fp) {
	float cover = weather.x;
	if (cover <= 0.02) {
		return 0.0;
	}
	float h = (alt - CLOUD_BASE) / (cloud_top(weather.y) - CLOUD_BASE);
	if (h <= 0.0 || h >= 1.0) {
		return 0.0;
	}
	float profile = smoothstep(0.0, 0.08, h) * smoothstep(1.0, 0.55, h);
	float far = profile * 0.35 * smoothstep(0.5, 0.56, cover * mix(1.0, 0.75, h));
	// the shapes: the noise's texels are 83 m, and its smallest shapes about 300 m
	float fine = smoothstep(260.0, 90.0, fp);
	if (fine <= 0.0) {
		return far;
	}
	float c = cover * mix(1.0, 0.6, h);
	vec3 p = vec3(w, alt * 1.6) / 5300.0 + vec3(weather.zw, 0.0);
	vec2 turned = mat2(vec2(0.8, 0.6), vec2(-0.6, 0.8)) * w;
	float lod = log2(max(fp / 83.0, 1.0));
	float shape = textureLod(zoom_cloud_noise, p, lod).r * 0.6
			+ textureLod(zoom_cloud_noise, vec3(turned, alt * 1.6) / 3700.0 + vec3(weather.wz, 0.31), lod).r * 0.4;
	float d = clamp((shape * profile - (1.0 - c)) / max(c, 0.05), 0.0, 1.0);
	// the edges' wear: cells of about 140 m and 60 m
	float wear = smoothstep(110.0, 30.0, fp);
	if (d > 0.0 && wear > 0.0) {
		float edge = textureLod(zoom_cloud_noise, p * 4.7 + vec3(0.37, 0.11, 0.0), log2(max(fp / 17.6, 1.0))).g * 0.6
				+ textureLod(zoom_cloud_noise, p * 11.3 + vec3(0.71, 0.53, 0.0), log2(max(fp / 7.3, 1.0))).b * 0.4;
		d = clamp(d - 0.5 * edge * (1.0 - d) * wear, 0.0, 1.0);
	}
	return mix(far, d, fine);
}

// The art book's flat clouds (variant C): one deck, the weather's cover worn at its edges by finer noise, hard-edged:
// 1 cloud, 0 clear.
float cloud_puff(vec2 w, vec4 weather, float fp) {
	float edge = value_noise(w / 4000.0, zoom_world.x / 4000.0) * 0.6
			+ value_noise(w / 1600.0, zoom_world.x / 1600.0) * 0.4 * smoothstep(1600.0, 400.0, fp);
	return step(0.5, weather.x + 0.45 * (edge - 0.5));
}

// How much of the sun reaches a place on the ground `ground` metres above the sea through the clouds of the variant
// shown, 0 to 1, the pixel `fp` metres across; the sky's light still reaches the shade.
float cloud_shadow(vec2 w, float ground, mat3 frame, float fp) {
	vec3 sun = normalize(zoom_sun);
	vec3 local = vec3(dot(sun, frame[0]), dot(sun, frame[1]), dot(sun, frame[2]));
	if (local.y <= 0.03) {
		return 1.0;
	}
	vec2 toward = local.xz / local.y;
	if (int(zoom_style.z + 0.5) == 2) {
		vec2 at = w + toward * (CLOUD_DECK - ground);
		return 1.0 - 0.5 * cloud_puff(at, cloud_weather(at), fp);
	}
	vec4 weather = cloud_weather(w + toward * (3500.0 - ground));
	float top = cloud_top(weather.y);
	float od = 0.0;
	for (int k = 0; k < 3; k++) {
		float alt = mix(CLOUD_BASE, top, (float(k) + 0.5) / 3.0);
		vec2 at = w + toward * (alt - ground);
		od += cloud_density(at, alt, cloud_weather(at), fp);
	}
	od *= CLOUD_SIGMA * (top - CLOUD_BASE) / 3.0 / local.y;
	return mix(0.3, 1.0, exp(-od));
}
