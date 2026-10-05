// Dave Hoskins's hash without sine, 0 to 1: shared by the look's shaders and the zoom's, which may both be included in
// one shader (P8's things stand in the zoom and are lit by the look). Pre-production code (research 00).
#ifndef HASH12
#define HASH12
float hash12(vec2 p) {
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}
#endif
