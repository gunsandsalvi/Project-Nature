// The river's surface (A4.5, PRE-26, A4.2): drawn over the bed, which the ground's shader has already tinted by depth
// ("clear water", with no read of the screen or its depth). Only the water over the bed is drawn, wherever the bed
// lies below the level. It mirrors the sky's colour by angle, carries the current's marks, which step in whole texture
// pixels ten times a second, glints of single texture pixels that live a few ticks, and a bright line of one texture
// pixel where the water meets the land, all worked out from the bed's height, never from depth. A blended pass drawn
// last (A4.1). Implements PRE-26 and PRE-22.
shader_type spatial;
render_mode blend_mix, depth_draw_never, cull_back, specular_disabled;

#include "res://look/picture.gdshaderinc"
#include "res://look/light.gdshaderinc"
#include "res://look/water.gdshaderinc"

// The marks' tiles, and for each tile how many versions it has and the band it starts at (ladder.gdshaderinc).
uniform sampler2DArray kd_layers : source_color, filter_linear_mipmap, repeat_enable;
uniform vec3 kd_count = vec3(1.0, 0.0, 0.0);
uniform vec3 kd_first = vec3(0.0, 99.0, 99.0);
// the material's number in the cloud's test pictures
uniform float kd_material = 5.0;

varying vec3 kd_world;
varying float kd_big_shadow;

void vertex() {
	// east is x, south is z, about the rig's origin; UV2.x is the bed's height under the vertex
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

// The surface at one band's level, for the point: cover, the colour of the marks or the shore line over it, and how
// much of it they cover; glint, 1 where a glint lights this texture pixel; water, 1 where the water is over the bed.
// over_bed: how deep the water is over the bed straight under the point.
void kd_surface_at(float band, vec2 world, float over_bed, vec2 width0, vec2 slope, float lobe, out vec4 cover,
		out float glint, out float water) {
	float run;
	float depth = kd_water_depth(world, over_bed, slope, band, run);
	cover = vec4(0.0);
	glint = 0.0;
	water = depth > 0.0 ? 1.0 : 0.0;
	if (depth > 0.0) {
		// the marks go with the current: the pattern is read where it was a tick's worth of metres ago, so it moves
		// by whole texture pixels of each level
		vec2 shifted = world - kd_water.yz * kd_water.w;
		cover = kd_ladder_band(kd_layers, kd_count, kd_first, 3.0, shifted, width0, band);
		// where the water meets the land, a line of whole texture pixels
		if (depth <= run * kd_water_surface.w) {
			cover = vec4(kd_water_shore.rgb, 1.0);
		}
		// a glint is one texture pixel lit for a few ticks, picked by a hash of its place and of when it is
		float era = floor(kd_water.w / max(kd_water_surface.y, 1.0));
		float chance = kd_hash(kd_texel_cell(world, band), 11.0 + era);
		glint = chance < kd_water_surface.z * lobe ? 1.0 : 0.0;
	}
}

void fragment() {
	vec2 world = kd_world.xz;
	vec2 width0;
	float x = kd_level_of(world * KD_BAND0_TEXELS, width0);
	float over_bed = kd_water.x - UV2.x;
	vec2 slope = kd_water_slope(world, over_bed);
	// glints gather where the sun's mirror image is: the view reflected in the surface, toward the sun
	vec3 look = normalize(CAMERA_POSITION_WORLD - kd_world);
	float lobe = 0.2 + 0.8 * pow(max(dot(reflect(-look, vec3(0.0, 1.0, 0.0)), kd_sun_toward), 0.0), 12.0);
	float level = clamp(ceil(x), 0.0, KD_LAST_BAND);
	vec4 cover;
	float glint;
	float water;
	kd_surface_at(level, world, over_bed, width0, slope, lobe, cover, glint, water);
	float w = kd_next_share(level, x, KD_LAST_BAND);
	if (w > 0.0) {
		vec4 cover_next;
		float glint_next;
		float water_next;
		kd_surface_at(level + 1.0, world, over_bed, width0, slope, lobe, cover_next, glint_next, water_next);
		cover = mix(cover, cover_next, w);
		glint = mix(glint, glint_next, w);
		water = mix(water, water_next, w);
	}
	// the sky's colour by angle: a little square on, more as the view grazes the surface
	float f0 = kd_water_surface.x;
	float sky = (f0 + (1.0 - f0) * pow(1.0 - clamp(dot(NORMAL, VIEW), 0.0, 1.0), 5.0)) * water;
	float marked = cover.a * water;
	float lit = glint * water;
	// from the bottom up: the sky's tint, the marks over it, the glints over both
	float alpha = 1.0 - (1.0 - sky) * (1.0 - marked) * (1.0 - lit);
	if (alpha < 1.0 / 255.0 && kd_picture == 0) {
		discard;
	}
	float share = 1.0 / max(alpha, 1e-4);
	vec3 albedo = cover.rgb * marked * (1.0 - lit) * share;
	vec3 emitted = (kd_sky * sky * (1.0 - marked) * (1.0 - lit) + kd_water_glint.rgb * lit) * share;
	ROUGHNESS = 1.0;
	vec3 normal = (INV_VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz;
	vec3 colour;
	float openness;
	vec3 glow;
	vec4 haze;
	float big_shadow;
	kd_light_surface(albedo, kd_world, normal, CAMERA_POSITION_WORLD, 1.0, vec4(1.0), colour, openness, glow, haze,
			big_shadow);
	kd_big_shadow = big_shadow;
	ALBEDO = colour;
	AO = openness;
	AO_LIGHT_AFFECT = 0.0;
	EMISSION = glow + emitted;
	FOG = haze;
	ALPHA = alpha;
	// in a test picture: the material's number where there is water, and no object
	if (kd_picture != 0) {
		ALBEDO = vec3(0.0);
		EMISSION = kd_number_colour(kd_picture == 1 ? int(kd_material) : 0);
		FOG = vec4(0.0);
		ALPHA = water > 0.5 ? 1.0 : 0.0;
		if (water <= 0.5) {
			discard;
		}
	}
}

void light() {
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, ATTENUATION, kd_big_shadow, 0.0);
}
