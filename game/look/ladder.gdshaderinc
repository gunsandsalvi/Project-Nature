// The ladder of a big surface (A5.3, A4.2): the tiles that serve each band, a near tile of 4 m for bands 0 and 1, a
// middle one of 16 m for bands 2 and 3 and a far one of 64 m for bands 4 to 6, each in two to four versions that join
// every other without a seam, and each cell of a tile's grid picking its version by a hash of its place, so the same
// piece never sits on a regular grid. All of a surface's tiles and versions are layers of one texture array, the near
// tile's versions first, then the middle's, then the far's. Every layer is read through the one sampling function's
// own reading (sample.gdshaderinc), a tile's level being its band less the band the tile starts at; the nearer tile's
// last level and the farther tile's first blend over the same short zoom as any two levels. Written once for every
// surface that has tiles: the ground, the river's bed and the water's marks. Implements PRE-22 and PRE-20.

#include "res://look/sample.gdshaderinc"

// The rig's (view/src/look.cpp): where its origin lies in the near, middle and far tiles' cells, east then south: the
// whole cells in xy and the part of a cell in zw, so every cell keeps its place when the origin moves (A8.2).
global uniform vec4 kd_origin_near;
global uniform vec4 kd_origin_middle;
global uniform vec4 kd_origin_far;

// Texture pixels a metre at band 0 (A5.3), a tile's side in texture pixels at its first band, and the last band a
// ladder serves.
const float KD_BAND0_TEXELS = 64.0;
const float KD_TILE_TEXELS = 256.0;
const float KD_LAST_BAND = 6.0;
// Added to a cell's number before it is made unsigned: cells run to about a million either way.
const float KD_CELL_BIAS = 1048576.0;

// A number from 0 up to but not 1 for a cell and a salt, the same on every machine since it is all whole numbers
// (a permuted congruential hash, Jarzynski and Olano's pcg3d).
float kd_hash(vec2 cell, float salt) {
	uvec3 v = uvec3(uint(cell.x + KD_CELL_BIAS), uint(cell.y + KD_CELL_BIAS), uint(salt));
	v = v * 1664525u + 1013904223u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v ^= v >> 16u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	return float(v.x >> 8u) * (1.0 / 16777216.0);
}

// Smooth noise from 0 to 1 over a lattice of whole cells, the same on every machine (kd_hash).
float kd_value_noise(vec2 at, float salt) {
	vec2 cell = floor(at);
	vec2 t = fract(at);
	t = t * t * (3.0 - 2.0 * t);
	float a = kd_hash(cell, salt);
	float b = kd_hash(cell + vec2(1.0, 0.0), salt);
	float c = kd_hash(cell + vec2(0.0, 1.0), salt);
	float d = kd_hash(cell + vec2(1.0, 1.0), salt);
	return mix(mix(a, b, t.x), mix(c, d, t.x), t.y);
}

// The texture pixel of a band's level that a point is in, counted from the world's centre: whole numbers east and
// south, the same wherever the rig's origin stands. The near cells are 4 m, a whole number of texture pixels of any
// level a ladder serves.
vec2 kd_texel_cell(vec2 world, float band) {
	float texel = exp2(band) / KD_BAND0_TEXELS;
	float a_cell = KD_TILE_TEXELS / exp2(band);
	return kd_origin_near.xy * a_cell + floor((kd_origin_near.zw * 4.0 + world) / texel);
}

// Where one band of a surface's ladder reads a point: the tile that serves the band (0 near, 1 middle, 2 far), which
// version of it the point's cell picks by its hash, the layer of the array that is, the point's place in the tile as
// a share of its width, and the band the tile starts at.
// count: how many versions each tile has, 0 for a tile the surface has none of; first: the band each tile starts at, a
// band past the last for a tile it has none of; salt: tells surfaces apart in the hash; world: the point in metres
// east and south of the rig's origin.
float kd_ladder_where(vec3 count, vec3 first, float salt, vec2 world, float band, out float version, out float layer,
		out vec2 uv, out float start) {
	float tile = band >= first.z ? 2.0 : (band >= first.y ? 1.0 : 0.0);
	start = tile == 2.0 ? first.z : (tile == 1.0 ? first.y : first.x);
	float versions = max(tile == 2.0 ? count.z : (tile == 1.0 ? count.y : count.x), 1.0);
	float before = tile == 2.0 ? count.x + count.y : (tile == 1.0 ? count.x : 0.0);
	vec4 origin = tile == 2.0 ? kd_origin_far : (tile == 1.0 ? kd_origin_middle : kd_origin_near);
	// the tile's width in metres: 256 texture pixels at 64 a metre, each twice as wide for each band it starts at
	float metres = KD_TILE_TEXELS * exp2(start) / KD_BAND0_TEXELS;
	vec2 at = world / metres + origin.zw;
	vec2 cell = origin.xy + floor(at);
	version = floor(kd_hash(cell, salt * 4.0 + tile) * versions);
	layer = before + version;
	uv = fract(at);
	return tile;
}

// A surface's colour at one band of its ladder. width0: the screen pixel's width in band 0's texture pixels, as
// kd_level_of gives it; the rest as kd_ladder_where.
vec4 kd_ladder_band(sampler2DArray tex, vec3 count, vec3 first, float salt, vec2 world, vec2 width0, float band) {
	float version;
	float layer;
	vec2 uv;
	float start;
	kd_ladder_where(count, first, salt, world, band, version, layer, uv, start);
	vec2 size0 = vec2(textureSize(tex, 0).xy);
	return kd_read_level(tex, uv, layer, band - start, size0, width0 / exp2(start));
}

// A surface's colour at a point whose level the rule picked: the level's own, and just before the next takes over,
// blended with the next, which may be another tile's.
vec4 kd_ladder(sampler2DArray tex, vec3 count, vec3 first, float salt, vec2 world, vec2 width0, float x) {
	float level = clamp(ceil(x), 0.0, KD_LAST_BAND);
	vec4 colour = kd_ladder_band(tex, count, first, salt, world, width0, level);
	float w = kd_next_share(level, x, KD_LAST_BAND);
	if (w > 0.0) {
		colour = mix(colour, kd_ladder_band(tex, count, first, salt, world, width0, level + 1.0), w);
	}
	return colour;
}
