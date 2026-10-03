// Water lines over coarse ground (A11.6, PRE-26): each instance a piece of a river's or a stream's line, read from
// the tile's lines texture, no vertex buffer, drawn as a strip across the screen as wide as its water, but never
// under LINE_MIN_PX art pixels either side, so every river shows at least an art pixel wide; its ends run on by as
// much, so the pieces join; lifted LINE_LIFT_M above the coarse ground, which carves no channel. Placed by area_place,
// as the ground's vertices are (A11.2).
uniform highp sampler2D u_lines;    // RGBA32F, 2 x LINES_ROW wide: a piece's start (east, south, up, half-width)
                                    // and its end (east, south, up, depth), metres from the tile's corner
uniform vec2 u_right;               // the screen's right in (east, north); it has no up
uniform vec3 u_up;                  // the screen's up in (east, north, up)
uniform vec3 u_fwd;                 // the view's direction
uniform float u_inv_texel;          // art pixels a metre
uniform vec2 u_area_frac;           // the tile corner's place on the screen: the fraction of an art pixel
uniform vec2 u_area_px;             // and the whole art pixels from the viewport's corner
uniform vec2 u_depth;               // the corner's depth less the near plane's, and 1 / (far - near)
uniform vec2 u_area_air;            // the corner's depth beyond the target's plane, and its height above the sea
flat out float v_water_depth;       // the water's depth, metres
out float v_depth;                  // 0 near to 1 far
out vec2 v_air;                     // metres beyond the target's plane, and above the sea (A11.4)

// A strip's corners: which end, and which side.
const ivec2 CORNERS[6] = ivec2[6](ivec2(0, -1), ivec2(1, -1), ivec2(1, 1), ivec2(0, -1), ivec2(1, 1), ivec2(0, 1));

void main() {
    int i = gl_InstanceID;
    ivec2 at = ivec2((i % LINES_ROW) * 2, i / LINES_ROW);
    vec4 a = texelFetch(u_lines, at, 0);
    vec4 b = texelFetch(u_lines, at + ivec2(1, 0), 0);
    vec3 la = vec3(a.x, -a.y, a.z + LINE_LIFT_M);
    vec3 lb = vec3(b.x, -b.y, b.z + LINE_LIFT_M);
    vec2 d = (vec2(dot(lb.xy, u_right), dot(lb, u_up)) - vec2(dot(la.xy, u_right), dot(la, u_up))) * u_inv_texel;
    float len = length(d);
    vec2 along = len > 1e-6 ? d / len : vec2(1.0, 0.0);
    vec2 across = vec2(-along.y, along.x);
    float half_px = max(a.w * u_inv_texel, LINE_MIN_PX);
    ivec2 c = CORNERS[gl_VertexID];
    vec3 l = c.x == 0 ? la : lb;
    vec2 shift = across * (float(c.y) * half_px) + along * (c.x == 0 ? -half_px : half_px);
    vec2 p = area_place(l, shift, u_right, u_up, u_inv_texel, u_area_frac, u_area_px);
    float dz = (dot(l, u_fwd) + u_depth.x) * u_depth.y;
    gl_Position = vec4(p / float(VIEWPORT / 2) - 1.0, dz * 2.0 - 1.0, 1.0);
    v_water_depth = b.w;
    v_depth = dz;
    v_air = vec2(dot(l, u_fwd) + u_area_air.x, l.z + u_area_air.y);
}
