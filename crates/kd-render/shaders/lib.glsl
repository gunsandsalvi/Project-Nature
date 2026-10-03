// Shared by every program (A11.13): it follows the generated defines, so it may use any of them. Each stage
// defines KD_VERTEX or KD_FRAGMENT first, so code for one stage stays out of the other.
// The per-pixel formulas here have Rust twins of the same names in `pixel.rs` (A11.13 rule 2), which the probe
// scene checks give the same steps exactly.

// The 4 x 4 Bayer threshold at a world pixel, (k + 0.5) / 16, as pixel::bayer.
float bayer(ivec2 p) {
    const int m[16] = int[16](0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5);
    return (float(m[(p.y & 3) * 4 + (p.x & 3)]) + 0.5) / 16.0;
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

#ifdef KD_FRAGMENT
// The art pixel this fragment covers in the bound target, counted from its bottom-left corner.
ivec2 art_pixel() {
    return ivec2(gl_FragCoord.xy);
}
#endif
