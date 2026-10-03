// α00's test card, drawn at art resolution into the art target (A11.2): a checker of single art pixels, eight
// grey steps and a bar that moves one art pixel each frame. Positions are art pixels from the screen's top-left
// corner, worked out on the CPU (A11.13 rule 1); the layout's numbers are generated defines (rule 6).
uniform ivec2 u_top_left;  // the art pixel at the screen's top-left: its column, and its row from the target's bottom
uniform int u_bar_x;       // the bar's column, from the screen's left
out vec4 o_colour;

void main() {
    ivec2 a = art_pixel();
    int x = a.x - u_top_left.x;
    int y = u_top_left.y - a.y;
    vec3 c = vec3(0.07, 0.08, 0.11);
    bool in_checker = x >= CARD_MARGIN && x < CARD_MARGIN + CARD_CHECKER
        && y >= CARD_MARGIN && y < CARD_MARGIN + CARD_CHECKER;
    bool in_greys = x >= CARD_MARGIN && x < CARD_MARGIN + CARD_GREY_STEPS * CARD_GREY_W
        && y >= CARD_GREY_Y && y < CARD_GREY_Y + CARD_GREY_H;
    bool on_bar = x == u_bar_x && y >= CARD_BAR_Y && y < CARD_BAR_Y + CARD_BAR_H;
    if (in_checker) {
        c = ((x + y) & 1) == 0 ? vec3(1.0) : vec3(0.0);
    } else if (in_greys) {
        c = vec3(float((x - CARD_MARGIN) / CARD_GREY_W) / float(CARD_GREY_STEPS - 1));
    } else if (on_bar) {
        c = vec3(1.0, 0.55, 0.1);
    }
    o_colour = vec4(c, 1.0);
}
