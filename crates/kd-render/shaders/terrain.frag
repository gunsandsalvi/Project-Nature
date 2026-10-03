// The ground (T01b.4, A11.5, PRE-20, PRE-21): the mockup's terrainFS without its scene-specific inputs (river
// distance, camp wear, path distance, plateau, canopy). The surface of the pixel's square metre, from the area's
// surface map (uSurfMap, one byte a square, A11.5), picks its row of the surfaces texture, which gives the ladder,
// the share and size of its stones and the share of its tufts; the light is the mockup's ground light, sunLight
// with shadowAt softened toward the shadow alone, plus sky; stones and tufts are stamped below their art-pixel
// limits; then finish, and packed as ground, or as rock where the ground's normal is under 0.72 upward, so the
// cliff is outlined. Under SHADOW only depth is written (pass 1).
in vec3 vNrm;
in vec2 vSq;
in vec4 vB;
in float vFlags;
uniform sampler2D uSurf;
uniform sampler2D uSurfMap;
out vec4 fragColor;
int surfAtSquare(ivec2 i) {
  return int(texelFetch(uSurfMap, clamp(i, ivec2(0), textureSize(uSurfMap, 0) - 1), 0).r * 255.0 + 0.5);
}
// The surface at q, metres from the area's corner: of the four squares whose middles surround q, the surface whose
// squares weigh most by nearness (bilinear weights), the first in a tie.
int surfaceAt(vec2 q) {
  vec2 c = q - 0.5;
  ivec2 i0 = ivec2(floor(c));
  vec2 f = c - floor(c);
  int s[4] = int[4](surfAtSquare(i0), surfAtSquare(i0 + ivec2(1, 0)), surfAtSquare(i0 + ivec2(0, 1)),
                    surfAtSquare(i0 + ivec2(1, 1)));
  float w[4] = float[4]((1.0 - f.x) * (1.0 - f.y), f.x * (1.0 - f.y), (1.0 - f.x) * f.y, f.x * f.y);
  int best = s[0];
  float most = -1.0;
  for (int a = 0; a < 4; a++) {
    float t = 0.0;
    for (int b = 0; b < 4; b++) t += s[b] == s[a] ? w[b] : 0.0;
    if (t > most) { most = t; best = s[a]; }
  }
  return best;
}
void main() {
#ifdef SHADOW
  fragColor = vec4(1.0);
#else
  vec3 n = normalize(vNrm);
  vec2 p = vWorld.xz + uWorldOff;
  float z = vWorld.y;
  // the surface of the nearest squares that weigh most, so edges run smooth between the squares' middles rather
  // than as a staircase, wandering a little with noise fixed to the world; only surfaces the rule placed are drawn
  vec2 wander = vec2(fbm3(p * 0.7 + 5.3), fbm3(p * 0.7 - 8.9)) - 0.5;
  int surf = surfaceAt(vSq + 0.7 * wander);
  vec4 s0 = texelFetch(uSurf, ivec2(0, surf), 0);
  float ramp = floor(s0.r * 255.0 + 0.5);
  float stoneDens = s0.g, stoneSize = s0.b * 2.55, tuftDens = s0.a;
  float b = bayer();
  float sh = shadowAt(vWorld, n);
  float sunL = sunLight(n, sh);
  // ground: soften how much small tilts change the low sun's light, so shading follows real slopes only
  float sun = mix(sh * uSunI, sunL, 0.55);
  float lit = sky(n) * 0.36 + sun * 0.58;
  setBand(lit);
  float n1 = vnoise(p * 0.55), nb = fbm3(p * 0.04), nm = vnoise(p * 0.16 + 11.0);
  // dry and green patches, as the mockup's grass
  float dry = clamp(0.5 + 1.5 * (nb - 0.5) + 0.7 * (nm - 0.5), 0.0, 1.0);
  float v = 0.07 + 0.6 * lit + 0.19 * dry + 0.05 * (n1 - 0.5);
  float tuft = tuftDens > 0.02 ? stampTuft(p, z, 0.62, tuftDens) : 0.0;
  v += tuft > 1.5 ? 0.17 : tuft > 0.5 ? 0.08 : tuft < -0.5 ? -0.14 : 0.0;
  float stone = stoneDens > 0.004 ? stampStone(p, z, 0.62, stoneDens, stoneSize, 3.0) : 0.0;
  if (stone > 0.5) v = stone > 2.5 ? 0.62 + 0.36 * lit : stone > 1.5 ? 0.42 + 0.32 * lit : 0.08;
  float idx = finish(rampPick(ramp, v, b), 0.0, b);
  fragColor = packOut(idx, n.y < 0.72 ? C_ROCK : C_GROUND, sunL > 0.3 ? 1.0 : 0.0);
#endif
}
