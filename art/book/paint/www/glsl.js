// Shader code for the painter. One geometry pass writes what each art pixel shows (material, step bias,
// object, normal, depth, position); the resolve pass turns light into a colour step (PRE-20, PRE-30); the final
// pass draws outlines and lit edges (PRE-21), grades the colour by the hour, and adds mist and haze.

export const NOISE = /* glsl */ `
float hash12(vec2 p){ vec3 p3 = fract(vec3(p.xyx) * .1031); p3 += dot(p3, p3.yzx + 33.33); return fract((p3.x + p3.y) * p3.z); }
float hash13(vec3 p3){ p3 = fract(p3 * .1031); p3 += dot(p3, p3.zyx + 31.32); return fract((p3.x + p3.y) * p3.z); }
float vnoise(vec2 p){ vec2 i = floor(p), f = fract(p); vec2 u = f * f * (3. - 2. * f);
  return mix(mix(hash12(i), hash12(i + vec2(1., 0.)), u.x), mix(hash12(i + vec2(0., 1.)), hash12(i + vec2(1., 1.)), u.x), u.y); }
float vnoise3(vec3 p){ vec3 i = floor(p), f = fract(p); vec3 u = f * f * (3. - 2. * f);
  float a = mix(mix(hash13(i), hash13(i + vec3(1,0,0)), u.x), mix(hash13(i + vec3(0,1,0)), hash13(i + vec3(1,1,0)), u.x), u.y);
  float b = mix(mix(hash13(i + vec3(0,0,1)), hash13(i + vec3(1,0,1)), u.x), mix(hash13(i + vec3(0,1,1)), hash13(i + vec3(1,1,1)), u.x), u.y);
  return mix(a, b, u.z); }
float fbm2(vec2 p){ float a = .5, s = 0.; for (int i = 0; i < 4; i++) { s += a * vnoise(p); p = p * 2.03 + 7.1; a *= .5; } return s / .9375; }
float fbm3(vec3 p){ float a = .5, s = 0.; for (int i = 0; i < 3; i++) { s += a * vnoise3(p); p = p * 2.03 + 7.1; a *= .5; } return s / .875; }
`;

