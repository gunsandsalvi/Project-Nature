// The stand-in area's land (A4.6, A4.5, A5.3): the ground family's shader for a big surface, its tiles and versions
// read through the ladder (ladder.gdshaderinc) from world metres, lit by the one shared light function (A4.3). With
// KD_RIVER defined it also draws the river's bed: where the water is deep enough, the bed's own ladder in place of the
// ground's, under the water's tint, and above the level the bank's gravel, wet and then dry, giving way to the grass.
// Written once for the flat carpet round the area and the strip the river runs in (CLAUDE.md rule 4). The ground also
// reads the area's patch picture: masses of taller and shorter growth colour it, and where it is worn or open the grass
// gives way to the art lane's trodden ground and open dry ground. Implements PRE-20, PRE-22, PRE-23, PRE-26 and
// WLD-31.
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
// the open dry ground's and the trodden ground's
uniform sampler2DArray kd_earth_layers : source_color, filter_linear_mipmap, repeat_enable;
uniform vec3 kd_earth_count = vec3(1.0, 0.0, 0.0);
uniform vec3 kd_earth_first = vec3(0.0, 99.0, 99.0);
uniform sampler2DArray kd_worn_layers : source_color, filter_linear_mipmap, repeat_enable;
uniform vec3 kd_worn_count = vec3(1.0, 0.0, 0.0);
uniform vec3 kd_worn_first = vec3(0.0, 99.0, 99.0);
#ifdef KD_RIVER
// the river bed's, and the bank's gravel
uniform sampler2DArray kd_bed_layers : source_color, filter_linear_mipmap, repeat_enable;
uniform vec3 kd_bed_count = vec3(1.0, 0.0, 0.0);
uniform vec3 kd_bed_first = vec3(0.0, 99.0, 99.0);
uniform sampler2DArray kd_gravel_layers : source_color, filter_linear_mipmap, repeat_enable;
uniform vec3 kd_gravel_count = vec3(1.0, 0.0, 0.0);
uniform vec3 kd_gravel_first = vec3(0.0, 99.0, 99.0);
#endif
// the material's number in the cloud's test pictures
uniform float kd_material = 3.0;

// The area's patch picture (A4.6, view/src/patches.hpp), 4 m a patch: red the growth, green the wear (the trodden
// ground round the camp), blue the open dry ground that shows where growth is thin. Its west and south edges in the rig's
// metres, its side, and how far the ground's colour swings with the growth.
global uniform sampler2D kd_patches;
global uniform vec4 kd_patches_place;
global uniform float kd_patch_swing;

varying vec3 kd_world;
varying float kd_big_shadow;

