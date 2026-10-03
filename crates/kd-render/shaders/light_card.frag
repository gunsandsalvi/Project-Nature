// The light card (A11.12), drawn as palette indices into colour 0: each look's ladder as a row of swatches. Positions
// are art pixels from the screen's top-left corner, worked out on the CPU (A11.13 rule 1).
uniform ivec2 u_top_left;          // the art pixel at the screen's top-left: its column, and its row from the bottom
uniform ivec2 u_swatches;          // the first row's first swatch
uniform int u_looks;               // how many ladders follow
uniform ivec2 u_ladders[CARD_MAX_LOOKS];  // each look's first palette index and its number of steps
out vec4 o_colour;

void main() {
    ivec2 a = art_pixel();
    int x = a.x - u_top_left.x - u_swatches.x;
    int y = u_top_left.y - a.y - u_swatches.y;
    int index = 0;
    int row = y >= 0 ? y / CARD_ROW : -1;
    int in_row = y - row * CARD_ROW - CARD_NAME_H;
    if (row >= 0 && row < u_looks && in_row >= 0 && in_row < CARD_SWATCH && x >= 0) {
        int k = x / CARD_SWATCH;
        if (k < u_ladders[row].y) {
            index = u_ladders[row].x + k;
        }
    }
    o_colour = pack_out(index, CAT_VOID, 0, 0.0);
}
