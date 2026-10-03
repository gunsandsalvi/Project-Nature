// Shared fragment code, prepended to every scene shader after the generated defines (A11.1, A11.3): the mockup's
// `noise` and `common` blocks from mockups/visual-style.html, written in GLSL ES 3.00 by its convert() rules
// (texture2D -> texture, varying -> in). Ramps (ladders), tables, dithering, light, shadow and output packing.
// `packShadow` stays out: the shadow pass writes a depth texture (A11.2), which shadowAt samples directly.
uniform sampler2D uRamps;
uniform sampler2D uLuts;
uniform sampler2D uShadow;
uniform vec2 uDith;
uniform float uBand;
uniform float uScreenDither;
uniform float uTexel;
uniform vec3 uSunDir;
uniform float uSunI;
uniform float uAmb;
uniform vec4 uFire;
uniform vec4 uFire2;
uniform mat4 uLightVP;
uniform float uShadowOn;
uniform vec2 uShadowBias;
uniform vec3 uShadowWin;
uniform vec3 uCamF;
uniform vec2 uDepthR;
uniform vec4 uCut;
uniform float uCutOn;
uniform vec3 uHaze;
uniform float uWinter;
uniform float uTime;
uniform float uMap;
uniform float uPitchC;
in vec3 vWorld;
float hash12(vec2 p) { p = mod(p, 289.0); vec3 p3 = fract(vec3(p.xyx) * 0.1031); p3 += dot(p3, p3.yzx + 33.33); return fract((p3.x + p3.y) * p3.z); }
float hash13(vec3 p) { p = mod(p, 289.0); p = fract(p * 0.1031); p += dot(p, p.zyx + 31.32); return fract((p.x + p.y) * p.z); }
float vnoise(vec2 p) {
  vec2 i = floor(p), f = fract(p), u = f * f * (3.0 - 2.0 * f);
  return mix(mix(hash12(i), hash12(i + vec2(1.0, 0.0)), u.x), mix(hash12(i + vec2(0.0, 1.0)), hash12(i + vec2(1.0, 1.0)), u.x), u.y);
}
float fbm3(vec2 p) { return 0.5 * vnoise(p) + 0.3 * vnoise(p * 2.03 + 17.1) + 0.2 * vnoise(p * 4.1 - 9.7); }
float bayer2(vec2 a) { a = floor(a); return fract(dot(a, vec2(0.5, a.y * 0.75))); }
float bayer() {
  vec2 a = floor(gl_FragCoord.xy) + (uScreenDither > 0.5 ? vec2(0.0) : uDith);
  return bayer2(0.5 * a) * 0.25 + bayer2(a) + 0.03125;
}
/* Dither only in a strip about two pixels wide where one shade meets the next. */
float gBand = 0.16;
void setBand(float lit) {
#ifdef HAS_DERIV
  gBand = clamp(fwidth(lit) * 3.5, 0.03, 0.3) * uBand / 0.32;
#else
  gBand = 0.16;
#endif
  if (uScreenDither > 0.5) gBand = 1.0;
}
uniform float uDbgA; uniform float uDbgB;
void setBandV(float v) {
#ifdef HAS_DERIV
  gBand = clamp(fwidth(v) * 7.0, 0.03, 0.3) * uBand / 0.32;
#endif
  if (uScreenDither > 0.5) gBand = 1.0;
}
float qlevel(float L, float b) { return floor(L + 0.5 + (b - 0.5) * gBand * (1.0 - uDbgA)); }
float rampLen(float r) { return floor(texture(uRamps, vec2(0.5 / 8.0, (r + 0.5) / RAMP_ROWS)).g * 255.0 + 0.5); }
float rampAt(float r, float i) { return floor(texture(uRamps, vec2((i + 0.5) / 8.0, (r + 0.5) / RAMP_ROWS)).r * 255.0 + 0.5); }
float rampPick(float r, float v, float b) {
  float n = rampLen(r);
  float i = clamp(qlevel(clamp(v, 0.0, 1.0) * (n - 1.0), b), 0.0, n - 1.0);
  return rampAt(r, i);
}
float lut(float row, float idx) { return floor(texture(uLuts, vec2((idx + 0.5) / 256.0, (row + 0.5) / 16.0)).r * 255.0 + 0.5); }
float shadowAt(vec3 wp, vec3 n) {
  if (uShadowOn < 0.5) return 1.0;
  float ndl = clamp(dot(n, uSunDir), 0.0, 1.0);
  float k = 1.0 + 2.5 * (1.0 - ndl);
  vec4 lp = uLightVP * vec4(wp + n * uShadowBias.x * k, 1.0);
  vec3 q = lp.xyz * 0.5 + 0.5;
  // the light's projection spans uShadowWin.x texels, the map's corner at uShadowWin.yz in it, so the map's texel is
  // found in whole numbers; the map is a depth texture read directly (A11.2), not the mockup's RGBA8 packing
  ivec2 t = ivec2(floor(q.xy * uShadowWin.x)) - ivec2(uShadowWin.yz);
  ivec2 size = textureSize(uShadow, 0);
  if (t.x < 0 || t.y < 0 || t.x >= size.x || t.y >= size.y) return 1.0;
  return q.z - uShadowBias.y * k > texelFetch(uShadow, t, 0).r ? 0.0 : 1.0;
}
/* Sun on a surface, normalised so flat open ground in sun reads 1. */
float sunLight(vec3 n, float sh) {
  float ndl = dot(n, uSunDir);
  return clamp(ndl / max(uSunDir.y, 0.3), 0.0, 1.7) * smoothstep(0.02, 0.14, ndl) * sh * uSunI;
}
float sky(vec3 n) { return uAmb * (0.6 + 0.4 * n.y); }
/* Warm light from the hearth (and the small one inside the cave), 0..1, falling off like a real flame. */
float gFireW = 0.02;
float fireLight(vec3 wp, vec3 n) {
  vec3 d = uFire.xyz - wp; float dist = length(d);
  float f = uFire.w * max(1.0 / (1.0 + dist * dist * 0.2) - 0.055, 0.0) * (0.42 + 0.58 * clamp(dot(n, d / max(dist, 0.01)), 0.0, 1.0));
  vec3 d2 = uFire2.xyz - wp; float dist2 = length(d2);
  f += uFire2.w * max(1.0 / (1.0 + dist2 * dist2 * 0.32) - 0.06, 0.0) * (0.4 + 0.6 * clamp(dot(n, d2 / max(dist2, 0.01)), 0.0, 1.0));
  f = clamp(f, 0.0, 1.0);
#ifdef HAS_DERIV
  gFireW = fwidth(f);
#endif
  return f;
}
float viewDepth() { return clamp((dot(uCamF, vWorld) - uDepthR.x) * uDepthR.y, 0.0, 1.0); }
float hazeAmt() { return clamp((dot(uCamF, vWorld) - uHaze.x) * uHaze.y, 0.0, 1.0) * uHaze.z; }
/* Detail of a given size shows only when it spans enough art pixels. */
float detail(float size) { return smoothstep(1.1, 2.2, size / uTexel); }
void cutTest() { if (uCutOn > 0.5 && dot(vWorld, uCut.xyz) > uCut.w) discard; }
/* Light level to palette index, then firelight and haze as palette-to-palette steps. */
float gWarmGain = 4.4, gWarmMax = 3.0;
float finish(float idx, float fire, float b) {
  // warm steps are clean rings: the blend between two steps is measured on the firelight itself,
  // so it is never more than about one pixel wide, whatever the angle or the light around it
  float wv = fire * gWarmGain - 0.1;
  float wband = clamp(gFireW * gWarmGain * 0.9, 0.0, 0.45);
  if (uScreenDither > 0.5) wband = 1.0;
  float wl = clamp(floor(wv + 0.5 + (b - 0.5) * wband), 0.0, gWarmMax);
  if (wl > 0.5) idx = lut(wl - 1.0, idx);
  float hl = clamp(qlevel(hazeAmt(), b), 0.0, 3.0);
  if (hl > 0.5) idx = lut(L_HAZE1 + hl - 1.0, idx);
  return idx;
}
vec4 packOut(float idx, float cat, float flags) {
  float d16 = floor(viewDepth() * 65535.0);
  float hi = floor(d16 / 256.0);
  return vec4(idx / 255.0, (cat + 8.0 * flags) / 255.0, hi / 255.0, (d16 - hi * 256.0) / 255.0);
}
/* Tiny grass tufts: points fixed in the world, each drawn as the same few pixels on screen.
   Returns 2 = highlight, 1 = mid, -1 = shadow, 0 = none. */
