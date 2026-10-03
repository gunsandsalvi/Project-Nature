// The probe scene (A11.13 rule 2): one art pixel per fixed input, through the same formulas as every lit pixel, so
// the read-back answers can be compared with the Rust twins exactly. The lower half holds the light's steps, the
// upper half the surface the four nearest squares vote for at a place the edges' noise moved, and the look its
// split noise picks.
uniform sampler2D u_inputs;  // R32F, PROBE_W * PROBE_H wide: rows 0-3 the light's inputs, 4-17 the surfaces'
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
        float sigma = input_at(i, 0);
        float tau = input_at(i, 1);
        float band = input_at(i, 2);
        int steps = int(input_at(i, 3) + 0.5);
        float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, steps);
        o_colour = pack_out(light_step(s, band, steps, p), CAT_ROCK, 0, 0.0);
    } else {
        int i = (p.y - PROBE_H) * PROBE_W + p.x;
        vec2 q = vec2(input_at(i, 4), input_at(i, 5));
        vec2 w = vec2(input_at(i, 6), input_at(i, 7));
        float inv_texel = input_at(i, 8);
        ivec4 ids = ivec4(int(input_at(i, 9) + 0.5), int(input_at(i, 10) + 0.5), int(input_at(i, 11) + 0.5),
            int(input_at(i, 12) + 0.5));
        vec4 oct = vec4(input_at(i, 14), input_at(i, 15), input_at(i, 16), input_at(i, 17));
        ivec2 base;
        vec2 f;
        vote_base(q + edge_wobble(w, inv_texel), 0, base, f);
        int surface = vote4(f, ids);
        int look = split_look(faded_noise(w, oct, inv_texel, uint(SEED_SPLIT)), vec2(input_at(i, 13), 2.0), 2);
        o_colour = pack_out(surface * 3 + look, CAT_GROUND, 0, 0.0);
    }
}
