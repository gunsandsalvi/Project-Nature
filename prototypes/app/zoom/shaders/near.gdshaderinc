// P8's ground close up (A4.2): which layer of the look's palette a place shows, decided in one place for the ground,
// which draws it (ground.gdshaderinc), and the small plants growing on it (plants.gdshader), so a tuft never stands on
// the camp's floor or in the water: meadow, dry grass where the land is dry, the woods' floor under trees, mud along
// the rivers' banks, the camp's trodden floor and its path, sand on beaches, rock where it is steep, snow, and water.
// Include planet.gdshaderinc and rivers.gdshaderinc first.

// The palette's rows (zoom.gd sets them from the painter's): meadow, dry grass, the woods' floor and mud; dirt, sand,
// rock and snow; water, leaves, reeds and fire; three flowers' and autumn's leaves.
global uniform vec4 zoom_rows_a;
global uniform vec4 zoom_rows_b;
global uniform vec4 zoom_rows_c;
global uniform vec4 zoom_rows_d;

const float LAYER_MEADOW = 0.0;
const float LAYER_DRY = 1.0;
const float LAYER_FLOOR = 2.0;
const float LAYER_MUD = 3.0;
const float LAYER_DIRT = 4.0;
const float LAYER_SAND = 5.0;
const float LAYER_ROCK = 6.0;
const float LAYER_SNOW = 7.0;
const float LAYER_WATER = 8.0;

// How dry the open land is, 0 to 1, from the climate's moisture index and the share under trees.
float dryness(float moisture, float trees) {
	return smoothstep(0.35, 0.08, moisture) * (1.0 - trees);
}

// How much of a place `h` metres above the sea is beach, where it is not rock.
float beach_of(float h, float rock) {
	return smoothstep(4.0, 1.0, h) * (1.0 - rock);
}

// How much snow lies at a place `d` metres from the focus, `above` metres higher than its cell, in this season's
// temperature there, °C, as a pixel `fp` metres across sees it.
float snow_of(float season_temp, float above, vec2 d, float fp) {
	float temp = season_temp - 0.0065 * above;
	return smoothstep(0.5, -2.0, temp + 2.5 * ground_detail(d, 2000.0, fp, 6));
}

// The layer a place `d` metres from the focus shows close up, a pixel `fp` metres across: x its palette row, y the steps
// it moves the light's step (its patches a step darker), z how open it is to the sky, w which it is (LAYER_...). The
// ground there: its dryness, how much it is the woods' floor, its river (river_near), its lake's depth, how much beach,
// rock and snow.
vec4 near_layer(vec2 d, float fp, float dry, float under_trees, vec3 river, float lake, float beach, float rock, float snow) {
	float blot = 0.6 * ground_noise(d + 3300.0, 13.0) + 0.4 * ground_noise(d + 900.0, 5.5);
	vec4 layer = vec4(zoom_rows_a.x, blot < 0.3 ? -1.0 : 0.0, 1.0, LAYER_MEADOW);
	if (dry > 0.42 + 0.25 * (blot - 0.5)) {
		layer.xw = vec2(zoom_rows_a.y, LAYER_DRY);
	}
	if (under_trees > 0.5) {
		layer = vec4(zoom_rows_a.z, layer.y - 1.0, 0.45, LAYER_FLOOR);
	}
	float bank = 1.5 + 2.5 * ground_noise(d + 5100.0, 9.0);
	if (river.z > 0.0 && river.z < bank && river.y > 3.0) {
		layer = vec4(zoom_rows_a.w, blot < 0.45 ? -1.0 : 0.0, 1.0, LAYER_MUD);
	}
	// the camp's trodden floor round its hearth, and its path down to the river (camp.gd)
	float hearth = length(d - zoom_camp.xy);
	float worn = zoom_camp.z * (1.0 + 0.3 * (ground_noise(d + 700.0, 6.0) - 0.5))
			+ 0.8 * (ground_noise(d + 2100.0, 1.5) - 0.5);
	float path = segment_distance(d, zoom_camp.xy, zoom_path.xy)
			- zoom_camp.w * (1.0 + 0.7 * (ground_noise(d + 400.0, 2.5) - 0.5));
	if (hearth < worn || (path < 0.0 && river.z > 0.0)) {
		layer = vec4(zoom_rows_b.x, blot < 0.4 ? -1.0 : 0.0, 1.0, LAYER_DIRT);
	}
	if (beach > 0.5) {
		layer = vec4(zoom_rows_b.y, layer.y, 1.0, LAYER_SAND);
	}
	if (rock > 0.5) {
		layer = vec4(zoom_rows_b.z, layer.y, 1.0, LAYER_ROCK);
	}
	if (snow > 0.5) {
		layer = vec4(zoom_rows_b.w, layer.y, 1.0, LAYER_SNOW);
	}
	// the water clear (PRE-26): a bright line where it meets the land, its shallows, then a step darker, and its deep
	// another
	float inside = river.x > 0.5 ? -river.z : (lake > 1.0 ? 10.0 : -1.0);
	if (inside >= 0.0) {
		float depth_step = inside < 1.2 * fp ? 1.0 : (inside < 1.6 ? 0.0 : (inside < 5.0 ? -1.0 : -2.0));
		layer = vec4(zoom_rows_c.x, depth_step, 1.0, LAYER_WATER);
	}
	return layer;
}
