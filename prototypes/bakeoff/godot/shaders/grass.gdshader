// Grass cards (research 03, step 6): each card turns about the vertical to face the camera, takes the ground's colour
// at its foot (the instance's custom data), sways in a noise-like wind, and is lit and shadowed as the ground at its
// foot is, so it blends in. Drawn after the outline pass, so blades get no outline.
shader_type spatial;
render_mode cull_disabled, specular_disabled, depth_draw_always;

#include "res://shaders/light.gdshaderinc"

uniform sampler2D card : source_color, filter_nearest_mipmap;
uniform float wind = 0.05;

varying vec3 base_view;
varying vec3 tint;
varying vec2 cell;

void vertex() {
	vec3 right = normalize(vec3(INV_VIEW_MATRIX[0].x, 0.0, INV_VIEW_MATRIX[0].z));
	float up = VERTEX.y;
	VERTEX = right * VERTEX.x + vec3(0.0, up, 0.0);
	float sprite = floor(INSTANCE_CUSTOM.a);
	float phase = fract(INSTANCE_CUSTOM.a) * 6.2832;
	float sway = up / 0.5;
	cell = vec2(mod(sprite, 8.0) * 8.0, sprite < 8.0 ? 16.0 : 24.0) / 64.0;
	VERTEX.xz += vec2(sin(TIME * 1.7 + phase), cos(TIME * 1.3 + phase * 1.7)) * wind * sway;
	NORMAL = vec3(0.0, 1.0, 0.0);
	base_view = (MODELVIEW_MATRIX * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	tint = INSTANCE_CUSTOM.rgb;
}

void fragment() {
	vec4 t = texture(card, cell + UV * (8.0 / 64.0));
	if (t.a < 0.5) {
		discard;
	}
	ALBEDO = tint * t.rgb * 1.3;
	ALPHA = 1.0;
	ROUGHNESS = 1.0;
	LIGHT_VERTEX = base_view;
}
