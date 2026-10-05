// P8's ground (A8.1, A8.5), the body its two shaders share: ground.gdshader, the full one; ground_light.gdshader, for
// the zoom's light mode (LIGHT), without the clouds' shadows, the forests' clumps or the sea's currents, and so without
// the weather's picture or any 3D texture. Include planet.gdshaderinc first, and for the full one the clouds' includes.


instance uniform float spacing = 1.0;  // metres between the chunk's points
instance uniform vec2 morph = vec2(1.0e12, 2.0e12);  // metres from the camera where it starts to morph, and is done

varying vec2 v_d;  // metres east and north of the focus
varying float v_up;  // metres above the sea, as drawn
varying vec3 v_normal;  // in the picture
varying vec3 v_up_dir;  // the ground's up here, in the picture
varying float v_slope;  // 0 flat to 1 sheer

#ifndef LIGHT
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
#ifdef CLOUD_NOISE
		od += cloud_density(at, alt, cloud_weather(at), fp);
#else
		od += cloud_far(alt, cloud_weather(at));
#endif
	}
	od *= CLOUD_SIGMA * (top - CLOUD_BASE) / 3.0 / local.y;
	return mix(0.3, 1.0, exp(-od));
}
#endif

const vec2 STEP[8] = vec2[8](
	vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(-1.0, 1.0), vec2(-1.0, 0.0), vec2(-1.0, -1.0),
	vec2(0.0, -1.0), vec2(1.0, -1.0)
);

void vertex() {
	vec3 corner = MODEL_MATRIX[3].xyz;
	vec2 local = vec2(VERTEX.x - corner.x, corner.z - VERTEX.z);
	vec2 d0 = vec2(VERTEX.x - zoom_focus_at.x, zoom_focus_at.z - VERTEX.z);
	vec3 p0 = planet_place(d0, VERTEX.y);
	float m = clamp((length(p0 - CAMERA_POSITION_WORLD) - morph.x) / (morph.y - morph.x), 0.0, 1.0);
	// each odd point slides onto the even point before it, so the chunk becomes the grid half as fine
	vec2 g = floor(local / spacing + 0.5);
	vec2 d = d0 - mod(g, 2.0) * spacing * m;
	float up = mix(VERTEX.y, CUSTOM0.x, m);
	vec3 n = normalize(mix(NORMAL, CUSTOM1.xyz, m));
	vec3 p = planet_place(d, up);
	mat3 frame = planet_frame(p);
	v_normal = normalize(frame[0] * n.x + frame[1] * n.y - frame[2] * n.z);
	v_up_dir = frame[1];
	v_slope = 1.0 - n.y;
	v_d = d;
	v_up = up;
	VERTEX = p;
	NORMAL = v_normal;
}

float dot2(vec2 v) {
	return dot(v, v);
}

// The distance from p to the segment a-b.
float segment_distance(vec2 p, vec2 a, vec2 b) {
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp(dot(pa, ba) / max(dot(ba, ba), 1e-6), 0.0, 1.0);
	return length(pa - ba * h);
}

// The distance from p to the quadratic Bézier curve a, b, c (Íñigo Quílez).
float bezier_distance(vec2 pos, vec2 A, vec2 B, vec2 C) {
	vec2 a = B - A;
	vec2 b = A - 2.0 * B + C;
	if (dot(b, b) < 1e-3) {
		return segment_distance(pos, A, C);
	}
	vec2 c = a * 2.0;
	vec2 d = A - pos;
	float kk = 1.0 / dot(b, b);
	float kx = kk * dot(a, b);
	float ky = kk * (2.0 * dot(a, a) + dot(d, b)) / 3.0;
	float kz = kk * dot(d, a);
	float p = ky - kx * kx;
	float p3 = p * p * p;
	float q = kx * (2.0 * kx * kx - 3.0 * ky) + kz;
	float h = q * q + 4.0 * p3;
	if (h >= 0.0) {
		h = sqrt(h);
		vec2 x = (vec2(h, -h) - q) / 2.0;
		vec2 uv = sign(x) * pow(abs(x), vec2(1.0 / 3.0));
		float t = clamp(uv.x + uv.y - kx, 0.0, 1.0);
		return sqrt(dot2(d + (c + b * t) * t));
	}
	float z = sqrt(-p);
	float v = acos(clamp(q / (p * z * 2.0), -1.0, 1.0)) / 3.0;
	float mm = cos(v);
	float nn = sin(v) * 1.732050808;
	vec3 t = clamp(vec3(mm + mm, -nn - mm, nn - mm) * z - kx, 0.0, 1.0);
	return sqrt(min(dot2(d + (c + b * t.x) * t.x), dot2(d + (c + b * t.y) * t.y)));
}

