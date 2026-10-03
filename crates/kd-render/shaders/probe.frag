// The probe scene (A11.13 rule 2): one art pixel per fixed input, through the same formulas as every lit pixel, so
// the read-back indices can be compared with the Rust twins exactly.
uniform sampler2D u_inputs;  // R32F, PROBE_W * PROBE_H wide, 4 rows: sigma, tau, band, steps
uniform vec2 u_y;            // the light's luminance from the sky and from the sun facing it
uniform vec2 u_range;        // the path's range in lightness
out vec4 o_colour;

void main() {
    ivec2 p = art_pixel();
    int i = p.y * PROBE_W + p.x;
    float sigma = texelFetch(u_inputs, ivec2(i, 0), 0).r;
    float tau = texelFetch(u_inputs, ivec2(i, 1), 0).r;
    float band = texelFetch(u_inputs, ivec2(i, 2), 0).r;
    int steps = int(texelFetch(u_inputs, ivec2(i, 3), 0).r + 0.5);
    float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, steps);
    o_colour = pack_out(light_step(s, band, steps, p), CAT_ROCK, 0, 0.0);
}
