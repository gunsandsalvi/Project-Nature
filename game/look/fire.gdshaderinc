// Fire shadows (A4.5): what stands between a point and a fire's flame, from the height maps of tops and
// undersides round the fires. The walk is written once (CLAUDE.md rule 4) and used three ways, which
// calibration scene C5 times: at every pixel of the world's surfaces; at every pixel of a picture at half
// resolution, read by screen position; and at every texel of a small map for each fire, read once a pixel.
// Implements PRE-30 and PRE-24.

// The height maps seen from above round the fires: r the top of what stands at a texel, g its
// underside, in metres; a point between them is inside something.
global uniform sampler2D kd_heights;
// Their south-west corner in the rig's metres east and north (xy), and their width in metres (z).
global uniform vec4 kd_heights_place;
// A map for each fire, side by side in the order of the fire table's rows: how much of the fire's light
// reaches the ground at each texel. Declared only where it is read: a pass never declares the picture it
// draws into (A4.7).
#ifdef KD_FIRE_MAP
global uniform sampler2D kd_fire_maps;
#endif
// The walk at half resolution, by screen position: a channel for each of the first three fires listed in
// a point's square of the light grid; declared only where it is read.
#ifdef KD_FIRE_SCREEN
global uniform sampler2D kd_fire_screen;
#endif

// The walk's steps from a point to a flame.
const int KD_WALK_STEPS = 32;
// The maps side by side, and the ground each covers round its fire, in metres: 128 texels each, about 6 cm.
const float KD_FIRE_MAPS = 8.0;
const float KD_FIRE_MAP_METRES = 8.0;

// A fire's flame in the world, from its row of the fire table: where its light comes from, east, up and
// north (A4.5).
vec3 kd_flame(vec4 fire) {
	return vec3(fire.x, fire.y, -fire.z);
}

// How much of a fire's light reaches a point: 0 when something stands between them, 1 when nothing does,
// by a walk of KD_WALK_STEPS steps from the point to the flame through the height maps.
float kd_fire_walk(vec3 from, vec3 flame) {
	for (int i = 1; i <= KD_WALK_STEPS; i++) {
		vec3 p = mix(from, flame, float(i) / float(KD_WALK_STEPS + 1));
		vec2 h = textureLod(kd_heights, (vec2(p.x, -p.z) - kd_heights_place.xy) / kd_heights_place.z, 0.0).rg;
		if (p.y < h.r && p.y > h.g) {
			return 0.0;
		}
	}
	return 1.0;
}

// The ground point a texel of a fire's map stands for: uv across the map, from its south-west corner.
vec3 kd_fire_map_point(vec4 fire, vec2 uv) {
	vec2 at = vec2(fire.x, fire.z) + (uv - 0.5) * KD_FIRE_MAP_METRES;
	return vec3(at.x, 0.0, -at.y);
}

#ifdef KD_FIRE_MAP
// How much of a fire's light reaches a point, read from the fire's map: its row of the fire table, and
// the fire. Outside the map, the fire's light is too faint for a shadow to show.
float kd_fire_map(vec3 world, int row, vec4 fire) {
	vec2 uv = (vec2(world.x, -world.z) - vec2(fire.x, fire.z)) / KD_FIRE_MAP_METRES + 0.5;
	if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
		return 1.0;
	}
	float edge = 0.5 / 128.0;
	return texture(kd_fire_maps, vec2((float(row) + clamp(uv.x, edge, 1.0 - edge)) / KD_FIRE_MAPS, uv.y)).r;
}
#endif