// A cell's water texel, the cell given as whole cells east and north of the world's corner.
vec4 water_at(ivec2 c, ivec2 size) {
	return texelFetch(zoom_water, ivec2(((c.x % size.x) + size.x) % size.x, ((c.y % size.y) + size.y) % size.y), 0);
}

// How much of this pixel a river covers, 0 to 1, and how much land it drains, km². Each river cell near the pixel
// adds its piece of the course: a quadratic curve from the middle of the step from its main donor, round its own middle,
// to the middle of the step to where its water goes; a tributary also runs on to its receiver's curve. A river shows
// once it drains enough land for the pixel's size, at least about a pixel wide, then at its true width.
vec2 river_at(vec2 w, float fp) {
	ivec2 size = textureSize(zoom_water, 0);
	float cm = zoom_world.x / float(size.x);
	vec2 cw = w / cm;
	vec2 base = floor(cw);
	// metres from the corner of the pixel's cell, meandering
	vec2 p = (cw - base) * cm;
	p += vec2(ground_noise(w - zoom_focus, 700.0), ground_noise(w - zoom_focus + 3100.0, 700.0)) * 300.0 - 150.0;
	p += vec2(ground_noise(w - zoom_focus, 230.0), ground_noise(w - zoom_focus + 1700.0, 230.0)) * 110.0 - 55.0;
	float least_area = 8.0 * fp;
	float cover = 0.0;
	float drained = 0.0;
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			ivec2 c = ivec2(base) + ivec2(dx, dy);
			vec4 here = water_at(c, size);
			int way = int(here.r * 255.0 + 0.5);
			if (way > 7) {
				continue;
			}
			float area = exp2(here.g * 255.0 / 16.0);
			float show = smoothstep(least_area, 2.0 * least_area, area);
			if (show <= 0.0) {
				continue;
			}
			vec2 x = (vec2(float(dx), float(dy)) + 0.5) * cm;
			vec2 to = x + STEP[way] * cm;
			int from_way = int(here.a * 255.0 + 0.5);
			vec2 from = from_way <= 7 ? x + 0.5 * STEP[from_way] * cm : x;
			vec4 next = water_at(c + ivec2(STEP[way]), size);
			// a river running into the sea runs on to the sea cell's middle, past the coast
			bool mouth = next.r * 255.0 > 7.5;
			float dist = bezier_distance(p, from, x, mouth ? to : 0.5 * (x + to));
			// a tributary runs on from the middle of its step to its receiver's curve
			int next_from = int(next.a * 255.0 + 0.5);
			if (!mouth && next_from <= 7 && length(STEP[next_from] + STEP[way]) > 0.1) {
				int next_way = int(next.r * 255.0 + 0.5);
				vec2 next_to = next_way <= 7 ? to + STEP[next_way] * cm : to;
				vec2 join = 0.25 * (to + 0.5 * STEP[next_from] * cm) + 0.5 * to + 0.25 * (0.5 * (to + next_to));
				dist = min(dist, segment_distance(p, 0.5 * (x + to), join));
			}
			float half_width = max(0.5 * (4.0 + 1.2 * sqrt(area)), 0.55 * fp);
			float covered = show * clamp((half_width - dist) / fp + 0.5, 0.0, 1.0);
			if (covered > cover) {
				cover = covered;
				drained = area;
			}
		}
	}
	return vec2(cover, drained);
}

