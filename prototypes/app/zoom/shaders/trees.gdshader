// P8's trees (A8.3, PRE-46): every slot of a chunk's grid that holds a tree (forest.gdshaderinc) draws it on one card
// turned to the camera, and in the sun's shadow pass to the sun, on which the tree is worked out pixel by pixel as the
// art book's trees are made (art/book/paint/www/kit/far.js): a trunk and two or three round crowns lit as balls; a
// needle tree's two cones; a flat-crowned tree's flattened balls; a bush's ball. Up close each crown breaks into clumps
// of leaves, its edge ragged with them. Each pixel writes its true depth, so trees stand before and behind each other
// and what stands among them, and their shadows, cast the same way in the sun's pass, have the crowns' shapes. Lit by
// the look's light (look.gdshaderinc) in the leaves', needles' and bark's ramps; autumn turns broad leaves where winters
// freeze. The coarsest chunks that carry trees fade them out one by one as they morph, and the ground's forest takes
// their place (ground.gdshaderinc), so the forest stays where it was as the zoom leaves it.
// The mesh is one card for each slot, four corners each, set out here from the vertex's number alone.
shader_type spatial;
render_mode world_vertex_coords, cull_disabled, specular_disabled, ambient_light_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/rivers.gdshaderinc"
#include "res://zoom/shaders/forest.gdshaderinc"
#include "res://zoom/shaders/chunk.gdshaderinc"
#include "res://look/shaders/look.gdshaderinc"

uniform ivec2 first_slot;  // the slot at the chunk's south-west corner
uniform int slots = 1;     // slots each way
uniform float spacing = 1.0;  // metres between the chunk's points
uniform vec2 morph = vec2(1.0e12, 2.0e12);  // as the ground's: metres from the camera where it starts, and is done
uniform float fade = 0.0;  // 1 where the trees fade out as the chunk morphs
uniform vec4 rows = vec4(16.0, 18.0, 19.0, 20.0);  // the palette's rows: leaves, autumn, needles, bark

varying flat vec4 v_foot;   // the tree's foot in the picture, and its height
varying flat vec4 v_tree;   // its kind, its own number, how far its leaves have turned
varying flat vec3 v_east;
varying flat vec3 v_up;
varying flat vec3 v_north;
varying vec3 v_card;        // this point of the card, in the picture

void vertex() {
	int slot_index = VERTEX_ID / 4;
	int corner_index = VERTEX_ID % 4;
	ivec2 local = ivec2(slot_index % slots, slot_index / slots);
	Tree t = slot_tree(first_slot + local);
	// the chunk's south-west corner, metres east and north of the focus; the tree's place in the chunk's grid
	vec3 corner = MODEL_MATRIX[3].xyz;
	vec2 chunk_d = vec2(corner.x - zoom_focus_at.x, zoom_focus_at.z - corner.z);
	vec2 g = (t.at - chunk_d) / spacing;
	vec2 rough = chunk_ground(g, 0.0);
	float m = clamp((length(planet_place(t.at, rough.x) - zoom_eye) - morph.x) / (morph.y - morph.x), 0.0, 1.0);
	vec2 ground = chunk_ground(g, m);
	bool shown = t.here && ground.y > 0.6 && (fade < 0.5 || t.fade > m);
	vec3 foot = planet_place(t.at, ground.x);
	mat3 frame = planet_frame(foot);
	v_foot = vec4(foot, t.height);
	v_tree = vec4(float(t.kind), t.own, t.autumn, 0.0);
	v_east = frame[0];
	v_up = frame[1];
	v_north = frame[2];
	// the card: round the tree's middle, as wide as its widest, turned to the camera (or the sun)
	float h = t.height;
	vec3 middle = foot + frame[1] * (t.kind == 3 ? 0.5 * h : 0.5 * h);
	float half_size = (t.kind == 1 ? 0.7 : 0.85) * h;
	vec2 c = vec2(corner_index % 2 == 0 ? -1.0 : 1.0, corner_index < 2 ? -1.0 : 1.0);
	vec3 right = normalize(INV_VIEW_MATRIX[0].xyz);
	vec3 up = normalize(INV_VIEW_MATRIX[1].xyz);
	vec3 p = middle + (right * c.x + up * c.y) * half_size;
	VERTEX = shown ? p : foot;
	v_card = VERTEX;
	NORMAL = normalize(INV_VIEW_MATRIX[2].xyz);
}

// The nearest of a ray's meetings with an upright ellipsoid round c of radius r, its height squashed by sy: the ray's
// length there, or -1.
float ball_hit(vec3 o, vec3 d, vec3 c, float r, float sy) {
	vec3 k = vec3(1.0, 1.0 / sy, 1.0);
	vec3 oc = (o - c) * k;
	vec3 dk = d * k;
	float a = dot(dk, dk);
	float b = dot(oc, dk);
	float q = dot(oc, oc) - r * r;
	float disc = b * b - a * q;
	if (disc < 0.0) {
		return -1.0;
	}
	float t = (-b - sqrt(disc)) / a;
	return t > 0.0 ? t : -1.0;
}