// Patterns fixed to the surface: brushy clusters and lines inside shapes, as steps up or down (PRE-20).
const PATTERNS = /* glsl */ `
float patGround(vec3 p){
  // calm fields: a few large darker clumps, and now and then a small lighter or darker dab
  float b = 0.;
  float c = fbm2(p.xz * .11 + 3.);
  if (c < .31) b -= 1.;
  float d = vnoise(p.xz * 1.2 + 11.);
  if (d > .9 && c > .45) b += 1.;
  float e = vnoise(p.xz * 1.7 + 37.);
  if (e < .06 && c < .5) b -= 1.;
  return b;
}
float patRock(vec3 p, vec3 n){
  float b = 0.;
  float side = 1. - smoothstep(.55, .8, abs(n.y));
  float y = p.y + (vnoise(p.xz * .8) - .5) * .22;
  float bed = floor(y / .42);
  float f = fract(y / .42);
  if (f < .1 && side > .5 && vnoise(vec2(dot(p.xz, vec2(.7, .7)) * 1.3, bed)) > .3) b -= 1.;
  float cr = vnoise(vec2(dot(p.xz, vec2(.71, -.71)) * 3.1, bed * 7.3));
  if (cr > .93 && side > .5) b -= 1.;
  float cl = fbm3(p * 1.6);
  if (cl > .66) b += 1.; else if (cl < .3) b -= 1.;
  return b;
}
float patBark(vec3 l){
  float b = 0.;
  float a = l.x * 9. + vnoise(vec2(l.y * 2., l.x * 3.)) * 1.2;
  if (fract(a) < .32) b -= 1.;
  if (vnoise(vec2(l.x * 7., l.y * 1.4)) > .8) b += 1.;
  return b;
}
float patWood(vec3 l){ return vnoise(vec2(l.x * 14., l.y * 1.3)) > .78 ? -1. : 0.; }
float patHide(vec3 l){
  float b = 0.;
  if (fract(l.x / .85 + .5) < .06) b -= 1.;
  if (fract(l.y / .62) < .05) b -= 1.;
  float c = fbm2(l.xy * 1.4 + 5.);
  if (c > .66) b += 1.; else if (c < .3) b -= 1.;
  return b;
}
float patThatch(vec3 l){
  // courses of straw laid down the slope: a dark line under each course, streaks of straw within it
  float b = 0.;
  float row = l.y / .3 + (vnoise(vec2(l.x * .9, 3.)) - .5) * .35;
  if (fract(row) < .13) b -= 1.;
  float h = hash12(vec2(floor(l.x / .05), floor(row)));
  if (h > .8) b += 1.; else if (h < .1) b -= 1.;
  return b;
}
float patFace(vec3 l){
  // l is normalised to the head: x across, y up, z forward. Eyes on the front face.
  if (l.z > .45 && l.y > .02 && l.y < .24 && abs(l.x) > .1 && abs(l.x) < .36) return -3.;
  return 0.;
}
float patFur(vec3 l){
  float c = vnoise3(l * 9.);
  return c > .72 ? 1. : (c < .24 ? -1. : 0.);
}
float patSnow(vec3 p){
  float c = fbm2(p.xz * .16 + 9.);
  return 1. + (c > .7 ? 1. : 0.) - (vnoise(p.xz * 2.1) > .9 ? 1. : 0.);
}
float patSand(vec3 p){
  float r = (p.x * .8 + p.z * .35) / .5 + vnoise(p.xz * .6) * 1.5;
  float b = fract(r) < .16 ? -1. : 0.;
  if (vnoise(p.xz * 2.5) > .85) b += 1.;
  return b;
}
float patDirt(vec3 p){
  float b = 0.;
  float c = vnoise(p.xz * 3.2);
  if (c > .86) b += 1.; else if (c < .1) b -= 1.;
  if (fbm2(p.xz * .5) < .32) b -= 1.;
  return b;
}
float patMoss(vec3 p){ float c = vnoise3(p * 3.); return c > .7 ? 1. : (c < .25 ? -1. : 0.); }
float patCloth(vec3 l){ return (fract(l.y / .1) < .2) ? -1. : 0.; }
float patPlank(vec3 l){
  float b = 0.;
  if (fract(l.x / .22) < .14) b -= 1.;
  if (vnoise(vec2(l.x * 9., l.y * .8)) > .8) b += 1.;
  return b;
}
float patWattle(vec3 l){
  float b = 0.;
  float r = l.y / .12; float k = floor(r);
  float w = sin(l.x * 6.283 / .5 + k * 3.14159);
  if (fract(r) < .25) b -= 1.;
  if (w > .6) b += 1.; else if (w < -.6) b -= 1.;
  return b;
}
float pattern(int k, vec3 p, vec3 l, vec3 n){
  if (k == 1) return patGround(p);
  if (k == 2) return patRock(p, n);
  if (k == 3) return patBark(l);
  if (k == 4) return patWood(l);
  if (k == 5) return patHide(l);
  if (k == 6) return patThatch(l);
  if (k == 7) return patFace(l);
  if (k == 8) return patFur(l);
  if (k == 9) return patSnow(p);
  if (k == 10) return patSand(p);
  if (k == 11) return patDirt(p);
  if (k == 13) return patCloth(l);
  if (k == 14) return patMoss(p);
  if (k == 15) return patPlank(l);
  if (k == 16) return patWattle(l);
  return 0.;
}
`;

export const SOLID_VERT = /* glsl */ `
attribute float aMat; attribute float aBias; attribute float aObj; attribute float aPat; attribute float aFlag;
attribute vec3 aLoc; attribute vec4 aW;
varying vec3 vWorld; varying vec3 vNormal; varying vec3 vLoc; varying vec4 vW; varying float vDepth;
flat varying float vMat; flat varying float vObj; flat varying float vPat; flat varying float vFlag;
varying float vBias;
void main(){
  vec4 wp = modelMatrix * vec4(position, 1.);
  vWorld = wp.xyz;
  vNormal = normalize(mat3(modelMatrix) * normal);
  vMat = aMat; vBias = aBias; vObj = aObj; vPat = aPat; vFlag = aFlag; vLoc = aLoc; vW = aW;
  vec4 mv = viewMatrix * wp;
  vDepth = -mv.z;
  gl_Position = projectionMatrix * mv;
}`;

