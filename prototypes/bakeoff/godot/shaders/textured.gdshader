// Made things (research 04, rules 2 and 9): the material's pixel texture by the model's UVs, which are in metres and
// scaled here so that every texture shows 16 pixels a metre.
shader_type spatial;
render_mode cull_back, specular_disabled;
#include "res://shaders/light.gdshaderinc"

uniform sampler2D tex : source_color, filter_nearest_mipmap, repeat_enable;
uniform vec2 uv_metres = vec2(1.0, 1.0);

void fragment() {
	ALBEDO = texture(tex, UV / uv_metres).rgb;
	ROUGHNESS = 1.0;
}
