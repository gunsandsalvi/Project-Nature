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

// Colour 0's category and flags (its G channel), and its view depth, 0 near to 1 far.
int cat_flags(vec4 colour0) {
    return int(colour0.g * 255.0 + 0.5);
}
float depth_of(vec4 colour0) {
    return (floor(colour0.b * 255.0 + 0.5) * 256.0 + floor(colour0.a * 255.0 + 0.5)) / 65535.0;
}

// Whether a pixel at `depth` stands in front of its neighbour at `far`: the neighbour lies beyond where the plane
// through the pixel and its opposite neighbour puts it by more than `gap` (A11.2), as pixel::outline_toward.
bool outline_toward(float depth, float far, float opposite, float gap) {
    return far - (2.0 * depth - opposite) > gap;
}

// The haze at a point `depth` metres beyond the view's target plane and `height` metres above the sea: the air's
// optical depth along its ray to the eye's plane, in closed form (A11.4), as pixel::haze.
float haze(float depth, float height, vec2 beta, vec2 scale, vec2 eye) {
    float rise = max(eye.y, 1e-3);
    float l = max(eye.x + depth, 0.0);
    float tau = 0.0;
    for (int k = 0; k < 2; k++) {
        tau += beta[k] * scale[k] / rise * exp(-height / scale[k]) * (1.0 - exp(-l * rise / scale[k]));
    }
    return 1.0 - exp(-tau);
}

// The haze's level, 0 to 3: how many of `levels` it is above, dithered within `band` (A11.4), as pixel::haze_level.
int haze_level(float hz, vec3 levels, float band, ivec2 p) {
    float b = bayer(p);
    int n = 0;
    for (int k = 0; k < 3; k++) {
        float d = hz - levels[k];
        bool up = (band > 0.0 && abs(d) < band) ? (d + band) / (2.0 * band) > b : d >= 0.0;
        n += up ? 1 : 0;
    }
    return n;
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

// One coverage texture's four shares at q (metres from its corner, 1 m texels at level 0, side to a side) at whole
// level l: bilinear between the four texels round q, clamped at the edge, read by texelFetch so every GPU gives the
// twin's numbers.
vec4 cover_bilinear(highp sampler2D tex, vec2 q, int l, int side) {
    int n = max(side >> l, 1);
    vec2 p = q * (1.0 / float(1 << l)) - 0.5;
    vec2 i = floor(p);
    vec2 f = p - i;
    ivec2 lo = clamp(ivec2(i), ivec2(0), ivec2(n - 1));
    ivec2 hi = clamp(ivec2(i) + 1, ivec2(0), ivec2(n - 1));
    vec4 a = texelFetch(tex, lo, l);
    vec4 b = texelFetch(tex, ivec2(hi.x, lo.y), l);
    vec4 c = texelFetch(tex, ivec2(lo.x, hi.y), l);
    vec4 d = texelFetch(tex, hi, l);
    vec4 upper = a + (b - a) * f.x;
    vec4 lower = c + (d - c) * f.x;
    return upper + (lower - upper) * f.y;
}

// The shares at q read at mip level `level` of a coverage `top` levels deep: bilinear at the levels either side,
// mixed by the level's fraction, as pixel::cover_sample.
vec4 cover_sample(highp sampler2D tex, vec2 q, float level, int side, int top) {
    int l0 = min(int(floor(max(level, 0.0))), top);
    int l1 = min(l0 + 1, top);
    float t = level - float(l0);
    vec4 lo = cover_bilinear(tex, q, l0, side);
    if (t <= 0.0 || l1 == l0) {
        return lo;
    }
    vec4 hi = cover_bilinear(tex, q, l1, side);
    return lo + (hi - lo) * t;
}

// The surface with the largest share among the eight channels holding one (ids -1 for none); a tie goes to the
// earlier channel, the lower number, as pixel::cover_pick.
int cover_pick(vec4 a, vec4 b, ivec4 ida, ivec4 idb) {
    int best = -1;
    float best_v = -1.0;
    for (int k = 0; k < 4; k++) {
        if (ida[k] >= 0 && a[k] > best_v) {
            best = ida[k];
            best_v = a[k];
        }
    }
    for (int k = 0; k < 4; k++) {
        if (idb[k] >= 0 && b[k] > best_v) {
            best = idb[k];
            best_v = b[k];
        }
    }
    return best;
}

// The micro-relief's tilt of the ground at world-fixed w metres, a slope east and south: relief is (lambda0,
// 1/lambda0, octaves, each octave's greatest tilt), each octave half the last's wavelength and as strong in slope,
// faded below four art pixels, as pixel::relief_tilt.
vec2 relief_tilt(vec2 w, vec4 relief, float inv_texel) {
    float lambda = relief.x;
    float inv = relief.y;
    int octaves = min(int(relief.z + 0.5), RELIEF_OCTAVES);
    vec2 sum = vec2(0.0);
    for (int k = 0; k < RELIEF_OCTAVES; k++) {
        if (k >= octaves) {
            break;
        }
        float fade = octave_fade(lambda, inv_texel);
        if (fade > 0.0) {
            vec2 p = vec2(w.x * inv, w.y * inv);
            uint s = uint(k) << 16u;
            sum.x += fade * noise2(p, uint(SEED_RELIEF_X) ^ s);
            sum.y += fade * noise2(p, uint(SEED_RELIEF_Y) ^ s);
        }
        lambda *= 0.5;
        inv *= 2.0;
    }
    return vec2(sum.x * relief.w, sum.y * relief.w);
}

// The ground's normal (east, north, up) where its slope east and south is slope, tilted by tilt, as
// pixel::ground_normal.
vec3 ground_normal(vec2 slope, vec2 tilt) {
    vec2 g = slope + tilt;
    vec3 v = vec3(-g.x, g.y, 1.0);
    return v * (1.0 / sqrt(dot(v, v)));
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
