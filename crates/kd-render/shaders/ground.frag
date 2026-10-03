// The ground's pixels (A11.5): the normal from the heights' central differences blended between the four points
// round the pixel, the surface with the largest share of the coverage read at the art pixel's footprint, at a place
// moved by the edges' world-fixed wobble, the surface's look by its split noise, both faded below four art pixels
// (A11.1 rule 2); the sky factor from the
// sky field, the share of the sky the horizon leaves open, and the sun factor from the sun field, the share of the
// sun's disc above the horizon toward it, both blended between the four points too (A11.4); the light's step
// dithered only in the band; the category ground, or rock for a rock surface (A11.2); and the haze's level from
// the air along the pixel's ray to the eye's plane, dithered in the same narrow bands (A11.4).
uniform highp sampler2D u_grads;     // RG32F, 257 x 257: each point's slope east and south
uniform highp sampler2D u_sun;       // R32F, 257 x 257: each point's horizon toward the light, as a slope
uniform highp sampler2D u_sky;       // R8, 257 x 257: the share of the sky each point's horizon leaves open
uniform highp sampler2D u_cover0;    // RGBA8, 256 x 256 at level 0, mipmapped: four surfaces' shares
uniform highp sampler2D u_cover1;    // the next four, for an area of more than four surfaces
uniform ivec4 u_cover_ids[2];        // the surface each channel holds, -1 for none
uniform int u_cover_textures;        // how many coverage textures the area has, 1 or 2
uniform float u_cover_level;         // the level the art pixel's footprint reads, log2(texel / 1 m)
uniform vec2 u_pattern_off;          // the area's corner within its 8,192 m block of the world, east and south
uniform float u_inv_texel;           // art pixels a metre
uniform ivec2 u_dither;              // the art target's corner in the world's art pixels, modulo 4
uniform vec3 u_light_dir;            // toward the sun, or the moon at night (east, north, up)
uniform float u_light_tan;           // the light's slope, tan e
uniform vec2 u_y;                    // the luminance of the sky's light, and of the sun's facing it
uniform vec2 u_range;                // the ladders' path in lightness, deep shade to full sun
uniform ivec2 u_looks[MAX_SURFACES * SURFACE_LOOKS];  // each surface's looks: first palette index, steps
uniform ivec2 u_surface_info[MAX_SURFACES];           // each surface's number of looks, and 1 for rock
uniform vec2 u_split_at[MAX_SURFACES];                // where each next look takes over
uniform vec4 u_split_oct[MAX_SURFACES];               // the split noise's octaves: (l0, 1/l0, l1, 1/l1)
uniform vec2 u_haze_beta;            // the aerosol's and the air's extinction a metre at the sea's level
uniform vec2 u_haze_scale;           // and their scale heights, metres
uniform vec2 u_eye;                  // the eye's plane before the target, metres, and the sine of the pitch
uniform vec3 u_haze_levels;          // where haze levels 1 to 3 begin
in vec2 v_local;
in float v_depth;
in vec2 v_air;
out vec4 o_colour;

void main() {
    vec2 x = clamp(v_local, vec2(0.0), vec2(256.0));
    ivec2 i0 = min(ivec2(floor(x)), ivec2(255));
    vec2 t = x - vec2(i0);
    vec2 g = mix(mix(texelFetch(u_grads, i0, 0).rg, texelFetch(u_grads, i0 + ivec2(1, 0), 0).rg, t.x),
        mix(texelFetch(u_grads, i0 + ivec2(0, 1), 0).rg, texelFetch(u_grads, i0 + ivec2(1, 1), 0).rg, t.x), t.y);
    float horizon = mix(mix(texelFetch(u_sun, i0, 0).r, texelFetch(u_sun, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_sun, i0 + ivec2(0, 1), 0).r, texelFetch(u_sun, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    float open = mix(mix(texelFetch(u_sky, i0, 0).r, texelFetch(u_sky, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_sky, i0 + ivec2(0, 1), 0).r, texelFetch(u_sky, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    vec3 n = normalize(vec3(-g.x, g.y, 1.0));
    float sigma = sky_factor(open, n.z);
    float tau = sun_factor(horizon, u_light_tan, dot(n, u_light_dir));
    vec2 w = u_pattern_off + x;
    vec2 q = x + edge_wobble(w, u_inv_texel);
    vec4 shares0 = cover_sample(u_cover0, q, u_cover_level, COVER_SIDE, COVER_TOP);
    vec4 shares1 = u_cover_textures > 1 ? cover_sample(u_cover1, q, u_cover_level, COVER_SIDE, COVER_TOP) : vec4(0.0);
    int surface = max(cover_pick(shares0, shares1, u_cover_ids[0], u_cover_ids[1]), 0);
    ivec2 info = u_surface_info[surface];
    int look = split_look(faded_noise(w, u_split_oct[surface], u_inv_texel, uint(SEED_SPLIT)), u_split_at[surface],
        info.x);
    ivec2 ladder = u_looks[surface * SURFACE_LOOKS + look];
    float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, ladder.y);
    int step = light_step(s, fwidth(s), ladder.y, art_pixel() + u_dither);
    float hz = haze(v_air.x, v_air.y, u_haze_beta, u_haze_scale, u_eye);
    int flags = (tau > 0.0 ? FLAG_SUNLIT : 0) | (haze_level(hz, u_haze_levels, fwidth(hz), art_pixel() + u_dither)
        << FLAG_HAZE_SHIFT);
    o_colour = pack_out(ladder.x + step, info.y == 1 ? CAT_ROCK : CAT_GROUND, flags, v_depth);
}
