// Points reading one texture pixel each in the vertex stage: one of calibration scene C6's reads
// (A18.1). Its body is reads.gdshaderinc. Implements PRE-27.
shader_type spatial;
render_mode unshaded, cull_disabled;

#define KD_READS 1
#include "res://look/reads.gdshaderinc"
