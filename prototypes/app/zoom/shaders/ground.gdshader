// P8's ground, from the globe down to a person (A8.1, A8.5): every chunk of every level drawn by this one shader, so
// nothing changes but the detail as the zoom moves. The chunk morphs into the grid half as fine toward its far edge
// (CDLOD), then sits on the sphere. Each pixel's colour comes from the world's cells, smoothly, with detail at every
// scale the pixel can show: cover, forest canopy, rock where it is steep, snow by season and height, beaches; the sea
// and lakes by their depth; rivers as smooth curves through the cells, as wide as the land they drain, and never thinner
// than about a pixel once they show, so they never vanish as the zoom closes in; lit by the sun, under the clouds'
// shadows. Variants: the land A the art book's banded light, B natural, C natural colours in banded light; the water A
// the art book's bands, B deep and shallow with the sun's glint, C B with waves near the shore.
shader_type spatial;
render_mode unshaded, world_vertex_coords, cull_disabled, shadows_disabled, fog_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"

instance uniform float spacing = 1.0;  // metres between the chunk's points
instance uniform vec2 morph = vec2(1.0e12, 2.0e12);  // metres from the camera where it starts to morph, and is done

varying vec2 v_d;  // metres east and north of the focus
varying float v_up;  // metres above the sea, as drawn
varying vec3 v_normal;  // in the picture
varying vec3 v_up_dir;  // the ground's up here, in the picture
varying float v_slope;  // 0 flat to 1 sheer

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

// The colour of water `depth` metres deep, seen along `view` (toward the eye) with the surface's normal `n`.
vec3 water_colour(float depth, vec3 view, vec3 n, vec3 up_dir, float fp, vec2 d, float lit) {
	int style = int(zoom_style.y + 0.5);
	vec3 sun = normalize(zoom_sun);
	if (style == 0) {
		// the art book's map: the shelves light, the deep sea one colour, in bands
		vec3 shelf = vec3(0.333, 0.525, 0.545);
		vec3 deep = vec3(0.235, 0.408, 0.475);
		float band = depth < 40.0 ? 0.0 : (depth < 250.0 ? 0.45 : 1.0);
		return mix(shelf, deep, band) * lit;
	}
	// deep and shallow: light the water takes in, more of red than of blue (Beer and Lambert)
	vec3 shallow = vec3(0.30, 0.66, 0.66);
	vec3 deep = vec3(0.04, 0.16, 0.30);
	vec3 body = mix(shallow, deep, 1.0 - exp(-depth / 55.0));
	// waves: rough from far off, so the sun's glint spreads; fine and moving close up
	float t = zoom_clock.x;
	vec2 wave = vec2(0.0);
	float rough = clamp(fp / 40.0, 0.05, 1.0);
	if (fp < 400.0) {
		wave.x = ground_noise(d + vec2(t * 1.3, t * 0.7), 6.0 + fp * 2.0) - 0.5;
		wave.y = ground_noise(d + vec2(-t * 0.9, t * 1.1) + 17.0, 6.0 + fp * 2.0) - 0.5;
	}
	mat3 frame = mat3(cross(up_dir, vec3(0.0, 0.0, 1.0)), up_dir, vec3(0.0));
	vec3 wn = normalize(up_dir + (frame[0] * wave.x + cross(frame[0], up_dir) * wave.y) * 0.35);
	float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(view, wn), 0.0), 5.0);
	vec3 sky = vec3(0.42, 0.58, 0.82) * smoothstep(-0.1, 0.2, dot(up_dir, sun));
	vec3 half_way = normalize(view + sun);
	float shine = pow(max(dot(wn, half_way), 0.0), mix(900.0, 30.0, rough)) * mix(4.0, 0.35, rough);
	vec3 colour = body * lit + sky * fresnel * 0.6 + vec3(1.0, 0.95, 0.85) * shine * step(0.0, dot(up_dir, sun));
	if (style == 2 && fp < 6.0 && depth < 4.0) {
		// waves breaking near the shore, moving in
		float surf = sin(depth * 2.6 - t * 1.7 + 6.0 * ground_noise(d, 9.0));
		colour = mix(colour, vec3(0.92, 0.95, 0.95), smoothstep(0.82, 0.95, surf) * (1.0 - depth / 4.0));
	}
	return colour;
}

