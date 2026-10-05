// P8's sky (A8.4, A8.6), a pass over the whole picture after the ground: along each pixel's ray, the clouds and the
// air in front of the ground or, past the planet's edge, of space and its stars. The air scatters sunlight as Earth's
// does, by Rayleigh's and Mie's laws (single scattering, after Nishita 1993 and Hillaire 2020): blue overhead, a glow
// along the planet's rim, haze over distant ground, the sun reddening as it sinks. The clouds, from the climate's
// cover (planet.gdshaderinc): A raymarched through their 3D shapes and lit by the sun through themselves (Schneider
// 2015), so the camera passes through them as it descends; B three thin decks, cheaper; C the art book's flat clouds.
// Light: A the art book's, the air only a thin glow at the rim; B the air's full scattering.
shader_type spatial;
render_mode unshaded, depth_draw_never, depth_test_disabled, cull_disabled, shadows_disabled, fog_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"

uniform sampler2D screen : hint_screen_texture, filter_nearest;
uniform sampler2D depth : hint_depth_texture, filter_nearest;

// The air, scaled to this small planet: thinner than Earth's by its scale heights, denser to match, so the sky over
// the ground is Earth's blue and the rim seen from space is as thin against the globe as Earth's.
const vec3 BETA_R = vec3(5.8e-6, 13.5e-6, 33.1e-6) * 4.0;
const float BETA_M = 21e-6 * 3.0;
const float H_R = 2000.0;
const float H_M = 500.0;
const float AIR_TOP = 16000.0;
const float SUN_POWER = 20.0;

void vertex() {
	POSITION = vec4(VERTEX.xy * 2.0, 0.0, 1.0);
}

// Where a ray from o along unit d enters and leaves a sphere at c of radius r; both negative if it misses.
vec2 sphere_hits(vec3 o, vec3 d, vec3 c, float r) {
	vec3 oc = o - c;
	float b = dot(oc, d);
	float q = dot(oc, oc) - r * r;
	float h = b * b - q;
	if (h < 0.0) {
		return vec2(-1.0);
	}
	h = sqrt(h);
	return vec2(-b - h, -b + h);
}

// The place in the world of a point in the picture: metres east and north of the focus, and its height above the sea.
vec3 world_of(vec3 p) {
	vec3 rel = p - planet_middle();
	float len = length(rel);
	vec3 u = rel / len;
	float lat0 = focus_latitude();
	float east = u.x;
	float north = -u.z;
	float sin_lat = north * cos(lat0) + u.y * sin(lat0);
	float gz = -north * sin(lat0) + u.y * cos(lat0);
	float lat = asin(clamp(sin_lat, -1.0, 1.0));
	float lon = atan(east, gz);
	vec2 d = vec2(lon * zoom_world.x / TAU, (lat - lat0) * zoom_world.y / PI);
	// undo the stretch that shows the ground near the focus at its true scale
	float stretch = 1.0 / max(cos(lat0), 0.33) - 1.0;
	d.x /= 1.0 + zoom_true.x * stretch * exp(-dot(d, d) / (zoom_true.y * zoom_true.y));
	return vec3(d, len - planet_radius());
}

// The air's optical depth from p toward the sun, Rayleigh's and Mie's; very deep if the planet stands in the way.
vec2 sun_depth(vec3 p, vec3 sun, vec3 c, float r) {
	vec2 ground = sphere_hits(p, sun, c, r);
	if (ground.x > 0.0) {
		return vec2(1.0e9);
	}
	float far = sphere_hits(p, sun, c, r + AIR_TOP).y;
	float ds = max(far, 0.0) / 4.0;
	vec2 od = vec2(0.0);
	for (int i = 0; i < 4; i++) {
		float h = length(p + sun * ((float(i) + 0.5) * ds) - c) - r;
		od += exp(-max(h, 0.0) / vec2(H_R, H_M)) * ds;
	}
	return od;
}

// The sunlight that reaches p through the air, 0 to 1 each colour.
vec3 sunlight_at(vec3 p, vec3 sun, vec3 c, float r) {
	vec2 od = sun_depth(p, sun, c, r);
	return exp(-(BETA_R * od.x + BETA_M * 1.1 * od.y));
}

