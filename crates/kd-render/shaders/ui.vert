// Pass 6, the pixel UI (A12.1): rectangles and glyphs, positioned in screen pixels on the upscale's grid.
in vec2 aPos;   // screen pixels from the bottom left
in vec2 aUi;    // UI pixels from the strip's top left, for the dissolve
in vec2 aUv;    // atlas pixels; negative for a solid rectangle
in float aCol;  // palette index
uniform vec2 uWin;
out vec2 vUi;
out vec2 vUv;
flat out float vCol;
void main() {
  vUi = aUi;
  vUv = aUv;
  vCol = aCol;
  gl_Position = vec4(aPos / uWin * 2.0 - 1.0, 0.0, 1.0);
}
