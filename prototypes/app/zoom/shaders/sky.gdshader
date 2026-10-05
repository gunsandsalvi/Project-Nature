// P8's sky (A8.4, A8.6), a pass over the whole picture after the ground: along each pixel's ray, the clouds and the
// air in front of the ground or, past the planet's edge, of space and its stars. The air scatters sunlight as Earth's
// does, by Rayleigh's and Mie's laws (single scattering, after Nishita 1993 and Hillaire 2020): blue overhead, a glow
// along the planet's rim, haze over distant ground, the sun reddening as it sinks. The clouds, from the climate's
// weather (weather.gdshader, planet.gdshaderinc): A raymarched through their 3D shapes and lit by the sun through
// themselves (Schneider 2015), so the camera passes through them as it descends; B the same, hard-edged and lit in the
// art book's bands; C the art book's flat clouds, one deck.
// Light: A the art book's, the air only a thin glow at the rim; B the air's full scattering.
shader_type spatial;
render_mode unshaded, depth_draw_never, depth_test_disabled, cull_disabled, shadows_disabled, fog_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/clouds.gdshaderinc"

uniform sampler2D screen : hint_screen_texture, filter_nearest;
uniform sampler2D depth : hint_depth_texture, filter_nearest;

// The air, scaled to this small planet: thinner than Earth's by its scale heights, denser to match, so the sky over
// the ground is Earth's blue and the rim seen from space is as thin against the globe as Earth's.
const vec3 BETA_R = vec3(5.8e-6, 13.5e-6, 33.1e-6) * 4.0;
const float BETA_M = 21e-6 * 3.0;
const float H_R = 2000.0;
const float H_M = 500.0;
const float AIR_TOP = 16000.0;
// The sun's light: the ground's lights a white surface facing it to 1, so the air's, which divides by no π, is π.
const float SUN_POWER = PI;

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
	d.x /= 1.0 + zoom_true.x * exp(-dot(d, d) / (zoom_true.y * zoom_true.y));
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

// The clouds along a ray up to t_end (A8.6): their light, and what they let through. Marched through the shell between
// their bases and the highest tops, each step's sunlight from four steps toward the sun; light scattered many times
// approximated by octaves of ever weaker extinction and ever rounder scattering (Wrenninge 2013); the sky's light from
// above, brighter toward the tops. `px` is the pixel's angle, radians, so steps far off read the clouds' broad shapes
// only.
vec4 clouds_volume(vec3 o, vec3 dir, float t_end, vec3 c, float r, vec2 frag, float px) {
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
	// the sun's way in the ground's frame where the ray meets the clouds, so steps toward it stay in the world's metres
	mat3 frame = planet_frame(o + dir * t0);
	vec3 sun_local = vec3(dot(sun, frame[0]), dot(sun, frame[1]), dot(sun, frame[2]));
	vec3 sunlight = sunlight_at(o + dir * t0, sun, c, r) * smoothstep(-0.12, 0.1, sun_local.y);
	vec3 sky = mix(vec3(0.01, 0.012, 0.025), vec3(0.42, 0.53, 0.72), smoothstep(-0.15, 0.25, sun_local.y));
	// first long steps, without light, to find where the ray meets cloud; from a step before it, short steps, lit,
	// so the clouds' tops are found to a short step and no pixel's brightness depends on where its steps fell
	const int FIND = 24;
	const int LIGHT_STEPS = 36;
	float ds = (t1 - t0) / float(FIND);
	float jitter = bayer4(frag);
	float t = t0;
	bool found = false;
	for (int i = 0; i < FIND; i++) {
		float tf = t0 + (float(i) + 0.5) * ds;
		vec3 at = world_of(o + dir * tf);
		vec2 w = zoom_focus + at.xy;
		vec4 weather = cloud_weather(w);
		if (weather.x > 0.02 && cloud_density(w, at.z, weather, tf * px) > 0.0) {
			t = max(tf - ds, t0);
			found = true;
			break;
		}
	}
	if (!found) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	float fine_ds = ds / 3.0;
	float through = 1.0;
	vec3 light = vec3(0.0);
	t += jitter * 0.25 * fine_ds;
	// the heaps' tops, kilometres across, lit on their sun side and shaded on the other, from the slope of the clouds'
	// tops round where the ray meets them: the light's steps toward the sun reach too short a way to find it
	vec3 top_at = world_of(o + dir * (t + ds));
	vec2 top_w = zoom_focus + top_at.xy;
	float span = max(2500.0, 3.0 * (t + ds) * px);
	float te = cloud_top(cloud_weather(top_w + vec2(span, 0.0)).y) - cloud_top(cloud_weather(top_w - vec2(span, 0.0)).y);
	float tn = cloud_top(cloud_weather(top_w + vec2(0.0, span)).y) - cloud_top(cloud_weather(top_w - vec2(0.0, span)).y);
	vec3 top_normal = normalize(vec3(-te, 2.0 * span * 0.2, -tn));
	float top_lit = mix(0.4, 1.2, clamp(dot(top_normal, sun_local) * 1.3, 0.0, 1.0));
	for (int i = 0; i < LIGHT_STEPS; i++) {
		if (t > t1) {
			break;
		}
		vec3 at = world_of(o + dir * t);
		vec2 w = zoom_focus + at.xy;
		vec4 weather = cloud_weather(w);
		float fp = t * px;
		float dense = weather.x > 0.02 ? cloud_density(w, at.z, weather, fp) : 0.0;
		float step_len = dense > 0.0 ? fine_ds : ds;
		if (dense > 0.0) {
			float od = 0.0;
			float reach = 120.0;
			float gone = 0.0;
			for (int k = 0; k < 4; k++) {
				float mid = gone + 0.5 * reach;
				vec2 qw = w + sun_local.xz * mid;
				od += cloud_density(qw, at.z + sun_local.y * mid, weather, max(fp, reach)) * reach;
				gone += reach;
				reach *= 2.5;
			}
			od *= CLOUD_SIGMA;
			vec3 sun_in = vec3(0.0);
			float a_k = 1.0;
			float b_k = 1.0;
			float g_k = 1.0;
			for (int n = 0; n < 3; n++) {
				float phase = mix(henyey_greenstein(mu, 0.6 * g_k), henyey_greenstein(mu, -0.25 * g_k), 0.3);
				sun_in += vec3(a_k * exp(-od * b_k) * phase);
				a_k *= 0.55;
				b_k *= 0.4;
				g_k *= 0.5;
			}
			// the octaves' weights sum to 1.85: a top in full sun, scattering evenly, is as bright as white ground
			// (the octaves leave out much of the light thick cloud sends back toward the sun: a third more)
			sun_in *= sunlight * 4.0 * PI / 1.85 * 1.45 * top_lit;
			float h01 = clamp((at.z - CLOUD_BASE) / (cloud_top(weather.y) - CLOUD_BASE), 0.0, 1.0);
			vec3 sky_in = sky * mix(0.25, 0.5, h01);
			float a = 1.0 - exp(-dense * CLOUD_SIGMA * step_len);
			light += through * a * (sun_in + sky_in);
			through *= 1.0 - a;
			if (through < 0.02) {
				break;
			}
		}
		t += step_len;
	}
	return vec4(light, through);
}