// uView is the scene pass's viewport (x, y, width, height): the projection spans its width and height (A11.2).
// The stamps take positions fixed to the world (metres from the floating origin plus uWorldOff, where the origin lies
// within an 8,192 m block of the world) and take uWorldOff off again to project them with uVP.
uniform mat4 uVP; uniform vec4 uView; uniform vec2 uWorldOff;
vec2 pixelOf(vec4 q) { return floor((q.xy / q.w * 0.5 + 0.5) * uView.zw + uView.xy); }
float stampTuft(vec2 p, float z, float cell, float dens) {
  if (uTexel > 0.095) return 0.0;
  vec2 base = floor(p / cell - 0.5);
  float r = 0.0;
  for (int j = 0; j < 2; j++) for (int i = 0; i < 2; i++) {
    vec2 c = base + vec2(float(i), float(j));
    if (hash12(c + 7.0) > dens) continue;
    vec2 w = (c + 0.2 + 0.6 * vec2(hash12(c + 11.0), hash12(c + 19.0))) * cell;
    vec4 q = uVP * vec4(w.x - uWorldOff.x, z, w.y - uWorldOff.y, 1.0);
    vec2 px = pixelOf(q);
    vec2 o = floor(gl_FragCoord.xy) - px;
    float tall = hash12(c + 23.0) > 0.5 ? 1.0 : 0.0;
    if (o.y == 0.0 && abs(o.x) <= 1.0) r = -1.0;
    else if (o.y == 1.0 && abs(o.x) == 1.0) r = 1.0;
    else if (o.y == 1.0 && o.x == 0.0) r = 2.0;
    else if (o.y == 2.0 && o.x == 0.0 && tall > 0.5) r = 2.0;
    else if (o.y == 2.0 && o.x == -1.0 && tall < 0.5) r = 2.0;
  }
  return r;
}
/* Pebbles and stones fixed in the world, each drawn as a crisp little pixel shape about sizeM across.
   Returns 3 = lit top, 2 = body, 1 = dark lower edge, 0 = none. */
