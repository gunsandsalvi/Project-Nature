// Pass 6, the UI (A12.1): each corner from UI pixels, counted from the screen's top-left, to the window.
layout(location = 0) in vec2 a_pos;   // the corner, in UI pixels
layout(location = 1) in vec2 a_tex;   // the font texel at that corner; negative for a filled rectangle
layout(location = 2) in uvec2 a_info; // the palette index, and how much shows in 255ths
uniform vec2 u_screen;                // the window in screen pixels
uniform float u_scale;                // screen pixels a UI pixel
out vec2 v_tex;
flat out uvec2 v_info;

void main() {
    vec2 p = a_pos * u_scale;
    gl_Position = vec4(p.x / u_screen.x * 2.0 - 1.0, 1.0 - p.y / u_screen.y * 2.0, 0.0, 1.0);
    v_tex = a_tex;
    v_info = a_info;
}
