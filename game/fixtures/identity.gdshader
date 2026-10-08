// Implements PRE-31, RES-05 (T2.7a.4): semantic captures preserve the colour frame's alpha coverage.
shader_type canvas_item;
render_mode unshaded;
uniform vec4 identity_colour;
uniform bool inspect_alpha = false;
void fragment() {
    float alpha = texture(TEXTURE, UV).a;
    if (alpha <= 0.1) { discard; }
    COLOR = inspect_alpha ? vec4(vec3(alpha), 1.0) : vec4(identity_colour.rgb, 1.0);
}