export const SOLID_FRAG = /* glsl */ `
precision highp float;
layout(location = 0) out vec4 o0;
layout(location = 1) out vec4 o1;
layout(location = 2) out vec4 o2;
uniform int uPass;
uniform vec4 uLayers; uniform vec4 uLayerPat;
uniform vec4 uSoot[4]; uniform int uSootN;
varying vec3 vWorld; varying vec3 vNormal; varying vec3 vLoc; varying vec4 vW; varying float vDepth;
flat varying float vMat; flat varying float vObj; flat varying float vPat; flat varying float vFlag;
varying float vBias;
${NOISE}
${PATTERNS}
void main(){
  if (uPass == 1) { o0 = vec4(vDepth, 0., 0., 1.); return; }
  vec3 n = normalize(vNormal); if (!gl_FrontFacing) n = -n;
  float m = vMat; float pat = vPat; float b = vBias;
  if (pat > 99.5) {
    float j = .5 * (vnoise(vWorld.xz * .9) - .5) + .35 * (vnoise(vWorld.xz * 3.1 + 5.) - .5);
    vec4 w = vW + vec4(j, -j, j * .7, -j * .7);
    int k = 0; float best = w.x;
    if (w.y > best) { best = w.y; k = 1; }
    if (w.z > best) { best = w.z; k = 2; }
    if (w.w > best) { best = w.w; k = 3; }
    m = uLayers[k]; pat = uLayerPat[k];
  }
  b += pattern(int(pat + .5), vWorld, vLoc, n);
  // soot above lived-in shelters, and damp stains (PRE-23): steps down near given points
  for (int i = 0; i < 4; i++) {
    if (i >= uSootN) break;
    vec3 d = (vWorld - uSoot[i].xyz) / vec3(1.3, 1., 1.);
    float k = 1. - length(d) / abs(uSoot[i].w);
    if (k > 0. && int(pat + .5) == 2) b -= floor((uSoot[i].w > 0. ? 2.6 : 1.4) * k * (.7 + .6 * vnoise3(vWorld * 2.)) + .3);
  }
  o0 = vec4(m + 1., b, vObj, vFlag);
  o1 = vec4(n, vDepth);
  o2 = vec4(vWorld, 1.);
}`;

// Cards: leaf clumps, grass tufts, flowers, reeds and flames, facing the camera (or the sun, for shadows).
export const CARD_VERT = /* glsl */ `
attribute vec3 aCenter; attribute vec2 aCorner; attribute vec2 aSize; attribute float aTile;
attribute float aMat; attribute float aBias; attribute float aObj; attribute float aFlag; attribute vec3 aNrm;
attribute float aUpright; attribute float aSpin;
uniform vec3 uRight; uniform vec3 uUp; uniform float uTiles;
varying vec2 vUV; varying vec3 vWorld; varying vec3 vN; varying float vDepth; varying vec3 vR; varying vec3 vU;
flat varying float vMat; flat varying float vObj; flat varying float vFlag; flat varying float vBias;
void main(){
  vec3 right = uRight, up = uUp;
  if (aUpright > .5) { right = normalize(vec3(uRight.x, 0., uRight.z)); up = vec3(0., 1., 0.); }
  float c = cos(aSpin), s = sin(aSpin);
  vec2 k = vec2(c * aCorner.x - s * aCorner.y, s * aCorner.x + c * aCorner.y);
  vec3 wp = aCenter + right * k.x * aSize.x + up * k.y * aSize.y;
  float tx = mod(aTile, uTiles), ty = floor(aTile / uTiles);
  vUV = (vec2(tx, ty) + aCorner + .5) / uTiles;
  vWorld = wp; vN = aNrm; vR = right; vU = up;
  vMat = aMat; vObj = aObj; vFlag = aFlag; vBias = aBias;
  vec4 mv = viewMatrix * vec4(wp, 1.);
  vDepth = -mv.z;
  gl_Position = projectionMatrix * mv;
}`;

export const CARD_FRAG = /* glsl */ `
precision highp float;
layout(location = 0) out vec4 o0;
layout(location = 1) out vec4 o1;
layout(location = 2) out vec4 o2;
uniform int uPass; uniform sampler2D tAtlas;
varying vec2 vUV; varying vec3 vWorld; varying vec3 vN; varying float vDepth; varying vec3 vR; varying vec3 vU;
flat varying float vMat; flat varying float vObj; flat varying float vFlag; flat varying float vBias;
void main(){
  vec4 t = texture(tAtlas, vUV);
  if (t.a < .5) discard;
  if (uPass == 1) { if (mod(floor(vFlag / 16.), 2.) > .5) discard; o0 = vec4(vDepth, 0., 0., 1.); return; }
  vec2 bump = (t.rg - .5) * 2.;
  vec3 n = normalize(vN + (vR * bump.x + vU * bump.y) * .9);
  float b = vBias + floor((t.b * 255. - 128.) / 40. + .5);
  o0 = vec4(vMat + 1., b, vObj, vFlag);
  o1 = vec4(n, vDepth);
  o2 = vec4(vWorld, 1.);
}`;

