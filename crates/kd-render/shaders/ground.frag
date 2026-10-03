// The ground's pixels (A11.5): the normal from the heights' central differences blended between the four points
// round the pixel, the surface the four nearest squares vote for at a place moved by the edges' world-fixed wobble,
// the surface's look by its split noise, both faded below four art pixels (A11.1 rule 2); the sky factor from the
// sky field, the share of the sky the horizon leaves open, and the sun factor from the sun field, the share of the
// sun's disc above the horizon toward it, both blended between the four points too (A11.4); the light's step
// dithered only in the band; the category ground, or rock for a rock surface (A11.2).
uniform highp sampler2D u_grads;     // RG32F, 257 x 257: each point's slope east and south
uniform highp sampler2D u_sun;       // R32F, 257 x 257: each point's horizon toward the light, as a slope
uniform highp sampler2D u_sky;       // R8, 257 x 257: the share of the sky each point's horizon leaves open
uniform highp sampler2D u_surfaces;  // R8, 256 x 256: each square metre's surface number
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
in vec2 v_local;
in float v_depth;
out vec4 o_colour;

int surface_at(ivec2 square) {
    return int(texelFetch(u_surfaces, square, 0).r * 255.0 + 0.5);
}

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
    ivec2 base;
    vec2 f;
    vote_base(x + edge_wobble(w, u_inv_texel), 254, base, f);
    int surface = vote4(f, ivec4(surface_at(base), surface_at(base + ivec2(1, 0)), surface_at(base + ivec2(0, 1)),
        surface_at(base + ivec2(1, 1))));
    ivec2 info = u_surface_info[surface];
    int look = split_look(faded_noise(w, u_split_oct[surface], u_inv_texel, uint(SEED_SPLIT)), u_split_at[surface],
        info.x);
    ivec2 ladder = u_looks[surface * SURFACE_LOOKS + look];
    float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, ladder.y);
    int step = light_step(s, fwidth(s), ladder.y, art_pixel() + u_dither);
    o_colour = pack_out(ladder.x + step, info.y == 1 ? CAT_ROCK : CAT_GROUND, tau > 0.0 ? FLAG_SUNLIT : 0, v_depth);
}
