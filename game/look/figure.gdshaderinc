// The figures' shader body (A6.3): a stand-in figure of one colour, lit by the one shared light
// function's sun (A4.3). Godot's own skeleton bends it before this runs; with KD_PALETTE this bends
// it, each vertex by its bones' rows of the palette texture, the row its copy's custom data names.
// With KD_DUMP it is the cloud's check of the palettes (A18.1): each vertex a point at a pixel of its
// own, by its index, its bent place its colour. Written once for every way (CLAUDE.md rule 4).
// Implements PRE-27.

#include "res://look/light.gdshaderinc"
#include "res://look/picture.gdshaderinc"

uniform vec3 kd_colour : source_color = vec3(0.62, 0.45, 0.33);
#ifdef KD_PALETTE
// Each figure's skinning matrices, a row a figure and three texels a bone, each texel a row of the
// matrix, in half floats (A6.3).
uniform sampler2D kd_palette : filter_nearest, repeat_disable;
#endif
#ifdef KD_DUMP
varying vec3 kd_bent;
#endif

#ifdef KD_PALETTE
// A bone's skinning matrix from a figure's row: its rows as the columns here, so that a point taken
// as a row is carried by it.
mat4 kd_bone(uint bone, int row) {
	int at = int(bone) * 3;
	return mat4(texelFetch(kd_palette, ivec2(at, row), 0), texelFetch(kd_palette, ivec2(at + 1, row), 0),
			texelFetch(kd_palette, ivec2(at + 2, row), 0), vec4(0.0, 0.0, 0.0, 1.0));
}
#endif

void vertex() {
#ifdef KD_PALETTE
	// the vertex's bones weighed, its first always and the others where they weigh anything, as
	// Godot's own skinning reads them
	int row = int(INSTANCE_CUSTOM.x + 0.5);
	mat4 bend = kd_bone(BONE_INDICES.x, row) * BONE_WEIGHTS.x;
	if (BONE_WEIGHTS.y > 0.0) {
		bend += kd_bone(BONE_INDICES.y, row) * BONE_WEIGHTS.y;
	}
	if (BONE_WEIGHTS.z > 0.0) {
		bend += kd_bone(BONE_INDICES.z, row) * BONE_WEIGHTS.z;
	}
	if (BONE_WEIGHTS.w > 0.0) {
		bend += kd_bone(BONE_INDICES.w, row) * BONE_WEIGHTS.w;
	}
	VERTEX = (vec4(VERTEX, 1.0) * bend).xyz;
	NORMAL = normalize((vec4(NORMAL, 0.0) * bend).xyz);
#endif
#ifdef KD_DUMP
	kd_bent = VERTEX;
	int across = int(VIEWPORT_SIZE.x);
	vec2 pixel = vec2(float(VERTEX_ID % across), float(VERTEX_ID / across)) + 0.5;
	POSITION = vec4(pixel / VIEWPORT_SIZE * 2.0 - 1.0, 0.5, 1.0);
	POINT_SIZE = 1.0;
#endif
}

void fragment() {
#ifdef KD_DUMP
	// the bent place in metres, from -1 to 3, as a colour from 0 to 2, which the view keeps
	ALBEDO = kd_bent * 0.5 + 0.5;
#else
	ALBEDO = kd_colour;
	ROUGHNESS = 1.0;
	// in a test picture: no material of the world's, and no object
	if (kd_picture != 0) {
		ALBEDO = vec3(0.0);
		EMISSION = kd_number_colour(0);
	}
#endif
}

#ifndef KD_DUMP
void light() {
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, ATTENUATION, 1.0, 0.0);
}
#endif
