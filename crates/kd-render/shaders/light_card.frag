// The light card (A11.12), drawn as palette indices into colour 0: each look's ladder as a row of swatches, and a
// 2 m block of one look turning in front of void, its faces lit by the sky and the sun through the same formulas as
// every lit pixel (A11.3). Positions are art pixels from the screen's top-left corner, worked out on the CPU (A11.13
// rule 1).
uniform ivec2 u_top_left;   // the art pixel at the screen's top-left: its column, and its row from the bottom
uniform ivec2 u_swatches;   // the first row's first swatch
uniform int u_looks;        // how many ladders follow, none for the block alone
uniform ivec2 u_ladders[CARD_MAX_LOOKS];  // each look's first palette index and its number of steps
uniform ivec2 u_block_at;   // the block's centre on the screen
uniform float u_px_per_m;   // art pixels a metre
uniform vec2 u_turn;        // the block's turn about the vertical: its cosine and sine
uniform vec2 u_pitch;       // the view's angle below the horizon: its sine and cosine
uniform ivec2 u_block;      // the block's look: its first palette index and its number of steps
uniform vec3 u_light_dir;   // toward the sun, or the moon at night (east, north, up)
uniform vec2 u_y;           // the luminance of the sky's light, and of the sun's facing it
uniform vec2 u_range;       // the ladders' path in lightness, deep shade to full sun
uniform int u_block_only;   // 1 for the block alone, the `block` golden
out vec4 o_colour;

// The swatch at screen pixel p, or 0.
int swatch(ivec2 p) {
    int x = p.x - u_swatches.x;
    int y = p.y - u_swatches.y;
    if (x < 0 || y < 0) return 0;
    int row = y / CARD_ROW;
    int in_row = y - row * CARD_ROW - CARD_NAME_H;
    if (row >= u_looks || in_row < 0 || in_row >= CARD_SWATCH) return 0;
    int k = x / CARD_SWATCH;
    return k < u_ladders[row].y ? u_ladders[row].x + k : 0;
}

void main() {
    ivec2 a = art_pixel();
    ivec2 p = ivec2(a.x - u_top_left.x, u_top_left.y - a.y);
    int index = u_block_only == 1 ? 0 : swatch(p);
    int flags = 0;
    int cat = CAT_VOID;
    float depth = 1.0;
    if (index == 0) {
        // An orthographic ray through the pixel's centre, from in front of the block, in metres from its centre.
        vec2 m = vec2(float(p.x - u_block_at.x) + 0.5, float(u_block_at.y - p.y) - 0.5) / u_px_per_m;
        vec3 fwd = vec3(0.0, u_pitch.y, -u_pitch.x);
        vec3 up = vec3(0.0, u_pitch.x, u_pitch.y);
        vec3 ro = vec3(m.x, 0.0, 0.0) + m.y * up - 4.0 * fwd;
        // Into the block's own frame, turned back about the vertical; it spans -1 to 1 there.
        vec3 o = vec3(u_turn.x * ro.x + u_turn.y * ro.y, -u_turn.y * ro.x + u_turn.x * ro.y, ro.z);
        vec3 d = vec3(u_turn.x * fwd.x + u_turn.y * fwd.y, -u_turn.y * fwd.x + u_turn.x * fwd.y, fwd.z);
        d = vec3(abs(d.x) < 1e-6 ? 1e-6 : d.x, abs(d.y) < 1e-6 ? 1e-6 : d.y, abs(d.z) < 1e-6 ? 1e-6 : d.z);
        vec3 t1 = (vec3(-1.0) - o) / d;
        vec3 t2 = (vec3(1.0) - o) / d;
        vec3 tn = min(t1, t2);
        vec3 tf = max(t1, t2);
        float near = max(max(tn.x, tn.y), tn.z);
        float far = min(min(tf.x, tf.y), tf.z);
        if (near <= far && far > 0.0) {
            vec3 n = tn.x >= tn.y && tn.x >= tn.z ? vec3(-sign(d.x), 0.0, 0.0)
                : tn.y >= tn.z ? vec3(0.0, -sign(d.y), 0.0) : vec3(0.0, 0.0, -sign(d.z));
            // Back to the world, then the face's light: the sky it sees and the sun on it. A flat face has no
            // gradient, so no band and no dither (A11.3).
            vec3 nw = vec3(u_turn.x * n.x - u_turn.y * n.y, u_turn.y * n.x + u_turn.x * n.y, n.z);
            float sigma = (1.0 + nw.z) / 2.0;
            float tau = max(dot(nw, u_light_dir), 0.0);
            float s = ladder_pos(lightness(sigma, tau, u_y.x, u_y.y), u_range, u_block.y);
            index = u_block.x + light_step(s, 0.0, u_block.y, p);
            cat = CAT_ROCK;
            flags = tau > 0.0 ? FLAG_SUNLIT : 0;
            depth = near / 8.0;
        }
    }
    o_colour = pack_out(index, cat, flags, depth);
}
