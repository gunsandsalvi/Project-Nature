// Shared by every program (A11.13): it follows the generated defines, so it may use any of them. Each stage
// defines KD_VERTEX or KD_FRAGMENT first, so code for one stage stays out of the other.
// The per-pixel formulas here have Rust twins of the same names in `pixel.rs` (A11.13 rule 2), which the probe
// scene checks give the same steps exactly.

// The 4 x 4 Bayer threshold at a world pixel, (k + 0.5) / 16, as pixel::bayer.
float bayer(ivec2 p) {
    const int m[16] = int[16](0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5);
    return (float(m[(p.y & 3) * 4 + (p.x & 3)]) + 0.5) / 16.0;
}

// The share of the light's disc showing over a horizon of slope `horizon` toward it, for light at slope `tan_e`
// (A11.4, A11.5), as pixel::sunlit.
float sunlit(float horizon, float tan_e) {
    return clamp((tan_e - horizon) / (SUN_TAN * max(1.0 + tan_e * horizon, 1e-6)) + 0.5, 0.0, 1.0);
}

// The sun factor tau: the sunlit share times how squarely the surface faces the light, as pixel::sun_factor.
float sun_factor(float horizon, float tan_e, float n_dot_l) {
    return sunlit(horizon, tan_e) * max(n_dot_l, 0.0);
}

// The sky factor sigma: the share of the sky the horizon leaves open times how much of it the surface faces, as
// pixel::sky_factor.
float sky_factor(float open, float n_up) {
    return open * (1.0 + n_up) / 2.0;
}

// The light's lightness at sky factor sigma and sun factor tau (A11.3), as pixel::lightness.
float lightness(float sigma, float tau, float y_sky, float y_sun) {
    return pow(sigma * y_sky + tau * y_sun, 1.0 / 3.0);
}

// Where lightness l falls on a ladder of `steps` over the path's range, as pixel::ladder_pos.
float ladder_pos(float l, vec2 range, int steps) {
    return (l - range.x) / (range.y - range.x) * float(steps - 1);
}

// The step at ladder position s: the nearest, except within `band` of a threshold, where the Bayer pattern at
// world pixel p mixes the two steps (A11.3), as pixel::light_step.
int light_step(float s, float band, int steps, ivec2 p) {
    float top = float(steps - 1);
    s = clamp(s, 0.0, top);
    float k = floor(s);
    float d = s - k - 0.5;
    bool up = (band > 0.0 && abs(d) < band) ? (d + band) / (2.0 * band) > bayer(p) : d >= 0.0;
    return min(int(k) + (up ? 1 : 0), steps - 1);
}

// Colour 0 (A11.2): R the palette index, G the category and flags, B and A the view depth in 16 bits, as
// pixel::pack.
vec4 pack_out(int index, int cat, int flags, float depth) {
    int d = int(clamp(depth, 0.0, 1.0) * 65535.0 + 0.5);
    int g = cat | (flags & ~7);
    return vec4(float(index), float(g), float(d >> 8), float(d & 255)) / 255.0;
}

// The palette index colour 0 holds.
int palette_index(vec4 colour0) {
    return int(colour0.r * 255.0 + 0.5);
}

// A lattice point's 32-bit hash, wrapping alike in GLSL and Rust, as pixel::hash3.
uint hash3(uint x, uint y, uint seed) {
    uint h = (x * 0x8da6b343u) ^ (y * 0xd8163841u) ^ (seed * 0xcb1ab31fu);
    h ^= h >> 15u;
    h *= 0x2c1b3c6du;
    h ^= h >> 12u;
    h *= 0x297a2d39u;
    return h ^ (h >> 15u);
}

// The noise's 8 gradient directions, 45 degrees apart, as pixel::GRADIENTS.
const float S2 = 0.70710677;
const vec2 GRADIENTS[8] = vec2[8](vec2(1.0, 0.0), vec2(S2, -S2), vec2(0.0, -1.0), vec2(-S2, -S2),
    vec2(-1.0, 0.0), vec2(-S2, S2), vec2(0.0, 1.0), vec2(S2, S2));

