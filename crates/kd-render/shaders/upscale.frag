// Pass 5 (A11.2): the art target enlarged to the window, nearest, by a whole-number scale, with the sub-pixel
// shift, so a pan moves the picture by whole screen pixels while the art grid stays where it is.
uniform sampler2D u_art;
uniform vec2 u_off;     // art pixels from the art target's corner to the window's bottom-left corner
uniform float u_scale;  // screen pixels per art pixel
out vec4 o_colour;

void main() {
    o_colour = texelFetch(u_art, ivec2(floor(u_off + gl_FragCoord.xy / u_scale)), 0);
}
