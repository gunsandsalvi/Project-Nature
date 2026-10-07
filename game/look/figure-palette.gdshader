// Figures bent by our bone palettes, every figure a copy of one MultiMesh (A6.3): calibration scene
// C6's second way. Its body is figure.gdshaderinc. Implements PRE-27.
shader_type spatial;
render_mode cull_back, specular_disabled;

#define KD_PALETTE
#include "res://look/figure.gdshaderinc"
