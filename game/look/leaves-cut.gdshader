// Leaves cut out where their design's alpha falls under a half (A4.6): plain cards, cards cut close to
// their leaves, and the fringes round solid cores, in calibration scene C2. Its body is
// leaves.gdshaderinc. Implements PRE-46.
shader_type spatial;
render_mode cull_disabled, specular_disabled;

#define KD_CUT
#include "res://look/leaves.gdshaderinc"
