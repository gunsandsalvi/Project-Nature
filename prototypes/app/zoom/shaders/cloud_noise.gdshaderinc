// P8's clouds close up (A8.6): their noise, a 3D texture with its mipmaps, and their density shaped by it. Only the
// shaders that need it include this, after clouds.gdshaderinc and with CLOUD_NOISE defined, so the light ones bind no
// 3D texture at all.

global uniform sampler3D zoom_cloud_noise : filter_linear_mipmap, repeat_enable;  // the clouds' shape and edges

// The clouds' density at a place in the world's metres and a height above the sea, 0 to 1, for the weather there.
// Close up, the cover's masses, rounded below and narrowing to their tops, shaped by Perlin's noise eroded by Worley's
// cells (Schneider 2015) at two scales turned to each other and shifted from place to place, so no pattern repeats,
// and worn at their edges by finer cells. A pixel `fp` metres across shows only the shaping larger than it: from far
// off, the clouds are their cover's masses with crisp edges, without flicker; the thin cover of small fair-weather
// clouds shows only once they can be seen. The ground's shadows read the same clouds.
float cloud_density(vec2 w, float alt, vec4 weather, float fp) {
	float far = cloud_far(alt, weather);
	float cover = weather.x;
	float h = (alt - CLOUD_BASE) / (cloud_top(weather.y) - CLOUD_BASE);
	if (cover <= 0.02 || h <= 0.0 || h >= 1.0) {
		return 0.0;
	}
	float profile = smoothstep(0.0, 0.08, h) * smoothstep(1.0, 0.55, h);
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

