// The ground family's shader body (A4.6): flat ground, its texture read through the one sampling function,
// each copy's layer its own. Lit by Godot's own light, as the Look page draws it; with KD_LIGHT defined, by
// the one shared light function too (A4.3), as the field of calibration scene C1 is, until α2.3b lights every
// family so. Written once for both (CLAUDE.md rule 4). Implements PRE-01 and PRE-22.
render_mode cull_back, specular_disabled;

#include "res://look/picture.gdshaderinc"
#include "res://look/sample.gdshaderinc"
#ifdef KD_LIGHT
#include "res://look/light.gdshaderinc"
#endif

// The rig's: where its origin lies within a texture tile, in tiles, so textures stay put as it moves.
global uniform vec2 kd_origin_tile;

uniform sampler2DArray kd_layers : source_color, filter_linear_mipmap, repeat_enable;
// a texture tile's width on the ground: 4 m at 64 texture pixels a metre (A5.3)
uniform float kd_tile_metres = 4.0;
instance uniform float kd_layer = 0.0;

varying vec2 ground;
#ifdef KD_LIGHT
varying vec3 kd_world;
varying float kd_big_shadow;
#endif

void vertex() {
	// east and north about the rig's origin, in metres: east is x, north is -z
	vec3 world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	ground = vec2(world.x, -world.z);
#ifdef KD_LIGHT
	kd_world = world;
#endif
}

void fragment() {
	// rows of a texture run south, so north is up on it
	vec2 uv = vec2(ground.x, -ground.y) / kd_tile_metres + kd_origin_tile;
	ALBEDO = kd_sample(kd_layers, uv, kd_layer).rgb;
	ROUGHNESS = 1.0;
#ifdef KD_LIGHT
	vec3 normal = (INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	vec3 colour;
	float openness;
	vec3 glow;
	vec4 haze;
	float big_shadow;
	vec4 fire_lit = vec4(1.0);
#ifdef KD_FIRE_SCREEN
	// the walk at half resolution, for the first three fires; a fourth is unshadowed
	fire_lit.rgb = texture(kd_fire_screen, SCREEN_UV).rgb;
#endif
	kd_light_surface(ALBEDO, kd_world, normal, CAMERA_POSITION_WORLD, 1.0, fire_lit, colour, openness, glow,
			haze, big_shadow);
	kd_big_shadow = big_shadow;
	ALBEDO = colour;
	AO = openness;
	AO_LIGHT_AFFECT = 0.0;
	EMISSION = glow;
	FOG = haze;
#endif
	// in a test picture: the material's number is its layer's, from 1; the ground is no object
	if (kd_picture != 0) {
		ALBEDO = vec3(0.0);
		EMISSION = kd_number_colour(kd_picture == 1 ? int(kd_layer) + 1 : 0);
#ifdef KD_LIGHT
		FOG = vec4(0.0);
#endif
	}
}

#ifdef KD_LIGHT
void light() {
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, ATTENUATION, kd_big_shadow, 0.0);
}
#endif