// The air along a ray up to t_end: what it lets through and what it scatters toward the eye.
void air(vec3 o, vec3 dir, float t_end, vec3 c, float r, out vec3 through, out vec3 glow) {
	through = vec3(1.0);
	glow = vec3(0.0);
	vec2 hit = sphere_hits(o, dir, c, r + AIR_TOP);
	float t0 = max(hit.x, 0.0);
	float t1 = min(hit.y, t_end);
	if (hit.y <= 0.0 || t1 <= t0) {
		return;
	}
	vec3 sun = normalize(zoom_sun);
	float mu = dot(dir, sun);
	float rayleigh = 3.0 / (16.0 * PI) * (1.0 + mu * mu);
	float g = 0.76;
	float mie = 3.0 / (8.0 * PI) * ((1.0 - g * g) * (1.0 + mu * mu)) / ((2.0 + g * g) * pow(1.0 + g * g - 2.0 * g * mu, 1.5));
	float ds = (t1 - t0) / 10.0;
	vec2 od = vec2(0.0);
	vec3 sum_r = vec3(0.0);
	vec3 sum_m = vec3(0.0);
	for (int i = 0; i < 10; i++) {
		vec3 p = o + dir * (t0 + (float(i) + 0.5) * ds);
		float h = length(p - c) - r;
		vec2 rho = exp(-max(h, 0.0) / vec2(H_R, H_M)) * ds;
		od += rho;
		vec2 sod = sun_depth(p, sun, c, r);
		vec3 att = exp(-(BETA_R * (od.x + sod.x) + BETA_M * 1.1 * (od.y + sod.y)));
		sum_r += rho.x * att;
		sum_m += rho.y * att;
	}
	glow = SUN_POWER * (sum_r * BETA_R * rayleigh + sum_m * BETA_M * mie);
	through = exp(-(BETA_R * od.x + BETA_M * 1.1 * od.y));
}

float henyey_greenstein(float mu, float g) {
	return (1.0 - g * g) / (4.0 * PI * pow(1.0 + g * g - 2.0 * g * mu, 1.5));
}

// The volumetric clouds along a ray up to t_end: their light and what they let through.
vec4 clouds_volume(vec3 o, vec3 dir, float t_end, vec3 c, float r, vec2 frag) {
	vec2 outer = sphere_hits(o, dir, c, r + CLOUD_TOP);
	if (outer.y <= 0.0) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	vec2 inner = sphere_hits(o, dir, c, r + CLOUD_BASE);
	float alt = length(o - c) - r;
	float t0 = max(outer.x, 0.0);
	float t1 = min(outer.y, t_end);
	if (alt < CLOUD_BASE) {
		t0 = max(t0, inner.y);
	} else if (inner.x > 0.0) {
		t1 = min(t1, inner.x);
	}
	if (t1 <= t0) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	vec3 sun = normalize(zoom_sun);
	float mu = dot(dir, sun);
	float phase = mix(henyey_greenstein(mu, 0.55), henyey_greenstein(mu, -0.2), 0.35) * 4.0 * PI;
	// the sun's way in the ground's frame where the ray meets the clouds, so steps toward it stay in the world's metres
	mat3 frame = planet_frame(o + dir * t0);
	vec3 sun_local = vec3(dot(sun, frame[0]), dot(sun, frame[1]), dot(sun, frame[2]));
	vec3 sunlight = sunlight_at(o + dir * t0, sun, c, r);
	float day = smoothstep(-0.15, 0.2, sun_local.y);
	int steps = 28;
	float ds = (t1 - t0) / float(steps);
	float jitter = bayer4(frag);
	float through = 1.0;
	vec3 light = vec3(0.0);
	float cover = 0.0;
	for (int i = 0; i < steps; i++) {
		vec3 p = o + dir * (t0 + (float(i) + jitter) * ds);
		vec3 at = world_of(p);
		vec2 w = zoom_focus + at.xy;
		if (i % 4 == 0) {
			cover = cloud_cover(w);
		}
		float dense = cloud_density(w, at.z, cover);
		if (dense <= 0.0) {
			continue;
		}
		float sigma = dense * 0.006;
		// the sun's light through the cloud above this point, three steps toward it
		float od = 0.0;
		float reach = 150.0;
		for (int k = 0; k < 3; k++) {
			vec2 qw = w + vec2(sun_local.x, sun_local.z) * reach;
			od += cloud_density(qw, at.z + sun_local.y * reach, cover) * 0.006 * reach;
			reach *= 2.6;
		}
		float h01 = (at.z - CLOUD_BASE) / (CLOUD_TOP - CLOUD_BASE);
		vec3 sun_in = sunlight * exp(-od) * (1.0 - exp(-od * 2.0 - 0.15)) * 2.2 * phase;
		vec3 sky_in = vec3(0.55, 0.65, 0.8) * (0.35 + 0.45 * h01) * day;
		float a = 1.0 - exp(-sigma * ds);
		light += through * a * (sun_in * 1.4 + sky_in);
		through *= 1.0 - a;
		if (through < 0.02) {
			break;
		}
	}
	return vec4(light, through);
}