// The nearest meeting of a ray with an upright cone, its base's middle at b, radius r, its tip `tall` above: the ray's
// length there, or -1.
float cone_hit(vec3 o, vec3 d, vec3 b, float r, float tall) {
	float k = r / tall;
	vec3 p = o - b;
	float top = tall - p.y;
	float a = d.x * d.x + d.z * d.z - k * k * d.y * d.y;
	float bb = 2.0 * (p.x * d.x + p.z * d.z + k * k * top * d.y);
	float c = p.x * p.x + p.z * p.z - k * k * top * top;
	float disc = bb * bb - 4.0 * a * c;
	if (disc < 0.0 || abs(a) < 1e-6) {
		return -1.0;
	}
	float s = sqrt(disc);
	float t0 = (-bb - s) / (2.0 * a);
	float t1 = (-bb + s) / (2.0 * a);
	for (int i = 0; i < 2; i++) {
		float t = i == 0 ? min(t0, t1) : max(t0, t1);
		float y = p.y + t * d.y;
		if (t > 0.0 && y >= 0.0 && y <= tall) {
			return t;
		}
	}
	return -1.0;
}

// The nearest meeting of a ray with an upright cylinder from b up `tall`, of radius r: the ray's length, or -1.
float trunk_hit(vec3 o, vec3 d, vec3 b, float r, float tall) {
	vec3 p = o - b;
	float a = d.x * d.x + d.z * d.z;
	float bb = p.x * d.x + p.z * d.z;
	float c = p.x * p.x + p.z * p.z - r * r;
	float disc = bb * bb - a * c;
	if (disc < 0.0 || a < 1e-8) {
		return -1.0;
	}
	float t = (-bb - sqrt(disc)) / a;
	float y = p.y + t * d.y;
	return t > 0.0 && y >= 0.0 && y <= tall ? t : -1.0;
}

// A number 0 to 1 for a tree's own number and a part of it.
float part_chance(float own, float part) {
	return hash12(vec2(own * 4096.0, part * 17.0 + 3.0));
}

// The nearest of the points set by chance one in each unit cell round q, and its distance (Worley), in 3D.
vec4 nearest_point(vec3 q) {
	vec3 i = floor(q);
	vec4 best = vec4(0.0, 0.0, 0.0, 9.0);
	for (int z = -1; z <= 1; z++) {
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				vec3 cell = i + vec3(float(x), float(y), float(z));
				uvec3 h = uvec3(ivec3(cell) + 4096);
				uvec2 a = pcg2d(h.xy + h.z * 7919u);
				uvec2 b = pcg2d(a ^ h.zx);
				vec3 at = cell + 0.15 + 0.7 * vec3(vec2(a >> 8u) / 16777216.0, float(b.x >> 8u) / 16777216.0);
				float dist = length(q - at);
				if (dist < best.w) {
					best = vec4(at, dist);
				}
			}
		}
	}
	return best;
}

