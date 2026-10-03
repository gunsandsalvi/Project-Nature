// The probe scene (A11.13 rule 2): one art pixel per fixed input, through the same formulas as every lit pixel, so
// the read-back answers can be compared with the Rust twins exactly. The lowest third holds the light's steps, lit
// by what a point's fields and normal give; the middle third the surface with the largest share of a fixture's
// coverage, read at a mip level at a place the edges' noise moved, and the look its split noise picks; the third band
// whether the plane test outlines a pixel and the haze's level; the top band the light's step of a point whose
// normal a surface's micro-relief tilts.
uniform sampler2D u_inputs;  // R32F, PROBE_W * PROBE_H wide: the light's PROBE_LIGHT_ROWS rows, then the surfaces',
                             // then from PROBE_EDGE_ROW the third band's, then from PROBE_RELIEF_ROW the relief's
uniform vec2 u_y;            // the light's luminance from the sky and from the sun facing it
uniform vec2 u_range;        // the path's range in lightness
uniform highp sampler2D u_cover0;  // the surface band's coverage, PROBE_COVER_SIDE wide at level 0, mipmapped
uniform highp sampler2D u_cover1;
uniform ivec4 u_cover_ids[2];      // the surface each channel holds
out vec4 o_colour;

float input_at(int i, int row) {
    return texelFetch(u_inputs, ivec2(i, row), 0).r;
}

void main() {
    ivec2 p = art_pixel();
    if (p.y < PROBE_H) {
        int i = p.y * PROBE_W + p.x;
        float sigma = sky_factor(input_at(i, 0), input_at(i, 1));
        float tau = sun_factor(input_at(i, 2), input_at(i, 3), input_at(i, 4));
        float band = input_at(i, 5);
        int steps = int(input_at(i, 6) + 0.5);
        float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, steps);
        o_colour = pack_out(light_step(s, band, steps, p), CAT_ROCK, 0, 0.0);
    } else if (p.y >= 3 * PROBE_H) {
        int i = (p.y - 3 * PROBE_H) * PROBE_W + p.x;
        int r = PROBE_RELIEF_ROW;
        vec3 l = vec3(input_at(i, r + 3), input_at(i, r + 4), input_at(i, r + 5));
        vec2 slope = vec2(input_at(i, r + 6), input_at(i, r + 7));
        vec2 w = vec2(input_at(i, r + 8), input_at(i, r + 9));
        vec4 relief = vec4(input_at(i, r + 11), input_at(i, r + 12), input_at(i, r + 13), input_at(i, r + 14));
        vec3 n = ground_normal(slope, relief_tilt(w, relief, input_at(i, r + 10)));
        float sigma = sky_factor(input_at(i, r), n.z);
        float tau = sun_factor(input_at(i, r + 1), input_at(i, r + 2), dot(n, l));
        int steps = int(input_at(i, r + 16) + 0.5);
        float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, steps);
        o_colour = pack_out(light_step(s, input_at(i, r + 15), steps, p), CAT_ROCK, 0, 0.0);
    } else if (p.y >= 2 * PROBE_H) {
        int i = (p.y - 2 * PROBE_H) * PROBE_W + p.x;
        int r = PROBE_EDGE_ROW;
        bool outline = outline_toward(input_at(i, r), input_at(i, r + 1), input_at(i, r + 2), input_at(i, r + 3));
        float hz = haze(input_at(i, r + 4), input_at(i, r + 5), PROBE_BETA, PROBE_SCALE, PROBE_EYE);
        int level = haze_level(hz, PROBE_LEVELS, input_at(i, r + 6), p);
        o_colour = pack_out(level * 2 + (outline ? 1 : 0), CAT_GROUND, 0, 0.0);
    } else {
        int i = (p.y - PROBE_H) * PROBE_W + p.x;
        int r = PROBE_LIGHT_ROWS;
        vec2 q = vec2(input_at(i, r), input_at(i, r + 1));
        vec2 w = vec2(input_at(i, r + 2), input_at(i, r + 3));
        float inv_texel = input_at(i, r + 4);
        float level = input_at(i, r + 5);
        vec4 oct = vec4(input_at(i, r + 7), input_at(i, r + 8), input_at(i, r + 9), input_at(i, r + 10));
        vec2 at = q + edge_wobble(w, inv_texel);
        int surface = cover_pick(cover_sample(u_cover0, at, level, PROBE_COVER_SIDE, PROBE_COVER_TOP),
            cover_sample(u_cover1, at, level, PROBE_COVER_SIDE, PROBE_COVER_TOP), u_cover_ids[0], u_cover_ids[1]);
        int look = split_look(faded_noise(w, oct, inv_texel, uint(SEED_SPLIT)), vec2(input_at(i, r + 6), 2.0), 2);
        o_colour = pack_out(surface * 3 + look, CAT_GROUND, 0, 0.0);
    }
}
