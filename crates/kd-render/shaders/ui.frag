// Pass 6, the pixel UI (A12.1): glyph pixels from the R8 atlas, colours from the palette's current row, so the UI is
// the world's pixel art (PRE-01); it dissolves by a 4 x 4 Bayer pattern per UI pixel as it fades (PRE-32).
in vec2 vUi;
in vec2 vUv;
flat in float vCol;
uniform sampler2D uAtlas;
uniform sampler2D uPal;
uniform float uPalRow;
uniform float uShow;  // 1 shown, 0 gone
out vec4 fragColor;
// The mockup's bayer2, twice: a 4 x 4 Bayer threshold in sixteenths.
float bayer2(vec2 a) { a = floor(a); return fract(dot(a, vec2(0.5, a.y * 0.75))); }
float bayer4(vec2 a) { return bayer2(0.5 * a) * 0.25 + bayer2(a); }
void main() {
  if (vUv.x >= 0.0 && texelFetch(uAtlas, ivec2(floor(vUv)), 0).r < 0.5) discard;
  if (bayer4(vUi) >= uShow) discard;
  fragColor = texelFetch(uPal, ivec2(int(vCol + 0.5), int(uPalRow + 0.5)), 0);
}