// Water surfaces go to their own buffer, so the ground beneath can show through.
export const WATER_VERT = /* glsl */ `
attribute vec3 aFlow;
varying vec3 vWorld; varying vec3 vFlow; varying float vDepth;
void main(){
  vec4 wp = modelMatrix * vec4(position, 1.);
  vWorld = wp.xyz; vFlow = aFlow;
  vec4 mv = viewMatrix * wp; vDepth = -mv.z;
  gl_Position = projectionMatrix * mv;
}`;
export const WATER_FRAG = /* glsl */ `
precision highp float;
layout(location = 0) out vec4 o0;
layout(location = 1) out vec4 o1;
uniform float uKind;
varying vec3 vWorld; varying vec3 vFlow; varying float vDepth;
void main(){
  o0 = vec4(vDepth, vWorld);
  o1 = vec4(vFlow, uKind);
}`;

export const QUAD_VERT = /* glsl */ `
varying vec2 vUv;
void main(){ vUv = uv; gl_Position = vec4(position.xy, 0., 1.); }`;

// Light becomes a step on the material's ladder (PRE-20, PRE-30).
export const RESOLVE_FRAG = /* glsl */ `
precision highp float;
layout(location = 0) out vec4 r0;
layout(location = 1) out vec4 r1;
uniform sampler2D tG0; uniform sampler2D tG1; uniform sampler2D tG2; uniform sampler2D tShadow;
uniform sampler2D tW0; uniform sampler2D tW1;
uniform sampler2D tSky; uniform mat4 uSkyView; uniform mat4 uSkyProj; uniform float uSkyCover;
uniform mat4 uLightView; uniform mat4 uLightProj; uniform vec2 uShadowSize; uniform float uShadowTexel;
uniform vec3 uSun; uniform float uSunK; uniform float uSkyK; uniform float uShift; uniform float uExposure;
uniform float uSunPow; uniform int uFires; uniform vec4 uFire[12]; uniform vec4 uFireC[12];
uniform vec2 uRes; uniform float uAORad; uniform float uAODist; uniform float uAOK;
uniform vec2 uHaze; uniform float uWaterOn; uniform float uWaterRow; uniform float uFoamRow; uniform float uGlintRow;
uniform float uSeaOn;
${NOISE}
float shadowAt(vec3 wp, vec3 n){
  vec4 lv = uLightView * vec4(wp + n * .05, 1.);
  float d = -lv.z;
  vec4 lc = uLightProj * lv; vec2 uv = lc.xy * .5 + .5;
  if (uv.x < 0. || uv.y < 0. || uv.x > 1. || uv.y > 1.) return 1.;
  float bsum = 0., bn = 0.;
  for (int i = -2; i <= 2; i++) for (int j = -2; j <= 2; j++) {
    float s = texture(tShadow, uv + vec2(i, j) * 3. / uShadowSize).r;
    if (s > 0. && s < d - .08) { bsum += s; bn += 1.; }
  }
  if (bn < .5) return 1.;
  float dist = d - bsum / bn;
  float rad = clamp(dist * .03 / uShadowTexel, .6, 7.);
  float lit = 0.;
  for (int i = -2; i <= 2; i++) for (int j = -2; j <= 2; j++) {
    float s = texture(tShadow, uv + vec2(i, j) * .5 * rad / uShadowSize).r;
    lit += (s > 0. && s < d - .08) ? 0. : 1.;
  }
  return lit / 25.;
}
// How much open sky a point sees: anything straight above it (an overhang, a roof, a crown) takes some away.
float skyOpen(vec3 P, vec3 N){
  float open = 0.;
  for (int i = 0; i < 5; i++) {
    vec2 o = i == 0 ? vec2(0.) : (i == 1 ? vec2(.5, 0.) : (i == 2 ? vec2(-.5, 0.) : (i == 3 ? vec2(0., .5) : vec2(0., -.5))));
    vec3 Q = P + N * .2 + vec3(o.x, 0., o.y);
    vec4 sv = uSkyView * vec4(Q, 1.);
    vec4 sc = uSkyProj * sv; vec2 uv = sc.xy * .5 + .5;
    float s = texture(tSky, uv).r;
    open += (s > 0. && s < -sv.z - .35) ? 0. : 1.;
  }
  return open / 5.;
}
float ambientOcclusion(vec2 fc, vec3 P, vec3 N){
  float occ = 0.;
  for (int i = 0; i < 20; i++) {
    float a = float(i) * 2.39996;
    float r = (float(i) + .5) / 20.;
    vec2 q = fc + vec2(cos(a), sin(a)) * r * uAORad;
    if (q.x < 0. || q.y < 0. || q.x >= uRes.x || q.y >= uRes.y) continue;
    vec4 g = texelFetch(tG2, ivec2(q), 0);
    if (g.w < .5) continue;
    vec3 v = g.xyz - P; float L = length(v);
    if (L < .02 || L > uAODist) continue;
    float c = dot(v / L, N);
    if (c > .2) occ += (c - .2) * (1. - L / uAODist);
  }
  return clamp(1. - occ * uAOK / 20., 0., 1.);
}
// The light as a continuous step (before the pattern's bias); the final pass rounds it, mixing two steps
// only on the pixels where they meet (PRE-20).
void light(vec3 P, vec3 N, vec2 fc, float flags, out float f, out float cls){
  bool foliage = mod(floor(flags / 2.), 2.) > .5;
  float ndl = dot(N, uSun);
  if (foliage) ndl = ndl * .55 + .45;
  float sh = ndl > 0. ? shadowAt(P + (foliage ? uSun * .45 : vec3(0.)), N) : 0.;
  float sun = pow(max(ndl, 0.), uSunPow) * sh;
  float sky = (.7 + .3 * N.y) * mix(1. - uSkyCover, 1., foliage ? 1. : skyOpen(P, N));
  float ao = foliage ? 1. : ambientOcclusion(fc, P, N);
  ao = ao > .82 ? 1. : (ao > .6 ? .8 : (ao > .4 ? .6 : .45));
  float fire = 0.;
  for (int i = 0; i < 12; i++) {
    if (i >= uFires) break;
    vec3 L = uFire[i].xyz - P; float d = length(L);
    float reach = uFire[i].w * (.78 + .44 * vnoise(P.xz * 1.3 + float(i) * 7.) );
    float att = clamp(1. - d / reach, 0., 1.);
    fire += uFireC[i].a * att * att * (.35 + .65 * max(dot(N, L / max(d, .001)), 0.));
  }
  float lum = (uSkyK * sky * ao + uSunK * sun) * uExposure;
  f = lum * 6. + uShift + fire * 4.5;
  if (ao < .55) f -= .7;
  if (mod(floor(flags / 32.), 2.) > .5) f += .9;   // people and animals stand out from the land
  if (foliage) f -= .3;
  float sunlit = (sh > .5 && ndl > .06) ? 1. : 0.;
  float fb = fire > .36 ? 2. : (fire > .15 ? 1. : 0.);
  cls = sunlit + fb * 2.;
}
void main(){
  vec2 fc = gl_FragCoord.xy; ivec2 p = ivec2(fc);
  vec4 g0 = texelFetch(tG0, p, 0); vec4 g1 = texelFetch(tG1, p, 0); vec4 g2 = texelFetch(tG2, p, 0);
  float hz = clamp((g1.w - uHaze.x) / max(uHaze.y - uHaze.x, .001), 0., 1.);
  hz = floor(hz * 4.) / 4.;
  r1 = vec4(0.);
  float f = 0., cls = 0.;
  if (g0.x < .5) { r0 = vec4(-1., 0., 0., 1.); }
  else {
    float flags = g0.w;
    if (mod(flags, 2.) > .5) { f = 0.; cls = 7.; }
    else light(g2.xyz, g1.xyz, fc, flags, f, cls);
    r0 = vec4(g0.x - 1., f, cls, hz);
  }
  if (uWaterOn > .5) {
    vec4 w0 = texelFetch(tW0, p, 0);
    if (w0.x > 0. && (g0.x < .5 || w0.x < g1.w - .01)) {
      vec4 w1 = texelFetch(tW1, p, 0);
      vec3 P = w0.yzw; vec3 N = vec3(0., 1., 0.);
      float th = g0.x < .5 ? 99. : g1.w - w0.x;
      float wst, wcls;
      light(P, N, fc, 0., wst, wcls);
      vec2 fl = length(w1.xy) > .01 ? normalize(w1.xy) : vec2(1., 0.);
      vec2 q = vec2(dot(P.xz, fl), dot(P.xz, vec2(-fl.y, fl.x)));
      float rip = vnoise(vec2(q.x * .55, q.y * 3.4));
      float sea = w1.w;
      float base = th < .35 ? 5. : (th < 1. ? 4. : (th < 2.4 ? 3. : 2.));
      if (sea > .5) base = th < .5 ? 5. : (th < 1.6 ? 4. : (th < 4. ? 3. : 2.));
      float s = base + (wst - 4.) * .5;
      if (rip > .74) s += 1.; else if (rip < .2) s -= 1.;
      s = clamp(floor(s + .5), 0., 6.);
      float alpha = th < .35 ? .4 : (th < 1. ? .62 : (th < 2.4 ? .82 : 1.));
      float row = sea > .5 ? uWaterRow + 1. : uWaterRow;
      float wc = mod(wcls, 2.) > .5 ? 1. : 0.;
      float brk = vnoise(vec2(q.x * .9, q.y * 2.));
      if ((th < .07 && brk > .62) || (sea > .5 && th < .3 && vnoise(vec2(q.x * .8, q.y * 1.5)) > .45)) { row = uFoamRow; s = wc > .5 ? 5. : 4.; alpha = 1.; }
      else if (wc > .5 && hash12(floor(P.xz * 9.)) > .992 && rip > .55) { row = uGlintRow; s = 6.; alpha = 1.; wcls = 7.; }
      r1 = vec4(r0.x, r0.y - 1., r0.z, alpha);
      r0 = vec4(row, s, wcls, hz);
    }
  }
}`;

