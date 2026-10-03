// The sea's surface over coarse ground (A11.6): a flat water grid at the sea's level over every point the sea lies
// over, drawn by the ground's patch at that level; each pixel in the water's look, its light the bed's darkened by
// the depth over a floor of light scattered in the water, mixed by Fresnel's term with the sky the surface reflects
// (water_light), lit by the sky and the sun as level ground; its step dithered only in the band, and hazed as the
// ground. Waves and ice wait for α20c.
uniform highp sampler2D u_heights;   // R32F, 257 x 257: the ground's height above the tile's corner
uniform highp sampler2D u_sea;       // R8, 257 x 257: 1 where the sea lies over the point
uniform float u_unit;                // metres between points
uniform float u_level;               // the sea's level above the tile's corner
uniform ivec2 u_water;               // the water's look: first palette index, steps
uniform vec2 u_water_light;          // deep water's floor, and the depth over which a bed's light fades by e
uniform vec3 u_fwd;                  // the view's direction
uniform float u_inv_texel;           // art pixels a metre
uniform ivec2 u_dither;              // the art target's corner in the world's art pixels, modulo 4
uniform vec3 u_light_dir;            // toward the sun, or the moon at night (east, north, up)
uniform float u_light_tan;           // the light's slope, tan e
uniform vec2 u_y;                    // the luminance of the sky's light, and of the sun's facing it
uniform vec2 u_range;                // the ladders' path in lightness, deep shade to full sun
uniform vec2 u_haze_beta;            // the aerosol's and the air's extinction a metre at the sea's level
uniform vec2 u_haze_scale;           // and their scale heights, metres
uniform vec2 u_eye;                  // the eye's plane before the target, metres, and the sine of the pitch
uniform vec3 u_haze_levels;          // where haze levels 1 to 3 begin
in vec2 v_local;
in float v_depth;
in vec2 v_air;
out vec4 o_colour;

void main() {
    vec2 x = clamp(v_local / u_unit, vec2(0.0), vec2(256.0));
    ivec2 i0 = min(ivec2(floor(x)), ivec2(255));
    vec2 t = x - vec2(i0);
    float wet = mix(mix(texelFetch(u_sea, i0, 0).r, texelFetch(u_sea, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_sea, i0 + ivec2(0, 1), 0).r, texelFetch(u_sea, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    if (wet <= 0.0) {
        discard;
    }
    float ground = mix(mix(texelFetch(u_heights, i0, 0).r, texelFetch(u_heights, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_heights, i0 + ivec2(0, 1), 0).r, texelFetch(u_heights, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    float k = water_light(u_level - ground, -u_fwd.z, u_water_light);
    float sigma = k * sky_factor(1.0, 1.0);
    float tau = k * sun_factor(0.0, u_light_tan, u_light_dir.z);
    float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, u_water.y);
    int step = light_step(s, fwidth(s), u_water.y, art_pixel() + u_dither);
    // Where the map look takes the ground, the water is as clean of haze as the map (PRE-29).
    float hz = haze(v_air.x, v_air.y, u_haze_beta, u_haze_scale, u_eye);
    int level = haze_level(hz, u_haze_levels, fwidth(hz), art_pixel() + u_dither);
    bool map = bayer(art_pixel() + u_dither) < map_weight(1.0 / u_inv_texel);
    int flags = (tau > 0.0 ? FLAG_SUNLIT : 0) | ((map ? 0 : level) << FLAG_HAZE_SHIFT);
    o_colour = pack_out(u_water.x + step, CAT_WATER, flags, v_depth);
}
