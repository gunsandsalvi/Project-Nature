// P8's rivers (A8.5): each river's course as a smooth curve through the world's cells, as wide as the land it drains,
// shared by the ground, which draws them, and the trees, which keep out of them. Include planet.gdshaderinc first.

const vec2 STEP[8] = vec2[8](
	vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(-1.0, 1.0), vec2(-1.0, 0.0), vec2(-1.0, -1.0),
	vec2(0.0, -1.0), vec2(1.0, -1.0)
);

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

// The river nearest a place `d` metres from the focus, as a pixel `fp` metres across sees it: x how much of the pixel
// it covers, 0 to 1; y the land it drains, km²; z how far the place lies outside its banks, metres, below 0 in the water.
// Each river cell near the place adds its piece of the course: a quadratic curve from the middle of the step from its
// main donor, round its own middle, to the middle of the step to where its water goes; a tributary also runs on to its
// receiver's curve. A river shows once it drains enough land for the pixel's size, at least about a pixel wide, then at
// its true width. The cells are found from the focus's own cell, so the banks are exact however close the camera.
vec3 river_near(vec2 d, float fp) {
	ivec2 size = textureSize(zoom_water, 0);
	float cm = zoom_world.x / float(size.x);
	vec2 in_cells = (zoom_in_cell + d) / cm;
	vec2 whole = floor(in_cells);
	ivec2 base = zoom_cell + ivec2(whole);
	// metres from the corner of the place's cell, meandering
	vec2 p = (in_cells - whole) * cm;
	p += vec2(ground_noise(d, 700.0), ground_noise(d + 3100.0, 700.0)) * 300.0 - 150.0;
	p += vec2(ground_noise(d, 230.0), ground_noise(d + 1700.0, 230.0)) * 110.0 - 55.0;
	float least_area = 8.0 * fp;
	vec3 best = vec3(0.0, 0.0, 1.0e6);
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			ivec2 c = base + ivec2(dx, dy);
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
			if (covered > best.x || (best.x <= 0.0 && dist - half_width < best.z)) {
				best = vec3(covered, area, dist - half_width);
			}
		}
	}
	return best;
}
