// P8 The zoom's shared placing (IMPLEMENTATION α0.5b, A8.1, A8.2, A8.4): a place on the map, in metres east and north
// of the focus, to the picture's metres round it, the focus sitting at the moving origin's offset; and, as the zoom
// passes the world map, bent onto the globe, the focus on top (WLD-02). The world wraps east to west: whoever places a
// mesh takes its places on one side of that seam, whole triangles and segments together, never vertex by vertex. And
// the dither by which one level gives way to the next.

uniform vec2 world_m;   // the world's metres around and from pole to pole
uniform vec2 focus_m;   // the focus, in the world's metres
uniform vec3 focus_at;  // where the focus lies in the picture, at sea level, from the moving origin
uniform float bend;     // 0 the flat map, 1 the globe

// A place `d` metres east and north of the focus, `up` metres above the sea, in the picture: flat round the focus, or
// bent onto a sphere as far around as the world, its poles squeezing the polar lands (PI and TAU are the shading
// language's own).
vec3 placed(vec2 d, float up) {
	vec3 flat_place = vec3(d.x, up, -d.y);
	if (bend <= 0.0) {
		return focus_at + flat_place;
	}
	float r = world_m.x / TAU;
	float lon = TAU * d.x / world_m.x;
	float lat = clamp(PI * ((focus_m.y + d.y) / world_m.y - 0.5), -0.5 * PI, 0.5 * PI);
	float lat0 = PI * (focus_m.y / world_m.y - 0.5);
	vec3 p = vec3(cos(lat) * sin(lon), sin(lat), cos(lat) * cos(lon));
	// turned about the east-west axis so the focus's latitude is on top, north toward the picture's north
	float a = lat0 - 0.5 * PI;
	vec3 q = vec3(p.x, p.y * cos(a) - p.z * sin(a), p.y * sin(a) + p.z * cos(a));
	vec3 sphere = q * (r + up) - vec3(0.0, r, 0.0);
	return focus_at + mix(flat_place, sphere, bend);
}

// How far to move a place east, a whole world's width or none, to take it on the side of the seam nearer the focus.
float seam_shift(float east) {
	float d = east - focus_m.x;
	return -world_m.x * floor(d / world_m.x + 0.5);
}

// Whether a level draws its pixel at FRAGCOORD `frag`, `d` metres from the focus: a finer level draws within `inner`,
// this one out to `outer`, each giving way to the next over the last sixth of its radius by a 4 × 4 ordered dither
// (Bayer's). The two share that ring's pixels, so none is left empty or drawn twice, and the pixel look never blurs.
bool shown(vec2 frag, float d, float inner, float outer) {
	int x = int(mod(frag.x, 4.0));
	int y = int(mod(frag.y, 4.0));
	int fine = 2 * ((x & 1) ^ (y & 1)) + (y & 1);
	int coarse = 2 * (((x >> 1) & 1) ^ ((y >> 1) & 1)) + ((y >> 1) & 1);
	float t = (float(4 * fine + coarse) + 0.5) / 16.0;
	float finer = inner > 0.0 ? clamp((inner - d) / (0.15 * inner), 0.0, 1.0) : 0.0;
	float own = outer > 0.0 ? clamp((outer - d) / (0.15 * outer), 0.0, 1.0) : 0.0;
	return t >= finer && t < own;
}
