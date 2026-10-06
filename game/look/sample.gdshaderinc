// The one sampling function (A4.2, CLAUDE.md rule 4): every texture of the world is read here.
// A texture holds its levels as its own mipmaps, each drawn for its zoom band (A5.3), never averaged.
// The level is picked from a texture pixel's area on screen, so each texture pixel is kd_texel_least to
// twice that many screen pixels wide, with a short blend into the next level just before it takes over.
// Each level is read with the "smooth pixel" filter: crisp inside a texture pixel, blended over one
// screen pixel at its edge, from a texture stored with blending and mipmaps on. The cloud's shimmer
// check can read every level nearest-pixel instead, to show the shimmer the smooth pixel prevents
// (A4.8). Implements PRE-22.

// The rig's, published each frame (view/src/rig.hpp): a texture pixel's least width on screen.
global uniform float kd_texel_least;
// 1 reads nearest-pixel, for the cloud's shimmer check only; 0, the game's, reads smooth pixels.
global uniform float kd_nearest;

// How much of a level's span, in powers of two of a texture pixel's size, blends into the next.
const float KD_LEVEL_BLEND = 0.15;

// One level, read crisp: the coordinate moves to the middle of its texture pixel except within one
// screen pixel of an edge, where it slides to the next one's middle, so the chip's own blending
// softens only the step. p + 0.5 puts the edges at half-whole numbers, where the slide is centred.
vec4 kd_read_level(sampler2DArray tex, vec2 uv, float layer, float level, vec2 size0, vec2 width0) {
	float scale = exp2(-level);
	vec2 size = max(floor(size0 * scale), vec2(1.0));
	vec2 p = uv * size + 0.5;
	vec2 width = max(width0 * scale, vec2(1e-5));
	vec2 q = floor(p) + clamp((fract(p) - 0.5) / width, -0.5, 0.5);
	// nearest-pixel: always the middle of the texture pixel the point is in
	q = kd_nearest > 0.5 ? floor(p - 0.5) + 0.5 : q;
	return textureLod(tex, vec3(q / size, layer), level);
}

// A texture's colour at a surface point. uv: the point in the texture's own tiles (level 0 spans one);
// layer: the material's layer of the array.
vec4 kd_sample(sampler2DArray tex, vec2 uv, float layer) {
	vec2 size0 = vec2(textureSize(tex, 0).xy);
	vec2 p0 = uv * size0;
	vec2 dx = dFdx(p0);
	vec2 dy = dFdy(p0);
	vec2 width0 = abs(dx) + abs(dy);
	float area = max(abs(dx.x * dy.y - dx.y * dy.x), 1e-12);
	float last = log2(size0.x);
	// the level whose texture pixel is just at least kd_texel_least screen pixels wide
	float x = log2(kd_texel_least) + 0.5 * log2(area);
	float level = clamp(ceil(x), 0.0, last);
	vec4 colour = kd_read_level(tex, uv, layer, level, size0, width0);
	// just before the next level takes over, blend into it, read the same crisp way
	float w = level < last ? 1.0 - smoothstep(0.0, KD_LEVEL_BLEND, level - x) : 0.0;
	if (w > 0.0) {
		colour = mix(colour, kd_read_level(tex, uv, layer, level + 1.0, size0, width0), w);
	}
	return colour;
}
