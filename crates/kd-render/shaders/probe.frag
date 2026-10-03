// The probe scene (A11.13 rule 2): one art pixel per fixed input, through the same formulas as every lit pixel, so
// the read-back answers can be compared with the Rust twins exactly. The lowest third holds the light's steps, lit
// by what a point's fields and normal give; the middle third the surface the four nearest squares vote for at a
// place the edges' noise moved, and the look its split noise picks; the top third whether the plane test outlines a
// pixel and the haze's level.
uniform sampler2D u_inputs;  // R32F, PROBE_W * PROBE_H wide: the light's PROBE_LIGHT_ROWS rows, then the surfaces',
                             // then from PROBE_EDGE_ROW the top third's
uniform vec2 u_y;            // the light's luminance from the sky and from the sun facing it
uniform vec2 u_range;        // the path's range in lightness
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
        ivec4 ids = ivec4(int(input_at(i, r + 5) + 0.5), int(input_at(i, r + 6) + 0.5), int(input_at(i, r + 7) + 0.5),
            int(input_at(i, r + 8) + 0.5));
        vec4 oct = vec4(input_at(i, r + 10), input_at(i, r + 11), input_at(i, r + 12), input_at(i, r + 13));
        ivec2 base;
        vec2 f;
        vote_base(q + edge_wobble(w, inv_texel), 0, base, f);
        int surface = vote4(f, ids);
        int look = split_look(faded_noise(w, oct, inv_texel, uint(SEED_SPLIT)), vec2(input_at(i, r + 9), 2.0), 2);
        o_colour = pack_out(surface * 3 + look, CAT_GROUND, 0, 0.0);
    }
}