// Three thin decks of cloud (variant B), each a sheet whose shading comes from the slope of its own thickness.
vec4 clouds_decks(vec3 o, vec3 dir, float t_end, vec3 c, float r) {
	vec3 sun = normalize(zoom_sun);
	float heights[3] = float[3](5500.0, 3500.0, 2000.0);
	float alt = length(o - c) - r;
	vec3 light = vec3(0.0);
	float through = 1.0;
	for (int k = 0; k < 3; k++) {
		// near decks first: from above, the highest; from below, the lowest
		float hk = alt > heights[0] ? heights[k] : heights[2 - k];
		vec2 hit = sphere_hits(o, dir, c, r + hk);
		float t = alt > hk ? hit.x : hit.y;
		if (t <= 0.0 || t >= t_end) {
			continue;
		}
		vec3 p = o + dir * t;
		vec2 w = zoom_focus + world_of(p).xy;
		float cover = cloud_cover(w);
		float z = 0.21 * float(k + 1);
		float thick = textureLod(zoom_cloud_noise, vec3(w / 14000.0, z), 0.0).r * 0.75
				+ textureLod(zoom_cloud_noise, vec3(w / 4200.0, z), 0.0).g * 0.25;
		float a = smoothstep(1.0 - cover, 1.0 - cover + 0.25, thick) * 0.9;
		if (a <= 0.0) {
			continue;
		}
		vec2 ahead = w + normalize(vec2(sun.x, -sun.z) + 1e-4) * 600.0;
		float thick_ahead = textureLod(zoom_cloud_noise, vec3(ahead / 14000.0, z), 0.0).r * 0.75
				+ textureLod(zoom_cloud_noise, vec3(ahead / 4200.0, z), 0.0).g * 0.25;
		float lit = clamp(0.75 + (thick - thick_ahead) * 4.0, 0.35, 1.15);
		vec3 sun_in = sunlight_at(p, sun, c, r) * lit * smoothstep(-0.1, 0.15, dot(normalize(p - c), sun));
		light += through * a * (sun_in * 0.95 + vec3(0.18, 0.22, 0.3));
		through *= 1.0 - a;
	}
	return vec4(light, through);
}

// The art book's flat clouds (variant C): one deck, hard-edged, in three tones, its sun side white.
vec4 clouds_flat(vec3 o, vec3 dir, float t_end, vec3 c, float r) {
	vec3 sun = normalize(zoom_sun);
	float hk = 3000.0;
	float alt = length(o - c) - r;
	vec2 hit = sphere_hits(o, dir, c, r + hk);
	float t = alt > hk ? hit.x : hit.y;
	if (t <= 0.0 || t >= t_end) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	vec3 p = o + dir * t;
	vec2 w = zoom_focus + world_of(p).xy;
	float cover = cloud_cover(w);
	if (cloud_puff(w, cover) < 0.5) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	vec2 toward = normalize(vec2(sun.x, -sun.z) + 1e-4);
	float edge_sun = cloud_puff(w + toward * 900.0, cover);
	float edge_shade = cloud_puff(w - toward * 900.0, cover);
	vec3 tone = vec3(0.93, 0.95, 0.94);
	if (edge_sun < 0.5) {
		tone = vec3(1.0, 1.0, 0.98);
	} else if (edge_shade < 0.5) {
		tone = vec3(0.62, 0.68, 0.8);
	}
	float day = smoothstep(-0.1, 0.15, dot(normalize(p - c), sun));
	return vec4(tone * sunlight_at(p, sun, c, r) * mix(0.08, 1.0, day), 0.0);
}

// Stars in the dark beyond the planet.
vec3 stars(vec3 dir) {
	vec3 q = floor(dir * 420.0);
	float h = hash12(q.xy + q.z * 17.13);
	return vec3(step(0.9965, h) * (0.4 + 0.6 * hash12(q.yz + 3.1)));
}

void fragment() {
	float raw = texture(depth, SCREEN_UV).r;
	vec4 view = INV_PROJECTION_MATRIX * vec4(SCREEN_UV * 2.0 - 1.0, max(raw, 1e-7), 1.0);
	view.xyz /= view.w;
	vec3 o = INV_VIEW_MATRIX[3].xyz;
	vec3 at = (INV_VIEW_MATRIX * vec4(view.xyz, 1.0)).xyz;
	vec3 dir = normalize(at - o);
	bool open = raw <= 0.0;
	float t_end = open ? 1.0e12 : length(at - o);
	vec3 c = planet_middle();
	float r = planet_radius();
	vec3 scene = open ? stars(dir) : texture(screen, SCREEN_UV).rgb;
	int cloud_style = int(zoom_style.z + 0.5);
	vec4 cloud;
	if (cloud_style == 0) {
		cloud = clouds_volume(o, dir, t_end, c, r, FRAGCOORD.xy);
	} else if (cloud_style == 1) {
		cloud = clouds_decks(o, dir, t_end, c, r);
	} else {
		cloud = clouds_flat(o, dir, t_end, c, r);
	}
	vec3 through;
	vec3 glow;
	air(o, dir, t_end, c, r, through, glow);
	vec3 colour;
	if (int(zoom_style.w + 0.5) == 0) {
		// the art book's light: no haze on the ground, only the rim's thin glow seen against space
		vec3 rim = open ? 1.0 - exp(-glow * 1.3) : vec3(0.0);
		colour = scene * cloud.a + cloud.rgb + rim;
	} else {
		colour = (scene * cloud.a + cloud.rgb) * through + (1.0 - exp(-glow));
	}
	ALBEDO = colour;
}
