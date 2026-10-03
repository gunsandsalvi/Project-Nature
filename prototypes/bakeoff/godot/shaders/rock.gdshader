// Rock (research 04, rule 8): flat facets; faces turned up show the mossy top texture, the rest the strata, whose
// layers are horizontal in the world so they run on from rock to rock along a cliff (PRE-23). 16 pixels a metre.
shader_type spatial;
render_mode cull_back, specular_disabled;
#include "res://shaders/light.gdshaderinc"

uniform sampler2D side : source_color, filter_nearest_mipmap, repeat_enable;
uniform sampler2D top : source_color, filter_nearest_mipmap, repeat_enable;
varying vec3 world;
varying vec3 wnormal;

void vertex() {
	world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	wnormal = normalize((MODEL_MATRIX * vec4(NORMAL, 0.0)).xyz);
}

void fragment() {
	vec3 c;
	if (wnormal.y > 0.62) {
		c = texture(top, world.xz / 2.0).rgb;
	} else {
		vec2 t = normalize(vec2(-wnormal.z, wnormal.x) + vec2(1e-4));
		float u = dot(world.xz, t);
		c = texture(side, vec2(u, -world.y) / 4.0).rgb;
	}
	ALBEDO = c;
	ROUGHNESS = 1.0;
}