// The sea's currents and eddies, at the scales a pixel `fp` metres across can show: noise warped by noise, drifting
// with the weather's clock. x the pattern, 0 to 1; y its ridges, thin streaks along the flow.
vec2 currents(vec2 d, float fp) {
	vec2 drift = vec2(0.35, 0.12) * zoom_clock.x;
	vec2 q = vec2(0.0);
	for (int o = 0; o < 3; o++) {
		vec3 a = octave_at(d + drift, 1 + o);
		vec3 b = octave_at(d - drift + 50000.0, 1 + o);
		q += vec2(value_noise(a.xy, a.z), value_noise(b.xy, b.z)) * exp2(-float(o));
	}
	q = q / 1.75 - 0.5;
	vec2 at_d = d + 70000.0 * q;
	int finest = octave_for(max(3.0 * fp, 250.0));
	float sum = 0.0;
	float total = 0.0;
	float amp = 1.0;
	for (int k = 2; k <= 10; k++) {
		if (k > finest) {
			break;
		}
		float keep = k == finest ? clamp(octave_cell(k) / max(3.0 * fp, 250.0) - 1.0, 0.0, 1.0) : 1.0;
		vec3 at = octave_at(at_d + drift * 0.5, k);
		sum += amp * keep * value_noise(at.xy, at.z);
		total += amp * keep;
		amp *= 0.6;
	}
	float f = sum / max(total, 1e-3);
	float ridge = 1.0 - abs(2.0 * f - 1.0);
	return vec2(f, pow(ridge, 8.0));
}

// The colour of water `depth` metres deep at a place (variant B and C): deep navy, the open sea's blue, the shelves'
// turquoise and the shallows' clear green over sand; the currents darker and lighter across it.
vec3 water_body(float depth, vec2 d, float fp) {
	vec3 deep = vec3(0.012, 0.04, 0.17);
	vec3 open_sea = vec3(0.02, 0.10, 0.33);
	vec3 shelf = vec3(0.04, 0.30, 0.52);
	vec3 shallow = vec3(0.16, 0.58, 0.62);
	vec3 col = mix(open_sea, deep, smoothstep(700.0, 3500.0, depth));
	col = mix(col, shelf, smoothstep(260.0, 40.0, depth));
	col = mix(col, shallow, smoothstep(30.0, 3.0, depth));
#ifndef LIGHT
	if (depth > 15.0) {
		vec2 flow = currents(d, fp);
		float open = smoothstep(15.0, 120.0, depth);
		col *= mix(1.0, mix(0.7, 1.3, flow.x), open);
		// streaks only where the flow runs fast, in places
		vec3 at = octave_at(d + 21000.0, 3);
		float fast = smoothstep(0.55, 0.8, value_noise(at.xy, at.z));
		col += flow.y * fast * vec3(0.08, 0.17, 0.22) * open;
	}
#endif
	return col;
}

