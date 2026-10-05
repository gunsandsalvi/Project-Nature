// P8's pixels (A4, A8.1): the picture is drawn with a pixel for every `texel` of the screen's, and shown here in pixels
// `pixel` of the screen's each way, on a grid fixed to the screen. Each pixel shown takes the average of the drawn pixels
// under it, each by how much of it lies under: drawn one for one (variants A and B), that is the drawn pixel itself,
// crisp; drawn two each way for each (C), small things blend into the pixel as the pixels grow, rather than flicker in
// and out. A pixel shown never spans more than three drawn ones each way, so a fixed three by three reads them all, those
// outside it weighing nothing.
shader_type canvas_item;

uniform sampler2D drawn : filter_nearest;
uniform vec2 drawn_size = vec2(540.0, 1200.0);  // the drawn picture's pixels
uniform float texel = 2.0;  // the screen's pixels a drawn pixel takes
uniform float pixel = 4.0;  // the screen's pixels a shown pixel takes

void fragment() {
	vec2 s = UV * drawn_size * texel;
	vec2 a = floor(s / pixel) * pixel;
	vec2 b = a + pixel;
	vec2 first = floor(a / texel);
	vec3 sum = vec3(0.0);
	for (int j = 0; j < 3; j++) {
		float y = first.y + float(j);
		float wy = max(min(b.y, (y + 1.0) * texel) - max(a.y, y * texel), 0.0);
		for (int i = 0; i < 3; i++) {
			float x = first.x + float(i);
			float wx = max(min(b.x, (x + 1.0) * texel) - max(a.x, x * texel), 0.0);
			vec2 at = (clamp(vec2(x, y), vec2(0.0), drawn_size - 1.0) + 0.5) / drawn_size;
			sum += texture(drawn, at).rgb * wx * wy;
		}
	}
	COLOR = vec4(sum / (pixel * pixel), 1.0);
}
