// The one sampling function (A4.2, CLAUDE.md rule 4): every texture of the world is read here.
// A texture holds its levels as its own mipmaps, each drawn for its zoom band (A5.3), never averaged.
// The level is picked from a texture pixel's area on screen, so each texture pixel is kd_texel_least to
// twice that many screen pixels wide, with a short blend into the next level just before it takes over.
// Each level is read with the "smooth pixel" filter: crisp inside a texture pixel, blended over one
// screen pixel at its edge, from a texture stored with blending and mipmaps on. The cloud's shimmer
// check can read every level nearest-pixel instead, to show the shimmer the smooth pixel prevents
// (A4.8). The rule that picks the level and the blend into the next are functions of their own, so a
// big surface's ladder of tiles (ladder.gdshaderinc) picks and blends by the same rule. Implements PRE-22.

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

// The level the rule picks for a surface point, continuous: p0 is the point in level 0's texture pixels,
// and width0 comes back the screen pixel's width in them. The level whose texture pixel is just at least
// kd_texel_least screen pixels wide is the integer at or above the result.
float kd_level_of(vec2 p0, out vec2 width0) {
	vec2 dx = dFdx(p0);
	vec2 dy = dFdy(p0);
	width0 = abs(dx) + abs(dy);
	float area = max(abs(dx.x * dy.y - dx.y * dy.x), 1e-12);
	return log2(kd_texel_least) + 0.5 * log2(area);
}

// How much of the next level shows at a continuous level x, whose integer at or above is level: 0 until
// just before the next takes over, then up to 1; 0 at the last level.
float kd_next_share(float level, float x, float last) {
	return level < last ? 1.0 - smoothstep(0.0, KD_LEVEL_BLEND, level - x) : 0.0;
}

// A texture's colour at a surface point. uv: the point in the texture's own tiles (level 0 spans one);
// layer: the material's layer of the array.
vec4 kd_sample(sampler2DArray tex, vec2 uv, float layer) {
	vec2 size0 = vec2(textureSize(tex, 0).xy);
	vec2 width0;
	float x = kd_level_of(uv * size0, width0);
	float last = log2(size0.x);
	// the level whose texture pixel is just at least kd_texel_least screen pixels wide
	float level = clamp(ceil(x), 0.0, last);
	vec4 colour = kd_read_level(tex, uv, layer, level, size0, width0);
	// just before the next level takes over, blend into it, read the same crisp way
	float w = kd_next_share(level, x, last);
	if (w > 0.0) {
		colour = mix(colour, kd_read_level(tex, uv, layer, level + 1.0, size0, width0), w);
	}
	return colour;
}
