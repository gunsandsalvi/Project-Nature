// A water line's pixels (A11.6): the water's look, its light by the line's depth and the view's angle as the sea's
// (water_light), lit as level water, one step along the whole line since its light does not change; hazed as the
// ground.
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
flat in float v_water_depth;
in float v_depth;
in vec2 v_air;
out vec4 o_colour;

void main() {
    float k = water_light(v_water_depth, -u_fwd.z, u_water_light);
    float sigma = k * sky_factor(1.0, 1.0);
    float tau = k * sun_factor(0.0, u_light_tan, u_light_dir.z);
    float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, u_water.y);
    int step = light_step(s, 0.0, u_water.y, art_pixel() + u_dither);
    // Where the map look takes the ground, the water is as clean of haze as the map (PRE-29).
    float hz = haze(v_air.x, v_air.y, u_haze_beta, u_haze_scale, u_eye);
    int level = haze_level(hz, u_haze_levels, fwidth(hz), art_pixel() + u_dither);
    bool map = bayer(art_pixel() + u_dither) < map_weight(1.0 / u_inv_texel);
    int flags = (tau > 0.0 ? FLAG_SUNLIT : 0) | ((map ? 0 : level) << FLAG_HAZE_SHIFT);
    o_colour = pack_out(u_water.x + step, CAT_WATER, flags, v_depth);
}
