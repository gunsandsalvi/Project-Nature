// The stones' and tufts' pixels (A11.5): each item lit as a whole by the sun and sky fields at its foot, blended
// between the four points round it as the ground's are (A11.4), and each face's or blade's own normal, its sky
// dimmed toward its foot, which the ground and its tuft hide, so a stone's side and a blade's foot are a step or
// two darker than its top or tip, with no dither; the category rock for a stone and plant for a tuft, and the
// haze's level as the ground's.
uniform highp sampler2D u_sun;       // R32F, 257 x 257: each point's horizon toward the light, as a slope
uniform highp sampler2D u_sky;       // R8, 257 x 257: the share of the sky each point's horizon leaves open
uniform int u_kind;                  // COVER_STONE or COVER_TUFT
uniform ivec2 u_dither;              // the art target's corner in the world's art pixels, modulo 4
uniform vec3 u_light_dir;            // toward the sun, or the moon at night (east, north, up)
uniform float u_light_tan;           // the light's slope, tan e
uniform vec2 u_y;                    // the luminance of the sky's light, and of the sun's facing it
uniform vec2 u_range;                // the ladders' path in lightness, deep shade to full sun
uniform vec2 u_haze_beta;            // the aerosol's and the air's extinction a metre at the sea's level
uniform vec2 u_haze_scale;           // and their scale heights, metres
uniform vec2 u_eye;                  // the eye's plane before the target, metres, and the sine of the pitch
uniform vec3 u_haze_levels;          // where haze levels 1 to 3 begin
flat in vec3 v_normal;
flat in ivec2 v_ladder;
flat in vec2 v_foot;
in float v_sky;
in float v_depth;
in vec2 v_air;
out vec4 o_colour;

void main() {
    vec2 x = clamp(v_foot, vec2(0.0), vec2(256.0));
    ivec2 i0 = min(ivec2(floor(x)), ivec2(255));
    vec2 t = x - vec2(i0);
    float horizon = mix(mix(texelFetch(u_sun, i0, 0).r, texelFetch(u_sun, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_sun, i0 + ivec2(0, 1), 0).r, texelFetch(u_sun, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    float open = mix(mix(texelFetch(u_sky, i0, 0).r, texelFetch(u_sky, i0 + ivec2(1, 0), 0).r, t.x),
        mix(texelFetch(u_sky, i0 + ivec2(0, 1), 0).r, texelFetch(u_sky, i0 + ivec2(1, 1), 0).r, t.x), t.y);
    float sigma = sky_factor(open * v_sky, v_normal.z);
    float tau = sun_factor(horizon, u_light_tan, dot(v_normal, u_light_dir));
    float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, v_ladder.y);
    int step = light_step(s, 0.0, v_ladder.y, art_pixel() + u_dither);
    float hz = haze(v_air.x, v_air.y, u_haze_beta, u_haze_scale, u_eye);
    int flags = (tau > 0.0 ? FLAG_SUNLIT : 0) | (haze_level(hz, u_haze_levels, fwidth(hz), art_pixel() + u_dither)
        << FLAG_HAZE_SHIFT);
    o_colour = pack_out(v_ladder.x + step, u_kind == COVER_STONE ? CAT_ROCK : CAT_PLANT, flags, v_depth);
}