// The hour's colour: sunlit colour leans to the sun's tint, shade to the sky's, firelit to the fire's; each
// tint keeps the step's brightness, so the step alone says how light a pixel is.
const GRADE = /* glsl */ `
vec3 tint(vec3 c, vec3 t){ float l = dot(t, vec3(.299, .587, .114)); return c * t / max(l, .05); }
vec3 grade(vec3 c, float cls){
  if (cls > 6.5) return c;
  float sunlit = mod(cls, 2.); float fb = floor(cls / 2. + .01);
  vec3 g = tint(c, sunlit > .5 ? uSunTint : uShadeTint);
  float lc = dot(c, vec3(.299, .587, .114));
  if (fb > .5) g = mix(g, mix(tint(c, uFireTint), uFireTint * lc * 1.55, .6), fb > 1.5 ? .82 : .62);
  float l = dot(g, vec3(.299, .587, .114));
  g = mix(g, vec3(l), uDesat * (fb > .5 ? .25 : 1.));
  return clamp(g, 0., 1.);
}`;

// Outlines, lit edges, the hour's colour, mist and haze.
export const FINAL_FRAG = /* glsl */ `
precision highp float;
layout(location = 0) out vec4 col;
uniform sampler2D tR0; uniform sampler2D tR1; uniform sampler2D tG0; uniform sampler2D tG1; uniform sampler2D tG2;
uniform sampler2D tPal; uniform sampler2D tW0;
uniform mat4 uView; uniform vec3 uSun; uniform float uMpp;
uniform vec3 uSunTint; uniform vec3 uShadeTint; uniform vec3 uFireTint; uniform vec3 uHazeCol; uniform float uHazeK;
uniform float uDesat; uniform vec3 uBack; uniform vec4 uMist; uniform vec3 uMistCol; uniform vec2 uRes;
uniform vec3 uWaterTint; uniform float uSkyRow; uniform vec3 uSkyCol; uniform int uInk;
${NOISE}
vec3 pal(float m, float s){ return texelFetch(tPal, ivec2(int(clamp(s, 0., 6.) + .5), int(m + .5)), 0).rgb; }
${GRADE}
bool bit(float f, float b){ return mod(floor(f / b), 2.) > .5; }
void main(){
  ivec2 p = ivec2(gl_FragCoord.xy);
  vec4 r = texelFetch(tR0, p, 0);
  if (r.x < -.5) { col = vec4(uInk == 1 ? vec3(.937, .886, .769) : uBack, uInk == 2 ? 0. : 1.); return; }
  vec4 g0 = texelFetch(tG0, p, 0); vec4 g1 = texelFetch(tG1, p, 0);
  vec4 u = texelFetch(tR1, p, 0);
  bool water = u.w > 0.;
  float cls = r.z, flags = g0.w;
  float st;
  if (water) st = r.y;
  else if (cls > 6.5) st = clamp(g0.y, 0., 6.);
  else {
    float f = r.y, s0 = floor(f + .5), fr = f - floor(f);
    if (abs(fr - .5) < .15) {
      bool meet = false;
      for (int k = 0; k < 4; k++) {
        ivec2 o = k == 0 ? ivec2(1, 0) : (k == 1 ? ivec2(-1, 0) : (k == 2 ? ivec2(0, 1) : ivec2(0, -1)));
        ivec2 q = clamp(p + o, ivec2(0), ivec2(uRes) - 1);
        vec4 qr = texelFetch(tR0, q, 0), qg = texelFetch(tG0, q, 0);
        if (abs(qr.x - r.x) < .5 && abs(qg.z - g0.z) < .5 && qr.z < 6.5 && floor(qr.y + .5) != s0) meet = true;
      }
      if (meet) s0 = floor(f) + mod(float(p.x + p.y), 2.);
    }
    st = clamp(s0 + g0.y, 0., 6.);
  }
  float d = g1.w; vec3 n = g1.xyz; vec3 nv = (uView * vec4(n, 0.)).xyz;
  vec2 sv = normalize((uView * vec4(uSun, 0.)).xy + vec2(1e-4));
  int edge = 0;
  bool canEdge = !water && g0.x > .5 && !bit(flags, 4.) && cls < 6.5;
  if (canEdge) {
    bool foliage = bit(flags, 2.);
    for (int k = 0; k < 4; k++) {
      ivec2 o = k == 0 ? ivec2(1, 0) : (k == 1 ? ivec2(-1, 0) : (k == 2 ? ivec2(0, 1) : ivec2(0, -1)));
      ivec2 q = clamp(p + o, ivec2(0), ivec2(uRes) - 1);
      vec4 q0 = texelFetch(tG0, q, 0); vec4 q1 = texelFetch(tG1, q, 0);
      float pred = d + (nv.x * float(o.x) + nv.y * float(o.y)) * uMpp / max(nv.z, .25);
      float thr = foliage && abs(q0.z - g0.z) < .5 ? 1.6 : .32;
      bool behind = q0.x < .5 || q1.w > pred + thr || (abs(q0.z - g0.z) > .5 && q1.w > d + .06);
      if (behind && !bit(q0.w, 4.) ) {
        bool rim = dot(vec2(o), sv) > .35 && mod(cls, 2.) > .5 && dot(n, uSun) > .25;
        if (rim && edge == 0) edge = 2; else if (!rim) edge = 1;
      }
    }
    if (edge == 0 && !foliage && mod(cls, 2.) > .5) {
      for (int k = 0; k < 4; k++) {
        ivec2 o = k == 0 ? ivec2(1, 0) : (k == 1 ? ivec2(-1, 0) : (k == 2 ? ivec2(0, 1) : ivec2(0, -1)));
        ivec2 q = clamp(p + o, ivec2(0), ivec2(uRes) - 1);
        vec4 q0 = texelFetch(tG0, q, 0); vec4 q1 = texelFetch(tG1, q, 0);
        if (abs(q0.z - g0.z) > .5) continue;
        float pred = d + (nv.x * float(o.x) + nv.y * float(o.y)) * uMpp / max(nv.z, .25);
        if (dot(n, q1.xyz) < .8 && dot(n, uSun) > dot(q1.xyz, uSun) + .2 && q1.w > pred - .03) edge = 2;
      }
    }
    if (edge == 1) st = max(st - (foliage || g0.z < .5 ? 1. : 2.), 0.);
    if (edge == 2) { st = min(st + 1., 6.); cls = mod(cls, 2.) > .5 ? cls : cls + 1.; }
  }
  if (uInk == 1) {
    // the field journal's ink sketch: outlines in ink, tone by hatching (crossed for the darkest, then lines,
    // then stipple), the lit side left as paper; the ground is drawn a tone lighter so figures stand out
    vec3 paper = vec3(.937, .886, .769), ink = vec3(.231, .165, .173), mid = vec3(.541, .416, .314);
    float L = dot(grade(pal(r.x, st), cls), vec3(.299, .587, .114)) + (g0.z < .5 ? .1 : 0.);
    vec3 o = paper;
    if (edge == 1) o = ink;
    else if (edge == 2) o = paper;
    else if (water) o = (p.y % 3 == 0 && (p.x / 3) % 2 == 0) ? mid : paper;
    else if (cls > 6.5) o = (p.x + p.y) % 2 == 0 ? mid : paper;
    else if (L < .2) o = (p.x + p.y) % 2 == 0 ? ink : mid;
    else if (L < .32) o = (p.x + p.y) % 2 == 0 ? mid : paper;
    else if (L < .44) o = (p.x + p.y) % 3 == 0 ? mid : paper;
    else if (L < .56) o = (p.x * 7 + p.y * 3) % 9 == 0 ? mid : paper;
    col = vec4(o, 1.);
    return;
  }
  vec3 c = grade(pal(r.x, st), cls);
  if (water) {
    vec3 cu = grade(pal(u.x, clamp(floor(u.y + .5) + g0.y, 0., 6.)), u.z) * uWaterTint;
    if (r.z < 6.5 && r.x < uSkyRow) c = mix(c, uSkyCol, mod(r.z, 2.) > .5 ? .3 : .22);   // water mirrors the sky
    c = mix(cu, c, u.w);
  }
  if (uMist.z > 0.) {
    vec3 wp = water ? texelFetch(tW0, p, 0).yzw : texelFetch(tG2, p, 0).xyz;
    float h = (wp.y - uMist.x) / uMist.y;
    float m = (1. - clamp(h, 0., 1.)) * uMist.z * (.3 + .95 * fbm2(vec2(wp.x * uMist.w * .45, wp.z * uMist.w * 1.6) + vec2(0., wp.y * .3)));
    m = floor(clamp(m, 0., 1.) * 2.5) / 2.5;
    c = mix(c, uMistCol, m * .5);
  }
  c = mix(c, uHazeCol, r.w * uHazeK);
  col = vec4(c, 1.);
}`;

