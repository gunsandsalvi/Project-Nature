// Water (research 03, step 8): tinted by how deep the ground lies below the surface, in two steps, with a foam line
// where it meets the bank that comes and goes, a glint of the sky, and sparkles that drift with the current.
shader_type spatial;
render_mode blend_mix, depth_draw_never, cull_back, specular_disabled, unshaded;

uniform sampler2D depth_tex : hint_depth_texture, filter_nearest;
uniform vec3 shallow : source_color = vec3(0.27, 0.65, 0.70);
uniform vec3 deep : source_color = vec3(0.16, 0.44, 0.54);
uniform vec3 foam : source_color = vec3(0.91, 0.96, 0.94);
uniform vec3 sky : source_color = vec3(0.61, 0.77, 0.91);

float hash(vec2 p) {
	return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void fragment() {
	float raw = texture(depth_tex, SCREEN_UV).r;
	vec4 behind = INV_PROJECTION_MATRIX * vec4(SCREEN_UV * 2.0 - 1.0, raw, 1.0);
	behind.xyz /= behind.w;
	float d = -behind.z - (-VERTEX.z);
	vec3 world = (INV_VIEW_MATRIX * vec4(VERTEX, 1.0)).xyz;
	vec3 c = d > 0.55 ? deep : shallow;
	c = mix(c, sky, 0.12);
	float edge = 0.14 + 0.05 * sin(TIME * 2.0 + world.x * 3.1 + world.z * 2.3);
	vec2 cell = floor(world.xz * 6.0 + vec2(0.0, -TIME * 1.5));
	float sparkle = step(0.985, hash(cell + floor(TIME * 2.0)));
	if (d < edge || sparkle > 0.5) {
		c = foam;
	}
	ALBEDO = c;
	ALPHA = d < edge ? 1.0 : 0.85;
}