float fade5(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// Gradient noise at p in lattice units (p at least 0), within +-1, as pixel::noise2.
float noise2(vec2 p, uint seed) {
    vec2 c = floor(p);
    vec2 f = p - c;
    uint x = uint(int(c.x));
    uint y = uint(int(c.y));
    vec2 g00 = GRADIENTS[hash3(x, y, seed) & 7u];
    vec2 g10 = GRADIENTS[hash3(x + 1u, y, seed) & 7u];
    vec2 g01 = GRADIENTS[hash3(x, y + 1u, seed) & 7u];
    vec2 g11 = GRADIENTS[hash3(x + 1u, y + 1u, seed) & 7u];
    float n00 = g00.x * f.x + g00.y * f.y;
    float n10 = g10.x * (f.x - 1.0) + g10.y * f.y;
    float n01 = g01.x * f.x + g01.y * (f.y - 1.0);
    float n11 = g11.x * (f.x - 1.0) + g11.y * (f.y - 1.0);
    float u = fade5(f.x);
    float v = fade5(f.y);
    float a = n00 + (n10 - n00) * u;
    float b = n01 + (n11 - n01) * u;
    return clamp((a + (b - a) * v) * 1.4142135, -1.0, 1.0);
}

// How much of an octave of `wavelength` metres shows at art pixels of 1 / inv_texel metres: all of it from four
// pixels a wavelength, none below two (A11.1 rule 2), as pixel::octave_fade.
float octave_fade(float wavelength, float inv_texel) {
    return smoothstep(2.0, 4.0, wavelength * inv_texel);
}

// Up to two octaves at p metres, oct = (lambda0, 1/lambda0, lambda1, 1/lambda1), each faded by its size in art
// pixels, scaled by the unfaded total, as pixel::faded_noise.
float faded_noise(vec2 p, vec4 oct, float inv_texel, uint seed) {
    float sum = 0.0;
    float total = 0.0;
    float amp = 1.0;
    for (int k = 0; k < 2; k++) {
        float lambda = k == 0 ? oct.x : oct.z;
        float inv = k == 0 ? oct.y : oct.w;
        if (lambda <= 0.0) break;
        float n = noise2(vec2(p.x * inv, p.y * inv), seed ^ (uint(k) << 16u));
        sum += amp * octave_fade(lambda, inv_texel) * n;
        total += amp;
        amp *= 0.5;
    }
    return total > 0.0 ? sum / total : 0.0;
}

// The shift, east and south, at which surfaces are looked up at world-fixed position w, as pixel::edge_wobble.
vec2 edge_wobble(vec2 w, float inv_texel) {
    return EDGE_WOBBLE_M * vec2(faded_noise(w, EDGE_OCTAVES, inv_texel, uint(SEED_EDGE_X)),
        faded_noise(w, EDGE_OCTAVES, inv_texel, uint(SEED_EDGE_Y)));
}

// The four squares nearest q: the lowest one's column and row, at most max_base, and where q lies between their
// centres, as pixel::vote_base.
void vote_base(vec2 q, int max_base, out ivec2 base, out vec2 f) {
    base = clamp(ivec2(floor(q - 0.5)), ivec2(0), ivec2(max_base));
    f = clamp(q - 0.5 - vec2(base), vec2(0.0), vec2(1.0));
}

// The surface four squares vote for at f, ids in the order (0, 0), (1, 0), (0, 1), (1, 1), as pixel::vote4.
int vote4(vec2 f, ivec4 ids) {
    vec4 w = vec4((1.0 - f.x) * (1.0 - f.y), f.x * (1.0 - f.y), (1.0 - f.x) * f.y, f.x * f.y);
    int best = ids.x;
    float best_w = -1.0;
    for (int k = 0; k < 4; k++) {
        float t = 0.0;
        for (int m = 0; m < 4; m++) {
            t += ids[m] == ids[k] ? w[m] : 0.0;
        }
        if (t > best_w || (t == best_w && ids[k] < best)) {
            best_w = t;
            best = ids[k];
        }
    }
    return best;
}

// Which of a surface's looks the split noise v picks, as pixel::split_look.
int split_look(float v, vec2 at, int looks) {
    return int(looks > 1 && v > at.x) + int(looks > 2 && v > at.y);
}

#ifdef KD_FRAGMENT
// The art pixel this fragment covers in the bound target, counted from its bottom-left corner.
ivec2 art_pixel() {
    return ivec2(gl_FragCoord.xy);
}
#endif
