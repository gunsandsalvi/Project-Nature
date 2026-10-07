// The water's part that the ground's shader and the surface's share (A4.5, PRE-26): how deep the water is over the bed
// at a point, worked out from height and never from depth, in whole texture pixels so the shore steps as pixel art
// does; and what the bed looks like under that depth, tinted toward teal and dark as light is lost in the water, none
// of it reading the screen. The ground's shader draws the bed with it; the surface's draws the marks, glints and
// shore line. Written once for both (CLAUDE.md rule 4). Implements PRE-26.

#include "res://look/ladder.gdshaderinc"

// The water: its level in metres up from the ground's datum, how far the current moves in a tick, east then south, in
// metres, and the tick, a whole number counted ten times a second (A4.2).
global uniform vec4 kd_water;
// How fast each colour of light fades in the water, a metre, in rgb; and how fast the water's own colour takes over (a).
global uniform vec4 kd_water_absorb;
// The colour deep water scatters back, in linear light.
global uniform vec4 kd_water_deep;
// How far above the level the bed is still wet, in metres (x); how much darker wet bed is, as a share of its own colour
// (y); and how ragged the shore is, as a share of the depth a texture pixel spans (z).
global uniform vec4 kd_water_wet;
// The share of the sky's light the surface mirrors when seen square on (x); the glints' life in ticks (y) and the
// most share of texture pixels one lights (z); the shore line's width in texture pixels (w).
global uniform vec4 kd_water_surface;
// The glints' and the shore line's colours, in linear light.
global uniform vec4 kd_water_glint;
global uniform vec4 kd_water_shore;
// The sky's colour, in linear light, which the surface mirrors by angle.
global uniform vec3 kd_sky;

// How the bed's depth changes over the ground, a metre east and south, worked out from how the depth and the point
// change across the screen's pixels. depth: how deep the water is over the bed straight under the point (the level
// less the bed's height, and below the level that is negative); world: the point in metres east and south of the rig's
// origin. It takes derivatives, so every pixel calls it, outside any branch.
vec2 kd_water_slope(vec2 world, float depth) {
	vec2 dpx = dFdx(world);
	vec2 dpy = dFdy(world);
	float det = dpx.x * dpy.y - dpx.y * dpy.x;
	vec2 dd = vec2(dFdx(depth), dFdy(depth));
	if (abs(det) <= 1e-12) {
		return vec2(0.0);
	}
	return vec2(dpy.y * dd.x - dpx.y * dd.y, dpx.x * dd.y - dpy.x * dd.x) / det;
}

// How deep the water is, in metres, at the middle of the texture pixel of a band's level that a point is in; run comes
// back how much deeper it gets across one texture pixel. The depth is taken to the middle of the texture pixel along
// the slope, so every pixel of a texture pixel gets the one depth and the shore, where it is nothing, is a line of
// whole texture pixels. It strays by up to a run either way, in the share kd_water_wet gives, by a hash of the texture
// pixel, which pixel art wants of an edge: ragged, never ruled.
float kd_water_depth(vec2 world, float depth, vec2 slope, float band, out float run) {
	float texel = exp2(band) / KD_BAND0_TEXELS;
	vec2 at = (kd_origin_near.zw * 4.0 + world) / texel;
	float middle = dot(slope, (vec2(0.5) - fract(at)) * texel);
	run = (abs(slope.x) + abs(slope.y)) * texel;
	float ragged = (kd_hash(kd_texel_cell(world, band), 5.0) - 0.5) * 2.0 * run * kd_water_wet.z;
	return depth + middle + ragged;
}

// The bed seen through a depth of water: the light it sends is lost as the water takes the red first, and the water's
// own deep colour takes over. Colours in linear light.
vec3 kd_water_over(vec3 bed, float depth) {
	vec3 through = bed * exp(-depth * kd_water_absorb.rgb);
	return through + kd_water_deep.rgb * (1.0 - exp(-depth * kd_water_absorb.a));
}
