// P8's world map and globe (A8.5, PRE-29, WLD-02): the world's cells in their cover's flat colours, as P7's map draws
// them, hills shaded the cartographers' way and the sea in depth bands, at the cells' heights; bent onto the globe past
// the world map. It draws beyond the middle ground, giving way to it by dithering.
shader_type spatial;
render_mode unshaded, cull_disabled;

#include "res://zoom/shaders/place.gdshaderinc"

uniform sampler2D map_tex : filter_nearest, repeat_enable;  // a texel a cell, RGB, rows from the south
uniform sampler2D height_tex : filter_linear, repeat_enable;  // the cells' heights, metres
uniform float inner;  // metres round the focus the middle ground draws

varying vec2 uv;
varying vec2 from_focus;

void vertex() {
	// the mesh spans the world: VERTEX.x from -0.5 to 0.5 of it round the focus, VERTEX.z from its south edge (0)
	// to its north (1)
	from_focus = vec2(VERTEX.x * world_m.x, VERTEX.z * world_m.y - focus_m.y);
	uv = (focus_m + from_focus) / world_m;
	VERTEX = placed(from_focus, max(textureLod(height_tex, uv, 0.0).r, 0.0));
}

void fragment() {
	if (!shown(FRAGCOORD.xy, length(from_focus), inner, 1.0e12)) {
		discard;
	}
	ALBEDO = texture(map_tex, uv).rgb;
}