void vertex() {
	// east is x, south is z, about the rig's origin
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

// The growth, the wear and the open dry ground at a place, from the patch picture; half growth and neither ground
// outside its square, fading to that over the picture's outer tenth so the square's edge is not seen.
vec3 kd_patch_at(vec2 world) {
	// the picture lies in metres east and north; the rig's z is south
	vec2 on_patches = (vec2(world.x, -world.y) - kd_patches_place.xy) / kd_patches_place.z;
	vec2 inside = min(on_patches, vec2(1.0) - on_patches);
	float within = clamp(min(inside.x, inside.y) / 0.1, 0.0, 1.0);
	if (within <= 0.0) {
		return vec3(0.5, 0.0, 0.0);
	}
	return mix(vec3(0.5, 0.0, 0.0), textureLod(kd_patches, on_patches, 0.0).rgb, within);
}

// The width in metres of the clumps that wear takes the grass from together, so the grass thins to islands (about the
// size of a tuft's cluster) instead of losing blade after blade.
const float KD_CLUMP = 0.7;

// The ground's texel as the patch picture changes it: its colour swings golden where the growth is short and dry and
// deeper where it is tall and lush, and where the ground is worn or open the grass is taken away in clumps, the more
// of them the more worn, leaving the art lane's trodden ground (wear) where the camp has worn it and its open dry ground
// (bare_earth) where growth is thin. Up close the clumps are texture pixels; far off they fade to the share of ground
// the wear says, so a far texture pixel never speckles. The clumps are read at the level's own texture pixel, from its
// place in the world, so they hold still as the rig moves. width0: the screen pixel's width in band 0's texture pixels.
vec3 kd_patch_ground(vec3 texel, vec2 world, vec2 width0, float band) {
	vec3 patch = kd_patch_at(world);
	float swing = kd_patch_swing * (patch.x - 0.5);
	vec3 grown = texel * vec3(1.0 - 0.55 * swing, 1.0 - 0.15 * swing, 1.0 + 0.25 * swing);
	float cover = max(patch.y, patch.z);
	if (cover < 0.02) {
		return grown;
	}
	// the level's texture pixel, and its centre in metres
	vec2 cell = kd_texel_cell(world, band);
	vec2 at = (cell + 0.5) * (exp2(band) / KD_BAND0_TEXELS);
	// how much of the clump shows: all of it to the fourth band, none from the sixth
	float detail = clamp(1.0 - (band - 3.0) / 2.0, 0.0, 1.0);
	float clump = 0.65 * kd_value_noise(at / KD_CLUMP, 3.0) + 0.35 * kd_value_noise(at / (0.4 * KD_CLUMP), 4.0);
	clump = clamp(0.5 + (clump - 0.5) * 1.8 + (kd_hash(cell, 5.0) - 0.5) * 0.2, 0.0, 1.0);
	clump = mix(0.5, clump, detail);
	// a clump is gone where the wear passes its number, and for a little below that it is sparse: groups of two by two
	// texture pixels give way by a hash, so the grass thins out at the edge of the ground without being peppered; far
	// off the edge widens to the share of ground the wear says
	float share = clamp((cover * 1.4 - 0.15 - clump) / mix(1.0, 0.22, detail) + 0.5, 0.0, 1.0);
	float gone = mix(share, step(kd_hash(floor(cell * 0.5), 11.0), share), detail);
	vec3 trodden = kd_ladder_band(kd_worn_layers, kd_worn_count, kd_worn_first, 5.0, world, width0, band).rgb;
	vec3 open = kd_ladder_band(kd_earth_layers, kd_earth_count, kd_earth_first, 4.0, world, width0, band).rgb;
	vec3 ground = mix(open, trodden, patch.y / max(patch.y + patch.z, 1.0e-4));
	return mix(grown, ground, gone);
}

#ifdef KD_RIVER
// How much of a second ground a first gives way to, in clumps, as t goes from 0 to 1: the grass reaching down in islands and
// the gravel up in bars, by single groups of two by two texture pixels at the edge as the worn ground does, so a shore is
// a margin and never a ruled line. t is 0 and 1 at the ends and the share is 0 and 1 there, so the two grounds meet their
// neighbours without a seam. salt tells the places apart in the noise.
float kd_clump_share(float t, vec2 world, float band, float salt) {
	vec2 cell = kd_texel_cell(world, band);
	vec2 at = (cell + 0.5) * (exp2(band) / KD_BAND0_TEXELS);
	float detail = clamp(1.0 - (band - 3.0) / 2.0, 0.0, 1.0);
	float clump = 0.6 * kd_value_noise(at / 0.8, salt) + 0.4 * kd_value_noise(at / 0.3, salt + 1.0);
	clump = clamp(0.5 + (clump - 0.5) * 1.8 + (kd_hash(cell, salt + 2.0) - 0.5) * 0.2, 0.0, 1.0);
	clump = mix(0.5, clump, detail);
	float share = clamp((1.25 * t - clump) / mix(1.0, 0.22, detail) + 0.5, 0.0, 1.0);
	return mix(share, step(kd_hash(floor(cell * 0.5), salt + 3.0), share), detail);
}

// The bank above the water's wet margin: the bank's gravel, dry, giving way to the grass up the bank, the stretches of
// gravel bar and of steep grass wandering. above: how high the point is over the wet margin, in metres; the bank is
// mixed up to kd_water_wet.w, and past that it is grass.
vec3 kd_bank(vec3 grass, vec3 gravel, float above, vec2 world, float band) {
	vec2 at = (kd_texel_cell(world, band) + 0.5) * (exp2(band) / KD_BAND0_TEXELS);
	float up = clamp(above / max(kd_water_wet.w, 1.0e-4), 0.0, 1.0);
	// the wandering vanishes at the top, so the bank meets the meadow without a seam
	up = clamp(up + (0.5 - kd_value_noise(at / 9.0, 21.0)) * 0.5 * (1.0 - up), 0.0, 1.0);
	return mix(gravel, grass, kd_clump_share(up, world, band, 22.0));
}

// How high up the bank the water's wet reaches at a place, in metres: the margin, wicking higher in some places and less
// in others.
float kd_wet_reach(vec2 world, float band) {
	vec2 at = (kd_texel_cell(world, band) + 0.5) * (exp2(band) / KD_BAND0_TEXELS);
	return kd_water_wet.x * mix(0.4, 1.5, kd_value_noise(at / 1.1, 52.0));
}
#endif

// The land's colour at one band's level, for the point: the ground's with its other grounds, or where the river is, the
// bank's gravel and the bed's.
vec3 kd_land_at(float band, vec2 world, vec2 width0, vec2 slope) {
	vec3 colour = kd_patch_ground(
			kd_ladder_band(kd_layers, kd_count, kd_first, 1.0, world, width0, band).rgb, world, width0, band);
#ifdef KD_RIVER
	float run;
	float depth = kd_water_depth(world, kd_water.x - kd_world.y, slope, band, run);
	float wet = kd_wet_reach(world, band);
	if (depth > 0.0 || depth > -(wet + kd_water_wet.w)) {
		vec3 gravel = kd_ladder_band(kd_gravel_layers, kd_gravel_count, kd_gravel_first, 6.0, world, width0, band).rgb;
		if (depth > 0.0) {
			// under the water the bank's gravel goes on for a little, mixed in clumps with the bed's stones, so the
			// two grounds overlap across the waterline; the bed is as dark as wet stones at the waterline and clears
			// over the first 8 cm
			vec3 bed = kd_ladder_band(kd_bed_layers, kd_bed_count, kd_bed_first, 2.0, world, width0, band).rgb;
			float t = clamp(depth / max(kd_water_edge.w, 1.0e-4), 0.0, 1.0);
			vec3 floor_colour = mix(gravel, bed, kd_clump_share(t, world, band, 61.0));
			colour = kd_water_over(floor_colour * mix(kd_water_wet.y, 1.0, smoothstep(0.0, 0.08, depth)), depth);
		} else if (depth > -wet) {
			// darkest at the waterline, dry again at the top of the wet margin
			colour = gravel * mix(kd_water_wet.y, 1.0, smoothstep(0.0, wet, -depth));
		} else {
			colour = kd_bank(colour, gravel, -depth - wet, world, band);
		}
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