// The same clouds in the art book's way (variant B): hard-edged where they are half opaque, lit in four bands of its
// palette meeting in clean edges.
vec4 clouds_banded(vec4 cloud) {
	float alpha = 1.0 - cloud.a;
	if (alpha < 0.45) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	vec3 own = cloud.rgb / max(alpha, 1e-3);
	float lum = dot(own, vec3(0.3, 0.59, 0.11));
	float tint_most = max(max(own.r, own.g), max(own.b, 1e-3));
	vec3 tint = own / tint_most;
	float band = clamp(floor(lum * 3.2 + 0.45), 0.0, 3.0);
	vec3 palette[4] = vec3[4](vec3(0.40, 0.46, 0.60), vec3(0.62, 0.69, 0.80), vec3(0.84, 0.88, 0.93), vec3(0.99, 0.99, 0.97));
	return vec4(palette[int(band)] * mix(vec3(1.0), tint, 0.6) * min(tint_most * 1.1, 1.0), 0.0);
}

// The art book's flat clouds (variant C): one deck, hard-edged, in three tones, its sun side white.
vec4 clouds_flat(vec3 o, vec3 dir, float t_end, vec3 c, float r, float px) {
	vec3 sun = normalize(zoom_sun);
	float alt = length(o - c) - r;
	vec2 hit = sphere_hits(o, dir, c, r + CLOUD_DECK);
	float t = alt > CLOUD_DECK ? hit.x : hit.y;
	if (t <= 0.0 || t >= t_end) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	vec3 p = o + dir * t;
	vec2 w = zoom_focus + world_of(p).xy;
	float fp = t * px;
	if (cloud_puff(w, cloud_weather(w), fp) < 0.5) {
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	mat3 frame = planet_frame(p);
	vec2 toward = normalize(vec2(dot(sun, frame[0]), dot(sun, frame[2])) + 1e-4);
	float edge = max(fp * 2.0, 600.0);
	vec2 sun_side = w + toward * edge;
	vec2 shade_side = w - toward * edge;
	vec3 tone = vec3(0.90, 0.93, 0.95);
	if (cloud_puff(sun_side, cloud_weather(sun_side), fp) < 0.5) {
		tone = vec3(1.0, 1.0, 0.98);
	} else if (cloud_puff(shade_side, cloud_weather(shade_side), fp) < 0.5) {
		tone = vec3(0.62, 0.68, 0.80);
	}
	float day = smoothstep(-0.1, 0.15, dot(normalize(p - c), sun));
	return vec4(tone * sunlight_at(p, sun, c, r) * mix(0.06, 1.0, day), 0.0);
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
	// the pixel's angle, radians
	float px = length(fwidth(dir));
	vec4 cloud;
	if (cloud_style == 2) {
		cloud = clouds_flat(o, dir, t_end, c, r, px);
	} else {
		cloud = clouds_volume(o, dir, t_end, c, r, FRAGCOORD.xy, px);
		if (cloud_style == 1) {
			cloud = clouds_banded(cloud);
		}
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