// The colour of water `depth` metres deep, seen along `view` (toward the eye) with the surface's normal `n`, under the
// sun (`sun_c`, `direct`) and the sky (`sky_c`, `ambient`).
vec3 water_colour(float depth, vec3 view, vec3 up_dir, float fp, vec2 d, vec3 sun_c, vec3 sky_c, float direct, float ambient) {
	int style = int(zoom_style.y + 0.5);
	vec3 sun = normalize(zoom_sun);
	if (style == 0) {
		// the art book's map: the shelves light, the deep sea one colour, in bands
		vec3 shelf = vec3(0.333, 0.525, 0.545);
		vec3 deep = vec3(0.235, 0.408, 0.475);
		float band = depth < 40.0 ? 0.0 : (depth < 250.0 ? 0.45 : 1.0);
		return mix(shelf, deep, band) * (sun_c * direct + sky_c * ambient);
	}
	vec3 body = water_body(depth, d, fp);
	// waves: rough from far off, so the sun's glint spreads; fine and moving close up
	float t = zoom_clock.x;
	vec2 wave = vec2(0.0);
	float rough = clamp(fp / 40.0, 0.08, 1.0);
	if (fp < 400.0) {
		wave.x = ground_noise(d + vec2(t * 1.3, t * 0.7), 6.0 + fp * 2.0) - 0.5;
		wave.y = ground_noise(d + vec2(-t * 0.9, t * 1.1) + 17.0, 6.0 + fp * 2.0) - 0.5;
	}
	vec3 east = normalize(cross(up_dir, vec3(0.0, 0.0, 1.0)) + vec3(1e-4, 0.0, 0.0));
	vec3 wn = normalize(up_dir + (east * wave.x + cross(east, up_dir) * wave.y) * 0.35);
	float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(view, wn), 0.0), 5.0);
	vec3 half_way = normalize(view + sun);
	float shine = pow(max(dot(wn, half_way), 0.0), mix(700.0, 60.0, rough)) * mix(2.5, 0.12, rough);
	vec3 colour = body * (sun_c * (0.35 + 0.65 * direct) + sky_c * ambient) + sky_c * fresnel * 0.5
			+ sun_c * shine * step(0.0, dot(up_dir, sun));
	if (style == 2 && fp < 6.0 && depth < 4.0) {
		// waves breaking near the shore, moving in
		float surf = sin(depth * 2.6 - t * 1.7 + 6.0 * ground_noise(d, 9.0));
		colour = mix(colour, vec3(0.92, 0.95, 0.95), smoothstep(0.82, 0.95, surf) * (1.0 - depth / 4.0));
	}
	return colour;
}

// A colour made more vivid: its saturation times `k`.
vec3 vivid(vec3 c, float k) {
	float l = dot(c, vec3(0.3, 0.59, 0.11));
	return clamp(mix(vec3(l), c, k), 0.0, 1.0);
}

