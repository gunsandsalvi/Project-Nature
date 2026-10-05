// P8's ground, near and middle, and the near ground's trees (A8.1): each chunk's vertices in the picture's metres from
// the moving origin, coloured by its cover and lit from high up in the north-west; drawn between the finer level's
// edge and its own, giving way to each by dithering.
shader_type spatial;
render_mode unshaded, cull_disabled;

#include "res://zoom/shaders/place.gdshaderinc"

uniform float inner;  // metres round the focus the finer level draws
uniform float outer;  // metres round the focus this level's chunks are all made
uniform vec3 sun = vec3(-0.4, 0.8, -0.45);

varying vec3 colour;
varying vec2 from_focus;

void vertex() {
	vec3 world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	from_focus = world.xz - focus_at.xz;
	vec3 normal = normalize((MODEL_MATRIX * vec4(NORMAL, 0.0)).xyz);
	colour = COLOR.rgb * (0.55 + 0.45 * max(dot(normal, normalize(sun)), 0.0));
}

void fragment() {
	if (!shown(FRAGCOORD.xy, length(from_focus), inner, outer)) {
		discard;
	}
	ALBEDO = colour;
}
