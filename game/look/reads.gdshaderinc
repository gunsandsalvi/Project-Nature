// Calibration scene C6's reads in the vertex stage (A6.3, A18.1, α2.2b): points, one a vertex, each
// reading KD_READS texture pixels before it is placed, from a table of half floats laid out as a bone
// palette is: a run of 800 vertices, a figure's, reads its row, and 32 vertices in turn read the same
// bones, three texels a bone. The points are set out over the screen in a grid by their indices, and
// what each reads moves it by a hair, so no compiler leaves a read out. The file that includes this
// defines KD_READS. Implements PRE-27.

#include "res://look/picture.gdshaderinc"

uniform sampler2D kd_table : filter_nearest, repeat_disable;
// The grid the points make on the screen: points across and down.
uniform ivec2 kd_grid = ivec2(1, 1);

varying vec3 kd_tint;

void vertex() {
	vec4 read = vec4(0.0);
#if KD_READS > 0
	int rows = textureSize(kd_table, 0).y;
	int row = (VERTEX_ID / 800) % rows;
	for (int k = 0; k < KD_READS; k++) {
		int bone = (VERTEX_ID / 32 + (k / 3) * 7) % 24;
		read += texelFetch(kd_table, ivec2(bone * 3 + k % 3, row), 0);
	}
#endif
	vec2 at = (vec2(float(VERTEX_ID % kd_grid.x), float(VERTEX_ID / kd_grid.x)) + 0.5) / vec2(kd_grid);
	float hair = 1e-6 * dot(read, vec4(1.0));
	POSITION = vec4(at * 2.0 - 1.0, 0.5 + hair, 1.0);
	kd_tint = vec3(0.85, 0.80, 0.70) + hair;
	POINT_SIZE = 1.0;
}

void fragment() {
	ALBEDO = kd_tint;
	// in a test picture: no material of the world's, and no object
	if (kd_picture != 0) {
		ALBEDO = kd_number_colour(0);
	}
}
