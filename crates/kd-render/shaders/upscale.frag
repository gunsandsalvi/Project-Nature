// Pass 5, upscale (A11.2): nearest, whole-number scale, the sub-pixel shift in uOff. The mockup's upscaleFS.
out vec4 fragColor;
uniform sampler2D uImg; uniform vec2 uImgSize; uniform vec2 uOff; uniform float uScale;
void main() {
  vec2 a = floor(uOff + gl_FragCoord.xy / uScale);
  fragColor = texture(uImg, (a + 0.5) / uImgSize);
}
