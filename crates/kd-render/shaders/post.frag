// Pass 3, post (A11.2): each art pixel's palette colour from colour 0 and the palette row in use, so every pixel
// shown is one palette colour (PRE-01). A pixel standing in front of a neighbour, by more than its category's gap
// beyond the plane through it and its opposite neighbour, takes its look two steps darker: an outline; or, sunlit
// with that neighbour toward the light on the screen, its look's top step: a lit edge (PRE-21). Then the table of
// its haze level, so an outline hazes like what it outlines (A11.4).
uniform sampler2D u_scene;    // colour 0 of the art target
uniform sampler2D u_palette;  // the row in use, PALETTE_SIZE x 1
uniform sampler2D u_tables;   // TABLE_ROWS rows of PALETTE_SIZE indices
uniform float u_depth_m;      // metres colour 0's depth spans, near to far
uniform vec2 u_sun_screen;    // the light's way across the screen, right and up
out vec4 o_colour;

const float GAPS[8] = OUTLINE_GAP_M;

int table_at(int row, int index) {
    return int(texelFetch(u_tables, ivec2(index, row), 0).r * 255.0 + 0.5);
}

// A pixel's depth in metres, void farther than anything.
float depth_m(ivec2 p) {
    vec4 c = texelFetch(u_scene, clamp(p, ivec2(0), textureSize(u_scene, 0) - 1), 0);
    return (cat_flags(c) & 7) == CAT_VOID ? 1e9 : depth_of(c) * u_depth_m;
}

void main() {
    ivec2 p = art_pixel();
    vec4 c = texelFetch(u_scene, p, 0);
    int index = palette_index(c);
    int g = cat_flags(c);
    int cat = g & 7;
    if (cat != CAT_VOID) {
        float d = depth_m(p);
        // The ways toward neighbours this pixel stands in front of; a void neighbour opposite leaves the plane flat.
        vec2 far_way = vec2(0.0);
        bool silhouette = false;
        ivec2 ways[4] = ivec2[4](ivec2(1, 0), ivec2(-1, 0), ivec2(0, 1), ivec2(0, -1));
        for (int k = 0; k < 4; k++) {
            float opposite = depth_m(p - ways[k]);
            if (opposite >= 1e8) {
                opposite = d;
            }
            if (outline_toward(d, depth_m(p + ways[k]), opposite, GAPS[cat])) {
                silhouette = true;
                far_way += vec2(ways[k]);
            }
        }
        if (silhouette) {
            bool lit = (g & FLAG_SUNLIT) != 0 && dot(far_way, u_sun_screen) > 0.0;
            index = table_at(lit ? TABLE_EDGE : TABLE_OUTLINE, index);
        }
        int level = (g >> FLAG_HAZE_SHIFT) & 3;
        if (level > 0) {
            index = table_at(TABLE_HAZE + level - 1, index);
        }
    }
    o_colour = texelFetch(u_palette, ivec2(index, 0), 0);
}
