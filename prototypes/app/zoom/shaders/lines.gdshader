// P8's rivers and shore (A8.5, PRE-29): line segments from the world's cells, a pixel wide at every zoom, placed as the
// map is, so they stay on it as it bends onto the globe; lifted over the map's coarse ground, which only a camera
// looking straight down sees them against. Beyond the middle ground, as the map.
shader_type spatial;
render_mode unshaded, cull_disabled;

#include "res://zoom/shaders/place.gdshaderinc"

uniform sampler2D height_tex : filter_linear, repeat_enable;  // the cells' heights, metres
uniform vec3 tint;
uniform float inner;
uniform float lift = 3000.0;  // metres over the cells' heights

varying vec2 from_focus;

void vertex() {
	// metres east and north of the world's corner, each end of a segment moved across the seam as its middle, UV,
	// is, so no segment spans the world
	vec2 m = VERTEX.xz;
	from_focus = m - focus_m + vec2(seam_shift(UV.x), 0.0);
	VERTEX = placed(from_focus, max(textureLod(height_tex, m / world_m, 0.0).r, 0.0) + lift);
}

void fragment() {
	if (!shown(FRAGCOORD.xy, length(from_focus), inner, 1.0e12)) {
		discard;
	}
	ALBEDO = tint;
}
