// P8 The zoom's planet (IMPLEMENTATION α0.5b, A8): what its ground and its sky share. The world is a torus of 2,000 by
// 1,000 km, shown as a sphere as far around, from the globe down to a person, never unrolled into a flat map (WLD-02):
// its poles squeeze the polar lands in the picture only, and near the focus, as the zoom closes in, the ground is
// stretched back east to west to its true size. Places are given in metres east and north of the focus, so the ground
// near it keeps its precision; the picture's axes are x east, y up and z south, the focus on top of the sphere.

global uniform vec2 zoom_world;     // the world's metres around and from pole to pole
global uniform vec2 zoom_focus;     // the focus, in the world's metres
global uniform vec3 zoom_focus_at;  // where the focus lies in the picture, at sea level
global uniform vec2 zoom_fine;      // the focus's metres modulo 4,096 each way, for fine noise exact near it
global uniform vec2 zoom_true;      // how much the ground near the focus is shown at its true scale, 0 to 1, and how far
global uniform vec3 zoom_sun;       // toward the sun, in the picture
global uniform vec4 zoom_clock;     // seconds, the year's fraction since spring, the day's fraction, and its time scale
global uniform vec4 zoom_style;     // the land's, the water's, the clouds' and the light's variant: 0 A, 1 B, 2 C
global uniform float zoom_pixel;    // the pixel shown, in the pixels drawn: dithers keep to its grid
global uniform sampler2D zoom_height : filter_linear_mipmap, repeat_enable;  // the cells' heights, metres
global uniform sampler2D zoom_climate : filter_linear_mipmap, repeat_enable;  // mean, coldest, warmest °C, moisture
global uniform sampler2D zoom_cover : source_color, filter_linear_mipmap, repeat_enable;  // map colour, tree share
global uniform sampler2D zoom_water : filter_nearest, repeat_enable;  // river's way, drained km², lake m, main donor
global uniform sampler2D zoom_water_soft : filter_linear_mipmap, repeat_enable;  // the same, smoothly, for lakes
global uniform sampler3D zoom_cloud_noise : filter_linear_mipmap, repeat_enable;  // the clouds' shape and edges

const float CLOUD_BASE = 1500.0;  // metres above the sea: cumulus bases and the stratus deck (WLD-30: real heights)
const float CLOUD_TOP = 7000.0;

float planet_radius() {
	return zoom_world.x / TAU;
}

vec3 planet_middle() {
	return zoom_focus_at - vec3(0.0, planet_radius(), 0.0);
}

float focus_latitude() {
	return PI * (zoom_focus.y / zoom_world.y - 0.5);
}

// A place `d` metres east and north of the focus, `up` metres above the sea, in the picture. The haversine forms keep
// the ground near the focus exact; the east-west stretch near the focus fades out with distance and as the zoom
// leaves the ground.
vec3 planet_place(vec2 d, float up) {
	float r = planet_radius();
	float lat0 = focus_latitude();
	float dlat = PI * d.y / zoom_world.y;
	float lat = clamp(lat0 + dlat, -0.5 * PI, 0.5 * PI);
	dlat = lat - lat0;
	float stretch = 1.0 / max(cos(lat0), 0.33) - 1.0;
	float k = 1.0 + zoom_true.x * stretch * exp(-dot(d, d) / (zoom_true.y * zoom_true.y));
	float dlon = TAU * d.x / zoom_world.x * k;
	float s_lat = sin(0.5 * dlat);
	float s_lon = sin(0.5 * dlon);
	float hav = s_lat * s_lat + cos(lat) * cos(lat0) * s_lon * s_lon;
	float east = cos(lat) * sin(dlon);
	float north = sin(dlat) + 2.0 * sin(lat0) * cos(lat) * s_lon * s_lon;
	float big = r + up;
	return zoom_focus_at + vec3(east * big, up - 2.0 * hav * big, -north * big);
}

// The ground's frame at a place in the picture: east, up and north.
mat3 planet_frame(vec3 p) {
	vec3 up = normalize(p - planet_middle());
	float lat0 = focus_latitude();
	vec3 axis = vec3(0.0, sin(lat0), -cos(lat0));
	vec3 east = cross(axis, up);
	float len = length(east);
	east = len > 1e-4 ? east / len : vec3(1.0, 0.0, 0.0);
	return mat3(east, up, cross(up, east));
}

// A 4 × 4 ordered dither's threshold (Bayer's), 0 to 1, on the grid of the pixels shown.
float bayer4(vec2 frag) {
	ivec2 q = ivec2(floor(frag / max(zoom_pixel, 1.0)));
	int x = q.x & 3;
	int y = q.y & 3;
	int fine = 2 * ((x & 1) ^ (y & 1)) + (y & 1);
	int coarse = 2 * (((x >> 1) & 1) ^ ((y >> 1) & 1)) + ((y >> 1) & 1);
	return (float(4 * fine + coarse) + 0.5) / 16.0;
}

// Dave Hoskins's hash without sine, 0 to 1.
float hash12(vec2 p) {
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

// Smooth value noise, 0 to 1, on a lattice that wraps every `period` cells.
float value_noise(vec2 p, float period) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	vec2 i0 = mod(i, period);
	vec2 i1 = mod(i + 1.0, period);
	float a = hash12(i0);
	float b = hash12(vec2(i1.x, i0.y));
	float c = hash12(vec2(i0.x, i1.y));
	float e = hash12(i1);
	return mix(mix(a, b, u.x), mix(c, e, u.x), u.y);
}

