// Leaf cards (research 04, rule 7): a leaf-cluster sprite tinted by the crown's ramp, lit as one round crown by the
// normals the model gives every corner, swaying a little; drawn after the outline pass, so leaves carry no outline.
shader_type spatial;
render_mode cull_disabled, specular_disabled, depth_draw_always;
#include "res://shaders/light.gdshaderinc"

uniform sampler2D sprites : source_color, filter_nearest_mipmap;
uniform vec3 tint : source_color = vec3(0.3, 0.55, 0.3);
uniform float sway = 0.04;

void vertex() {
	vec3 w = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	float s = sin(TIME * 1.3 + w.x * 0.7 + w.z * 0.5) * sway * clamp(VERTEX.y / 4.0, 0.0, 1.0);
	VERTEX.x += s;
	VERTEX.z += s * 0.6;
}

void fragment() {
	vec4 t = texture(sprites, UV);
	if (t.a < 0.5) {
		discard;
	}
	ALBEDO = tint * t.r * 1.25;
	ALPHA = 1.0;
	ROUGHNESS = 1.0;
}
