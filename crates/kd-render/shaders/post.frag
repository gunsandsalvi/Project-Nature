// Pass 3, post (A11.2): each art pixel's palette colour from the index in colour 0 and the palette row in use, so
// every pixel shown is one palette colour (PRE-01). Outlines, lit edges, haze and glow join in α01c.
uniform sampler2D u_scene;    // colour 0 of the art target
uniform sampler2D u_palette;  // the row in use, PALETTE_SIZE x 1
out vec4 o_colour;

void main() {
    int index = palette_index(texelFetch(u_scene, art_pixel(), 0));
    o_colour = texelFetch(u_palette, ivec2(index, 0), 0);
}
