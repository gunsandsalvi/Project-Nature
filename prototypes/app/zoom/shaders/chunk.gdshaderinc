// P8's chunks' own ground for what stands on it (trees.gdshader, plants.gdshader): the chunk's points as a picture
// (chunks.gd's ground_image), so what stands on a chunk stands on the ground the chunk draws, as it morphs.

uniform sampler2D heights : filter_nearest;  // the chunk's points: drawn height, the half-as-fine grid's, true height

// The chunk's ground at a point `g` of its grid, counted in points from its south-west corner, as its morph `m` draws
// it: x its height, y its true height (below 0 under the sea).
vec2 chunk_ground(vec2 g, float m) {
	float last = float(textureSize(heights, 0).x - 1);
	vec2 i0 = clamp(floor(g), vec2(0.0), vec2(last - 1.0));
	vec2 f = clamp(g - i0, 0.0, 1.0);
	ivec2 p = ivec2(i0);
	vec4 a = texelFetch(heights, p, 0);
	vec4 b = texelFetch(heights, p + ivec2(1, 0), 0);
	vec4 c = texelFetch(heights, p + ivec2(0, 1), 0);
	vec4 e = texelFetch(heights, p + ivec2(1, 1), 0);
	vec4 fine = mix(mix(a, b, f.x), mix(c, e, f.x), f.y);
	// the grid half as fine it morphs into: its points are the even ones, at their own heights
	vec2 j0 = clamp(floor(g * 0.5) * 2.0, vec2(0.0), vec2(last - 2.0));
	vec2 h = clamp((g - j0) * 0.5, 0.0, 1.0);
	ivec2 q = ivec2(j0);
	float a2 = texelFetch(heights, q, 0).g;
	float b2 = texelFetch(heights, q + ivec2(2, 0), 0).g;
	float c2 = texelFetch(heights, q + ivec2(0, 2), 0).g;
	float e2 = texelFetch(heights, q + ivec2(2, 2), 0).g;
	float coarse = mix(mix(a2, b2, h.x), mix(c2, e2, h.x), h.y);
	return vec2(mix(fine.r, coarse, m), fine.b);
}

