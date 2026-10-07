// Points reading twelve texture pixels each in the vertex stage, four bones': one of calibration
// scene C6's reads (A18.1). Its body is reads.gdshaderinc. Implements PRE-27.
shader_type spatial;
render_mode unshaded, cull_disabled;

#define KD_READS 12
#include "res://look/reads.gdshaderinc"