// Smoke and other air, drawn over the picture, hidden by what stands in front.
export const PUFF_VERT = /* glsl */ `
attribute vec3 aCenter; attribute vec2 aCorner; attribute float aSize; attribute float aTone; attribute float aAlpha;
uniform vec3 uRight; uniform vec3 uUp;
varying vec2 vC; varying float vDepth; varying float vTone; varying float vAlpha; varying float vSize;
void main(){
  vec3 wp = aCenter + (uRight * aCorner.x + uUp * aCorner.y) * aSize;
  vC = aCorner; vTone = aTone; vAlpha = aAlpha; vSize = aSize;
  vec4 mv = viewMatrix * vec4(wp, 1.); vDepth = -mv.z;
  gl_Position = projectionMatrix * mv;
}`;
export const PUFF_FRAG = /* glsl */ `
precision highp float;
layout(location = 0) out vec4 col;
uniform sampler2D tG1; uniform sampler2D tPal; uniform float uRow; uniform vec3 uSunTint; uniform vec3 uShadeTint;
uniform vec3 uFireTint; uniform float uDesat; uniform vec3 uHazeCol; uniform float uHazeK; uniform vec2 uHaze; uniform float uStepShift;
varying vec2 vC; varying float vDepth; varying float vTone; varying float vAlpha; varying float vSize;
${NOISE}
${GRADE}
void main(){
  float r = length(vC) * 2.;
  float edge = .82 + .22 * (vnoise(vC * 5. + vTone * 13.) - .5);
  if (r > edge) discard;
  vec4 g1 = texelFetch(tG1, ivec2(gl_FragCoord.xy), 0);
  if (g1.w > 0. && g1.w < vDepth - vSize * .5) discard;
  float lit = dot(normalize(vec3(vC, .6)), normalize(vec3(-.55, .6, .6)));
  float s = clamp(floor(vTone + (lit > .55 ? 1. : (lit < .1 ? -1. : 0.)) + uStepShift + .5), 0., 6.);
  vec3 c = grade(texelFetch(tPal, ivec2(int(s), int(uRow)), 0).rgb, lit > .3 ? 1. : 0.);
  float hz = floor(clamp((vDepth - uHaze.x) / max(uHaze.y - uHaze.x, .001), 0., 1.) * 4.) / 4.;
  c = mix(c, uHazeCol, hz * uHazeK);
  col = vec4(c, vAlpha);
}`;
