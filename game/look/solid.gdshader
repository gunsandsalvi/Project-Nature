// Solid things of one colour, lit by the one shared light function's sun (A4.3), as every family is
// and never by Godot's stock material: the stand-in material of calibration scene C3's rocks and
// copies (A18.1, α2.2a). Implements PLT-04 and PRE-30.
shader_type spatial;
render_mode cull_back, specular_disabled;

#include "res://look/light.gdshaderinc"
#include "res://look/picture.gdshaderinc"

uniform vec3 kd_colour : source_color = vec3(0.54, 0.51, 0.47);

void fragment() {
	ALBEDO = kd_colour;
	ROUGHNESS = 1.0;
	// in a test picture: no material of the world's, and no object
	if (kd_picture != 0) {
		ALBEDO = vec3(0.0);
		EMISSION = kd_number_colour(0);
	}
}

void light() {
	DIFFUSE_LIGHT += kd_light_sun(NORMAL, LIGHT, LIGHT_COLOR, ATTENUATION, 1.0, 0.0);
}
