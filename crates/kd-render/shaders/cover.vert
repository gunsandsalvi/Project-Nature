// The stones and tufts (A11.5): each instance an item read from the area's items texture at u_first plus the
// instance's number, no vertex buffer. A stone is the eight faces of an octahedron of its size, squat, leaning and
// turned by its shape and heading, sunk a little into the ground; a tuft its three to five blades, each a strip
// BLADE_HALF_PX art pixels either side of its line across the screen, so never narrower than an art pixel; each
// sees less of the sky toward its foot (u_foot_sky). An item
// spanning less than 1.5 + 2u art pixels, and a tuft's blades beyond its count, are moved outside the view, so
// nothing of them draws. Placed by area_place, as the ground's vertices are (A11.2).
uniform highp sampler2D u_items;    // RGBA32F, 2 x ITEMS_ROW wide: an item's place and size, then its heading,
                                    // importance, ladder (base + 256 steps) and shape
uniform int u_first;                // the draw's first item
uniform int u_kind;                 // COVER_STONE or COVER_TUFT
uniform vec2 u_right;               // the screen's right in (east, north); it has no up
uniform vec3 u_up;                  // the screen's up in (east, north, up)
uniform vec3 u_fwd;                 // the view's direction
uniform float u_inv_texel;          // art pixels a metre
uniform vec2 u_area_frac;           // the area corner's place on the screen: the fraction of an art pixel
uniform vec2 u_area_px;             // and the whole art pixels from the viewport's corner
uniform vec2 u_depth;               // the corner's depth less the near plane's, and 1 / (far - near)
uniform vec2 u_area_air;            // the corner's depth beyond the target's plane, and its height above the sea
uniform vec2 u_foot_sky;            // the share of the sky a stone's middle and a blade's foot see
flat out vec3 v_normal;             // the face's or blade's normal (east, north, up)
flat out ivec2 v_ladder;            // its ladder: first palette index, steps
flat out vec2 v_foot;               // its foot, metres east and south of the corner, where its light is read
out float v_sky;                    // the share of the open sky it sees, less toward its foot
out float v_depth;                  // 0 near to 1 far
out vec2 v_air;                     // metres beyond the target's plane, and above the sea (A11.4)

// Outside the view on every side: a triangle of these corners draws nothing.
const vec4 HIDDEN = vec4(2.0, 2.0, 2.0, 1.0);

// A uniform draw in [0, 1] from a hash's ten bits from bit `at`.
float bits(uint h, int at) {
    return float((h >> uint(at)) & 1023u) / 1023.0;
}

void main() {
    int i = u_first + gl_InstanceID;
    ivec2 at = ivec2((i % ITEMS_ROW) * 2, i / ITEMS_ROW);
    vec4 a = texelFetch(u_items, at, 0);
    vec4 b = texelFetch(u_items, at + ivec2(1, 0), 0);
    float size = a.w;
    if (!cover_shows(size, b.y, u_inv_texel)) {
        gl_Position = HIDDEN;
        return;
    }
    int code = int(b.z + 0.5);
    uint shape = uint(b.w + 0.5);
    float c = cos(b.x);
    float s = sin(b.x);
    vec3 foot = vec3(a.x, -a.y, a.z);
    vec3 p;
    vec3 n;
    vec2 shift = vec2(0.0);
    float sky = 1.0;
    if (u_kind == COVER_STONE) {
        // Face f's corners on the axes: east or west, north or south, and the top or the bottom.
        int f = gl_VertexID / 3;
        int k = gl_VertexID - f * 3;
        float rx = 0.5 * size;
        float ry = rx * (0.6 + 0.4 * float(shape & 15u) / 15.0);
        float rz = rx * (0.45 + 0.3 * float((shape >> 4u) & 15u) / 15.0);
        vec2 lean = size * 0.15 * (vec2(float((shape >> 8u) & 15u), float((shape >> 12u) & 15u)) / 15.0 - 0.5);
        vec3 c0 = vec3((f & 1) == 0 ? rx : -rx, 0.0, 0.0);
        vec3 c1 = vec3(0.0, (f & 2) == 0 ? ry : -ry, 0.0);
        vec3 c2 = (f & 4) == 0 ? vec3(lean, rz) : vec3(0.0, 0.0, -0.5 * rx);
        vec3 m = cross(c1 - c0, c2 - c0);
        m = dot(m, c0 + c1 + c2) < 0.0 ? -m : m;
        vec3 q = k == 0 ? c0 : (k == 1 ? c1 : c2);
        sky = k == 2 && (f & 4) == 0 ? 1.0 : u_foot_sky.x;
        p = foot + vec3(c * q.x - s * q.y, s * q.x + c * q.y, q.z - 0.1 * rx);
        n = normalize(vec3(c * m.x - s * m.y, s * m.x + c * m.y, m.z));
    } else {
        // Blade j of the tuft's three to five, round its foot, leaning out; corners 0, 1 and 3 at its foot.
        int j = gl_VertexID / 6;
        int k = gl_VertexID - j * 6;
        int blades = 3 + int(shape % 3u);
        if (j >= blades) {
            gl_Position = HIDDEN;
            return;
        }
        uint h = hash3(shape, uint(j), uint(SEED_BLADE));
        float turn = b.x + float(j) * (6.2831853 / float(blades)) + (bits(h, 0) - 0.5) * 0.9;
        vec2 out_dir = vec2(cos(turn), sin(turn));
        float len = size * (0.65 + 0.35 * bits(h, 10));
        float lean = 0.2 + 0.5 * bits(h, 20);
        vec3 base = foot + vec3(out_dir * (0.12 * size), -0.02);
        bool tip = k == 2 || k == 4 || k == 5;
        p = tip ? base + vec3(out_dir * (len * sin(lean)), len * cos(lean)) : base;
        sky = tip ? 1.0 : u_foot_sky.y;
        shift = vec2(k == 1 || k == 2 || k == 4 ? BLADE_HALF_PX : -BLADE_HALF_PX, 0.0);
        n = normalize(vec3(out_dir, 1.2));
    }
    vec2 place = area_place(p, shift, u_right, u_up, u_inv_texel, u_area_frac, u_area_px);
    float dz = (dot(p, u_fwd) + u_depth.x) * u_depth.y;
    gl_Position = vec4(place / float(VIEWPORT / 2) - 1.0, dz * 2.0 - 1.0, 1.0);
    v_normal = n;
    v_ladder = ivec2(code % 256, code / 256);
    v_foot = a.xy;
    v_sky = sky;
    v_depth = dz;
    v_air = vec2(dot(p, u_fwd) + u_area_air.x, p.z + u_area_air.y);
}
