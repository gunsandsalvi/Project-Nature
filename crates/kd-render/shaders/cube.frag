// The golden cube as pixel art (T01a.6, PRE-20, PRE-21): sun and sky light pick a step of the birch ladder, two steps
// mixed by the Bayer pattern only in a narrow band where they meet; finish() adds no fire and no haze; packed as rock
// (C_ROCK), so the post pass outlines it and rims its sunlit side. Light falls off gently across each face toward
// the sun, so steps meet inside the faces, not only at their edges.
in vec3 vNrm;
out vec4 fragColor;
void main() {
  vec3 n = normalize(vNrm);
  float sun = sunLight(n, 1.0);
  float v = 0.04 + 0.42 * (sun + sky(n)) + 0.25 * dot(vWorld, uSunDir);
  setBand(v);
  float b = bayer();
  float idx = rampPick(R_BIRCH, v, b);
  idx = finish(idx, 0.0, b);
  fragColor = packOut(idx, C_ROCK, sun > 0.3 ? 1.0 : 0.0);
}
