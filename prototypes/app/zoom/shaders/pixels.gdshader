// P8's pixels (A4, A8.1): the picture is drawn with a pixel for every `texel` of the screen's, and shown here in pixels
// `pixel` of the screen's each way, on a grid fixed to the screen. Each pixel shown takes the average of the drawn pixels
// under it, each by how much of it lies under: drawn one for one (variants A and B), that is the drawn pixel itself,
// crisp; drawn two each way for each (C), small things blend into the pixel as the pixels grow, rather than flicker in
// and out.
shader_type canvas_item;

uniform sampler2D drawn : filter_nearest;
uniform vec2 drawn_size = vec2(540.0, 1200.0);  // the drawn picture's pixels
uniform float texel = 2.0;  // the screen's pixels a drawn pixel takes
uniform float pixel = 4.0;  // the screen's pixels a shown pixel takes

void fragment() {
	vec2 s = UV * drawn_size * texel;
	vec2 a = floor(s / pixel) * pixel;
	vec2 b = a + pixel;
	ivec2 first = ivec2(floor(a / texel));
	ivec2 last = ivec2(floor((b - 0.001) / texel));
	ivec2 most = ivec2(drawn_size) - 1;
	vec3 sum = vec3(0.0);
	for (int j = 0; j < 4; j++) {
		int y = first.y + j;
		if (y > last.y) {
			break;
		}
		float wy = min(b.y, float(y + 1) * texel) - max(a.y, float(y) * texel);
		for (int i = 0; i < 4; i++) {
			int x = first.x + i;
			if (x > last.x) {
				break;
			}
			float wx = min(b.x, float(x + 1) * texel) - max(a.x, float(x) * texel);
			sum += texelFetch(drawn, clamp(ivec2(x, y), ivec2(0), most), 0).rgb * max(wx, 0.0) * max(wy, 0.0);
		}
	}
	COLOR = vec4(sum / (pixel * pixel), 1.0);
}
