// Calibration scene C5's field with fire shadows by a walk at every pixel (A4.5, A18.1): the field of C1
// with each fire's light walked through the height maps at every pixel. Implements PLT-04 and PRE-30.
shader_type spatial;

#define KD_LIGHT
#define KD_FIRE_WALK
#include "res://look/ground.gdshaderinc"
