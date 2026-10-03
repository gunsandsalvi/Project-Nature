// Shared by every lit material: the sun's light, after its shadow, in three bands (research 03, step 4); the sky's
// cool light (the environment's ambient) fills the shade (research 04, rule 4).
uniform vec2 light_steps = vec2(0.12, 0.5);

void light() {
	float lit = clamp(dot(NORMAL, LIGHT), 0.0, 1.0) * ATTENUATION;
	float band = lit > light_steps.y ? 1.0 : (lit > light_steps.x ? 0.55 : 0.0);
	DIFFUSE_LIGHT += band * LIGHT_COLOR / PI;
}

// A 4 x 4 Bayer threshold, 0 to 1, for the fine pattern where two tones meet (PRE-20).
float bayer4(ivec2 p) {
	int m[16] = int[16](0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5);
	return (float(m[(p.y & 3) * 4 + (p.x & 3)]) + 0.5) / 16.0;
}
