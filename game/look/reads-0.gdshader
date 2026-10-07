// Points reading nothing in the vertex stage, against which calibration scene C6's reads are
// measured (A18.1). Its body is reads.gdshaderinc. Implements PRE-27.
shader_type spatial;
render_mode unshaded, cull_disabled;

#define KD_READS 0
#include "res://look/reads.gdshaderinc"
