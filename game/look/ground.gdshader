// The ground family's shader (A4.6): flat ground for the Look page's first pictures, its texture read
// through the one sampling function, each copy's layer its own. The shared light function comes with
// the ground's real light (α2.3b); until then Godot's own lights it. Implements PRE-01 and PRE-22.
shader_type spatial;
render_mode cull_back, specular_disabled;

#include "res://look/sample.gdshaderinc"

// The rig's: where its origin lies within a texture tile, in tiles, so textures stay put as it moves.
global uniform vec2 kd_origin_tile;

uniform sampler2DArray kd_layers : source_color, filter_linear_mipmap, repeat_enable;
// a texture tile's width on the ground: 4 m at 64 texture pixels a metre (A5.3)
uniform float kd_tile_metres = 4.0;
instance uniform float kd_layer = 0.0;

varying vec2 ground;

void vertex() {
	// east and north about the rig's origin, in metres: east is x, north is -z
	vec3 world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	ground = vec2(world.x, -world.z);
}

void fragment() {
	// rows of a texture run south, so north is up on it
	vec2 uv = vec2(ground.x, -ground.y) / kd_tile_metres + kd_origin_tile;
	ALBEDO = kd_sample(kd_layers, uv, kd_layer).rgb;
	ROUGHNESS = 1.0;
}
