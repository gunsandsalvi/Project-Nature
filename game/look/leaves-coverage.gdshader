// Leaves cut out with their edges smoothed by alpha to coverage (A4.6), which moves them to Godot's
// blended pass: calibration scene C2's fourth way. Its body is leaves.gdshaderinc. Implements PRE-46.
shader_type spatial;
render_mode cull_disabled, specular_disabled, alpha_to_coverage;

#define KD_COVERAGE
#include "res://look/leaves.gdshaderinc"