float stampStone(vec2 p, float z, float cell, float dens, float sizeM, float seed) {
  float r = 0.0;
  vec2 base = floor(p / cell - 0.5);
  for (int j = 0; j < 2; j++) for (int i = 0; i < 2; i++) {
    vec2 c = base + vec2(float(i), float(j));
    if (hash12(c + seed) > dens) continue;
    vec2 w = (c + 0.15 + 0.7 * vec2(hash12(c + seed + 11.0), hash12(c + seed + 19.0))) * cell;
    float wpx = floor(clamp(sizeM * (0.55 + 0.9 * hash12(c + seed + 5.0)) / uTexel, 0.0, 9.0) + 0.5);
    if (wpx < 1.0) continue;
    float hpx = max(1.0, floor(wpx * 0.55 + 0.25));
    vec4 q = uVP * vec4(w.x - uWorldOff.x, z, w.y - uWorldOff.y, 1.0);
    vec2 o = floor(gl_FragCoord.xy) - pixelOf(q) + vec2(floor(wpx * 0.5), 0.0);
    if (o.x < 0.0 || o.x > wpx - 1.0 || o.y < 0.0 || o.y > hpx - 1.0) continue;
    if (wpx > 2.5 && hpx > 1.5 && (o.x < 0.5 || o.x > wpx - 1.5) && (o.y < 0.5 || o.y > hpx - 1.5)) continue;
    float lvl = o.y > hpx - 1.5 ? 3.0 : (o.y < 0.5 ? 1.0 : 2.0);
    if (hpx < 1.5) lvl = (o.x < 0.5 || wpx < 1.5) ? 3.0 : 2.0;
    r = max(r, lvl);
  }
  return r;
}
