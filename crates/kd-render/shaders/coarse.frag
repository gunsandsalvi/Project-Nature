// Coarse ground's pixels (A11.5): the ground's rules, at points 32 m apart. The normal and the fields are blended
// between the four points round the pixel as the ground's are; the material with the largest share of the squares'
// coverage at the footprint picks the surface, and where it is the soil's, the cell's cover takes over: a world-fixed
// noise drawn against the cover's shares, blended between the cells' middles, picks a group, each shown as the
// surface its cell's biome names, and where trees stand a crown may cover the point, a dome lit by its own normal,
// while it spans a few art pixels (crown_at). Between 1.2 and 4.5 m art pixels the map look dissolves in by the
// Bayer pattern (PRE-29): each cell in the first look of its commonest group's surface, the rock and scree of a face
// in their own, shaded by the smooth ground under the open sky and the sun, as hills are on a map, and clean of the
// air's haze, as a map is.
uniform highp sampler2D u_grads;     // RG32F, 257 x 257: each point's slope east and south
uniform highp sampler2D u_sun;       // R32F, 257 x 257: each point's horizon toward the light, as a slope
uniform highp sampler2D u_sky;       // R8, 257 x 257: the share of the sky each point's horizon leaves open
uniform highp sampler2D u_cover0;    // RGBA8, 256 x 256 at level 0, mipmapped: four materials' shares of the squares
uniform highp sampler2D u_cover1;    // the next four
uniform highp sampler2D u_cells;     // RGBA8, TILE_RING square: each cell's trees, bushes, reeds and bare ground
uniform highp sampler2D u_groups;    // RGBA8, TILE_RING square: the surface each group shows as, two to a channel
uniform ivec4 u_cover_ids[2];        // the surface each coverage channel holds, -1 for none
uniform int u_cover_textures;        // how many coverage textures the tile has, 1 or 2
uniform float u_cover_level;         // the level the art pixel's footprint reads, log2(texel / 32 m)
uniform int u_soil;                  // the soil's surface, which shows its cell's cover
uniform vec2 u_crown_r;              // the crowns' radius, least and most, metres
uniform vec4 u_cover_oct;            // the flat cover's noise: (l0, 1/l0, l1, 1/l1)
uniform float u_cover_spread;        // and how widely its draws spread round a half
uniform float u_unit;                // metres between points
uniform vec2 u_pattern_off;          // the tile's corner within its 8,192 m block of the world, east and south
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
uniform vec4 u_relief[MAX_SURFACES];                  // the micro-relief: (l0, 1/l0, octaves, greatest tilt)
uniform vec2 u_haze_beta;            // the aerosol's and the air's extinction a metre at the sea's level
uniform vec2 u_haze_scale;           // and their scale heights, metres
uniform vec2 u_eye;                  // the eye's plane before the target, metres, and the sine of the pitch
uniform vec3 u_haze_levels;          // where haze levels 1 to 3 begin
in vec2 v_local;
in float v_depth;
in vec2 v_air;
out vec4 o_colour;

// A cell's shares (trees, bushes, reeds, bare ground) at ring cell c.
vec4 cell_shares(ivec2 c) {
    return texelFetch(u_cells, clamp(c, ivec2(0), ivec2(TILE_RING - 1)), 0);
}

