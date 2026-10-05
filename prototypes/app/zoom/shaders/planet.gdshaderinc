// P8 The zoom's planet (IMPLEMENTATION α0.5b, A8): what its ground and its sky share. The world is a torus of 2,000 by
// 1,000 km, shown as a sphere as far around, from the globe down to a person, never unrolled into a flat map (WLD-02):
// its poles squeeze the polar lands in the picture only, and near the focus, as the zoom closes in, the ground is
// stretched back east to west to its true size. Places are given in metres east and north of the focus, so the ground
// near it keeps its precision; the picture's axes are x east, y up and z south, the focus on top of the sphere.

global uniform vec2 zoom_world;     // the world's metres around and from pole to pole
global uniform vec2 zoom_focus;     // the focus, in the world's metres
global uniform vec3 zoom_focus_at;  // where the focus lies in the picture, at sea level
global uniform vec2 zoom_fine;      // the focus's metres modulo 4,096 each way, for fine noise exact near it
global uniform vec2 zoom_true;      // how much the ground at the focus is stretched east to west, less one, and how far
global uniform vec3 zoom_sun;       // toward the sun, in the picture
global uniform vec4 zoom_clock;     // seconds of weather, the year's fraction since spring, the day's fraction, and the
                                    // sun's declination this season, radians
global uniform vec4 zoom_style;     // the land's, the water's, the clouds' and the light's variant: 0 A, 1 B, 2 C
global uniform float zoom_pixel;    // the pixel shown, in the pixels drawn: dithers keep to its grid
global uniform sampler2D zoom_height : filter_linear_mipmap, repeat_enable;  // the cells' heights, metres
global uniform sampler2D zoom_climate : filter_linear_mipmap, repeat_enable;  // mean, coldest, warmest °C, moisture
global uniform sampler2D zoom_cover : source_color, filter_linear_mipmap, repeat_enable;  // map colour, tree share
global uniform sampler2D zoom_water : filter_nearest, repeat_enable;  // river's way, drained km², lake m, main donor
global uniform sampler2D zoom_water_soft : filter_linear_mipmap, repeat_enable;  // the same, smoothly, for lakes
global uniform sampler3D zoom_cloud_noise : filter_linear_mipmap, repeat_enable;  // the clouds' shape and edges

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
// leaves the ground (zoom.gd sets it). The camera's culling in zoom.gd bounds how far this moves the ground from where
// it lies flat: change one, check the other.
vec3 planet_place(vec2 d, float up) {
	float r = planet_radius();
	float lat0 = focus_latitude();
	float dlat = PI * d.y / zoom_world.y;
	float lat = clamp(lat0 + dlat, -0.5 * PI, 0.5 * PI);
	dlat = lat - lat0;
	float k = 1.0 + zoom_true.x * exp(-dot(d, d) / (zoom_true.y * zoom_true.y));
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

// The ladder of lattices the ground's textures use, from 125 km down, each half the last: octave k's cells. Down to 61 m
// they divide the world round, so they wrap without a seam, read from the world's metres; from 32 m down they divide
// 4,096 m, read from the focus's metres modulo that, so they are exact near the focus wherever it is.
float octave_cell(int k) {
	return k <= 11 ? 125000.0 / exp2(float(k)) : 32.0 / exp2(float(k - 12));
}

// A place `d` metres from the focus in octave k's cells, xy, and that lattice's period, z.
vec3 octave_at(vec2 d, int k) {
	float cell = octave_cell(k);
	if (k <= 11) {
		return vec3((zoom_focus + d) / cell, exp2(float(k + 4)));
	}
	return vec3((zoom_fine + d) / cell, 4096.0 / cell);
}

// The finest octave whose cells are at least `metres` across.
int octave_for(float metres) {
	if (metres > 32.0) {
		return clamp(int(floor(log2(125000.0 / metres))), 0, 11);
	}
	return clamp(12 + int(floor(log2(32.0 / metres))), 12, 24);
}

// The nearest of points set by chance one in each cell round p (Worley), in cells, the lattice wrapping every `period`:
// x the distance to it, yz the way from it to p, w a number 0 to 1 of its own.
vec4 worley(vec2 p, float period) {
	vec2 i = floor(p);
	vec2 f = p - i;
	vec4 best = vec4(9.0, 0.0, 0.0, 0.0);
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			vec2 g = vec2(float(x), float(y));
			vec2 c = mod(i + g, period);
			vec2 r = f - g - (vec2(hash12(c), hash12(c + 71.3)) * 0.8 + 0.1);
			float dd = dot(r, r);
			if (dd < best.x) {
				best = vec4(dd, r, hash12(c + 13.7));
			}
		}
	}
	best.x = sqrt(best.x);
	return best;
}

// Round clumps, as a forest's crowns and stands are, at every scale a pixel `fp` metres across can show: domes round
// Worley's points, octave by octave, each weighed by how many pixels its cells span, most at about five, less for
// each octave larger, fading in as it grows past two and a half, so as the zoom moves no octave appears or goes at
// once; none finer than `least` metres. x, y the surface's slope east and north; z how high it stands within its
// clumps, 0 to 1; w the most visible clump's own number, 0 to 1, to tint it.
vec4 clumps(vec2 d, float fp, float least) {
	float smallest = max(2.5 * fp, least);
	int k0 = octave_for(smallest);
	vec2 slope = vec2(0.0);
	float high = 0.0;
	float total = 0.0;
	float own = 0.0;
	for (int o = 0; o < 4; o++) {
		int k = k0 - o;
		if (k < 0) {
			break;
		}
		// the octave's place: 0 as its cells reach two and a half pixels, 1 at five, 2 at ten
		float level = log2(octave_cell(k) / smallest);
		float weight = clamp(level, 0.0, 1.0) * pow(0.5, max(level - 1.0, 0.0)) * clamp(4.0 - level, 0.0, 1.0);
		if (weight <= 0.0) {
			continue;
		}
		vec3 at = octave_at(d, k);
		vec4 cell = worley(at.xy, at.z);
		// a dome k·cell·(1 - r²) high: its slope is -2k·r, the same at every scale
		slope += weight * -1.3 * cell.yz;
		high += weight * (1.0 - min(cell.x * cell.x * 1.4, 1.0));
		total += weight;
		if (level < 1.5 && level >= 0.5) {
			own = cell.w;
		}
	}
	return vec4(slope, high / max(total, 1e-3), own);
}

// The land's slope, east and north, from the cells' heights round a place in the world's metres, over the span a pixel
// `fp` metres across shows, the sea flat.
vec2 relief(vec2 w, float fp) {
	float span = max(1.5 * fp, 600.0);
	float lod = log2(max(span / 977.0, 1.0));
	vec2 e = vec2(span, 0.0);
	float east = max(textureLod(zoom_height, (w + e.xy) / zoom_world, lod).r, 0.0)
			- max(textureLod(zoom_height, (w - e.xy) / zoom_world, lod).r, 0.0);
	float north = max(textureLod(zoom_height, (w + e.yx) / zoom_world, lod).r, 0.0)
			- max(textureLod(zoom_height, (w - e.yx) / zoom_world, lod).r, 0.0);
	return vec2(east, north) / (2.0 * span);
}

// How warm it is in this season, 0 at the coldest month to 1 at the warmest, at a latitude in radians: the year starts
// at the northern spring.
float season_warmth(float lat) {
	return 0.5 + 0.5 * sin(TAU * zoom_clock.y) * sign(lat);
}
