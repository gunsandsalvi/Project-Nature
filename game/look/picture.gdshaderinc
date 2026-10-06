// The engine's test pictures (A4.8): each surface's material or object number as a colour, which the
// cloud's drawing run draws through a camera of its own with plain light, so the colour comes back
// as drawn. Every material shows them the same way (CLAUDE.md rule 4). Implements PRE-31.

// What the surfaces show: 0 the colour picture, the game's; 1 their material's number; 2 their
// object's, 0 for none.
global uniform int kd_picture;

// A number from 0 to 511 as a colour the screen's 8 bits keep: 8 levels a channel, red the lowest,
// each in the middle of its 32 values of a byte; in linear light, as EMISSION wants it.
vec3 kd_number_colour(int n) {
	vec3 levels = vec3(float(n % 8), float((n / 8) % 8), float((n / 64) % 8));
	vec3 srgb = (levels * 32.0 + 16.0) / 255.0;
	return mix(srgb / 12.92, pow((srgb + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), srgb));
}
