// P8's small plants (A4.2, A8.3): grass tufts, dry grass, a few flowers on the meadow and reeds along the banks: the
// kit's tuft (kit_shapes.gd) set on each slot of the nearest chunks' grid by keyed chance (WLD-13), where the ground
// close up shows the layer it grows on (near.gdshaderinc), so none stands on the camp's floor or in the water; each in
// its own size and colour, turned its own way; lit as the ground at its foot and a step lighter, as the art book's
// tufts are (PRE-20), and casting no shadow. They fade out one by one as their chunks morph away. Lit by the look
// (look.gdshaderinc).
shader_type spatial;
render_mode world_vertex_coords, cull_disabled, specular_disabled, ambient_light_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/rivers.gdshaderinc"
#include "res://zoom/shaders/forest.gdshaderinc"
#include "res://zoom/shaders/chunk.gdshaderinc"
#include "res://zoom/shaders/near.gdshaderinc"
#include "res://look/shaders/look.gdshaderinc"

const float PLANT_CELL = 500000.0 / 262144.0;  // 1.91 m: 512 to a world cell's side, 16 to the finest chunks'
const int PLANTS_PER_CELL = 512;

uniform ivec2 first_slot;  // the slot at the chunk's south-west corner
uniform int slots = 16;    // slots each way
uniform float spacing = 1.0;  // metres between the chunk's points
uniform vec2 morph = vec2(1.0e12, 2.0e12);  // as the ground's

varying flat vec2 v_plant;  // its palette row and step
varying flat vec3 v_foot;   // its foot, in the picture
varying flat vec3 v_up;     // the ground's up there

void vertex() {
	vec3 corner = MODEL_MATRIX[3].xyz;
	vec3 own = VERTEX - corner;
	ivec2 slot = first_slot + ivec2(INSTANCE_ID % slots, INSTANCE_ID / slots);
	vec2 r1 = grid_chance(slot, PLANTS_PER_CELL, 10u);
	vec2 r2 = grid_chance(slot, PLANTS_PER_CELL, 11u);
	vec2 at = grid_corner(slot, PLANTS_PER_CELL, PLANT_CELL) + (0.1 + 0.8 * r1) * PLANT_CELL;
	// the ground it stands on, as the chunk draws it
	vec2 chunk_d = vec2(corner.x - zoom_focus_at.x, zoom_focus_at.z - corner.z);
	vec2 g = (at - chunk_d) / spacing;
	float m = clamp((length(planet_place(at, chunk_ground(g, 0.0).x) - zoom_eye) - morph.x) / (morph.y - morph.x), 0.0, 1.0);
	vec2 ground = chunk_ground(g, m);
	// the layer the ground shows there close up
	vec2 uv = cell_uv(at);
	vec4 cover = textureLod(zoom_cover, uv, 0.0);
	vec4 clim = textureLod(zoom_climate, uv, 0.0);
	vec2 shore = shore_uv(at);
	float h = textureLod(zoom_height, shore, 0.0).r;
	float lake = textureLod(zoom_water_soft, shore, 0.0).b * 255.0;
	float lat = PI * ((zoom_focus.y + at.y) / zoom_world.y - 0.5);
	float snow = snow_of(mix(clim.y, clim.z, season_warmth(lat)), max(ground.x - max(h, 0.0), 0.0), at, 0.05);
	vec4 layer = near_layer(at, 0.05, dryness(clim.a, cover.a), forest_stand(at, cover.a, 0.0), river_near(at, 0.05),
			lake, beach_of(h, 0.0), 0.0, snow);
	// what grows there: tufts and a few flowers on the meadow, taller tufts on dry grass, a few on the woods' floor,
	// reeds along the banks
	float grows = -1.0;
	vec2 size = vec2(1.0);
	if (layer.w == LAYER_MEADOW && r2.x < 0.42) {
		bool flower = r2.x > 0.37;
		grows = flower ? (r2.y < 0.4 ? zoom_rows_d.x : (r2.y < 0.75 ? zoom_rows_d.y : zoom_rows_d.z)) : zoom_rows_a.x;
		size = flower ? vec2(0.8, 1.1) : vec2(1.0, mix(1.0, 1.8, r2.y));
	} else if (layer.w == LAYER_DRY && r2.x < 0.45) {
		grows = zoom_rows_a.y;
		size = vec2(1.2, mix(1.4, 2.2, r2.y));
	} else if (layer.w == LAYER_FLOOR && r2.x < 0.15) {
		grows = zoom_rows_a.z;
		size = vec2(1.3, mix(1.2, 2.0, r2.y));
	} else if (layer.w == LAYER_MUD && r2.x < 0.6) {
		grows = zoom_rows_c.z;
		size = vec2(1.6, mix(5.0, 8.5, r2.y));
	}
	bool shown = grows >= 0.0 && ground.y > 0.3 && r1.y > m;
	vec3 foot = planet_place(at, ground.x);
	mat3 frame = planet_frame(foot);
	float turn = TAU * fract(r1.x * 7.31 + r1.y * 3.17);
	vec3 p = vec3(own.x * cos(turn) - own.z * sin(turn), own.y, own.x * sin(turn) + own.z * cos(turn));
	p *= vec3(size.x, size.y, size.x);
	VERTEX = shown ? foot + frame[0] * p.x + frame[1] * p.y - frame[2] * p.z : foot;
	NORMAL = frame[1];
	v_plant = vec2(grows, round(CUSTOM0.y * 255.0) - 128.0 + layer.y);
	v_foot = foot;
	v_up = frame[1];
}

void fragment() {
	flags = FLAG_FOLIAGE + FLAG_NOOUTLINE;
	row = v_plant.x;
	bias = v_plant.y;
	edge = 0.0;
	SKY_AND_FIRE(v_foot, v_up)
	NORMAL = (VIEW_MATRIX * vec4(v_up, 0.0)).xyz;
	LIGHT_VERTEX = (VIEW_MATRIX * vec4(v_foot, 1.0)).xyz;
	ALBEDO = vec3(1.0);
	ROUGHNESS = 1.0;
}

void light() {
	LOOK_LIGHT
}