void fragment() {
	vec2 d = v_d;
	vec2 w = zoom_focus + d;
	vec2 uv = w / zoom_world;
	float fp = max(length(fwidth(d)), 0.005);
	vec3 n = normalize(v_normal);
	vec3 up_dir = normalize(v_up_dir);
	vec3 view = normalize((INV_VIEW_MATRIX * vec4(VIEW, 0.0)).xyz);
	vec3 sun = normalize(zoom_sun);
	int land_style = int(zoom_style.x + 0.5);

	// the light: the sun, reddening as it sinks, dark behind the planet; the sky's light; the clouds' shadows
	float sun_up = dot(up_dir, sun);
	float day = smoothstep(-0.1, 0.06, sun_up);
	vec3 sun_colour = mix(vec3(1.0, 0.5, 0.25), vec3(1.0, 0.97, 0.92), smoothstep(0.0, 0.3, sun_up));
	vec3 sky_colour = mix(vec3(0.015, 0.02, 0.05), vec3(0.42, 0.55, 0.78), day);
	mat3 frame = mat3(cross(normalize(vec3(0.0, sin(focus_latitude()), -cos(focus_latitude()))), up_dir), up_dir, vec3(0.0));
	frame[0] = length(frame[0]) > 1e-4 ? normalize(frame[0]) : vec3(1.0, 0.0, 0.0);
	frame[2] = cross(up_dir, frame[0]);
	float shade = cloud_shadow(w, frame);
	float direct = max(dot(n, sun), 0.0) * day * shade;
	float ambient = 0.32 * (0.55 + 0.45 * dot(n, up_dir));

	// the cells' height, smooth, coastlines a little worn so they are not the cells' straight edges
	vec2 worn = d + vec2(ground_noise(d, 600.0), ground_noise(d + 5300.0, 600.0)) * 260.0 - 130.0;
	float h = textureLod(zoom_height, (zoom_focus + worn) / zoom_world, log2(max(fp / 977.0, 1.0))).r;
	vec4 clim = texture(zoom_climate, uv);
	vec3 colour;
	if (h < 0.0) {
		colour = water_colour(-h, view, n, up_dir, fp, d, (direct + ambient) * 0.9);
	} else {
		vec4 cover = texture(zoom_cover, uv);
		vec3 base = cover.rgb;
		if (land_style != 0) {
			// variation at every scale the pixel can show
			base *= 1.0 + 0.55 * ground_detail(d, 3000.0, fp, 9);
			// forest canopy close up: crowns a few metres across with darker gaps between
			float canopy = smoothstep(80.0, 15.0, fp) * cover.a;
			float crowns = smoothstep(0.3, 0.7, ground_noise(d, 5.0)) * 0.6 + smoothstep(0.3, 0.7, ground_noise(d, 2.3)) * 0.4;
			base *= mix(1.0, 0.72 + 0.45 * crowns, canopy);
		} else {
			base *= 1.0 + 0.3 * ground_detail(d, 8000.0, max(fp, 300.0), 3);
		}
		// rock where it is steep, beaches by the sea
		float rock = smoothstep(0.25, 0.45, v_slope + 0.08 * ground_detail(d, 400.0, fp, 4));
		base = mix(base, vec3(0.50, 0.46, 0.47), rock);
		float beach = smoothstep(4.0, 1.0, h) * (1.0 - rock);
		base = mix(base, vec3(0.86, 0.78, 0.6), beach * 0.8);
		// snow where this season is cold enough at this height
		float lat = PI * (uv.y - 0.5);
		float warmth = season_warmth(lat);
		float temp = mix(clim.y, clim.z, warmth) - 0.0065 * max(v_up - max(h, 0.0), 0.0);
		float snow = smoothstep(0.5, -2.0, temp + 2.5 * ground_detail(d, 2000.0, fp, 6));
		base = mix(base, vec3(0.93, 0.95, 0.97), snow);
		// lakes, their shores worn as the coasts' are
		float lake = textureLod(zoom_water_soft, (zoom_focus + worn) / zoom_world, 0.0).b * 255.0;
		vec3 lit;
		if (land_style == 0 || land_style == 2) {
			// banded light: four steps, dithered where they meet, as the art book's hills
			float level = clamp(direct * 1.15 + ambient * 0.55, 0.0, 1.25);
			float step_level = floor(level * 4.0 + bayer4(FRAGCOORD.xy) * 0.9) / 4.0;
			lit = base * sun_colour * (0.35 + 0.8 * step_level);
		} else {
			lit = base * (sun_colour * direct + sky_colour * ambient * 1.3);
		}
		colour = lit;
		if (lake > 1.0) {
			colour = water_colour(lake, view, up_dir, up_dir, fp, d, (direct + ambient) * 0.9);
		}
	}
	// rivers over land and lakes alike
	vec2 river = river_at(w, fp);
	if (river.x > 0.0 && h >= 0.0) {
		vec3 flowing = water_colour(2.0 + log2(river.y), view, up_dir, up_dir, fp, d, (direct + ambient) * 0.9);
		colour = mix(colour, flowing, river.x);
	}
	ALBEDO = colour;
}