// Noise on a lattice `cell` metres across at `d` metres from the focus: exact near the focus for cells up to 4 km,
// from the world's own metres above, where single precision is ample.
float ground_noise(vec2 d, float cell) {
	if (cell <= 4096.0) {
		return value_noise((zoom_fine + d) / cell, 4096.0 / cell);
	}
	return value_noise((zoom_focus + d) / cell, 1.0e7);
}

// Fractal noise about -0.5 to 0.5 from lattices `big` metres across, each octave half the last, those finer than about
// two pixels of `fp` metres fading out, so the ground never sparkles.
float ground_detail(vec2 d, float big, float fp, int octaves) {
	float sum = 0.0;
	float amp = 0.5;
	float cell = big;
	for (int o = 0; o < octaves; o++) {
		float keep = clamp(cell / (2.0 * fp) - 1.0, 0.0, 1.0);
		if (keep <= 0.0) {
			break;
		}
		sum += amp * keep * (ground_noise(d, cell) - 0.5);
		amp *= 0.55;
		cell *= 0.5;
	}
	return sum;
}

// How warm it is in this season, 0 at the coldest month to 1 at the warmest, at a latitude in radians: the year starts
// at the northern spring.
float season_warmth(float lat) {
	return 0.5 + 0.5 * sin(TAU * zoom_clock.y) * sign(lat);
}

// The clouds' cover at a place in the world's metres, 0 to 1: from the climate, more where it is moist and in the
// rising belts by the equator and along the storm tracks, less under the dry belts of the subtropics; with weather
// systems a few hundred kilometres across, carried by the trade winds and the westerlies (A8.6, WLD-16).
float cloud_cover(vec2 w) {
	vec2 uv = w / zoom_world;
	float lat = PI * (uv.y - 0.5);
	float deg = abs(lat) * 57.29578;
	vec4 clim = textureLod(zoom_climate, uv, 2.0);
	float cover = 0.18 + 0.4 * clamp(clim.a, 0.0, 1.2);
	cover += 0.22 * exp(-pow(deg / 9.0, 2.0)) - 0.2 * exp(-pow((deg - 27.0) / 8.0, 2.0))
			+ 0.18 * exp(-pow((deg - 55.0) / 11.0, 2.0));
	// the winds: from the east in the tropics and by the poles, from the west between, in metres a second
	float wind = deg < 30.0 ? -8.0 : (deg < 62.0 ? 14.0 : -5.0);
	float t = zoom_clock.x * zoom_clock.w;
	vec2 drift = vec2(wind * t, 0.0);
	float systems = value_noise((w - drift) / 160000.0, 1.0e7) * 0.6 + value_noise((w - drift) / 60000.0, 1.0e7) * 0.4;
	return clamp(cover + 0.75 * (systems - 0.5), 0.0, 1.0);
}

// The clouds' density at a place in the world's metres and a height above the sea, 0 to 1, for the cover there: the
// shape of Perlin noise eroded by Worley's cells (Schneider 2015), flat-bottomed, rising higher where the cover is
// thick, its edges worn by finer cells. The ground's shadows read the same clouds.
float cloud_density(vec2 w, float alt, float cover) {
	float h = (alt - CLOUD_BASE) / (CLOUD_TOP - CLOUD_BASE);
	if (h <= 0.0 || h >= 1.0 || cover <= 0.01) {
		return 0.0;
	}
	float top = mix(0.2, 1.0, cover * cover);
	float profile = smoothstep(0.0, 0.07, h) * (1.0 - smoothstep(top * 0.55, top, h));
	vec3 p = vec3(w / 11000.0, alt / 11000.0);
	float shape = textureLod(zoom_cloud_noise, p, 0.0).r;
	float base = clamp((shape * profile - (1.0 - cover)) / max(cover, 0.05), 0.0, 1.0);
	if (base <= 0.0) {
		return 0.0;
	}
	float edge = textureLod(zoom_cloud_noise, p * 5.3, 0.0).g * 0.65 + textureLod(zoom_cloud_noise, p * 13.1, 0.0).b * 0.35;
	return clamp(base - 0.45 * edge * (1.0 - base), 0.0, 1.0);
}

// The flat clouds of the art book's map (variant C): one deck at 3 km, hard-edged.
float cloud_puff(vec2 w, float cover) {
	float shape = textureLod(zoom_cloud_noise, vec3(w / 16000.0, 0.31), 0.0).r * 0.7
			+ textureLod(zoom_cloud_noise, vec3(w / 5000.0, 0.62), 0.0).g * 0.3;
	return step(1.0 - cover * 0.85, shape);
}

// How much of the sun reaches a place on the ground through the clouds of the variant shown, 0 to 1.
float cloud_shadow(vec2 w, mat3 frame) {
	vec3 sun = normalize(zoom_sun);
	vec3 local = vec3(dot(sun, frame[0]), dot(sun, frame[1]), dot(sun, frame[2]));
	if (local.y <= 0.03) {
		return 1.0;
	}
	float mid = 0.5 * (CLOUD_BASE + CLOUD_TOP) * 0.6;
	vec2 at = w + local.xz / local.y * mid;
	float cover = cloud_cover(at);
	int clouds = int(zoom_style.z + 0.5);
	if (clouds == 2) {
		return 1.0 - 0.55 * cloud_puff(at, cover);
	}
	float dense = cloud_density(at, mid, cover) + cloud_density(at, mid * 1.5, cover);
	return 1.0 - 0.6 * smoothstep(0.0, 0.5, dense);
}
