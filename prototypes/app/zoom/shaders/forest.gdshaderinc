// P8's forests (A8.3, A9): where trees stand, one rule for the trees drawn near the camera (trees.gdshader) and the
// forests the ground draws farther off (ground.gdshaderinc), so a forest stands in the same place at every zoom and
// the one hands over to the other without a jump. Trees stand on a grid of slots TREE_CELL metres apart, at most one
// to a slot, by keyed chance (WLD-13): dense in the forests' stands, a few trees and bushes in the open land between.
// The stands are where the cover's share under trees, shaped by noise, puts them. Include planet.gdshaderinc and
// rivers.gdshaderinc first.

const float TREE_CELL = 500000.0 / 65536.0;  // 7.63 m: 128 to a world cell, a whole number in each chunk up to 488 m
const int SLOTS_PER_CELL = 128;

// Jarzynski and Olano's hash (2020): two well mixed whole numbers from two.
uvec2 pcg2d(uvec2 v) {
	v = v * 1664525u + 1013904223u;
	v.x += v.y * 1664525u;
	v.y += v.x * 1664525u;
	v = v ^ (v >> 16u);
	v.x += v.y * 1664525u;
	v.y += v.x * 1664525u;
	v = v ^ (v >> 16u);
	return v;
}

// Two numbers 0 to 1 of a slot's own, on a grid of `per_cell` slots to a world cell's side, a different pair for each
// `salt`; the grid wraps round the world east to west, as the world does.
vec2 grid_chance(ivec2 slot, int per_cell, uint salt) {
	int around = textureSize(zoom_cover, 0).x * per_cell;
	ivec2 s = ivec2(((slot.x % around) + around) % around, slot.y);
	uvec2 h = pcg2d(uvec2(s) + uvec2(salt * 2654435761u, salt * 2246822519u));
	return vec2(h >> 8u) / 16777216.0;
}

// A slot's south-west corner on such a grid, its slots `cell` metres apart, in metres from the focus, the shorter way
// round the world.
vec2 grid_corner(ivec2 slot, int per_cell, float cell) {
	ivec2 rel = slot - zoom_cell * per_cell;
	int around = textureSize(zoom_cover, 0).x * per_cell;
	rel.x -= around * int(round(float(rel.x) / float(around)));
	return vec2(rel) * cell - zoom_in_cell;
}

// The trees' slots.
vec2 slot_chance(ivec2 slot, uint salt) {
	return grid_chance(slot, SLOTS_PER_CELL, salt);
}

vec2 slot_corner(ivec2 slot) {
	return grid_corner(slot, SLOTS_PER_CELL, TREE_CELL);
}

// The tree slot a place `d` metres from the focus lies in.
ivec2 slot_of(vec2 d) {
	return zoom_cell * SLOTS_PER_CELL + ivec2(floor((zoom_in_cell + d) / TREE_CELL));
}

// How much of a place `d` metres from the focus is a forest's stand, 0 to 1, where the share under trees is `share`:
// noise at 320, 110 and 38 m shapes the stands, as much land in them as the share, their edges clean. Noise finer than
// a pixel `fp` metres across can show is left out (0 for all of it).
float forest_stand(vec2 d, float share, float fp) {
	float n = 0.0;
	float cells[3] = float[3](320.0, 110.0, 38.0);
	float amps[3] = float[3](0.52, 0.31, 0.17);
	for (int k = 0; k < 3; k++) {
		float keep = fp > 0.0 ? clamp(cells[k] / (2.0 * fp) - 1.0, 0.0, 1.0) : 1.0;
		n += amps[k] * mix(0.5, ground_noise(d + vec2(float(k) * 3700.0 + 450.0, 1900.0), cells[k]), keep);
	}
	float edge = mix(0.76, 0.24, smoothstep(0.0, 1.0, share));
	return smoothstep(edge - 0.02, edge + 0.02, n);
}

// The chance a slot holds a tree or a bush: most slots in a stand, a few in the open land.
float tree_chance(float stand, float share) {
	return mix(0.02 + share * 0.3, 0.86, stand);
}

// A slot's tree, if one stands there.
struct Tree {
	bool here;
	vec2 at;        // metres from the focus
	float height;   // metres, its top above its foot
	int kind;       // 0 broad-leaved, 1 needle, 2 flat-crowned (warm and dry), 3 a bush
	float own;      // its own number, 0 to 1, for its shape and shade
	float fade;     // another, for fading out one by one as the zoom leaves it
	float autumn;   // how far its leaves have turned, 0 to 1
};

// The tree in a slot (WLD-13): where in it, how tall and of what kind, by the climate of its cell, as the season has
// coloured it; none where the ground is coloured as sea, beach, lake or river, nor at the camp.
Tree slot_tree(ivec2 slot) {
	vec2 r1 = slot_chance(slot, 0u);
	vec2 r2 = slot_chance(slot, 1u);
	vec2 r3 = slot_chance(slot, 2u);
	vec2 r4 = slot_chance(slot, 3u);
	Tree t;
	t.at = slot_corner(slot) + (0.15 + 0.7 * r1) * TREE_CELL;
	t.own = r4.x;
	t.fade = r4.y;
	vec2 uv = cell_uv(t.at);
	vec4 cover = textureLod(zoom_cover, uv, 0.0);
	vec4 clim = textureLod(zoom_climate, uv, 0.0);
	float share = cover.a;
	float stand = forest_stand(t.at, share, 0.0);
	t.here = r2.x < tree_chance(stand, share);
	// kinds by climate: needle trees where it is cold, flat crowns where it is warm and dry; in the open, mostly bushes
	float needle = smoothstep(9.0, 1.0, clim.x);
	float dry = smoothstep(0.45, 0.25, clim.w) * smoothstep(12.0, 20.0, clim.x);
	t.kind = r2.y < needle ? 1 : (r3.x < dry ? 2 : 0);
	if (stand < 0.5 && r3.y < 0.55) {
		t.kind = 3;
	}
	float tall = mix(7.0, 13.0, r1.x * 0.5 + r4.x * 0.5) * mix(0.8, 1.0, smoothstep(-5.0, 8.0, clim.x));
	t.height = t.kind == 3 ? mix(1.4, 2.8, r3.x) : (t.kind == 1 ? tall * 1.1 : (t.kind == 2 ? tall * 0.7 : tall));
	// leaves turn in autumn where winters freeze, the half of the year past midsummer, here or in the south
	float lat = PI * (uv.y - 0.5);
	float phase = fract(zoom_clock.y + (lat < 0.0 ? 0.5 : 0.0));
	t.autumn = (t.kind == 0 || t.kind == 3) ? smoothstep(0.42, 0.52, phase + 0.04 * r3.x) * smoothstep(0.72, 0.6, phase)
			* smoothstep(4.0, -2.0, clim.y) : 0.0;
	if (t.here) {
		// on the land the ground colours as land, above its beaches, and out of lakes and rivers
		vec2 shore = shore_uv(t.at);
		float land = textureLod(zoom_height, shore, 0.0).r;
		float lake = textureLod(zoom_water_soft, shore, 0.0).b * 255.0;
		t.here = land > 2.5 && lake < 1.0 && river_near(t.at, 1.0).z > 2.5;
		// none on the camp's floor and its path, nor close round them (camp.gd)
		t.here = t.here && length(t.at - zoom_camp.xy) > zoom_camp.z + 12.0
				&& segment_distance(t.at, zoom_camp.xy, zoom_path.xy) > 4.0;
	}
	return t;
}
