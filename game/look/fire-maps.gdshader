// The fires' maps (A4.5): a small two-dimensional pass, never a compute program, drawing a map for each
// fire side by side, each texel how much of its fire's light reaches the ground there, by the walk
// (fire.gdshaderinc); remade 20 times a second as people move, and read once a pixel by the field
// (field-map.gdshader). Implements PLT-04 and PRE-30.
shader_type canvas_item;

#include "res://look/fire.gdshaderinc"

global uniform sampler2D kd_fire_table;

void fragment() {
	float tile = floor(UV.x * KD_FIRE_MAPS);
	int row = int(tile);
	float lit = 1.0;
	if (row < textureSize(kd_fire_table, 0).x) {
		vec4 fire = texelFetch(kd_fire_table, ivec2(row, 0), 0);
		vec2 uv = vec2(UV.x * KD_FIRE_MAPS - tile, UV.y);
		lit = kd_fire_walk(kd_fire_map_point(fire, uv), kd_flame(fire));
	}
	COLOR = vec4(vec3(lit), 1.0);
}
