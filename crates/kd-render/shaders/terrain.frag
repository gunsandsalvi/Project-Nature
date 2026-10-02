// The ground (T01b.4, A11.5, PRE-20, PRE-21): the mockup's terrainFS without its scene-specific inputs (river
// distance, camp wear, path distance, plateau, canopy). The surface's row of the surfaces texture gives the ladder,
// the share and size of its stones and the share of its tufts; the light is the mockup's ground light, sunLight
// with shadowAt softened toward the shadow alone, plus sky; stones and tufts are stamped below their art-pixel
// limits; then finish, and packed as ground, or as rock where the ground's normal is under 0.72 upward, so the
// cliff is outlined. Under SHADOW only depth is written (pass 1).
in vec3 vNrm;
in vec4 vB;
in float vFlags;
uniform sampler2D uSurf;
// where the floating origin lies within an 8,192 m block of the world, so the patterns stay on the ground
uniform vec2 uWorldOff;
out vec4 fragColor;
void main() {
#ifdef SHADOW
  fragColor = vec4(1.0);
#else
  vec3 n = normalize(vNrm);
  vec2 p = vWorld.xz + uWorldOff;
  float z = vWorld.y;
  vec4 s0 = texelFetch(uSurf, ivec2(0, int(vB.x + 0.5)), 0);
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