void fragment() {
	vec2 d = v_d;
	vec2 w = zoom_focus + d;
	vec2 uv = w / zoom_world;
	float fp = max(length(fwidth(d)), 0.005);
	vec3 n_mesh = normalize(v_normal);
	vec3 up_dir = normalize(v_up_dir);
	vec3 view = normalize((INV_VIEW_MATRIX * vec4(VIEW, 0.0)).xyz);
	vec3 sun = normalize(zoom_sun);
	int land_style = int(zoom_style.x + 0.5);
	mat3 frame = mat3(cross(normalize(vec3(0.0, sin(focus_latitude()), -cos(focus_latitude()))), up_dir), up_dir, vec3(0.0));
	frame[0] = length(frame[0]) > 1e-4 ? normalize(frame[0]) : vec3(1.0, 0.0, 0.0);
	frame[2] = cross(up_dir, frame[0]);

	// the light: the sun, warm, reddening as it sinks, dark behind the planet; the sky's, blue; the clouds' shadows
	float sun_up = dot(up_dir, sun);
	float day = smoothstep(-0.1, 0.06, sun_up);
	vec3 sun_c = mix(vec3(1.0, 0.45, 0.2), vec3(1.0, 0.9, 0.74), smoothstep(0.0, 0.35, sun_up)) * day;
	vec3 sky_c = mix(vec3(0.02, 0.025, 0.06), vec3(0.28, 0.40, 0.72), day);
#ifdef LIGHT
	float shade = 1.0;
#else
	float shade = cloud_shadow(w, max(v_up, 0.0), frame, fp);
#endif

	// the cells' height, smooth, coastlines a little worn so they are not the cells' straight edges
	vec2 worn = d + vec2(ground_noise(d, 600.0), ground_noise(d + 5300.0, 600.0)) * 260.0 - 130.0;
	float h = textureLod(zoom_height, (zoom_focus + worn) / zoom_world, log2(max(fp / 977.0, 1.0))).r;
	// the cells' cover and climate read a little off where the pixel is, by noise, so their edges wander as the land's
	// do rather than follow the cells
	vec2 wander = vec2(ground_noise(d + 900.0, 4000.0), ground_noise(d + 7100.0, 4000.0)) * 2600.0 - 1300.0
			+ vec2(ground_noise(d + 300.0, 1100.0), ground_noise(d + 4100.0, 1100.0)) * 700.0 - 350.0;
	vec2 uv_cell = (w + wander) / zoom_world;
	vec4 clim = texture(zoom_climate, uv_cell);
	float lat = PI * (uv.y - 0.5);
	float warmth = season_warmth(lat);
	float season_temp = mix(clim.y, clim.z, warmth);

	// the poles' ice, round each pole whatever the cells hold there, its edge wandering by noise read round the pole,
	// so nothing of the cells squeezed into it shows; and sea frozen this season, and land frozen all year (WLD-26)
	float lon = TAU * uv.x;
	vec2 round_pole = vec2(cos(lon), sin(lon)) * (0.5 * PI - abs(lat)) * planet_radius() + sign(lat) * 3.0e6;
	float ragged = value_noise(round_pole / 90000.0, 1.0e7) * 0.6 + value_noise(round_pole / 30000.0, 1.0e7) * 0.3
			+ value_noise(round_pole / 10000.0, 1.0e7) * 0.1;
	float ragged_fine = value_noise(round_pole / 4000.0, 1.0e7);
	float cap = radians(73.0) + radians(3.0) * (warmth - 0.5) + radians(12.0) * (ragged - 0.5)
			+ radians(1.5) * (ragged_fine - 0.5);
	float ice = smoothstep(cap, cap + radians(0.3), abs(lat));
	// the ice's own texture, floes and ridges, read round the pole too
	vec4 floes = worley(round_pole / 9000.0, 1.0e7);
	float ice_tone = mix(0.86, 1.04, floes.w) - 0.12 * smoothstep(0.35, 0.05, floes.x)
			+ 0.06 * (value_noise(round_pole / 2500.0, 1.0e7) - 0.5);
	if (h < 0.0) {
		ice = max(ice, smoothstep(-1.0, -3.0, season_temp + 2.0 * (ragged - 0.5)));
	} else {
		ice = max(ice, smoothstep(0.5, -1.5, clim.z));
	}

	// the surface's slope: the land's relief from its heights, deepened from far off; and the clumps of its cover
	float exaggerate = mix(1.0, 5.0, smoothstep(30.0, 1500.0, fp)) * smoothstep(40.0, 200.0, fp);
	vec2 slope = h >= 0.0 ? relief(w, fp) * exaggerate : vec2(0.0);
	vec4 cover = texture(zoom_cover, uv_cell);
	float trees = cover.a;
	float dry = smoothstep(0.35, 0.08, clim.a) * (1.0 - trees);
#ifdef LIGHT
	vec4 clump = vec4(0.0, 0.0, 0.5, 0.5);
#else
	vec4 clump = clumps(d, fp, 1.5);
#endif
	float bumpy = 0.0;
	// the forest's edge: clumps standing where they are high enough for the share under trees, so its edge is ragged
	// with single clumps, not a blur
	float forest = smoothstep(0.0, 0.12, clump.z - (1.0 - trees) * 0.9 + 0.05) * step(0.08, trees);
	if (h >= 0.0 && land_style != 0) {
		// open land's clumps only from a few metres up: closer, grass is drawn by its own tufts, not domes
		float open_land = mix(0.22, 0.12, dry) * smoothstep(0.25, 2.5, fp);
		bumpy = mix(open_land, 1.0, forest) * (1.0 - ice);
		slope += clump.xy * bumpy;
	}
	// the mesh's own slope deepened too, as the zoom leaves the ground, so hills show from above as on a map
	float deepen = mix(1.0, 3.0, smoothstep(5.0, 300.0, fp));
	vec3 n_flat = n_mesh - up_dir * dot(n_mesh, up_dir);
	vec3 n = normalize(up_dir * dot(n_mesh, up_dir) + n_flat * deepen - frame[0] * slope.x - frame[2] * slope.y);
	float direct = max(dot(n, sun), 0.0) * shade;
	float ambient = (0.55 + 0.45 * dot(n, up_dir)) * mix(1.0, mix(0.55, 1.0, clump.z), bumpy);

	vec3 colour;
	if (h < 0.0) {
		colour = water_colour(-h, view, up_dir, fp, d, sun_c, sky_c, max(sun_up, 0.0) * shade, 0.55);
		if (ice > 0.0) {
			vec3 floe = vec3(0.88, 0.93, 0.98) * ice_tone;
			colour = mix(colour, floe * (sun_c * (0.45 + 0.7 * max(sun_up, 0.0) * shade) + sky_c * 0.8), ice);
		}
	} else {
		vec3 base = cover.rgb;
		if (land_style != 0) {
			// the cover's colour, vivid; where trees stand, a forest's green, the open land's own between them
			vec3 open_land = vivid(mix(cover.rgb, vec3(0.62, 0.66, 0.30), trees * 0.6), 1.5);
			vec3 woods = vivid(cover.rgb * vec3(0.82, 1.0, 0.86), 1.7) * mix(0.8, 1.0, trees);
			base = mix(open_land, woods, forest);
			// each clump its own shade, the crowns' tops lighter and yellower, stands lighter and darker
			base *= mix(1.0, mix(0.8, 1.15, clump.w), forest);
			base = mix(base, base * vec3(1.2, 1.24, 0.76), clump.z * forest * 0.5);
			base *= 1.0 + 0.4 * ground_detail(d, 3000.0, 2.0 * fp, 9);
			// the deserts' sand and stone
			base = mix(base, base * vec3(0.78, 0.62, 0.5), dry * smoothstep(0.55, 0.75, clump.z + 0.3 * ground_detail(d, 6000.0, fp, 5)));
		} else {
			base *= 1.0 + 0.3 * ground_detail(d, 8000.0, max(fp, 300.0), 3);
		}
		// rock where it is steep, beaches by the sea
		float steep = max(v_slope, length(slope) * 0.5 * (1.0 - bumpy));
		float rock = smoothstep(0.25, 0.45, steep + 0.08 * ground_detail(d, 400.0, fp, 4));
		base = mix(base, vec3(0.50, 0.44, 0.42), rock);
		float beach = smoothstep(4.0, 1.0, h) * (1.0 - rock);
		base = mix(base, vec3(0.90, 0.80, 0.56), beach * 0.85);
		// snow where this season is cold enough at this height, and ice
		float temp = season_temp - 0.0065 * max(v_up - max(h, 0.0), 0.0);
		float snow = smoothstep(0.5, -2.0, temp + 2.5 * ground_detail(d, 2000.0, fp, 6));
		base = mix(base, vec3(0.93, 0.95, 0.98), max(snow, ice));
		vec3 lit;
		if (land_style == 0 || land_style == 2) {
			// banded light: four steps meeting in clean edges, the sharp look chosen in P1 (A4.1); the shade blue
			float level = clamp(direct * 1.1 + ambient * 0.3, 0.0, 1.3);
			float step_level = floor(level * 4.0 + 0.5) / 4.0;
			lit = base * mix(sky_c * 1.3 + 0.12, sun_c * 1.15, clamp(step_level, 0.0, 1.0)) * (0.5 + 0.5 * step_level);
		} else {
			lit = base * (sun_c * direct * 1.25 + sky_c * ambient * 0.55);
		}
		colour = lit;
		// lakes, their shores worn as the coasts' are
		float lake = textureLod(zoom_water_soft, (zoom_focus + worn) / zoom_world, 0.0).b * 255.0;
		if (lake > 1.0) {
			colour = water_colour(lake, view, up_dir, fp, d, sun_c, sky_c, max(sun_up, 0.0) * shade, 0.55);
		}
		// rivers over land and lakes alike
		vec2 river = river_at(w, fp);
		if (river.x > 0.0) {
			// as deep as the land it drains is wide, so larger rivers show bluer
			float depth = 4.0 + 2.5 * log2(max(river.y, 1.0));
			vec3 flowing = water_colour(depth, view, up_dir, fp, d, sun_c, sky_c, max(sun_up, 0.0) * shade, 0.55);
			colour = mix(colour, flowing, river.x);
		}
	}
	ALBEDO = colour;
}
