// The stand-in area's land (A4.6, A4.5, A5.3): the ground family's shader for a big surface, its tiles and versions
// read through the ladder (ladder.gdshaderinc) from world metres, lit by the one shared light function (A4.3). With
// KD_RIVER defined it also draws the river's bed: where the water is deep enough, the bed's own ladder in place of the
// ground's, under the water's tint, and for a little above the level, wet. Written once for the flat carpet round the
// area and the strip the river runs in (CLAUDE.md rule 4). The ground also reads the area's patch picture: masses of
// taller and shorter growth colour it, and where it is worn the grass gives way to bare earth. Implements PRE-20,
// PRE-22, PRE-23, PRE-26 and WLD-31.
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

// The area's patch picture (A4.6, view/src/patches.hpp), 4 m a patch: red the growth, green the wear. Its west and south
// edges in the rig's metres, its side, and the look the tuning gives: the bare earth's colour (linear) and how far the
// ground's colour swings with the growth.
global uniform sampler2D kd_patches;
global uniform vec4 kd_patches_place;
global uniform vec4 kd_patch_look;

varying vec3 kd_world;
varying float kd_big_shadow;

void vertex() {
	// east is x, south is z, about the rig's origin
	kd_world = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

// The growth and the wear at a place, from the patch picture; half growth and no wear outside its square, fading to that
// over the picture's outer tenth so the square's edge is not seen.
vec2 kd_patch_at(vec2 world) {
	// the picture lies in metres east and north; the rig's z is south
	vec2 on_patches = (vec2(world.x, -world.y) - kd_patches_place.xy) / kd_patches_place.z;
	vec2 inside = min(on_patches, vec2(1.0) - on_patches);
	float within = clamp(min(inside.x, inside.y) / 0.1, 0.0, 1.0);
	if (within <= 0.0) {
		return vec2(0.5, 0.0);
	}
	return mix(vec2(0.5, 0.0), textureLod(kd_patches, on_patches, 0.0).rg, within);
}

// Smooth noise from 0 to 1 over a lattice of whole cells, the same on every machine (kd_hash).
float kd_value_noise(vec2 at, float salt) {
	vec2 cell = floor(at);
	vec2 t = fract(at);
	t = t * t * (3.0 - 2.0 * t);
	float a = kd_hash(cell, salt);
	float b = kd_hash(cell + vec2(1.0, 0.0), salt);
	float c = kd_hash(cell + vec2(0.0, 1.0), salt);
	float d = kd_hash(cell + vec2(1.0, 1.0), salt);
	return mix(mix(a, b, t.x), mix(c, d, t.x), t.y);
}

// The width in metres of the clumps that wear takes the grass from together, so the grass thins to islands (about the
// size of a tuft's cluster) instead of losing blade after blade.
const float KD_CLUMP = 0.7;

// The ground's texel as the patch picture changes it: its colour swings golden where the growth is short and dry and
// deeper where it is tall and lush, and where the ground is worn the grass is taken away in clumps, the more worn the
// more of them, leaving bare earth with a drawing of its own (a hash of the texture pixel: dusty grains and a few
// pebbles). Up close the clumps and the grains are texture pixels; far off they fade to the share of earth the wear
// says, so a far texture pixel never speckles. The clumps and grains are read at the level's own texture pixel, from its
// place in the world, so they hold still as the rig moves. A stand-in for the art lane's bare earth and wear tiles (A5.3).
vec3 kd_patch_ground(vec3 texel, vec2 world, float band) {
	vec2 patch = kd_patch_at(world);
	float swing = kd_patch_look.w * (patch.x - 0.5);
	vec3 grown = texel * vec3(1.0 - 0.55 * swing, 1.0 - 0.15 * swing, 1.0 + 0.25 * swing);
	// the level's texture pixel, and its centre in metres
	vec2 cell = kd_texel_cell(world, band);
	vec2 at = (cell + 0.5) * (exp2(band) / KD_BAND0_TEXELS);
	// how much of the clump and grain shows: all of it to the fourth band, none from the sixth
	float detail = clamp(1.0 - (band - 3.0) / 2.0, 0.0, 1.0);
	float clump = 0.65 * kd_value_noise(at / KD_CLUMP, 3.0) + 0.35 * kd_value_noise(at / (0.4 * KD_CLUMP), 4.0);
	clump = clamp(0.5 + (clump - 0.5) * 1.8 + (kd_hash(cell, 5.0) - 0.5) * 0.2, 0.0, 1.0);
	clump = mix(0.5, clump, detail);
	// a clump is bare where the wear passes its number, and for a little below that it is sparse: groups of two by two
	// texture pixels give way to earth by a hash, so the grass thins out at the edge of the bare ground without being
	// peppered; far off the edge widens to the share of earth the wear says
	float share = clamp((patch.y * 1.4 - 0.15 - clump) / mix(1.0, 0.22, detail) + 0.5, 0.0, 1.0);
	float bare = mix(share, step(kd_hash(floor(cell * 0.5), 11.0), share), detail);
	// the earth's own drawing: dust in drifts about three-quarters of a metre wide, mottled about a hand wide, a grain
	// to each pixel, and pale and dark pebbles that gather in places (about half a metre wide) instead of lying evenly
	float drift = kd_value_noise(at / 0.75, 9.0);
	float mottle = kd_value_noise(at / 0.14, 8.0);
	float gathered = smoothstep(0.55, 0.85, kd_value_noise(at / 0.45, 12.0));
	float grain = kd_hash(cell, 6.0);
	float stone = kd_hash(cell, 7.0);
	float pebble = stone > 0.995 - 0.2 * gathered ? 1.5 : (stone < 0.01 + 0.1 * gathered ? 0.6 : 1.0);
	float speckle = (0.86 + 0.28 * drift) * (0.8 + 0.4 * mottle) * (0.9 + 0.2 * grain) * pebble;
	vec3 earth = kd_patch_look.rgb * mix(1.0, speckle, detail);
	return mix(grown, earth, bare);
}

// The land's colour at one band's level, for the point: the ground's, or where the river's bed shows, the bed's.
vec3 kd_land_at(float band, vec2 world, vec2 width0, vec2 slope) {
	vec3 colour = kd_patch_ground(
			kd_ladder_band(kd_layers, kd_count, kd_first, 1.0, world, width0, band).rgb, world, band);
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
