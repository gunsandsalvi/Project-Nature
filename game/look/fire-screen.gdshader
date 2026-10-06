// The walk at half resolution (A4.5): calibration scene C5's flat ground drawn into a picture of its own at
// half the screen's size, each pixel how much of the first three fires its square of the light grid lists
// reaches it, by the walk at every pixel (fire.gdshaderinc); the field reads it by screen position
// (field-screen.gdshader). Drawn through a camera with plain light, so the numbers come back as written.
// Implements PLT-04 and PRE-30.
shader_type spatial;
render_mode unshaded, cull_back;

#include "res://look/fire.gdshaderinc"

global uniform sampler2D kd_fire_grid;
global uniform sampler2D kd_fire_table;
global uniform vec4 kd_grid_place;

varying vec3 kd_world;

void vertex() {
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

void fragment() {
	vec3 lit = vec3(1.0);
	vec2 at = vec2(kd_world.x, -kd_world.z);
	ivec2 square = ivec2(floor((at - kd_grid_place.xy) / kd_grid_place.z));
	ivec2 squares = textureSize(kd_fire_grid, 0);
	if (all(greaterThanEqual(square, ivec2(0))) && all(lessThan(square, squares))) {
		vec4 rows = texelFetch(kd_fire_grid, square, 0) * 255.0;
		for (int i = 0; i < 3; i++) {
			int row = int(rows[i] + 0.5) - 1;
			if (row < 0) {
				break;
			}
			lit[i] = kd_fire_walk(kd_world, kd_flame(texelFetch(kd_fire_table, ivec2(row, 0), 0)));
		}
	}
	ALBEDO = lit;
}