void fragment() {
	// the ray through this pixel: from the eye, or in the sun's shadow pass from the sun, all one way
	bool parallel = PROJECTION_MATRIX[3][3] > 0.5;
	vec3 eye = INV_VIEW_MATRIX[3].xyz;
	vec3 dir = parallel ? -normalize(INV_VIEW_MATRIX[2].xyz) : normalize(v_card - eye);
	float h = v_foot.w;
	int kind = int(v_tree.x + 0.5);
	float own = v_tree.y;
	// in the tree's own frame: x east, y up, z north, from its foot
	mat3 to_local = transpose(mat3(v_east, v_up, v_north));
	vec3 o = to_local * (v_card - dir * 2.0 * h - v_foot.xyz);
	vec3 d = to_local * dir;
	// how many metres a pixel spans here, for how fine the leaves' clumps may be
	float fp = max(length(fwidth(v_card)), 1e-3);
	float best = 1.0e9;
	vec3 n = vec3(0.0, 1.0, 0.0);
	float crown = 0.0;
	float gap = 0.0;
	// the trunk
	float rb = kind == 1 ? 0.1 + h * 0.01 : 0.14 + h * 0.012;
	if (kind != 3) {
		float tall = kind == 1 ? 0.9 * h : (kind == 2 ? 0.75 * h : 0.5 * h);
		float t = trunk_hit(o, d, vec3(0.0, -0.4, 0.0), rb, tall + 0.4);
		if (t > 0.0) {
			best = t;
			vec3 p = o + d * t;
			n = normalize(vec3(p.x, 0.0, p.z));
		}
	}
	if (kind == 1) {
		// a needle tree: two cones, the lower wider
		for (int i = 0; i < 2; i++) {
			vec3 b = vec3(0.0, i == 0 ? 0.18 * h : 0.48 * h, 0.0);
			float r = (i == 0 ? 0.26 : 0.19) * h * mix(0.9, 1.1, part_chance(own, float(i)));
			float tall = (i == 0 ? 0.5 : 0.52) * h;
			float t = cone_hit(o, d, b, r, tall);
			if (t > 0.0 && t < best) {
				best = t;
				vec3 p = o + d * t - b;
				n = normalize(vec3(p.x, (r / tall) * length(p.xz), p.z));
				crown = 1.0;
			}
		}
	} else {
		// broad leaves: two or three balls; flat crowns: three flattened ones up high; a bush: one or two low
		int balls = kind == 0 ? 2 + int(part_chance(own, 9.0) * 1.99) : (kind == 2 ? 3 : 1 + int(part_chance(own, 9.0) * 1.99));
		for (int i = 0; i < 3; i++) {
			if (i >= balls) {
				break;
			}
			float a = part_chance(own, float(i) * 3.0);
			float b2 = part_chance(own, float(i) * 3.0 + 1.0);
			float c2 = part_chance(own, float(i) * 3.0 + 2.0);
			vec3 c;
			float r;
			float sy;
			if (kind == 0) {
				r = h * mix(0.26, 0.34, a);
				c = vec3(mix(-0.18, 0.18, b2) * h, h * mix(0.52, 0.64, c2), mix(-0.18, 0.18, a) * h);
				sy = 0.86;
			} else if (kind == 2) {
				float turn = TAU * (float(i) / 3.0 + 0.3 * a);
				r = h * 0.34;
				c = vec3(cos(turn) * h * mix(0.2, 0.34, b2), h * mix(0.82, 0.94, c2), sin(turn) * h * mix(0.2, 0.34, b2));
				sy = 0.36;
			} else {
				r = h * mix(0.4, 0.5, a);
				c = vec3(mix(-0.2, 0.2, b2) * h, r * 0.85, mix(-0.2, 0.2, c2) * h);
				sy = 0.8;
			}
			// up close, the crown breaks into clumps of leaves: the ball a little larger, met only where a clump
			// reaches out so far, its edge ragged; within it, each clump its own little dome
			float clump = clamp(r * 0.36, 0.4, 1.2);
			float detail = smoothstep(2.0, 4.0, clump / fp);
			float t = -1.0;
			vec4 near_point = vec4(0.0, 0.0, 0.0, 0.5);
			if (detail > 0.0) {
				float t_out = ball_hit(o, d, c, r * 1.12, sy);
				if (t_out > 0.0 && t_out < best) {
					vec4 np = nearest_point((o + d * t_out - c) / clump);
					if (np.w < mix(0.0, 0.42, detail)) {
						t = t_out;
						near_point = np;
					}
				}
			}
			if (t < 0.0) {
				t = ball_hit(o, d, c, r, sy);
				if (t > 0.0 && t < best && detail > 0.0) {
					near_point = nearest_point((o + d * t - c) / clump);
				}
			}
			if (t > 0.0 && t < best) {
				best = t;
				vec3 p = o + d * t - c;
				vec3 ball_n = normalize(p * vec3(1.0, 1.0 / (sy * sy), 1.0));
				vec3 dome = normalize(p / clump - near_point.xyz + ball_n * 0.4);
				n = normalize(mix(ball_n, dome, 0.45 * detail));
				crown = 1.0;
				gap = detail * smoothstep(0.42, 0.62, near_point.w);
			}
		}
	}
	if (best > 1.0e8) {
		discard;
	}
	// the meeting point and its facing, back in the picture
	mat3 from_local = mat3(v_east, v_up, v_north);
	vec3 hit = v_foot.xyz + from_local * (o + d * best);
	vec3 wn = normalize(from_local * n);
	vec4 clip = PROJECTION_MATRIX * VIEW_MATRIX * vec4(hit, 1.0);
	DEPTH = clip.z / clip.w;
	vec3 view_hit = (VIEW_MATRIX * vec4(hit, 1.0)).xyz;
	LIGHT_VERTEX = view_hit;
	// the look's material: leaves (turned in autumn, tree by tree), needles or bark; some trees a step lighter or
	// darker, bushes darker; the gaps between clumps a step deeper
	bool turned = v_tree.z > own;
	row = crown > 0.5 ? (kind == 1 ? rows.z : (turned ? rows.y : rows.x)) : rows.w;
	bias = crown > 0.5 ? (own < 0.18 ? -1.0 : (own > 0.84 ? 1.0 : 0.0)) - (kind == 3 ? 1.0 : 0.0) - floor(gap + 0.5) : 0.0;
	flags = crown > 0.5 ? FLAG_FOLIAGE : 0.0;
	edge = 0.0;
	SKY_AND_FIRE(hit, wn)
	NORMAL = (VIEW_MATRIX * vec4(wn, 0.0)).xyz;
	ALBEDO = vec3(1.0);
	ROUGHNESS = 1.0;
}

void light() {
	LOOK_LIGHT
}
