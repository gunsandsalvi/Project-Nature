// Flames: the sprite sheet's four flame frames in turn, about 8 a second, unlit and bright.
shader_type spatial;
render_mode cull_disabled, unshaded, depth_draw_always;

uniform sampler2D sprites : source_color, filter_nearest;

void fragment() {
	float frame = floor(mod(TIME * 8.0, 4.0));
	vec2 uv = vec2(UV.x - floor(UV.x * 4.0) / 4.0 + frame / 4.0, UV.y);
	vec4 t = texture(sprites, uv);
	if (t.a < 0.5) {
		discard;
	}
	ALBEDO = t.rgb * 1.4;
	ALPHA = 1.0;
}
