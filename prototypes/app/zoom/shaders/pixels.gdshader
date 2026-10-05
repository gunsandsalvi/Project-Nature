// P8's pixels (A4, A8.1): the picture is drawn with a pixel for every two of the screen's, and shown here as pixels of
// any size, `pixel` of the screen's each way, on a grid fixed to the screen. Point: each pixel shown takes the colour
// drawn at its middle, the crisp look of pixel art. Blended: it takes the average of everything drawn under it, so as
// the pixels grow, small things blend into them rather than flicker in and out.
shader_type canvas_item;

uniform sampler2D drawn_near : filter_nearest;
uniform sampler2D drawn_soft : filter_linear;
uniform vec2 screen_size = vec2(1080.0, 2400.0);  // the screen's pixels the picture covers
uniform float pixel = 4.0;
uniform bool blend = false;

void fragment() {
	vec2 s = UV * screen_size;
	vec2 cell = floor(s / pixel);
	if (!blend) {
		COLOR = vec4(texture(drawn_near, (cell + 0.5) * pixel / screen_size).rgb, 1.0);
	} else {
		// three by three reads, each of four drawn pixels, spread over the pixel shown
		vec3 sum = vec3(0.0);
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 3; i++) {
				vec2 q = (cell + (vec2(float(i), float(j)) + 0.5) / 3.0) * pixel;
				sum += texture(drawn_soft, q / screen_size).rgb;
			}
		}
		COLOR = vec4(sum / 9.0, 1.0);
	}
}
