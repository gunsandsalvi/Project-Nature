// Pass 3, post (A11.2): outlines by category with their depth thresholds, ink for figures, the sun rim through the
// rim_sun table toward uSunScr, the fire rim through rim_fire toward uFireScr (kept for alpha 14a), then the palette
// row uPalRow. The mockup's postFS, in GLSL ES 3.00, with categories named by kd_render::Cat's defines.
out vec4 fragColor;
uniform sampler2D uImg; uniform sampler2D uPal; uniform sampler2D uLuts;
uniform vec2 uRes; uniform vec2 uSunScr; uniform vec3 uFireScr; uniform float uOutline; uniform float uPalRow;
uniform float uDepthM; uniform float uTexel;
vec4 px(vec2 p) { return texture(uImg, (p + 0.5) / uRes); }
float dep(vec4 c) { return (c.b * 255.0 * 256.0 + c.a * 255.0) / 65535.0; }
float catOf(vec4 c) { return mod(floor(c.g * 255.0 + 0.5), 8.0); }
float lut(float row, float idx) { return floor(texture(uLuts, vec2((idx + 0.5) / 256.0, (row + 0.5) / 16.0)).r * 255.0 + 0.5); }
void main() {
  vec2 p = floor(gl_FragCoord.xy);
  vec4 c = px(p);
  float idx = floor(c.r * 255.0 + 0.5);
  float g = floor(c.g * 255.0 + 0.5);
  float cat = mod(g, 8.0), flags = floor(g / 8.0);
  float sunlit = mod(flags, 2.0), firelit = floor(flags / 2.0);
  if (uOutline > 0.5 && cat > C_GROUND && cat != C_WATER && cat != C_EFFECT) {
    float d = dep(c) * uDepthM;
    float thr = cat == C_FIGURE ? max(0.22, uTexel * 2.0) : cat == C_PLANT ? max(2.4, uTexel * 4.0) : cat == C_ROCK ? max(1.2, uTexel * 3.5) : max(0.3, uTexel * 2.0);
    vec4 nr = px(p + vec2(1.0, 0.0)), nl = px(p - vec2(1.0, 0.0)), nu = px(p + vec2(0.0, 1.0)), nd = px(p - vec2(0.0, 1.0));
    float fr = dep(nr) * uDepthM - d > thr ? 1.0 : 0.0;
    float fl = dep(nl) * uDepthM - d > thr ? 1.0 : 0.0;
    float fu = dep(nu) * uDepthM - d > thr ? 1.0 : 0.0;
    float fd = dep(nd) * uDepthM - d > thr ? 1.0 : 0.0;
    if (fr + fl + fu + fd > 0.5) {
      // which side of the silhouette faces the light?
      vec2 ls = uSunScr;
      float towardSun = max(max(fr * ls.x, fl * -ls.x), max(fu * ls.y, fd * -ls.y));
      vec2 fdir = uFireScr.xy - p; float fdist = length(fdir); fdir /= max(fdist, 0.001);
      float towardFire = max(max(fr * fdir.x, fl * -fdir.x), max(fu * fdir.y, fd * -fdir.y));
      if (firelit > 0.5 && towardFire > 0.35 && fdist < uFireScr.z) idx = lut(8.0, idx);
      else if (sunlit > 0.5 && towardSun > 0.4 && cat != C_THING) idx = lut(7.0, idx);
      else idx = cat == C_FIGURE ? I_INK : lut(6.0, idx);
    }
  }
  fragColor = texture(uPal, vec2((idx + 0.5) / 256.0, (uPalRow + 0.5) / 4.0));
}
