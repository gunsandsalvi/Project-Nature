// The stand-in area's land (A4.6, A4.5, A5.3): the ground family's shader for a big surface, its tiles and versions
// read through the ladder (ladder.gdshaderinc) from world metres, lit by the one shared light function (A4.3). With
// KD_RIVER defined it also draws the river's bed: where the water is deep enough, the bed's own ladder in place of the
// ground's, under the water's tint, and for a little above the level, wet. Written once for the flat carpet round the
// area and the strip the river runs in (CLAUDE.md rule 4). Implements PRE-22, PRE-23 and PRE-26.
render_mode cull_back, specular_disabled;

// the ground lies under what roofs it (light.gdshaderinc)
#define KD_UNDER_ROOFS
#include "res://look/picture.gdshaderinc"
#include "res://look/ladder.gdshaderinc"
#include "res://look/light.gdshaderinc"
#ifdef KD_RIVER
#include "res://look/water.gdshaderinc"
#endif

// The ground's tiles, and for each tile how many versions it has and the band it starts at (ladder.gdshaderinc).
uniform sampler2DArray kd_layers : source_color, filter_linear_mipmap, repeat_enable;
uniform vec3 kd_count = vec3(1.0, 0.0, 0.0);
uniform vec3 kd_first = vec3(0.0, 99.0, 99.0);
#ifdef KD_RIVER
// the river bed's
uniform sampler2DArray kd_bed_layers : source_color, filter_linear_mipmap, repeat_enable;
uniform vec3 kd_bed_count = vec3(1.0, 0.0, 0.0);
uniform vec3 kd_bed_first = vec3(0.0, 99.0, 99.0);
#endif
// the material's number in the cloud's test pictures
uniform float kd_material = 3.0;

varying vec3 kd_world;
varying float kd_big_shadow;

void vertex() {
	// east is x, south is z, about the rig's origin
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

// The land's colour at one band's level, for the point: the ground's, or where the river's bed shows, the bed's.
vec3 kd_land_at(float band, vec2 world, vec2 width0, vec2 slope) {
	vec3 colour = kd_ladder_band(kd_layers, kd_count, kd_first, 1.0, world, width0, band).rgb;
#ifdef KD_RIVER
	float run;
	float depth = kd_water_depth(world, kd_water.x - kd_world.y, slope, band, run);
	if (depth > -kd_water_wet.x) {
		vec3 bed = kd_ladder_band(kd_bed_layers, kd_bed_count, kd_bed_first, 2.0, world, width0, band).rgb;
		colour = depth > 0.0 ? kd_water_over(bed, depth) : bed * kd_water_wet.y;
	}
#endif
	return colour;
}

void fragment() {
	vec2 world = kd_world.xz;
	vec2 width0;
	float x = kd_level_of(world * KD_BAND0_TEXELS, width0);
	vec2 slope = vec2(0.0);
#ifdef KD_RIVER
	slope = kd_water_slope(world, kd_water.x - kd_world.y);
#endif
	// the level the rule picks, and just before the next takes over, the next's, which may be another tile's
	float level = clamp(ceil(x), 0.0, KD_LAST_BAND);
	vec3 texel = kd_land_at(level, world, width0, slope);
	float w = kd_next_share(level, x, KD_LAST_BAND);
	if (w > 0.0) {
		texel = mix(texel, kd_land_at(level + 1.0, world, width0, slope), w);
	}
	ROUGHNESS = 1.0;
	vec3 normal = (INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	vec3 colour;
	float openness;
	vec3 glow;
	vec4 haze;
	float big_shadow;
	kd_light_surface(texel, kd_world, normal, CAMERA_POSITION_WORLD, 1.0, vec4(1.0), colour, openness, glow, haze,
			big_shadow);
	kd_big_shadow = big_shadow;
	ALBEDO = colour;
	AO = openness;
	AO_LIGHT_AFFECT = 0.0;
	EMISSION = glow;
	FOG = haze;
	// in a test picture: the material's number, no object, or the tile and version the ground reads
	if (kd_picture != 0) {
		float version;
		float layer;
		vec2 uv;
		float start;
		float tile = kd_ladder_where(kd_count, kd_first, 1.0, world, level, version, layer, uv, start);
		ALBEDO = vec3(0.0);
		EMISSION = kd_number_colour(kd_picture == 1 ? int(kd_material) : (kd_picture == 3 ? int(tile * 8.0 + version) : 0));
		FOG = vec4(0.0);
	}
}

void light() {
	// the ground's shadows are the view's maps': every thing on it is in them, and they soften with distance, which
	// Godot's sun map, read here as well, would sharpen (A4.4)
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, 1.0, kd_big_shadow, 0.0);
}