void main() {
    vec2 x = clamp(v_local / u_unit, vec2(0.0), vec2(256.0));
    ivec2 i0 = min(ivec2(floor(x)), ivec2(255));
    vec2 t = x - vec2(i0);
    vec2 g = mix(mix(texelFetch(u_grads, i0, 0).rg, texelFetch(u_grads, i0 + ivec2(1, 0), 0).rg, t.x),
        mix(texelFetch(u_grads, i0 + ivec2(0, 1), 0).rg, texelFetch(u_grads, i0 + ivec2(1, 1), 0).rg, t.x), t.y);
    float horizon = mix(mix(texelFetch(u_sun, i0, 0).r, texelFetch(u_sun, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_sun, i0 + ivec2(0, 1), 0).r, texelFetch(u_sun, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    float open = mix(mix(texelFetch(u_sky, i0, 0).r, texelFetch(u_sky, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_sky, i0 + ivec2(0, 1), 0).r, texelFetch(u_sky, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    vec2 w = u_pattern_off + v_local;
    vec4 shares0 = cover_sample(u_cover0, x, u_cover_level, COVER_SIDE, COVER_TOP);
    vec4 shares1 = u_cover_textures > 1 ? cover_sample(u_cover1, x, u_cover_level, COVER_SIDE, COVER_TOP) : vec4(0.0);
    int material = max(cover_pick(shares0, shares1, u_cover_ids[0], u_cover_ids[1]), 0);
    // The pixel's cell in the ring of cells, the cover's shares between the cells' middles, and the surfaces its
    // cell's groups show as.
    vec2 cp = v_local / CELL_M + 1.0;
    ivec2 cell = clamp(ivec2(floor(cp)), ivec2(1), ivec2(TILE_RING - 2));
    vec2 cm = cp - 0.5;
    ivec2 c0 = ivec2(floor(cm));
    vec2 ct = cm - vec2(c0);
    vec4 shares = mix(mix(cell_shares(c0), cell_shares(c0 + ivec2(1, 0)), ct.x),
        mix(cell_shares(c0 + ivec2(0, 1)), cell_shares(c0 + ivec2(1, 1)), ct.x), ct.y);
    ivec4 packed = ivec4(texelFetch(u_groups, cell, 0) * 255.0 + 0.5);
    int groups[5] = int[5](packed.r & 15, packed.r >> 4, packed.g & 15, packed.g >> 4, packed.b & 15);
    bool soil = material == u_soil;
    // The dither's band is as wide as the smooth ground's light changes across a pixel, in lightness, the same for
    // every branch below, so neither the crowns, the relief nor the map look's dissolve widens it (PRE-20).
    vec3 n0 = ground_normal(g, vec2(0.0));
    float band = fwidth(lightness(sky_factor(open, n0.z), sun_factor(horizon, u_light_tan, dot(n0, u_light_dir)),
        u_y.x, u_y.y)) / (u_range.y - u_range.x);
    int surface = material;
    vec3 n;
    float sigma;
    float tau;
    int look = 0;
    bool map = bayer(art_pixel() + u_dither) < map_weight(1.0 / u_inv_texel);
    if (map) {
        // The map look: the cell's commonest group, ties to the first, flat in its surface's first look.
        if (soil) {
            vec4 own = cell_shares(cell);
            float own_v[5] = float[5](own.x, own.y, max(1.0 - own.x - own.y - own.z - own.w, 0.0), own.z, own.w);
            int best = 0;
            for (int k = 1; k < 5; k++) {
                if (own_v[k] > own_v[best]) {
                    best = k;
                }
            }
            surface = groups[best];
        }
        n = n0;
        sigma = sky_factor(1.0, n.z);
        tau = sun_factor(0.0, u_light_tan, dot(n, u_light_dir));
    } else {
        vec4 crown = soil ? crown_at(w, shares.x, u_inv_texel, u_crown_r) : vec4(0.0);
        if (soil) {
            surface = crown.w > 0.0 ? groups[0]
                : groups[cover_group(shares, cover_draw(w, u_inv_texel, u_cover_oct, u_cover_spread))];
        }
        n = crown.w > 0.0 ? crown.xyz : ground_normal(g, relief_tilt(w, u_relief[surface], u_inv_texel));
        sigma = sky_factor(open, n.z);
        tau = sun_factor(horizon, u_light_tan, dot(n, u_light_dir));
        ivec2 info = u_surface_info[surface];
        look = split_look(faded_noise(w, u_split_oct[surface], u_inv_texel, uint(SEED_SPLIT)), u_split_at[surface],
            info.x);
    }
    ivec2 ladder = u_looks[surface * SURFACE_LOOKS + look];
    float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, ladder.y);
    int step = light_step(s, band * float(ladder.y - 1), ladder.y, art_pixel() + u_dither);
    // The map look is clean, as a map is (PRE-29): only the ground drawn as itself takes the air's haze.
    float hz = haze(v_air.x, v_air.y, u_haze_beta, u_haze_scale, u_eye);
    int level = haze_level(hz, u_haze_levels, fwidth(hz), art_pixel() + u_dither);
    int flags = (tau > 0.0 ? FLAG_SUNLIT : 0) | ((map ? 0 : level) << FLAG_HAZE_SHIFT);
    o_colour = pack_out(ladder.x + step, u_surface_info[surface].y == 1 ? CAT_ROCK : CAT_GROUND, flags, v_depth);
}
