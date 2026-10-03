// The ground's vertices (A11.5): one shared patch of PATCH_QUADS x PATCH_QUADS quads made from the vertex and
// instance numbers, no vertex buffer, each vertex reading its height from the area's texture; or, with u_skirt,
// the skirt hanging SKIRT_M below the area's edge. Projected from the area's own corner (A11.2): the fraction of an
// art pixel is added and the place rounded to 1/256 of one before the whole pixels, and the viewport is
// VIEWPORT pixels, so a picture moved by whole pixels draws every pixel the same.
uniform highp sampler2D u_heights;  // R32F, 257 x 257: metres above the area's corner
uniform float u_spacing;            // metres between vertices, a power of two
uniform float u_morph;              // 0 to 1: how far the odd vertices have slid onto the next spacing's mesh
uniform ivec4 u_patches;            // the first patch's column and row, and how many columns and rows are drawn
uniform int u_skirt;                // 1 for the skirts, 0 for the patches
uniform vec2 u_right;               // the screen's right in (east, north); it has no up
uniform vec3 u_up;                  // the screen's up in (east, north, up)
uniform vec3 u_fwd;                 // the view's direction
uniform float u_inv_texel;          // art pixels a metre
uniform vec2 u_area_frac;           // the area corner's place on the screen: the fraction of an art pixel
uniform vec2 u_area_px;             // and the whole art pixels from the viewport's corner
uniform vec2 u_depth;               // the corner's depth less the near plane's, and 1 / (far - near)
uniform vec2 u_area_air;            // the corner's depth beyond the target's plane, and its height above the sea
out vec2 v_local;                   // metres east and south of the area's corner
out float v_depth;                  // 0 near to 1 far
out vec2 v_air;                     // metres beyond the target's plane, and above the sea (A11.4)

// The two triangles of a quad, split along its diagonal from the north-west.
const ivec2 CORNERS[6] = ivec2[6](ivec2(0, 0), ivec2(1, 0), ivec2(1, 1), ivec2(0, 0), ivec2(1, 1), ivec2(0, 1));

float height_at(ivec2 g) {
    return texelFetch(u_heights, clamp(g, ivec2(0), ivec2(256)), 0).r;
}

// Grid point g's height at spacing s, its odd vertices morph of the way onto the next spacing's mesh, as
// ground::vertex_height.
float vertex_height(ivec2 g, int s, float morph) {
    float h = height_at(g);
    ivec2 odd = (g / s) & 1;
    float coarse = h;
    if (odd.x == 1 && odd.y == 1) {
        coarse = 0.5 * (height_at(g - ivec2(s)) + height_at(g + ivec2(s)));
    } else if (odd.x == 1) {
        coarse = 0.5 * (height_at(g - ivec2(s, 0)) + height_at(g + ivec2(s, 0)));
    } else if (odd.y == 1) {
        coarse = 0.5 * (height_at(g - ivec2(0, s)) + height_at(g + ivec2(0, s)));
    }
    return h + (coarse - h) * morph;
}

void main() {
    int s = int(u_spacing);
    ivec2 g;
    float h;
    if (u_skirt == 0) {
        int q = gl_VertexID / 6;
        ivec2 tile = u_patches.xy + ivec2(gl_InstanceID % u_patches.z, gl_InstanceID / u_patches.z);
        g = (tile * PATCH_QUADS + ivec2(q % PATCH_QUADS, q / PATCH_QUADS) + CORNERS[gl_VertexID - q * 6]) * s;
        h = vertex_height(g, s, u_morph);
    } else {
        // Instance k is a segment of side k / (256 / s): north, east, south and west, each run along its edge.
        int per_side = 256 / s;
        int side = gl_InstanceID / per_side;
        int along = (gl_InstanceID - side * per_side + CORNERS[gl_VertexID].x) * s;
        ivec2 edge[4] = ivec2[4](ivec2(along, 0), ivec2(256, along), ivec2(along, 256), ivec2(0, along));
        g = edge[side];
        h = vertex_height(g, s, u_morph) - (CORNERS[gl_VertexID].y == 1 ? SKIRT_M : 0.0);
    }
    vec2 en = vec2(float(g.x), -float(g.y));
    vec3 l = vec3(en, h);
    vec2 a = vec2(dot(en, u_right), dot(l, u_up)) * u_inv_texel + u_area_frac;
    a = floor(a * 256.0 + 0.5) / 256.0 + u_area_px;
    float dz = (dot(l, u_fwd) + u_depth.x) * u_depth.y;
    gl_Position = vec4(a / float(VIEWPORT / 2) - 1.0, dz * 2.0 - 1.0, 1.0);
    v_local = vec2(g);
    v_depth = dz;
    v_air = vec2(dot(l, u_fwd) + u_area_air.x, h + u_area_air.y);
}
