// Pass 6, the UI (A12.1): a UI pixel is 4 x 4 screen pixels on the screen's grid, in a fixed palette colour; an item
// shows through the 4 x 4 Bayer pattern by its fade, so the strip dissolves pixel by pixel (PRE-32).
uniform sampler2D u_font;     // the font's atlas, 255 where a glyph has ink
uniform sampler2D u_palette;  // the row in use
uniform vec2 u_screen;
uniform float u_scale;
in vec2 v_tex;
flat in uvec2 v_info;
out vec4 o_colour;

void main() {
    ivec2 ui = ivec2(floor(gl_FragCoord.x / u_scale), floor((u_screen.y - gl_FragCoord.y) / u_scale));
    if (bayer(ui) >= float(v_info.y) / 255.0) discard;
    if (v_tex.x >= 0.0 && texelFetch(u_font, ivec2(floor(v_tex)), 0).r < 0.5) discard;
    o_colour = texelFetch(u_palette, ivec2(int(v_info.x), 0), 0);
}
